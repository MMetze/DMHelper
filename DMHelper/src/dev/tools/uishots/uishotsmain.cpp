// DMHelperUiShots - renders every Qt Designer .ui file to a PNG for UX review and visual regression.
//
// Usage:
//   DMHelperUiShots --src <dir with .ui files> --out <png dir>
//                   [--baseline <previous png dir>] [--theme classic|light|dark] [--theme-dir <dir>] [--qss <file>] [--style <name>]
//                   [--strip-inline-qss] [--filter <regex>]
//
// Defaults to QT_QPA_PLATFORM=offscreen. Promoted custom widgets fall back to their Designer base class.
// Exit codes: 0 ok, 1 load failures, 2 visual differences vs baseline.

#include "dmhtheme.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QImage>
#include <QPixmap>
#include <QRegularExpression>
#include <QStyleFactory>
#include <QTextStream>
#include <QUiLoader>
#include <QWidget>
#include <QtGlobal>

static constexpr int SETTLE_EVENT_PASSES = 3;
static constexpr int MAX_SHOT_WIDTH = 2560;
static constexpr int MAX_SHOT_HEIGHT = 1600;
static constexpr int DIFF_CHANNEL_TOLERANCE = 8;
static constexpr int DIFF_DIM_FACTOR = 4;
static constexpr double PERCENT = 100.0;
static constexpr int EXIT_LOAD_FAILURE = 1;
static constexpr int EXIT_VISUAL_DIFF = 2;
static constexpr int DEFAULT_FONT_POINT_SIZE = 10;
static const QString DEFAULT_FONT_FAMILY = QStringLiteral("Trebuchet MS");
static const QColor DIFF_HIGHLIGHT_COLOR(255, 0, 255);
static const QStringList SKIP_DIR_PREFIXES = {QStringLiteral("out"), QStringLiteral("build"), QStringLiteral("bin-"),
                                              QStringLiteral("vlc"), QStringLiteral("."), QStringLiteral("doc"),
                                              QStringLiteral("installer"), QStringLiteral("release notes")};

static bool isSkippedPath(const QString& relativePath)
{
    const QStringList parts = relativePath.split(QLatin1Char('/'));
    for(int i = 0; i < parts.count() - 1; ++i)
    {
        for(const QString& prefix : SKIP_DIR_PREFIXES)
        {
            if(parts.at(i).startsWith(prefix))
                return true;
        }
    }
    return false;
}

static QStringList findUiFiles(const QDir& srcDir, const QRegularExpression& filter)
{
    QStringList result;
    QDirIterator it(srcDir.absolutePath(), {QStringLiteral("*.ui")}, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        const QString relative = srcDir.relativeFilePath(it.next());
        if(!isSkippedPath(relative) && filter.match(relative).hasMatch())
            result.append(relative);
    }
    result.sort();
    return result;
}

// Returns the fraction of changed pixels, or -1 when sizes differ. Writes a highlight image when anything changed.
static double compareImages(const QImage& current, const QImage& baseline, const QString& diffPath)
{
    if(current.size() != baseline.size())
        return -1.0;

    const QImage a = current.convertToFormat(QImage::Format_ARGB32);
    const QImage b = baseline.convertToFormat(QImage::Format_ARGB32);
    QImage diff(a.size(), QImage::Format_ARGB32);
    qint64 changed = 0;
    for(int y = 0; y < a.height(); ++y)
    {
        const QRgb* lineA = reinterpret_cast<const QRgb*>(a.constScanLine(y));
        const QRgb* lineB = reinterpret_cast<const QRgb*>(b.constScanLine(y));
        QRgb* lineD = reinterpret_cast<QRgb*>(diff.scanLine(y));
        for(int x = 0; x < a.width(); ++x)
        {
            const bool differs = qAbs(qRed(lineA[x]) - qRed(lineB[x])) > DIFF_CHANNEL_TOLERANCE ||
                                 qAbs(qGreen(lineA[x]) - qGreen(lineB[x])) > DIFF_CHANNEL_TOLERANCE ||
                                 qAbs(qBlue(lineA[x]) - qBlue(lineB[x])) > DIFF_CHANNEL_TOLERANCE ||
                                 qAbs(qAlpha(lineA[x]) - qAlpha(lineB[x])) > DIFF_CHANNEL_TOLERANCE;
            if(differs)
            {
                ++changed;
                lineD[x] = DIFF_HIGHLIGHT_COLOR.rgb();
            }
            else
            {
                const int grey = qGray(lineA[x]) / DIFF_DIM_FACTOR;
                lineD[x] = qRgb(grey, grey, grey);
            }
        }
    }

    if(changed > 0)
        diff.save(diffPath);

    return static_cast<double>(changed) / (static_cast<double>(a.width()) * a.height());
}

static void filteredMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if(msg.contains(QStringLiteral("propagateSizeHints")))
        return;
    qt_message_output(type, context, msg);
}

int main(int argc, char* argv[])
{
    if(!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");
#ifdef Q_OS_WIN
    // The offscreen plugin uses a FreeType font database that finds no fonts on Windows unless pointed at them.
    if(!qEnvironmentVariableIsSet("QT_QPA_FONTDIR"))
        qputenv("QT_QPA_FONTDIR", QFile::encodeName(QDir::fromNativeSeparators(qEnvironmentVariable("WINDIR")) + QStringLiteral("/Fonts")));
#endif
    qInstallMessageHandler(filteredMessageHandler);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("DMHelperUiShots"));

    QCommandLineParser parser;
    parser.addHelpOption();
    const QCommandLineOption srcOpt(QStringLiteral("src"), QStringLiteral("Directory searched recursively for .ui files."), QStringLiteral("dir"));
    const QCommandLineOption outOpt(QStringLiteral("out"), QStringLiteral("Output directory for PNGs."), QStringLiteral("dir"));
    const QCommandLineOption baselineOpt(QStringLiteral("baseline"), QStringLiteral("Previous output directory to diff against."), QStringLiteral("dir"));
    const QCommandLineOption qssOpt(QStringLiteral("qss"), QStringLiteral("Application stylesheet to apply."), QStringLiteral("file"));
    const QCommandLineOption styleOpt(QStringLiteral("style"), QStringLiteral("QStyle to use (e.g. Fusion, windows11)."), QStringLiteral("name"));
    const QCommandLineOption stripOpt(QStringLiteral("strip-inline-qss"), QStringLiteral("Clear per-widget styleSheet properties from the .ui files."));
    const QCommandLineOption filterOpt(QStringLiteral("filter"), QStringLiteral("Regex on the relative .ui path."), QStringLiteral("regex"), QStringLiteral("."));
    const QCommandLineOption fontOpt(QStringLiteral("font"), QStringLiteral("Application font family (DMHelper default: Trebuchet MS)."), QStringLiteral("family"), DEFAULT_FONT_FAMILY);
    const QCommandLineOption fontSizeOpt(QStringLiteral("font-size"), QStringLiteral("Application font point size."), QStringLiteral("pt"), QString::number(DEFAULT_FONT_POINT_SIZE));
    const QCommandLineOption themeOpt(QStringLiteral("theme"), QStringLiteral("DMHelper theme: classic, light, dark or system."), QStringLiteral("name"), DMHTheme::themeName(DMHTheme::ThemeType_Classic));
    const QCommandLineOption themeDirOpt(QStringLiteral("theme-dir"), QStringLiteral("User theme folder whose files override the built-in theme files."), QStringLiteral("dir"));
    parser.addOptions({srcOpt, outOpt, baselineOpt, qssOpt, styleOpt, stripOpt, filterOpt, fontOpt, fontSizeOpt, themeOpt, themeDirOpt});
    parser.process(app);

    QFont appFont(parser.value(fontOpt));
    appFont.setPointSize(parser.value(fontSizeOpt).toInt());
    QApplication::setFont(appFont);

    QTextStream out(stdout);
    if(!parser.isSet(srcOpt) || !parser.isSet(outOpt))
    {
        out << parser.helpText();
        return EXIT_LOAD_FAILURE;
    }

    if(parser.isSet(styleOpt))
        QApplication::setStyle(QStyleFactory::create(parser.value(styleOpt)));

    if(parser.isSet(themeDirOpt))
        DMHTheme::setUserThemeDirectory(parser.value(themeDirOpt));
    DMHTheme::apply(DMHTheme::themeFromName(parser.value(themeOpt)));

    if(parser.isSet(qssOpt))
    {
        QFile qssFile(parser.value(qssOpt));
        if(!qssFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            out << "Cannot read stylesheet " << qssFile.fileName() << Qt::endl;
            return EXIT_LOAD_FAILURE;
        }
        app.setStyleSheet(QString::fromUtf8(qssFile.readAll()));
    }

    const QDir srcDir(parser.value(srcOpt));
    const QDir outDir(parser.value(outOpt));
    outDir.mkpath(QStringLiteral("."));
    const bool hasBaseline = parser.isSet(baselineOpt);
    const QDir baselineDir(parser.value(baselineOpt));
    if(hasBaseline)
        outDir.mkpath(QStringLiteral("diff"));

    QFile reportFile(outDir.filePath(QStringLiteral("report.tsv")));
    if(!reportFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        out << "Cannot write report in " << outDir.absolutePath() << Qt::endl;
        return EXIT_LOAD_FAILURE;
    }
    QTextStream report(&reportFile);
    report << "ui\tpng\twidth\theight\tstatus\tchangedPercent\n";

    int failures = 0;
    int differences = 0;
    const QStringList uiFiles = findUiFiles(srcDir, QRegularExpression(parser.value(filterOpt)));
    for(const QString& relative : uiFiles)
    {
        const QString pngName = QString(relative).replace(QLatin1Char('/'), QStringLiteral("__")).replace(QStringLiteral(".ui"), QStringLiteral(".png"));
        QFile uiFile(srcDir.filePath(relative));
        if(!uiFile.open(QIODevice::ReadOnly))
        {
            report << relative << "\t\t0\t0\tunreadable\t\n";
            ++failures;
            continue;
        }

        QUiLoader loader;
        loader.setWorkingDirectory(QFileInfo(uiFile).absoluteDir());
        QWidget* widget = loader.load(&uiFile);
        if(!widget)
        {
            out << "FAIL " << relative << ": " << loader.errorString() << Qt::endl;
            report << relative << "\t\t0\t0\tload-failed\t\n";
            ++failures;
            continue;
        }

        if(parser.isSet(stripOpt))
        {
            widget->setStyleSheet(QString());
            const QList<QWidget*> children = widget->findChildren<QWidget*>();
            for(QWidget* child : children)
                child->setStyleSheet(QString());
        }

        widget->setAttribute(Qt::WA_DontShowOnScreen);
        const QSize wanted = widget->size().expandedTo(widget->sizeHint()).boundedTo(QSize(MAX_SHOT_WIDTH, MAX_SHOT_HEIGHT));
        widget->resize(wanted);
        widget->show();
        for(int i = 0; i < SETTLE_EVENT_PASSES; ++i)
            QCoreApplication::processEvents();

        const QImage image = widget->grab().toImage();
        widget->close();
        delete widget;

        const QString pngPath = outDir.filePath(pngName);
        if(!image.save(pngPath))
        {
            report << relative << "\t" << pngName << "\t0\t0\tsave-failed\t\n";
            ++failures;
            continue;
        }

        QString status = QStringLiteral("ok");
        QString changedText;
        if(hasBaseline)
        {
            const QImage baseline(baselineDir.filePath(pngName));
            if(baseline.isNull())
            {
                status = QStringLiteral("new");
                ++differences;
            }
            else
            {
                const double changed = compareImages(image, baseline, outDir.filePath(QStringLiteral("diff/") + pngName));
                if(changed < 0.0)
                {
                    status = QStringLiteral("resized");
                    ++differences;
                }
                else if(changed > 0.0)
                {
                    status = QStringLiteral("changed");
                    changedText = QString::number(changed * PERCENT, 'f', 2);
                    ++differences;
                }
                else
                {
                    status = QStringLiteral("same");
                }
            }
        }

        report << relative << "\t" << pngName << "\t" << image.width() << "\t" << image.height() << "\t" << status << "\t" << changedText << "\n";
        out << status << " " << relative << Qt::endl;
    }

    out << uiFiles.count() << " ui files, " << failures << " failures, " << differences << " differences -> " << outDir.absolutePath() << Qt::endl;
    if(failures > 0)
        return EXIT_LOAD_FAILURE;
    return differences > 0 ? EXIT_VISUAL_DIFF : 0;
}
