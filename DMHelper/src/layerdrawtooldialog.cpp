#include "layerdrawtooldialog.h"
#include "ui_layerdrawtooldialog.h"
#include "colorpushbutton.h"
#include <QMenu>

LayerDrawToolDialog::LayerDrawToolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerDrawToolDialog),
    _currentLineType{Qt::SolidLine}
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // Line type menu
    QMenu* lineTypeMenu = new QMenu(this);
    LayerDrawToolDialog_LineTypeAction* solidAction = new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pensolid.png"), QString("Solid Line"), Qt::SolidLine);
    lineTypeMenu->addAction(solidAction);
    ui->btnLineType->setIcon(solidAction->icon());
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendash.png"), QString("Dashed Line"), Qt::DashLine));
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendot.png"), QString("Dotted Line"), Qt::DotLine));
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendashdot.png"), QString("Dash Dot Line"), Qt::DashDotLine));
    lineTypeMenu->addAction(new LayerDrawToolDialog_LineTypeAction(QIcon(":/img/data/icon_pendashdotdot.png"), QString("Dash Dot Dot Line"), Qt::DashDotDotLine));
    ui->btnLineType->setMenu(lineTypeMenu);
    connect(lineTypeMenu, &QMenu::triggered, this, &LayerDrawToolDialog::handleLineTypeTriggered);

    ui->btnColor->setRotationVisible(false);
    ui->btnFillColor->setColor(Qt::white);
    ui->btnFillColor->setRotationVisible(false);

    connect(ui->fillCheck, &QCheckBox::toggled, this, &LayerDrawToolDialog::updateControlStates);
    connect(ui->buttonGroup, &QButtonGroup::idClicked, this, &LayerDrawToolDialog::handleToolChanged);

    updateControlStates();
}

LayerDrawToolDialog::~LayerDrawToolDialog()
{
    delete ui;
}

DMHelper::DrawToolType LayerDrawToolDialog::getToolType() const
{
    QAbstractButton* checked = ui->buttonGroup->checkedButton();
    if(!checked)
        return DMHelper::DrawToolType_Path;

    if(checked == ui->btnLine)
        return DMHelper::DrawToolType_Path;
    if(checked == ui->btnErase)
        return DMHelper::DrawToolType_Eraser;
    if(checked == ui->btnCircle)
        return DMHelper::DrawToolType_Ellipse;
    if(checked == ui->btnRectangle)
        return DMHelper::DrawToolType_Rect;
    if(checked == ui->btnStraightLine)
        return DMHelper::DrawToolType_Line;
    if(checked == ui->btnText)
        return DMHelper::DrawToolType_Text;

    return DMHelper::DrawToolType_Path;
}

Qt::PenStyle LayerDrawToolDialog::getToolLineType() const
{
    return _currentLineType;
}

QColor LayerDrawToolDialog::getToolLineColor() const
{
    return ui->btnColor->getColor();
}

int LayerDrawToolDialog::getToolLineWidth() const
{
    return ui->spinSize->value();
}

QColor LayerDrawToolDialog::getToolFillColor() const
{
    return ui->btnFillColor->getColor();
}

bool LayerDrawToolDialog::isToolFilled() const
{
    return ui->fillCheck->isChecked();
}

QString LayerDrawToolDialog::getToolFontFamily() const
{
    return ui->fontCombo->currentFont().family();
}

int LayerDrawToolDialog::getToolFontSize() const
{
    return ui->fontSizeSpin->value();
}

void LayerDrawToolDialog::handleLineTypeTriggered(QAction* action)
{
    LayerDrawToolDialog_LineTypeAction* lineAction = dynamic_cast<LayerDrawToolDialog_LineTypeAction*>(action);
    if(!lineAction)
        return;

    _currentLineType = static_cast<Qt::PenStyle>(lineAction->getLineType());
    ui->btnLineType->setIcon(action->icon());
}

void LayerDrawToolDialog::handleToolChanged()
{
    updateControlStates();
    emit toolTypeChanged(getToolType());
}

void LayerDrawToolDialog::updateControlStates()
{
    DMHelper::DrawToolType tool = getToolType();

    bool isPen    = (tool != DMHelper::DrawToolType_Text) && (tool != DMHelper::DrawToolType_Eraser);
    bool isFill   = (tool == DMHelper::DrawToolType_Rect) || (tool == DMHelper::DrawToolType_Ellipse);
    bool isFont   = (tool == DMHelper::DrawToolType_Text);
    bool isEraser = (tool == DMHelper::DrawToolType_Eraser);

    ui->btnColor->setEnabled(!isEraser);
    ui->btnLineType->setEnabled(isPen);
    ui->lblSize->setEnabled(isPen);
    ui->spinSize->setEnabled(isPen);

    ui->fillCheck->setEnabled(isFill);
    ui->btnFillColor->setEnabled(isFill && ui->fillCheck->isChecked());

    ui->fontLabel->setEnabled(isFont);
    ui->fontCombo->setEnabled(isFont);
    ui->fontSizeSpin->setEnabled(isFont);
}





LayerDrawToolDialog_LineTypeAction::LayerDrawToolDialog_LineTypeAction(const QIcon &icon, const QString &text, int lineType, QObject *parent) :
    QAction(icon, text, parent),
    _lineType(lineType)
{
}

LayerDrawToolDialog_LineTypeAction::~LayerDrawToolDialog_LineTypeAction()
{
}

int LayerDrawToolDialog_LineTypeAction::getLineType() const
{
    return _lineType;
}

