#include "chasedialog.h"
#include "chaseanimationdialog.h"
#include "chaser.h"
#include "chaserwidget.h"
#include "dice.h"
#include "dmconstants.h"
#include "ui_chasedialog.h"
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>
#include <QDebug>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QScrollBar>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QTimer>
#include <QGraphicsDropShadowEffect>

/*
Todo:
    Reposition items on resize of viewport
    Each complication once before repeating
    Check with the DM before assigning a complication - popup!
    Click on a complication to get its data
    Add/remove chasers
    DONE: Follow multiple complications
    Click on complication to get details
    Query to pass/fail complication
    Add icon to widget
    Make widget use a form
    Debug/test different types of complications
*/

const int CHASER_ICON_WIDTH = DMHelper::CHARACTER_ICON_WIDTH * 3;
const int CHASER_ICON_HEIGHT = DMHelper::CHARACTER_ICON_HEIGHT * 3;
const int CHASER_ICON_GAP = DMHelper::CHARACTER_ICON_WIDTH;
const int CHASER_ICON_TOTAL_WIDTH = CHASER_ICON_WIDTH + CHASER_ICON_GAP;

ChaseDialog::ChaseDialog(const QString& filename, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChaseDialog),
    _chaseLayout(0),
    _chaseScene(0),
    _centralLine(0),
    _animation(0),
    _animationTimer(0),
    _complicationList(),
    _chaserList(),
    _activeChaser(DMH_GLOBAL_INVALID_ID),
    _chaseFileName(filename),
    _chaseData()
{
    ui->setupUi(this);
    connect(ui->btnCheckComplication,SIGNAL(clicked()),this,SLOT(generateComplication()));
    connect(ui->btnEndChase,SIGNAL(clicked()),this,SIGNAL(chaseComplete()));
    connect(ui->chkDistance,SIGNAL(clicked()),this,SLOT(sortChasers()));
    connect(ui->btnMove,SIGNAL(clicked()),this,SLOT(moveChaser()));
    connect(ui->btnDash,SIGNAL(clicked()),this,SLOT(dashChaser()));
    connect(ui->btnSkipChaser,SIGNAL(clicked()),this,SLOT(selectNextChaser()));
    connect(ui->btnPublish,SIGNAL(clicked()),this,SLOT(handlePublishButton()));

    _chaseLayout = new QVBoxLayout;
    _chaseLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->scrollAreaWidgetContents->setLayout(_chaseLayout);

    ui->edtRoundsPerComplication->setValidator(new QDoubleValidator(0.1, 999.0, 2));

    loadData();

    _animation = new ChaseAnimationDialog(this);

    _chaseScene = new QGraphicsScene(this);
    _centralLine = _chaseScene->addLine(0, 0, 0, 15);
    connect(_chaseScene,SIGNAL(selectionChanged()),this,SLOT(sceneSelectionChanged()));

    _animation->getGraphicsView()->setScene(_chaseScene);
    ui->stackedSelectionDetails->setCurrentIndex(DMHelper::ChaseContent_Blank);

    _animation->show();

    _animationTimer = new QTimer(this);
    connect(_animationTimer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
    _animationTimer->start(30);
}

ChaseDialog::~ChaseDialog()
{
    _animationTimer->stop();

    QLayoutItem *child;
    while ((child = _chaseLayout->takeAt(0)) != 0)
    {
        child->widget()->deleteLater();
        delete child;
    }

    delete ui;
}

void ChaseDialog::addCombatant(Combatant* combatant, bool pursuer)
{
    int xPosition = getChaserXPos(_chaserList.count());

    QGraphicsTextItem* textItem = _chaseScene->addText(combatant->getName());
    textItem->setPos( xPosition - textItem->document()->size().width() / 2, 0 );
    textItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

    QGraphicsPixmapItem* pixmapItem = _chaseScene->addPixmap(combatant->getIconPixmap(DMHelper::PixmapSize_Animate));
    pixmapItem->setOffset(-pixmapItem->pixmap().width()/2, -pixmapItem->pixmap().height());
    pixmapItem->setPos(xPosition, 0);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(5);
    shadow->setOffset(0,0);
    pixmapItem->setGraphicsEffect(shadow);

    registerChaser(new ChaserCombatant(textItem, pixmapItem, combatant, 0, pursuer));
}

void ChaseDialog::addCombatants(QList<Combatant*> combatants, bool pursuer)
{
    for(int i = 0; i < combatants.count(); ++i)
    {
        addCombatant(combatants.at(i), pursuer);
    }
}

void ChaseDialog::addCombatantGroup(CombatantGroup group, bool pursuer)
{
    if(group.second == NULL)
        return;

    QList<Combatant*> combatants = Combatant::instantiateCombatants(group);
    for(int i = 0; i < combatants.count(); ++i)
    {
        addCombatant(combatants.at(i), pursuer);
    }
}

void ChaseDialog::addCombatantGroups(CombatantGroupList groups, bool pursuer)
{
    for(int i = 0; i < groups.count(); ++i)
    {
        addCombatantGroup(groups.at(i), pursuer);
    }
}

void ChaseDialog::removeCombatant(Combatant* combatant)
{
    Q_UNUSED(combatant);
    // TODO
}

void ChaseDialog::generateComplication()
{
    if(_chaseLayout->count() == 0)
        return;

    double roundsPerComplication = ui->edtRoundsPerComplication->text().toDouble();
    if(( roundsPerComplication <= 0.0 ) || (_chaseLayout->count() <= 0 ))
        return;

    double targetPercent = 100.0 / (roundsPerComplication * _chaseLayout->count());

    int percentRoll = Dice::d100();
    ui->edtPercentage->setText(QString::number(percentRoll));

    if( percentRoll > targetPercent )
       return;

    if(!_chaseData.contains(ui->cmbLocation->currentText()))
        return;

    int roll = Dice::dX(_chaseData.value(ui->cmbLocation->currentText()).size() - 1);
    ui->edtRoll->setText(QString::number(roll));

    ComplicationData data;
    if( !getComplication(ui->cmbLocation->currentText(), roll, data) )
        return;

    QGraphicsTextItem* textItem = _chaseScene->addText(data._title);
    textItem->setDefaultTextColor(QColor(Qt::blue));
    textItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
    textItem->hide();

    int position = 0;
    if(data._sticky)
    {
        // Position this complication 10-50 ft in front of the lead runner
        Chaser* furthest = getFurthest();
        position = furthest->getPosition() + 10 + (Dice::d4() * 10);

        if(QMessageBox::question(this,QString("Large Complication Confirmation"), QString("Should the ") + data._title + (" complication be added at position ") + QString::number(position) + QString("?")) == QMessageBox::No )
        {
            delete textItem;
            return;
        }

        QPair<int,int> minmax = getChaserMinMaxX();
        _chaseScene->addRect(minmax.first, position, minmax.second - minmax.first, data._length, QPen(), QBrush(Qt::BDiagPattern) );
        textItem->setPos(0, position + data._length);
    }
    else
    {
        // Position this complication 10 ft in front of the next runner in the initiative order
        Chaser* next = getNextMover();
        if(next == NULL)
            next = getFirstMover();

        position = next->getPosition() + 10;
        if(QMessageBox::question(this,QString("Personal Complication Confirmation"), QString("Should the ") + data._title + QString(" complication be added at position ") + QString::number(position) + QString(" in front of ") + next->getName() + QString("?")) == QMessageBox::No )
        {
            delete textItem;
            return;
        }

        int x = next->getX() + (next->getWidth() / 2);
        _chaseScene->addRect(x - textItem->document()->idealWidth() / 2 - 5, position, textItem->document()->idealWidth() +  10, data._length, QPen(), QBrush(Qt::FDiagPattern));
        textItem->setPos(x - textItem->document()->idealWidth() / 2, position + data._length);
    }

    textItem->show();

    ChaserComplication* newComplication = new ChaserComplication(textItem, data, true, position);

    textItem->setData(DMHelper::ChaserContent_Type, DMHelper::ChaseContent_Complication);
    textItem->setData(DMHelper::ChaserContent_Pointer, QVariant::fromValue(newComplication));

    _complicationList.append(newComplication);
    validateLine();

    _chaseScene->clearSelection();
    newComplication->getTextItem()->setSelected(true);
}

void ChaseDialog::sortChasers()
{
    clearChaserWidgets();
    std::sort(_chaserList.begin(), _chaserList.end(), ui->chkDistance->isChecked() ? CompareChasersPosition : CompareChasersInitiative);
    buildChaserWidgets();
}

void ChaseDialog::selectNextChaser()
{
    if(_activeChaser >= _chaserList.count() - 1)
    {
        sortChasers();
        setActiveChaser(0);
    }
    else
    {
        setActiveChaser(_activeChaser + 1);
    }
}

void ChaseDialog::moveChaser()
{
    if((_activeChaser < 0) || (_activeChaser >= _chaserList.count()))
        return;

    ChaserCombatant* chaser = _chaserList.at(_activeChaser);
    moveChaserBy(chaser, chaser->getSpeed());
    selectNextChaser();
}

void ChaseDialog::dashChaser()
{
    if((_activeChaser < 0) || (_activeChaser >= _chaserList.count()))
        return;

    ChaserCombatant* chaser = _chaserList.at(_activeChaser);
    moveChaserBy(chaser, chaser->getSpeed() * 2);

    if(chaser->getDashCount() > 0)
    {
        chaser->useDash();
    }
    else
    {
        if(QMessageBox::question(this,
                                 QString("Exhaustion Check"),
                                 QString("WARNING: Potential Exhaustion!\n\nDoes this chaser pass a DC 10 Consitution check?")) == QMessageBox::No )
        {
            chaser->addExhaustion();
        }
    }

    selectNextChaser();
}

void ChaseDialog::sceneSelectionChanged()
{
    if(!_chaseScene->selectedItems().isEmpty())
    {
        QGraphicsItem* item = _chaseScene->selectedItems().first();
        QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item);
        if(textItem)
        {
            int contentType = textItem->data(DMHelper::ChaserContent_Type).toInt();
            switch(contentType)
            {
                case DMHelper::ChaseContent_Complication:
                    {
                        ChaserComplication* complication = textItem->data(DMHelper::ChaserContent_Pointer).value<ChaserComplication*>();
                        if(complication)
                        {
                            showComplication(complication->getData());
                            ui->stackedSelectionDetails->setCurrentIndex(DMHelper::ChaseContent_Complication);
                            return;
                        }
                    }
                    break;
                case DMHelper::ChaseContent_Character:
                    {
                        ChaserCombatant* chaser = textItem->data(DMHelper::ChaserContent_Pointer).value<ChaserCombatant*>();
                        if(chaser)
                        {
                            showCombatant(chaser->getCombatant());
                            ui->stackedSelectionDetails->setCurrentIndex(DMHelper::ChaseContent_Character);
                            return;
                        }
                    }
                    break;
                case DMHelper::ChaseContent_Blank:
                default:
                    break;
            }
        }
    }

    ui->stackedSelectionDetails->setCurrentIndex(DMHelper::ChaseContent_Blank);
}

void ChaseDialog::keyPressEvent(QKeyEvent * event)
{
    if( event->key() == Qt::Key_Escape )
    {
        hide();
    }
    else
    {
        QDialog::keyPressEvent(event);
    }
}

void ChaseDialog::showEvent(QShowEvent * event)
{
    if(_animation)
        _animation->show();

    QDialog::showEvent(event);
}

void ChaseDialog::hideEvent(QHideEvent * event)
{
    if(_animation)
        _animation->hide();

    QDialog::hideEvent(event);
}


bool ChaseDialog::eventFilter(QObject *obj, QEvent *event)
{
    QResizeEvent *resizeEvent = 0;

    if(event->type()==QEvent::Resize)
    {
        resizeEvent = dynamic_cast<QResizeEvent *>(event);
        if(resizeEvent)
        {
            for(int i=0; i < _chaserList.count(); ++i)
            {
                if(_chaserList.at(i)->getTextItem())
                {
                    _chaserList.at(i)->getTextItem()->setX(getChaserXPos(i));
                }
            }

            validateLine();
        }
    }

    return QDialog::eventFilter(obj, event);
}

bool ChaseDialog::getComplication(const QString& location, int value, ComplicationData& data) const
{
    if(!_chaseData.contains(location))
        return false;

    QList<ComplicationData> complicationList = _chaseData.value(location);
    if((value < 0) || (value >= complicationList.size()))
    {
        data = ComplicationData(QString("No Complication"));
    }
    else
    {
        data = complicationList.at(value);
    }

    return true;
}

ChaserComplication* ChaseDialog::findComplication(QGraphicsTextItem* textItem) const
{
    for(int i = 0; i < _complicationList.count(); ++i)
    {
        if(_complicationList.at(i)->getTextItem() == textItem)
        {
            return _complicationList.at(i);
        }
    }

    return NULL;
}

void ChaseDialog::showComplication(const ComplicationData& data)
{
    ui->edtTitle->setText(data._title);
    ui->edtDescription->setPlainText(data._description);
    ui->edtCheck->setPlainText(data._check);
    ui->edtDamage->setText(data._damage);
    ui->edtHindrance->setText(data._obstacle);
    ui->chkSticky->setChecked(data._sticky);
}

void ChaseDialog::showCombatant(Combatant* combatant)
{
    if(!combatant)
        return;

    // TODO: this isn't a real implementation

    ui->edtName->setText(combatant->getName());

    ui->lblIcon->setPixmap(combatant->getIconPixmap(DMHelper::PixmapSize_Thumb));
}

void ChaseDialog::loadData()
{
    QDomDocument doc( "DMHelperChaseXML" );
    QFile file( _chaseFileName );
    if( !file.open( QIODevice::ReadOnly ) )
        return;

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent( in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if( contentResult == false )
    {
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if( (root.isNull()) || (root.tagName() != "root") )
        return;

    _chaseData.clear();
    ui->cmbLocation->clear();

    QDomElement chaseElement = root.firstChildElement( QString("chase") );
    while( !chaseElement.isNull() )
    {
        QString locationName = chaseElement.attribute(QString("location"));
        QList<ComplicationData> newList;

        QDomElement complicationElement = chaseElement.firstChildElement( QString("complication") );
        while( !complicationElement.isNull() )
        {
            ComplicationData data;
            data._title = complicationElement.attribute(QString("title"));
            data._description = complicationElement.firstChildElement(QString("description")).text();
            data._check = complicationElement.firstChildElement(QString("check")).text();
            data._failure = complicationElement.firstChildElement(QString("failure")).text();
            data._damage = complicationElement.firstChildElement(QString("damage")).text();
            data._obstacle = complicationElement.firstChildElement(QString("obstacle")).text();
            data._sticky = (bool)complicationElement.firstChildElement(QString("sticky")).text().toInt();
            if((data._obstacle == QString("Prone"))||(data._obstacle == QString("Stop")))
            {
                data._length = 0;
            }
            else
            {
                data._length = data._obstacle.left(data._obstacle.indexOf(" ")).toInt();
            }

            // Create the list in a random order
            newList.insert(Dice::dX(newList.count()) - 1, data);

            complicationElement = complicationElement.nextSiblingElement( QString("complication") );
        }

        _chaseData.insert(locationName, newList);
        ui->cmbLocation->addItem(locationName);

        chaseElement = chaseElement.nextSiblingElement( QString("chase") );
    }
}

void ChaseDialog::registerChaser(ChaserCombatant* chaser)
{
    ChaserWidget* widget = new ChaserWidget(chaser, this);

    connect(widget,SIGNAL(positionChanged(int)),this,SLOT(sortChasers()));
    connect(widget,SIGNAL(initiativeChanged(int)),this,SLOT(sortChasers()));

    chaser->getTextItem()->setData(DMHelper::ChaserContent_Type, DMHelper::ChaseContent_Character);
    chaser->getTextItem()->setData(DMHelper::ChaserContent_Pointer, QVariant::fromValue(chaser));

    chaser->setWidget(widget);
    _chaseLayout->addWidget(widget);
    _chaserList.append(chaser);

    if(_chaserList.count() == 1)
    {
        setActiveChaser(0);
    }
}

void ChaseDialog::setActiveChaser(int newChaser)
{
    setHighlight(_activeChaser, false);
    _activeChaser = newChaser;
    setHighlight(_activeChaser, true);
}

void ChaseDialog::clearChaserWidgets()
{
    // TODO: something here?
}

void ChaseDialog::buildChaserWidgets()
{
    for(int i = 0; i < _chaserList.count(); ++i)
    {
        _chaseLayout->removeWidget(_chaserList.at(i)->getWidget());
    }

    for(int i = 0; i < _chaserList.count(); ++i)
    {
        _chaseLayout->addWidget(_chaserList.at(i)->getWidget());
    }
}

void ChaseDialog::setHighlight(int chaser, bool highlightOn)
{
    if((chaser < 0) || (chaser >= _chaserList.count()))
        return;

    ChaserCombatant* chaserCombatant = _chaserList.at(chaser);
    if(!chaserCombatant)
        return;

    ChaserWidget* widget = chaserCombatant->getWidget();
    if(!widget)
        return;

    if(highlightOn)
    {
        widget->setLineWidth(5);
        widget->setStyleSheet("color: rgb(242, 236, 226); background-color: rgb(115, 18, 0);");
    }
    else
    {
        widget->resetStyleSheet();
    }
}

void ChaseDialog::moveChaserBy(ChaserCombatant* chaser, int move)
{
    int i = 0;
    while(move > 0)
    {
        if(move <= chaser->getDeficit())
        {
            chaser->reduceDeficit(move);
            move = 0;
        }
        else
        {
            move -= chaser->getDeficit();
            chaser->setDeficit(0);

            if(i < _complicationList.count())
            {
                ChaserComplication* complication = _complicationList[i];
                const ComplicationData& data = complication->getData();
                if( complication->isActive() && ( chaser->getPosition() < complication->getPosition() ) && ( chaser->getPosition() + move > complication->getPosition() ) )
                {
                    if(QMessageBox::question(this,
                                             QString("Complication Check: ") + data._title,
                                             QString("Does ") + chaser->getName() + QString(" succeed in the check:\n") + data._check) == QMessageBox::No )
                    {
                        if(!data._failure.isEmpty())
                        {
                            QMessageBox::critical(this,
                                                  QString("Complication Failure: ") + data._title,
                                                  data._failure);
                        }

                        if(data._length == 0)
                        {
                            move -= complication->getPosition() - chaser->getPosition();
                            chaser->setPosition(complication->getPosition());
                            chaser->setDeficit(chaser->getSpeed() / 2);
                        }
                        else
                        {
                            move -= complication->getPosition() - chaser->getPosition();
                            int terrainCost = data._length * 2;
                            if(terrainCost <= move)
                            {
                                chaser->setPosition(complication->getPosition() + data._length);
                                move -= terrainCost;
                            }
                            else
                            {
                                chaser->setPosition(complication->getPosition() + (move / 2));
                                chaser->setDeficit(data._length - (move / 2));
                                move = 0;
                            }
                        }
                    }

                    if(!data._sticky)
                    {
                        complication->setActive(false);
                    }
                }
                ++i;
            }
            else
            {
                chaser->setPosition(chaser->getPosition() + move);
                move = 0;
            }
        }
    }

    validateLine();

    generateComplication();
}

void ChaseDialog::validateLine()
{
    _centralLine->setLine(0, _animation->getGraphicsView()->sceneRect().top(), 0, calculateSceneHeight() + 40);
}

int ChaseDialog::calculateSceneHeight() const
{
    int sceneHeight = 0;

    for(int i = 0; i < _chaserList.count(); ++i)
    {
        if( _chaserList.at(i)->getPosition() > sceneHeight )
            sceneHeight = _chaserList.at(i)->getPosition();
    }

    for(int j = 0; j < _complicationList.count(); ++j)
    {
        if( _complicationList.at(j)->getPosition() > sceneHeight )
            sceneHeight = _complicationList.at(j)->getPosition();
    }

    return sceneHeight;
}

int ChaseDialog::getChaserXPos(int index) const
{
    int split = index / 2;

    if(index % 2 == 0)
        return (CHASER_ICON_TOTAL_WIDTH / 2) + (split * CHASER_ICON_TOTAL_WIDTH);
    else
        return (CHASER_ICON_TOTAL_WIDTH / 2) - ((split + 1) * CHASER_ICON_TOTAL_WIDTH);
}

QPair<int,int> ChaseDialog::getChaserMinMaxX() const
{
    QPair<int,int> minmax(0,0);

    for(int i = 0; i < _chaserList.count(); ++i)
    {
        if(_chaserList.at(i)->getX() < minmax.first)
            minmax.first = _chaserList.at(i)->getX();

        if(_chaserList.at(i)->getX() > minmax.second)
            minmax.second = _chaserList.at(i)->getX();
    }

    return minmax;
}

Chaser* ChaseDialog::getFurthest()
{
    Chaser* result = NULL;
    int highestPosition = -1;

    for(int i = 0; i < _chaserList.count(); ++i)
    {
        Chaser* chaser = _chaserList.at(i);
        if(chaser)
        {
            if(chaser->getPosition() > highestPosition)
            {
                result = chaser;
                highestPosition = chaser->getPosition();
            }
        }
    }

    return result;
}

Chaser* ChaseDialog::getFirstMover()
{
    Chaser* result = NULL;
    int highestInitiative = -1;

    for(int i = 0; i < _chaserList.count(); ++i)
    {
        Chaser* chaser = _chaserList.at(i);
        if(chaser)
        {
            if(chaser->getInitiative() > highestInitiative)
            {
                result = chaser;
                highestInitiative = chaser->getInitiative();
            }
        }
    }

    return result;
}

Chaser* ChaseDialog::getNextMover()
{
    Chaser* result = NULL;
    int highestInitiative = -1;

    for(int i = 0; i < _chaserList.count(); ++i)
    {
        Chaser* chaser = _chaserList.at(i);
        if(chaser)
        {
            if(chaser->getInitiative() > highestInitiative)
            {
                result = chaser;
                highestInitiative = chaser->getInitiative();
            }
        }
    }

    return result;
}

void ChaseDialog::handlePublishButton()
{
    QImage image(_animation->getGraphicsView()->size(), QImage::Format_ARGB32);  // Create the image with the exact size of the shrunk scene
    image.fill(Qt::transparent);                                                    // Start all pixels transparent
    QPainter painter(&image);
    _animation->getGraphicsView()->render(&painter);
    emit publishChaseScene(image, Qt::black);
}

void ChaseDialog::updateAnimation()
{
    for(int i = 0; i < _chaserList.count(); ++i)
    {
        Chaser* chaser = _chaserList.at(i);
        if(chaser)
        {
            chaser->update();
        }
    }

}

bool ChaseDialog::CompareChasersPosition(const Chaser* a, const Chaser* b)
{
    if((!a)||(!b))
        return false;

    return a->getPosition() > b->getPosition();
}

bool ChaseDialog::CompareChasersInitiative(const Chaser* a, const Chaser* b)
{
    if((!a)||(!b))
        return false;

    return a->getInitiative() > b->getInitiative();
}

/*
if(location == QString("Urban"))
{
    switch(value)
    {
        case 1: return QString("A large obstacle such as a horse or cart blocks your way. Make a DC 15 Dexterity (Acrobatics) check to get past the obstacle. On a failed check, the obstacle counts as 10 feet of difficult terrain.");
        case 2: return QString("A crowd blocks your way. Make a DC 10 Strength (Athletics) or Dexterity (Acrobatics) check (your choice) to make your way through the crowd unimpeded. On a failed check, the crowd counts as 10 feet of difficult terrain.");
        case 3: return QString("A large stained-glass window or similar barrier blocks your path. Make a DC 10 Strength saving throw to smash through the barrier and keep going. On a failed save, you bounce off the barrier and fall prone.");
        case 4: return QString("A maze of barrels , crates, or similar obstacles stands in your way. Make a DC 10 Dexterity (Acrobatics) or Intelligence check (your choice) to navigate the maze. On a failed check, the maze counts as 10 feet of difficult terrain.");
        case 5: return QString("The ground beneath your feet is slippery with rain, spilled oil, or some other liquid. Make a DC 10 Dexterity saving throw. On a failed save, you fall prone.");
        case 6: return QString("You come upon a pack of dogs fighting over food. Make a DC 10 Dexterity (Acrobatics) check to get through the pack unimpeded. On a failed check, you are bitten and take 1d4 piercing damage, and the dogs count as 5 feet of difficult terrain.");
        case 7: return QString("You run into a brawl in progress. Make a DC 15 Strength (Athletics), Dexterity (Acrobatics), or Charisma (Intimidation) check (your choice) to get past the brawlers unimpeded. On a failed check, you take 2d4 bludgeoning damage, and the brawlers count as 10 feet of difficult terrain.");
        case 8: return QString("A beggar blocks your way. Make a DC 10 Strength (Athletics), Dexterity (Acrobatics), or Charisma (Intimidation) check (your choice) to slip past the beggar. You succeed automatically if you toss the beggar a coin. On a failed check, the beggar counts as 5 feet of difficult terrain.");
        case 9: return QString("An overzealous guard (see the Monster Manual (or game statistics) mistakes you for someone else. If you move 20 feet or more on your turn, the guard makes an opportunity attack against you with a spear (+3 to hit; 1d6 + 1 piercing damage on a hit).");
        case 10: return QString("You are forced to make a sharp turn to avoid colliding with something impassable. Make a DC 10 Dexterity saving throw to navigate the turn. On a failed save, you collide with something hard and take 1d4 bludgeoning damage.");
        default: return QString("No complication.");
    }
}
else if(location == QString("Wilderness"))
{
    switch(value)
    {
        case 1: return QString("Your path takes you through a rough patch of brush. Make a DC 10 Strength (Athletics) or Dexterity (Acrobatics) check (your choice) to get past the brush. On a failed check, the brush counts as 5 feet of difficult terrain.");
        case 2: return QString("Uneven ground threatens to slow your progress. Make a DC 10 Dexterity (Acrobatics) check to navigate the area. On a failed check, the ground counts as 10 feet of difficult terrain.");
        case 3: return QString("You run through a swarm ofinsects (see the Monster Manual for game statistics, with the DM choosing whichever kind of insects makes the most sense). The swarm makes an opportunity attack against you. (+3 to hit; 4d4 piercing damage on a hit).");
        case 4: return QString("A stream, ravine, or rock bed blocks you r path. Make a DC 10 Strength (Athletics) or Dexterity (Acrobatics) check (your choice) to cross the impediment. On a failed check, the impediment counts as 10 feet of difficult terrain.");
        case 5: return QString("Make a DC 10 Constitution saving throw. On a failed save, you are blinded by blowing sand, dirt, ash, snow, or pollen until the end of your turn. While blinded in this way, your speed is halved.");
        case 6: return QString("A sudden drop catches you by surprise. Make a DC 10 Dexterity saving throw to navigate the impediment. On a failed save, you fall 1d4 x 5 feet, taking 1d6 bludgeoning damage per 10 feet fallen as normal, and land prone.");
        case 7: return QString("You blunder into a hunter's snare. Make a DC 15 Dexterity saving throw to avoid it. On a failed save, you are caught in a net and restrained. See chapter 5 ""Equipment,"" of the Player's Handbook for rules on escaping a net.");
        case 8: return QString("You are caught in a stampede of spooked animals. Make a DC 10 Dexterity saving throw. On a failed save, you are knocked about and take 1d4 bludgeoning damage and 1d4 piercing damage.");
        case 9: return QString("Your path takes you near a patch of razorvine. Make a DC 15 Dexterity saving throw or use 10 feet of movement (your choice) to avoid the razorvine. On failed save, you take 1d10 slashing damage.");
        case 10: return QString("A creature indigenous to the area chases after you. The DM chooses a creature appropriate for the terrain.");
        default: return QString("No complication.");
    }
}
*/
