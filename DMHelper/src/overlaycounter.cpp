#include "overlaycounter.h"
#include "publishglimage.h"
#include "dmconstants.h"
#include "overlayframe.h"
#include <QDomElement>
#include <QPainter>
#include <QPainterPath>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>

OverlayCounter::OverlayCounter(int counter, const QString& name, QObject *parent) :
    Overlay{name, parent},
    _counterImage(nullptr),
    _counter(counter)
{
}

void OverlayCounter::inputXML(const QDomElement &element)
{
    setCounterValue(element.attribute(QString("counter"), QString::number(0)).toInt());

    Overlay::inputXML(element);
}

int OverlayCounter::getOverlayType() const
{
    return DMHelper::OverlayType_Counter;
}

QSize OverlayCounter::getSize() const
{
    return _counterImage ? _counterImage->getSize() : QSize();
}

void OverlayCounter::prepareFrame(QBoxLayout* frameLayout, int insertIndex)
{
    if(!frameLayout)
        return;

    QLineEdit* edtCounter = new QLineEdit();
    edtCounter->setText(QString::number(_counter));
    edtCounter->setStyleSheet("font-weight: bold; font-size: 14pt;");
    connect(edtCounter, &QLineEdit::textEdited, this, &OverlayCounter::setCounterString);
    connect(this, &OverlayCounter::counterChanged, edtCounter, &QLineEdit::setText);

    QVBoxLayout* upDownLayout = new QVBoxLayout();
    upDownLayout->setSpacing(0);
    QPushButton* btnUp = new QPushButton(QIcon(":/img/data/icon_plus.png"), QString());
    connect(btnUp, &QPushButton::clicked, [this, edtCounter](){ this->increase(); edtCounter->setText(QString::number(this->_counter)); } );
    upDownLayout->addWidget(btnUp);
    QPushButton* btnDown = new QPushButton(QIcon(":/img/data/icon_minus.png"), QString());
    connect(btnDown, &QPushButton::clicked, [this, edtCounter](){ this->decrease(); edtCounter->setText(QString::number(this->_counter)); } );
    upDownLayout->addWidget(btnDown);

    frameLayout->insertLayout(insertIndex, upDownLayout);
    frameLayout->insertWidget(insertIndex, edtCounter);
}

int OverlayCounter::getCounterValue() const
{
    return _counter;
}

void OverlayCounter::setX(int x)
{
    if(_counterImage)
        _counterImage->setX(static_cast<qreal>(x));
}

void OverlayCounter::setY(int y)
{
    if(_counterImage)
        _counterImage->setY(static_cast<qreal>(y));
}

void OverlayCounter::setCounterValue(int value)
{
    if(_counter == value)
        return;

    _counter = value;
    recreateContents();
    emit counterChanged(QString::number(_counter));
}

void OverlayCounter::setCounterString(const QString& valueString)
{
    setCounterValue(valueString.toInt());
}

void OverlayCounter::increase()
{
    setCounterValue(getCounterValue() + 1);
    recreateContents();
}

void OverlayCounter::decrease()
{
    setCounterValue(getCounterValue() - 1);
    recreateContents();
}

void OverlayCounter::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory)
{
    element.setAttribute(QString("counter"), QString::number(_counter));

    Overlay::internalOutputXML(doc, element, targetDirectory);
}

void OverlayCounter::doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix)
{
    Q_UNUSED(targetSize);

    if((!_counterImage) || (!functions))
        return;

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _counterImage->getMatrixData(), _counterImage->getMatrix());
    functions->glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _counterImage->getMatrixData());
    _counterImage->paintGL(functions, nullptr);
}

void OverlayCounter::doResizeGL(int w, int h)
{
    Q_UNUSED(w);

    if(!_counterImage)
        return;

    _counterImage->setScale(static_cast<qreal>(h) * getScale() / static_cast<qreal>(_counterImage->getImageSize().height()));
//    _counterImage->setX(static_cast<qreal>(w) - _counterImage->getSize().width());
//    _counterImage->setY(static_cast<qreal>(h) - (_counterImage->getSize().height()*2));
}

void OverlayCounter::createContentsGL()
{
    if(_counterImage)
    {
        delete _counterImage;
        _counterImage = nullptr;
    }

    QImage counterImageBorder(QString(":/img/data/icon_overlaycounterbackground.png"));

    // Draw the number for the counter
    QImage counterNumberImage = textToImage(QString::number(_counter)).scaled(counterImageBorder.size() * 0.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPoint textLocation = QPoint((counterImageBorder.width() - counterNumberImage.width()) / 2,
                                 (counterImageBorder.height() - counterNumberImage.height()) / 2);

    QImage counterImage(counterImageBorder.size(), QImage::Format_ARGB32_Premultiplied);
    counterImage.fill(Qt::transparent);

    QPainter p(&counterImage);
        QColor redColor(qBound(0, 255 * _counter / 12, 255), 0, 255, 196);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        //p.fillRect(fearCounterImage.rect(), redColor);
        //p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        //p.drawImage(0, 0, fearCounterImageGrey);
        //p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawImage(0, 0, counterImageBorder);

        p.drawImage(textLocation, counterNumberImage);

        // Draw outline
        //p.setPen(QPen(Qt::white, 5));
        //p.setBrush(redColor);
        //p.drawPath(centeredPath);
    p.end();

    _counterImage = new PublishGLImage(counterImage, false);
}
