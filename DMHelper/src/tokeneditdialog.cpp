#include "tokeneditdialog.h"
#include "ui_tokeneditdialog.h"
#include "optionscontainer.h"
#include <QPainter>
#include <QFileDialog>
#include <QImageReader>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>

TokenEditDialog::TokenEditDialog(const QString& tokenFilename, bool backgroundFill, const QColor& backgroundFillColor, bool transparent, const QColor& transparentColor, int transparentLevel, bool maskApplied, const QString& maskFile, bool frameApplied, const QString& frameFile, qreal zoom, const QPoint& offset, bool browsable, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TokenEditDialog),
    _editor(nullptr),
    _mouseDownPos()
{
    ui->setupUi(this);

    initialize(backgroundFill, backgroundFillColor, transparent, transparentColor, transparentLevel, maskApplied, maskFile, frameApplied, frameFile, zoom, offset, browsable);
    if(_editor)
    {
        _editor->setSourceFile(tokenFilename);
        updateImage();
    }
}

TokenEditDialog::TokenEditDialog(const QString& tokenFilename, const OptionsContainer& options, qreal zoom, const QPoint& offset, bool browsable, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TokenEditDialog),
    _editor(nullptr),
    _mouseDownPos()
{
    ui->setupUi(this);

    initialize(options.getTokenBackgroundFill(),
               options.getTokenBackgroundFillColor(),
               options.getTokenTransparent(),
               options.getTokenTransparentColor(),
               options.getTokenTransparentLevel(),
               options.getTokenMaskApplied(),
               options.getTokenMaskFile(),
               options.getTokenFrameApplied(),
               options.getTokenFrameFile(),
               zoom,
               offset,
               browsable);

    if(_editor)
    {
        _editor->setSourceFile(tokenFilename);
        updateImage();
    }
}

TokenEditDialog::TokenEditDialog(const QImage& sourceImage, bool backgroundFill, const QColor& backgroundFillColor, bool transparent, const QColor& transparentColor, int transparentLevel, bool maskApplied, const QString& maskFile, bool frameApplied, const QString& frameFile, qreal zoom, const QPoint& offset, bool browsable, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TokenEditDialog),
    _editor(nullptr),
    _mouseDownPos()
{
    ui->setupUi(this);

    initialize(backgroundFill, backgroundFillColor, transparent, transparentColor, transparentLevel, maskApplied, maskFile, frameApplied, frameFile, zoom, offset, browsable);
    if(_editor)
    {
        _editor->setSourceImage(sourceImage);
        updateImage();
    }
}

TokenEditDialog::TokenEditDialog(const QImage& sourceImage, const OptionsContainer& options, qreal zoom, const QPoint& offset, bool browsable, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TokenEditDialog),
    _editor(nullptr),
    _mouseDownPos()
{
    ui->setupUi(this);

    initialize(options.getTokenBackgroundFill(),
               options.getTokenBackgroundFillColor(),
               options.getTokenTransparent(),
               options.getTokenTransparentColor(),
               options.getTokenTransparentLevel(),
               options.getTokenMaskApplied(),
               options.getTokenMaskFile(),
               options.getTokenFrameApplied(),
               options.getTokenFrameFile(),
               zoom,
               offset,
               browsable);

    if(_editor)
    {
        _editor->setSourceImage(sourceImage);
        updateImage();
    }
}

TokenEditDialog::~TokenEditDialog()
{
    delete ui;
}

void TokenEditDialog::setSourceImage(const QImage& sourceImage)
{
    if(_editor)
    {
        _editor->setSourceImage(sourceImage);
        _editor->setOffset(QPoint());
        zoomReset();
    }
}

void TokenEditDialog::setBackgroundFillColor(const QColor& color)
{
    if(_editor)
    {
        _editor->setBackgroundFillColor(color);
        ui->btnFillColor->setColor(color);
    }
}

TokenEditor* TokenEditDialog::getEditor()
{
    return _editor;
}

QImage TokenEditDialog::getFinalImage()
{
    return _editor ? _editor->getFinalImage() : QImage();
}

void TokenEditDialog::updateImage()
{
    if(!_editor)
        return;

    ui->lblToken->setPixmap(QPixmap::fromImage(_editor->getFinalImage()));
}

bool TokenEditDialog::eventFilter(QObject *o, QEvent *e)
{
    if(o == ui->lblToken)
    {
        if(e->type() == QEvent::Wheel)
        {
            QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(e);
            if(wheelEvent->angleDelta().y() > 0)
                zoomIn();
            else
                zoomOut();

            return true;
        }
        else if(e->type() == QEvent::MouseMove)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(e);
            if(mouseEvent->buttons() & Qt::LeftButton)
            {
                if(_editor)
                    _editor->moveOffset((mouseEvent->position() - _mouseDownPos).toPoint());
                _mouseDownPos = mouseEvent->position();
                updateImage();
            }

            return true;
        }
        else if(e->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(e);
            if(mouseEvent->button() == Qt::LeftButton)
                _mouseDownPos = mouseEvent->position();

            return true;
        }
    }
    else if(o == ui->frameToken)
    {
        if(e->type() == QEvent::Resize)
        {
            QSize size = ui->frameToken->size();
            QMargins margins = ui->frameToken->contentsMargins();
            int sideLength = qMin(size.width() - margins.left() - margins.right() - (ui->lblToken->lineWidth() * 2) - 10,
                                  size.height() - margins.top() - margins.bottom() - (ui->lblToken->lineWidth() * 2) - 10);
            ui->lblToken->setFixedSize(sideLength, sideLength);
        }
    }

    return QDialog::eventFilter(o, e);
}

void TokenEditDialog::zoomIn()
{
    if(_editor)
        _editor->zoomIn();
}

void TokenEditDialog::zoomOut()
{
    if(_editor)
        _editor->zoomOut();
}

void TokenEditDialog::zoomReset()
{
    if(_editor)
        _editor->setZoom(1.0);
}

void TokenEditDialog::browseImage()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, QString("Select Image..."));
    if((filename.isEmpty()) || (!QImageReader(filename).canRead()))
        return;

    QImage newImage(filename);
    if(newImage.isNull())
    {
        QMessageBox::critical(nullptr, QString("Error"), QString("Unable to load image from: ") + filename);
        qDebug() << "[TokenEditDialog] ERROR: Unable to load source image from: " << filename;
    }

    setSourceImage(newImage);
}

void TokenEditDialog::browseFrame()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, QString("Select image frame..."));
    if((filename.isEmpty()) || (!QImageReader(filename).canRead()))
        return;

    ui->edtFrameImage->setText(filename);
}

void TokenEditDialog::browseMask()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, QString("Select image mask..."));
    if((filename.isEmpty()) || (!QImageReader(filename).canRead()))
        return;

    ui->edtMaskImage->setText(filename);
}

void TokenEditDialog::initialize(bool backgroundFill, const QColor& backgroundFillColor, bool transparent, const QColor& transparentColor, int transparentLevel, bool maskApplied, const QString& maskFile, bool frameApplied, const QString& frameFile, qreal zoom, const QPoint& offset, bool browsable)
{
    ui->lblToken->installEventFilter(this);
    ui->frameToken->installEventFilter(this);
    ui->btnBrowse->setVisible(browsable);

    connect(ui->btnZoomIn, &QAbstractButton::clicked, this, &TokenEditDialog::zoomIn);
    connect(ui->btnZoomOut, &QAbstractButton::clicked, this, &TokenEditDialog::zoomOut);
    connect(ui->btnZoomReset, &QAbstractButton::clicked, this, &TokenEditDialog::zoomReset);
    connect(ui->btnBrowse, &QAbstractButton::clicked, this, &TokenEditDialog::browseImage);

    ui->chkFill->setChecked(backgroundFill);
    ui->btnFillColor->setColor(backgroundFillColor);
    ui->btnFillColor->setRotationVisible(false);

    ui->chkTransparent->setChecked(transparent);
    ui->btnTransparentColor->setColor(transparentColor);
    ui->btnTransparentColor->setRotationVisible(false);
    ui->sliderFuzzy->setValue(transparentLevel);

    ui->chkMask->setChecked(maskApplied);
    ui->edtMaskImage->setText(maskFile);

    ui->chkFrame->setChecked(frameApplied);
    ui->edtFrameImage->setText(frameFile);

    _editor = new TokenEditor(QString(), backgroundFill, backgroundFillColor, transparent, transparentColor, transparentLevel, maskApplied, maskFile, frameApplied, frameFile, zoom, offset, this);

    connect(ui->chkFill, &QAbstractButton::toggled, _editor, &TokenEditor::setBackgroundFill);
    connect(ui->btnFillColor, &ColorPushButton::colorChanged, _editor, &TokenEditor::setBackgroundFillColor);
    connect(ui->chkTransparent, &QAbstractButton::toggled, _editor, &TokenEditor::setTransparent);
    connect(ui->btnTransparentColor, &ColorPushButton::colorChanged, _editor, &TokenEditor::setTransparentColor);
    connect(ui->sliderFuzzy, &QAbstractSlider::valueChanged, _editor, &TokenEditor::setTransparentLevel);
    connect(ui->chkMask, &QAbstractButton::toggled, _editor, &TokenEditor::setMaskApplied);
    connect(ui->edtMaskImage, &QLineEdit::textChanged, _editor, &TokenEditor::setMaskFile);
    connect(ui->btnBrowseMaskImage, &QAbstractButton::clicked, this, &TokenEditDialog::browseMask);
    connect(ui->chkFrame, &QAbstractButton::toggled, _editor, &TokenEditor::setFrameApplied);
    connect(ui->edtFrameImage, &QLineEdit::textChanged, _editor, &TokenEditor::setFrameFile);
    connect(ui->btnBrowseFrameImage, &QAbstractButton::clicked, this, &TokenEditDialog::browseFrame);

    connect(_editor, &TokenEditor::imageDirty, this, &TokenEditDialog::updateImage);
}

