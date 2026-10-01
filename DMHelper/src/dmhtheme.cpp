#include "dmhtheme.h"
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QList>
#include <QMap>
#include <QPalette>
#include <QPixmap>
#include <QPointer>
#include <QStyle>
#include <QStyleHints>
#include <QWidget>
#include <QDebug>
#include <algorithm>

namespace
{

constexpr int BEVEL_LIGHT_FACTOR = 125;
constexpr int BEVEL_DARK_FACTOR = 150;
constexpr int OPAQUE_ALPHA = 255;
const QColor SHADOW_COLOR(0, 0, 0);

const QString MODERN_STYLE_NAME = QStringLiteral("Fusion");
const QString BUILTIN_THEME_PATH = QStringLiteral(":/img/data/theme/");
const QString DEFAULTS_SUBDIRECTORY = QStringLiteral("defaults");
const QString BASE_QSS = QStringLiteral("base.qss");
const QString CLASSIC_QSS = QStringLiteral("classic.qss");
const QString MODERN_QSS = QStringLiteral("modern.qss");
const QString LIGHT_COLORS = QStringLiteral("light.colors");
const QString DARK_COLORS = QStringLiteral("dark.colors");
const QString PARCHMENT_PATH = QStringLiteral(":/img/data/parchment.jpg");
const QChar TOKEN_PREFIX = QLatin1Char('@');
const char* const THEME_NAMES[DMHTheme::ThemeType_Count] = {"classic", "light", "dark", "system"};

typedef QMap<QString, QColor> ThemeColors;

DMHTheme::ThemeType s_currentTheme = DMHTheme::ThemeType_Classic;
DMHTheme::ThemeType s_resolvedTheme = DMHTheme::ThemeType_Classic;
QString s_userDirectory;
bool s_originalCaptured = false;
QString s_originalStyleName;
QPalette s_originalPalette;
QList<QPointer<QWidget>> s_parchmentWidgets;

QByteArray readThemeFile(const QString& fileName)
{
    if(!s_userDirectory.isEmpty())
    {
        QFile userFile(QDir(s_userDirectory).filePath(fileName));
        if(userFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "[DMHTheme] Using user theme file" << userFile.fileName();
            return userFile.readAll();
        }
    }

    QFile builtinFile(BUILTIN_THEME_PATH + fileName);
    if(!builtinFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "[DMHTheme] Unable to read built-in theme file" << builtinFile.fileName();
        return QByteArray();
    }
    return builtinFile.readAll();
}

// Lines are "name = colour"; lines starting with ';' or '//' are comments.
void parseColors(const QByteArray& data, const QString& source, ThemeColors& colors)
{
    const QStringList lines = QString::fromUtf8(data).split(QLatin1Char('\n'));
    for(const QString& rawLine : lines)
    {
        const QString line = rawLine.trimmed();
        if(line.isEmpty() || line.startsWith(QLatin1Char(';')) || line.startsWith(QStringLiteral("//")))
            continue;

        const int separator = line.indexOf(QLatin1Char('='));
        const QString name = line.left(separator).trimmed();
        const QColor color = QColor::fromString(line.mid(separator + 1).trimmed());
        if((separator <= 0) || name.isEmpty() || !color.isValid())
        {
            qWarning() << "[DMHTheme] Ignoring invalid colour line in" << source << ":" << line;
            continue;
        }
        colors.insert(name, color);
    }
}

ThemeColors loadColors(const QString& fileName)
{
    // Built-in values first so a user file only needs the entries it changes.
    ThemeColors colors;
    QFile builtinFile(BUILTIN_THEME_PATH + fileName);
    if(builtinFile.open(QIODevice::ReadOnly))
        parseColors(builtinFile.readAll(), builtinFile.fileName(), colors);

    if(!s_userDirectory.isEmpty())
    {
        QFile userFile(QDir(s_userDirectory).filePath(fileName));
        if(userFile.open(QIODevice::ReadOnly))
            parseColors(userFile.readAll(), userFile.fileName(), colors);
    }
    return colors;
}

QString qssColor(const QColor& color)
{
    if(color.alpha() == 0)
        return QStringLiteral("transparent");
    return color.name(color.alpha() == OPAQUE_ALPHA ? QColor::HexRgb : QColor::HexArgb);
}

QString substituteTokens(QString sheet, const ThemeColors& colors)
{
    // Longest names first so @text does not clobber @textMuted.
    QStringList names = colors.keys();
    std::sort(names.begin(), names.end(), [](const QString& a, const QString& b) { return a.length() > b.length(); });
    for(const QString& name : names)
        sheet.replace(TOKEN_PREFIX + name, qssColor(colors.value(name)));
    return sheet;
}

QPalette modernPalette(const ThemeColors& colors)
{
    const QColor window = colors.value(QStringLiteral("window"));
    const QColor surface = colors.value(QStringLiteral("surface"));
    const QColor surfaceHover = colors.value(QStringLiteral("surfaceHover"));
    const QColor border = colors.value(QStringLiteral("border"));
    const QColor text = colors.value(QStringLiteral("text"));
    const QColor textMuted = colors.value(QStringLiteral("textMuted"));
    const QColor accent = colors.value(QStringLiteral("accent"));
    const QColor onAccent = colors.value(QStringLiteral("onAccent"));

    QPalette palette;
    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, surface);
    palette.setColor(QPalette::AlternateBase, surfaceHover);
    palette.setColor(QPalette::ToolTipBase, surface);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::PlaceholderText, textMuted);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, surface);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::BrightText, onAccent);
    palette.setColor(QPalette::Light, surface.lighter(BEVEL_LIGHT_FACTOR));
    palette.setColor(QPalette::Midlight, surfaceHover);
    palette.setColor(QPalette::Mid, border);
    palette.setColor(QPalette::Dark, border.darker(BEVEL_DARK_FACTOR));
    palette.setColor(QPalette::Shadow, SHADOW_COLOR);
    palette.setColor(QPalette::Highlight, accent);
    palette.setColor(QPalette::HighlightedText, onAccent);
    palette.setColor(QPalette::Link, accent);
    palette.setColor(QPalette::LinkVisited, accent.darker(BEVEL_DARK_FACTOR));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, textMuted);
    palette.setColor(QPalette::Disabled, QPalette::Text, textMuted);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, textMuted);
    return palette;
}

void setStyleIfDifferent(const QString& styleName)
{
    if(QApplication::style() && QApplication::style()->name().compare(styleName, Qt::CaseInsensitive) == 0)
        return;
    QApplication::setStyle(styleName);
}

void setColorScheme(DMHTheme::ThemeType themeType)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QStyleHints* hints = QGuiApplication::styleHints();
    if(!hints)
        return;

    if(themeType == DMHTheme::ThemeType_System)
        hints->unsetColorScheme();
    else
        hints->setColorScheme(themeType == DMHTheme::ThemeType_Dark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);
#else
    Q_UNUSED(themeType);
#endif
}

DMHTheme::ThemeType resolveSystem()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QStyleHints* hints = QGuiApplication::styleHints();
    return (hints && hints->colorScheme() == Qt::ColorScheme::Dark) ? DMHTheme::ThemeType_Dark : DMHTheme::ThemeType_Light;
#else
    return DMHTheme::ThemeType_Light;
#endif
}

void updateParchmentBase(QWidget* widget)
{
    if(!widget)
        return;

    if(s_resolvedTheme == DMHTheme::ThemeType_Classic)
    {
        static const QPixmap parchment(PARCHMENT_PATH);
        QPalette palette;
        palette.setBrush(QPalette::Base, QBrush(parchment));
        widget->setPalette(palette);
    }
    else
    {
        widget->setPalette(QPalette());
    }
}

void writeDefaults(const QString& directory)
{
    const QDir defaultsDir(QDir(directory).filePath(DEFAULTS_SUBDIRECTORY));
    if(!defaultsDir.exists() && !QDir().mkpath(defaultsDir.absolutePath()))
    {
        qWarning() << "[DMHTheme] Unable to create theme defaults directory" << defaultsDir.absolutePath();
        return;
    }

    for(const QString& fileName : DMHTheme::themeFileNames())
    {
        QFile builtinFile(BUILTIN_THEME_PATH + fileName);
        if(!builtinFile.open(QIODevice::ReadOnly))
            continue;
        const QByteArray builtin = builtinFile.readAll();

        QFile defaultFile(defaultsDir.filePath(fileName));
        if(defaultFile.open(QIODevice::ReadOnly) && (defaultFile.readAll() == builtin))
            continue;
        defaultFile.close();

        if(defaultFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            defaultFile.write(builtin);
        else
            qWarning() << "[DMHTheme] Unable to write theme default" << defaultFile.fileName();
    }
}

}

void DMHTheme::apply(ThemeType themeType)
{
    if(!qApp)
        return;

    if(!s_originalCaptured)
    {
        s_originalStyleName = QApplication::style() ? QApplication::style()->name() : QString();
        s_originalPalette = QApplication::palette();
        s_originalCaptured = true;
    }

    if((themeType < ThemeType_Classic) || (themeType >= ThemeType_Count))
        themeType = ThemeType_Classic;

    // An explicit scheme masks the OS value, so System must unset it before resolving.
    setColorScheme(themeType);
    s_currentTheme = themeType;
    s_resolvedTheme = (themeType == ThemeType_System) ? resolveSystem() : themeType;

    qDebug() << "[DMHTheme] Applying theme" << themeName(s_currentTheme) << "resolved to" << themeName(s_resolvedTheme);

    const QString baseSheet = QString::fromUtf8(readThemeFile(BASE_QSS));
    if(s_resolvedTheme == ThemeType_Classic)
    {
        if(!s_originalStyleName.isEmpty())
            setStyleIfDifferent(s_originalStyleName);
        QApplication::setPalette(s_originalPalette);
        qApp->setStyleSheet(QString::fromUtf8(readThemeFile(CLASSIC_QSS)) + QLatin1Char('\n') + baseSheet);
    }
    else
    {
        const ThemeColors colors = loadColors(s_resolvedTheme == ThemeType_Dark ? DARK_COLORS : LIGHT_COLORS);
        setStyleIfDifferent(MODERN_STYLE_NAME);
        QApplication::setPalette(modernPalette(colors));
        qApp->setStyleSheet(substituteTokens(QString::fromUtf8(readThemeFile(MODERN_QSS)) + QLatin1Char('\n') + baseSheet, colors));
    }

    s_parchmentWidgets.removeAll(QPointer<QWidget>());
    for(const QPointer<QWidget>& widget : std::as_const(s_parchmentWidgets))
        updateParchmentBase(widget);
}

DMHTheme::ThemeType DMHTheme::currentTheme()
{
    return s_currentTheme;
}

bool DMHTheme::isClassic()
{
    return s_resolvedTheme == ThemeType_Classic;
}

void DMHTheme::setUserThemeDirectory(const QString& directory)
{
    s_userDirectory = directory;
    if(!s_userDirectory.isEmpty())
        writeDefaults(s_userDirectory);
}

QString DMHTheme::userThemeDirectory()
{
    return s_userDirectory;
}

QStringList DMHTheme::themeFileNames()
{
    return {BASE_QSS, CLASSIC_QSS, MODERN_QSS, LIGHT_COLORS, DARK_COLORS};
}

void DMHTheme::setParchmentBase(QWidget* widget)
{
    if(!widget)
        return;

    if(!s_parchmentWidgets.contains(widget))
        s_parchmentWidgets.append(widget);
    updateParchmentBase(widget);
}

QString DMHTheme::themeName(ThemeType themeType)
{
    if((themeType < ThemeType_Classic) || (themeType >= ThemeType_Count))
        return QString();
    return QString::fromLatin1(THEME_NAMES[themeType]);
}

DMHTheme::ThemeType DMHTheme::themeFromName(const QString& name)
{
    for(int i = ThemeType_Classic; i < ThemeType_Count; ++i)
    {
        if(name.compare(QLatin1String(THEME_NAMES[i]), Qt::CaseInsensitive) == 0)
            return static_cast<ThemeType>(i);
    }
    return ThemeType_Classic;
}
