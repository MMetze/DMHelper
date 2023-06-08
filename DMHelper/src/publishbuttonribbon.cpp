#include "publishbuttonribbon.h"
#include "colorpushbutton.h"
#include "layer.h"
#include "ui_publishbuttonribbon.h"
#include <QKeyEvent>
#include <QMenu>

PublishButtonRibbon::PublishButtonRibbon(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::PublishButtonRibbon)
{
    ui->setupUi(this);

    connect(ui->btnPublish, SIGNAL(clicked(bool)), this, SLOT(handleClicked(bool)));
    connect(ui->btnPublish, SIGNAL(toggled(bool)), this, SLOT(handleToggle(bool)));

    connect(ui->btnLayerTool, &QAbstractButton::clicked, this, &PublishButtonRibbon::layersClicked);
    connect(ui->btnCW, SIGNAL(clicked()), ui->btnColor, SLOT(rotateCW()));
    connect(ui->btnCW, SIGNAL(clicked()), this, SIGNAL(rotateCW()));
    connect(ui->btnCW, SIGNAL(clicked()), this, SLOT(handleRotation()));
    connect(ui->btnCCW, SIGNAL(clicked()), ui->btnColor, SLOT(rotateCCW()));
    connect(ui->btnCCW, SIGNAL(clicked()), this, SIGNAL(rotateCCW()));
    connect(ui->btnCCW, SIGNAL(clicked()), this, SLOT(handleRotation()));

    connect(ui->btnColor, SIGNAL(rotationChanged(int)), this, SIGNAL(rotationChanged(int)));
    connect(ui->btnColor, SIGNAL(colorChanged(const QColor&)), this, SLOT(handleColorChanged(const QColor&)));

    connect(ui->btnPlayersWindow, SIGNAL(clicked(bool)), this, SIGNAL(playersWindowClicked(bool)));

    setDefaults();

    setCheckable(true);
}

PublishButtonRibbon::~PublishButtonRibbon()
{
    delete ui;
}

bool PublishButtonRibbon::isChecked() const
{
    return ui->btnPublish->isChecked();
}

bool PublishButtonRibbon::isCheckable() const
{
    return ui->btnPublish->isCheckable();
}

QColor PublishButtonRibbon::getColor() const
{
    return ui->btnColor->getColor();
}

int PublishButtonRibbon::getRotation() const
{
    return ui->btnColor->getRotation();
}

PublishButtonRibbon* PublishButtonRibbon::getPublishRibbon()
{
    return this;
}

void PublishButtonRibbon::setChecked(bool checked)
{
    if(ui->btnPublish->isChecked() != checked)
    {
        ui->btnPublish->setChecked(checked);
        //emit clicked(checked);
    }
}

void PublishButtonRibbon::setCheckable(bool checkable)
{
    ui->btnPublish->setCheckable(checkable);
}

void PublishButtonRibbon::setRotation(int rotation)
{
    ui->btnColor->setRotation(rotation);
}

void PublishButtonRibbon::setColor(const QColor& color)
{
    ui->btnColor->setColor(color);
}

void PublishButtonRibbon::clickPublish()
{
    ui->btnPublish->click();
}

void PublishButtonRibbon::cancelPublish()
{
    if((ui->btnPublish->isCheckable()) && (ui->btnPublish->isChecked()))
    {
        setChecked(false);
        handleClicked(false);
    }
}

void PublishButtonRibbon::setPlayersWindow(bool checked)
{
    ui->btnPlayersWindow->setChecked(checked);
}

void PublishButtonRibbon::setLayersEnabled(bool enabled)
{
    ui->btnLayerTool->setEnabled(enabled);
}

void PublishButtonRibbon::setLayers(QList<Layer*> layers, int selected)
{
    QMenu* oldMenu = ui->btnLayerTool->menu();
    if(oldMenu)
    {
        ui->btnLayerTool->setMenu(nullptr);
        oldMenu->deleteLater();
    }

    //ui->btnLayerTool->setEnabled(layers.count() > 0);

    QAction* newDefaultAction = nullptr;
    if(layers.count() > 0)
    {
        QMenu* newMenu = new QMenu(this);
        QAction* newAction = nullptr;

        for(int i = 0; i < layers.count(); ++i)
        {
            if(layers.at(i))
            {
                newAction = new QAction(QIcon(QPixmap::fromImage(layers.at(i)->getLayerIcon())), layers.at(i)->getName());
                newAction->setData(i);
                newMenu->addAction(newAction);
                if(selected == i)
                    newDefaultAction = newAction;
            }
        }

        if(!newDefaultAction)
            newDefaultAction = newAction;

        newMenu->addSeparator();
        newMenu->addAction(new QAction(QIcon(":/img/data/icon_layers.png"), QString("Edit Layers...")));

        connect(newMenu, &QMenu::triggered, this, &PublishButtonRibbon::selectLayerAction);
        ui->btnLayerTool->setMenu(newMenu);
    }

    selectLayerAction(newDefaultAction);
}

void PublishButtonRibbon::updateLayerIcons()
{

}

void PublishButtonRibbon::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblPublish, *ui->btnPublish, frameHeight);

    setLineHeight(*ui->line_2, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);

    int labelHeight = getLabelHeight(*ui->lblPublish, frameHeight);
    int buttonSize = height() - labelHeight;
    setButtonSize(*ui->btnPlayersWindow, buttonSize, buttonSize);
    setLineHeight(*ui->line_3, buttonSize);
    setWidgetSize(*ui->btnLayerTool, buttonSize * 10 / 8, buttonSize);
    setWidgetSize(*ui->lblLayers, buttonSize * 10 / 8, labelHeight);
    setButtonSize(*ui->btnCW, buttonSize, buttonSize);
    setButtonSize(*ui->btnColor, buttonSize, buttonSize);
    setButtonSize(*ui->btnCCW, buttonSize, buttonSize);
    setWidgetSize(*ui->lblPlayersWindow, buttonSize * 4 + 10, labelHeight);
}

void PublishButtonRibbon::handleToggle(bool checked)
{
    if(checked)
    {
        emit colorChanged(ui->btnColor->getColor());
        ui->btnPublish->setIcon(QIcon(QPixmap(":/img/data/icon_publishon.png")));
    }
    else
    {
        ui->btnPublish->setIcon(QIcon(QPixmap(":/img/data/icon_publish.png")));
    }
}

void PublishButtonRibbon::handleClicked(bool checked)
{
    if((!ui->btnPublish->isCheckable()) || (checked))
        emit colorChanged(ui->btnColor->getColor());

    emit clicked(checked);
}

void PublishButtonRibbon::handleRotation()
{
    emit rotationChanged(ui->btnColor->getRotation());
}

void PublishButtonRibbon::handleColorChanged(const QColor& color)
{
    emit buttonColorChanged(color);

    if((ui->btnPublish->isCheckable()) && (ui->btnPublish->isChecked()))
        emit colorChanged(color);
}

void PublishButtonRibbon::selectLayerAction(QAction* action)
{
    if((action) && (action->text() == QString("Edit Layers...")))
    {
        emit layersClicked();
        return;
    }

    ui->btnLayerTool->setIcon(action ? action->icon() : QIcon(":/img/data/icon_layers.png"));
    if(ui->btnLayerTool->menu())
        ui->btnLayerTool->menu()->setDefaultAction(action);

    if(action)
        emit layerSelected(action->data().toInt());
}

void PublishButtonRibbon::setDefaults()
{
    ui->btnPublish->setAutoDefault(false);
    ui->btnCW->setAutoDefault(false);
    ui->btnCCW->setAutoDefault(false);
    ui->btnColor->setAutoDefault(false);

    ui->btnPublish->setDefault(false);
    ui->btnCW->setDefault(false);
    ui->btnCCW->setDefault(false);
    ui->btnColor->setDefault(false);
}
