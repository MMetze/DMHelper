#include "smoketestrunner.h"
#include "optionscontainer.h"
#include "dmh_vlc.h"
#include "battledialogmodel.h"
#include "battledialogmodelmonsterclass.h"
#include "battledialogmodelcharacter.h"
#include "characterv2.h"
#include "combatanttemplateadapter.h"
#include "monsterclassv2.h"
#include "monsterfactory.h"
#include "campaign.h"
#include "encounterbattle.h"
#include "layertokens.h"
#include "rulehealthdaggerheart.h"
#include "rulehealth5e.h"
#include <QDomDocument>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

namespace
{
    static constexpr int TEST_HEALTH_MAX = 8;
    static constexpr int TEST_LEGACY_HEALTH = 4;
    static constexpr int TEST_EDITED_HEALTH = 2;
    static constexpr int TEST_STRESS_MAX = 5;
    static constexpr int TEST_DAMAGE = 3;
    static constexpr int TEST_TOKEN_SIZE = 16;
    static const QColor TEST_TOKEN_COLOR(Qt::red);

    class HealthTestCharacterCombatant : public BattleDialogModelCharacter
    {
    public:
        using BattleDialogModelCharacter::BattleDialogModelCharacter;

        QPixmap getIconPixmap(DMHelper::PixmapSize) const override
        {
            QPixmap icon(TEST_TOKEN_SIZE, TEST_TOKEN_SIZE);
            icon.fill(TEST_TOKEN_COLOR);
            return icon;
        }
    };

    class HealthTestFactory : public MonsterFactory
    {
    public:
        explicit HealthTestFactory(bool resources)
        {
            _attributes.insert(QStringLiteral("name"), DMHAttribute(TemplateType_string, QString()));
            _attributes.insert(QStringLiteral("hit_points"), DMHAttribute(resources ? TemplateType_resource : TemplateType_integer, QStringLiteral("0")));
            _attributes.insert(QStringLiteral("stress"), DMHAttribute(TemplateType_resource, QStringLiteral("0")));
        }
    };
}

SmokeTestRunner::SmokeTestRunner(bool skipOpenGL) :
    _skipOpenGL(skipOpenGL),
    _checks(),
    _passed(0),
    _failed(0),
    _skipped(0)
{
}

int SmokeTestRunner::run(bool healthResourcesOnly, const QString& reportPath)
{
    qInfo() << "[SmokeTest] Starting smoke test...";

    if(!healthResourcesOnly)
    {
        checkEmbeddedResources();
        checkFontRegistration();
        checkAppResources();
        checkAppDirectories();
        checkVlcFiles();
        checkVlcInit();
        checkOpenGL();
    }
    checkHealthResources();

    int total = _passed + _failed + _skipped;
    bool allPassed = (_failed == 0);

    QJsonObject summary;
    summary[QString("total")] = total;
    summary[QString("passed")] = _passed;
    summary[QString("failed")] = _failed;
    summary[QString("skipped")] = _skipped;

    QJsonObject root;
    root[QString("passed")] = allPassed;
    root[QString("checks")] = _checks;
    root[QString("summary")] = summary;

    QJsonDocument doc(root);
    if(!reportPath.isEmpty())
    {
        QFile report(reportPath);
        const QByteArray json = doc.toJson(QJsonDocument::Indented);
        if(!report.open(QIODevice::WriteOnly) || report.write(json) != json.size())
        {
            qCritical() << "[SmokeTest] Cannot write report:" << reportPath << report.errorString();
            return 1;
        }
    }
    QTextStream out(stdout);
    out << doc.toJson(QJsonDocument::Compact) << Qt::endl;

    qInfo() << "[SmokeTest] Complete:" << _passed << "passed," << _failed << "failed," << _skipped << "skipped";
    return allPassed ? 0 : 1;
}

void SmokeTestRunner::addResult(const QString& name, bool passed, const QString& detail)
{
    QJsonObject check;
    check[QString("name")] = name;
    check[QString("passed")] = passed;
    if(!detail.isEmpty())
        check[QString("detail")] = detail;
    _checks.append(check);

    if(passed)
        _passed++;
    else
        _failed++;
}

void SmokeTestRunner::addSkipped(const QString& name, const QString& detail)
{
    QJsonObject check;
    check[QString("name")] = name;
    check[QString("skipped")] = true;
    if(!detail.isEmpty())
        check[QString("detail")] = detail;
    _checks.append(check);
    _skipped++;
}

void SmokeTestRunner::checkHealthResources()
{
    HealthTestFactory resourceFactory(true);
    MonsterClassv2 source(QStringLiteral("Health resource test"));
    source.setFactory(&resourceFactory);
    source.setResourceValue(QStringLiteral("hit_points"), ResourcePair(0, TEST_HEALTH_MAX));
    source.setResourceValue(QStringLiteral("stress"), ResourcePair(0, TEST_STRESS_MAX));

    Campaign campaign;
    campaign.getRuleset().setRuleHealth(RuleHealthDaggerheart::HealthType);
    EncounterBattle encounter(QString(), &campaign);
    BattleDialogModel model(&encounter);
    LayerTokens* layer = new LayerTokens(&model);
    model.getLayerScene().appendLayer(layer);

    BattleDialogModelMonsterClass monster(&source);
    monster.setLayer(layer);
    RuleHealth* health = RuleHealth::forCombatant(&monster);
    addResult(QStringLiteral("health_ruleset_resolution"), health != nullptr);
    if(!health)
        return;

    QDomDocument legacyDoc;
    QDomElement legacy = legacyDoc.createElement(QStringLiteral("battlecombatant"));
    legacy.setAttribute(QStringLiteral("monsterHP"), TEST_LEGACY_HEALTH);
    legacy.setAttribute(QStringLiteral("monsterMaxHP"), TEST_HEALTH_MAX);
    monster.inputXML(legacy, false);
    int dirtyCount = 0;
    QObject::connect(&monster, &BattleDialogModelCombatant::dirty, &monster, [&dirtyCount]() { ++dirtyCount; });
    monster.postProcessXML(legacy, false);
    addResult(QStringLiteral("health_legacy_migration"),
              health->getHealth(&monster) == TEST_LEGACY_HEALTH &&
              health->getMaxHealth(&monster) == TEST_HEALTH_MAX &&
              !monster.hasOverride(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_HEALTH)) && dirtyCount == 0);

    CombatantTemplateAdapter adapter(&monster);
    adapter.setValue(QStringLiteral("stress"), QString::number(TEST_EDITED_HEALTH));
    addResult(QStringLiteral("resource_scalar_current"),
              adapter.getResourceValue(QStringLiteral("stress")) == ResourcePair(TEST_EDITED_HEALTH, TEST_STRESS_MAX));
    adapter.setResourceValue(QStringLiteral("hit_points"), ResourcePair(TEST_EDITED_HEALTH, TEST_HEALTH_MAX));
    addResult(QStringLiteral("health_tracker_authoritative"),
              health->getHealth(&monster) == TEST_EDITED_HEALTH && monster.getHitPoints() == TEST_EDITED_HEALTH && !health->isDead(&monster));
    health->applyDamage(&monster, TEST_DAMAGE);
    addResult(QStringLiteral("health_damage_updates_tracker"),
              adapter.getResourceValue(QStringLiteral("hit_points")) == ResourcePair(TEST_EDITED_HEALTH + TEST_DAMAGE, TEST_HEALTH_MAX));
    health->applyDamage(&monster, TEST_HEALTH_MAX);
    addResult(QStringLiteral("health_dead_at_max"), RuleHealth::combatantIsDead(&monster) && health->getHealthFraction(&monster) == 0.0);
    health->applyDamage(&monster, -TEST_HEALTH_MAX - TEST_DAMAGE);
    addResult(QStringLiteral("health_healing_floor"), !RuleHealth::combatantIsDead(&monster) && health->getHealthFraction(&monster) == 1.0);

    monster.setOverride(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_HEALTH), TEST_LEGACY_HEALTH);
    monster.postProcessXML(legacy, false);
    addResult(QStringLiteral("health_resource_precedence"), health->getHealth(&monster) == 0);

    BattleDialogModelMonsterClass legacyOverrideMonster(&source);
    legacyOverrideMonster.setLayer(layer);
    legacyOverrideMonster.inputXML(legacy, false);
    legacyOverrideMonster.setOverride(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_HEALTH), TEST_EDITED_HEALTH);
    legacyOverrideMonster.postProcessXML(legacy, false);
    addResult(QStringLiteral("health_legacy_override_precedence"),
              health->getHealth(&legacyOverrideMonster) == TEST_EDITED_HEALTH);

    QDomDocument savedDoc;
    QDomElement root = savedDoc.createElement(QStringLiteral("combatants"));
    QDir target;
    QDomElement saved = monster.outputXML(savedDoc, root, target, false);
    addResult(QStringLiteral("health_no_duplicate_xml"),
              !saved.hasAttribute(QStringLiteral("monsterHP")) && !saved.hasAttribute(QStringLiteral("monsterMaxHP")));
    BattleDialogModelMonsterClass reloaded(&source);
    reloaded.setLayer(layer);
    reloaded.inputXML(saved, false);
    reloaded.postProcessXML(saved, false);
    addResult(QStringLiteral("health_resource_roundtrip"),
              health->getHealth(&reloaded) == health->getHealth(&monster) && health->getMaxHealth(&reloaded) == TEST_HEALTH_MAX);
    health->rollInitial(&reloaded);
    addResult(QStringLiteral("health_initial_track"), health->getHealth(&reloaded) == 0);

    adapter.setResourceValue(QStringLiteral("hit_points"), ResourcePair(TEST_EDITED_HEALTH, TEST_LEGACY_HEALTH));
    adapter.setValue(QStringLiteral("hit_points"), QString::number(TEST_LEGACY_HEALTH));
    addResult(QStringLiteral("health_instance_maximum"),
              health->getMaxHealth(&monster) == TEST_LEGACY_HEALTH && health->isDead(&monster));
    addResult(QStringLiteral("health_source_unchanged"),
              source.getResourceValue(QStringLiteral("hit_points")) == ResourcePair(0, TEST_HEALTH_MAX));

    Characterv2 character;
    character.setFactory(&resourceFactory);
    character.setResourceValue(QStringLiteral("hit_points"), ResourcePair(0, TEST_HEALTH_MAX));
    HealthTestCharacterCombatant characterCombatant(&character);
    characterCombatant.setLayer(layer);
    health->applyDamage(&characterCombatant, TEST_DAMAGE);
    addResult(QStringLiteral("health_character_resource"),
              health->getHealth(&characterCombatant) == TEST_DAMAGE &&
              characterCombatant.getHitPoints() == TEST_DAMAGE &&
              health->getMaxHealth(&characterCombatant) == TEST_HEALTH_MAX);
    addResult(QStringLiteral("health_character_source_unchanged"),
              character.getResourceValue(QStringLiteral("hit_points")) == ResourcePair(0, TEST_HEALTH_MAX));

    QGraphicsScene tokenScene;
    layer->addCombatant(&characterCombatant);
    layer->dmInitialize(&tokenScene);
    QGraphicsPixmapItem* characterItem = qgraphicsitem_cast<QGraphicsPixmapItem*>(layer->getCombatantItem(&characterCombatant));
    const auto tokenIsGrey = [characterItem]() {
        return characterItem && characterItem->pixmap().toImage().isGrayscale();
    };
    addResult(QStringLiteral("daggerheart_pc_healthy_token_colored"),
              characterItem && !tokenIsGrey() && !RuleHealth::combatantIsUnconscious(&characterCombatant));
    health->setHealth(&characterCombatant, TEST_HEALTH_MAX);
    addResult(QStringLiteral("daggerheart_pc_full_track_token_grey"),
              tokenIsGrey() && RuleHealth::combatantIsUnconscious(&characterCombatant) && characterItem->isVisible());
    addResult(QStringLiteral("daggerheart_pc_grey_visual_only"),
              !characterCombatant.hasConditionId(QStringLiteral("unconscious")) && !model.getActiveCombatant());
    health->applyDamage(&characterCombatant, -1);
    addResult(QStringLiteral("daggerheart_pc_healed_token_colored"), characterItem && !tokenIsGrey());
    characterCombatant.clearOverride(QStringLiteral("hit_points"));
    character.setResourceValue(QStringLiteral("hit_points"), ResourcePair(TEST_HEALTH_MAX, TEST_HEALTH_MAX));
    addResult(QStringLiteral("daggerheart_pc_sheet_update_token_grey"), tokenIsGrey());
    layer->dmUninitialize();
    layer->dmInitialize(&tokenScene);
    characterItem = qgraphicsitem_cast<QGraphicsPixmapItem*>(layer->getCombatantItem(&characterCombatant));
    addResult(QStringLiteral("daggerheart_pc_initial_full_track_token_grey"),
              characterItem && characterItem->pixmap().toImage().isGrayscale());
    layer->removeCombatant(&characterCombatant);
    layer->dmUninitialize();

    campaign.getRuleset().setRuleHealth(RuleHealth5e::HealthType);
    health = campaign.getRuleset().getRuleHealth();
    characterCombatant.setLayer(layer);
    health->setHealth(&characterCombatant, TEST_HEALTH_MAX);
    addResult(QStringLiteral("dnd_pc_full_hp_not_unconscious"), !RuleHealth::combatantIsUnconscious(&characterCombatant));
    health->setHealth(&characterCombatant, 0);
    addResult(QStringLiteral("dnd_pc_zero_hp_unconscious"), RuleHealth::combatantIsUnconscious(&characterCombatant));
    health->setHealth(&characterCombatant, 1);
    addResult(QStringLiteral("dnd_pc_healed_not_unconscious"), !RuleHealth::combatantIsUnconscious(&characterCombatant));

    HealthTestFactory scalarFactory(false);
    MonsterClassv2 scalarSource(QStringLiteral("Scalar health test"));
    scalarSource.setFactory(&scalarFactory);
    scalarSource.setIntValue(QStringLiteral("hit_points"), TEST_HEALTH_MAX);
    BattleDialogModelMonsterClass scalarMonster(&scalarSource);
    RuleHealth5e scalarHealth;
    scalarHealth.rollInitial(&scalarMonster);
    scalarHealth.applyDamage(&scalarMonster, TEST_DAMAGE);
    addResult(QStringLiteral("health_5e_unchanged"),
              scalarHealth.getHealth(&scalarMonster) == TEST_HEALTH_MAX - TEST_DAMAGE &&
              scalarHealth.getMaxHealth(&scalarMonster) == TEST_HEALTH_MAX && !scalarHealth.isDead(&scalarMonster));
}

void SmokeTestRunner::checkEmbeddedResources()
{
    const QStringList resources = {
        QString(":/img/data/dmhelper_opaque.png"),
        QString(":/img/data/parchment.jpg"),
        QString(":/img/data/fonts/Rellanic-Agx7.ttf"),
        QString(":/img/data/fonts/Davek-vGXA.ttf"),
        QString(":/img/data/fonts/Iokharic-dqvK.ttf"),
        QString(":/img/data/plus.png")
    };

    for(const QString& res : resources)
    {
        bool exists = QFile::exists(res);
        QString shortName = res.mid(res.lastIndexOf('/') + 1);
        addResult(QString("embedded_") + shortName, exists, exists ? QString() : QString("not found: ") + res);
    }
}

void SmokeTestRunner::checkFontRegistration()
{
    const QStringList fonts = {
        QString(":/img/data/fonts/Rellanic-Agx7.ttf"),
        QString(":/img/data/fonts/Davek-vGXA.ttf"),
        QString(":/img/data/fonts/Iokharic-dqvK.ttf")
    };

    for(const QString& font : fonts)
    {
        int id = QFontDatabase::addApplicationFont(font);
        QString shortName = font.mid(font.lastIndexOf('/') + 1);
        addResult(QString("font_") + shortName, id >= 0, id >= 0 ? QString() : QString("registration failed"));
    }
}

void SmokeTestRunner::checkAppResources()
{
    QStringList expected = OptionsContainer::getExpectedAppResources();
    for(const QString& filePath : expected)
    {
        bool exists = QFile::exists(filePath);
        QString shortName = QFileInfo(filePath).fileName();
        addResult(QString("app_resource_") + shortName, exists, exists ? QString() : QString("not found: ") + filePath);
    }
}

void SmokeTestRunner::checkAppDirectories()
{
    QStringList expected = OptionsContainer::getExpectedAppDirectories();
    for(const QString& dirPath : expected)
    {
        bool exists = QDir(dirPath).exists();
        QString shortName = QDir(dirPath).dirName();
        addResult(QString("app_dir_") + shortName, exists, exists ? QString() : QString("not found: ") + dirPath);
    }
}

void SmokeTestRunner::checkVlcFiles()
{
    QString appDir = QCoreApplication::applicationDirPath();

#ifdef Q_OS_MAC
    QDir frameworksDir(appDir);
    frameworksDir.cdUp();
    QString libPath = frameworksDir.filePath(QString("Frameworks/libvlc.dylib"));
    QString pluginsPath = frameworksDir.filePath(QString("Frameworks/plugins"));

    addResult(QString("vlc_library"), QFile::exists(libPath), QFile::exists(libPath) ? QString() : QString("not found: ") + libPath);
    addResult(QString("vlc_plugins_dir"), QDir(pluginsPath).exists(), QDir(pluginsPath).exists() ? QString() : QString("not found: ") + pluginsPath);
#else
    QString libPath = appDir + QString("/libvlc.dll");
    QString pluginsPath = appDir + QString("/plugins/plugins.dat");

    addResult(QString("vlc_library"), QFile::exists(libPath), QFile::exists(libPath) ? QString() : QString("not found: ") + libPath);
    addResult(QString("vlc_plugins_dat"), QFile::exists(pluginsPath), QFile::exists(pluginsPath) ? QString() : QString("not found: ") + pluginsPath);
#endif
}

void SmokeTestRunner::checkVlcInit()
{
    // Create a standalone VLC instance with --reset-plugins-cache to avoid
    // stale cache errors that produce massive stderr noise on CI.
    const char *args[] = {
        "--reset-plugins-cache",
        "--no-audio",
        "--no-video",
        "--verbose=0",
        ""
    };

    libvlc_instance_t* instance = libvlc_new(sizeof(args) / sizeof(*args), args);
    bool ok = (instance != nullptr);
    addResult(QString("vlc_init"), ok, ok ? QString() : QString("libvlc_new returned null"));
    if(instance)
        libvlc_release(instance);
}

void SmokeTestRunner::checkOpenGL()
{
    if(_skipOpenGL)
    {
        addSkipped(QString("opengl_context"), QString("skipped via --skip-opengl"));
        return;
    }

    QOffscreenSurface surface;
    surface.create();
    if(!surface.isValid())
    {
        addResult(QString("opengl_context"), false, QString("QOffscreenSurface creation failed"));
        return;
    }

    QOpenGLContext context;
    bool created = context.create();
    if(!created)
    {
        addResult(QString("opengl_context"), false, QString("QOpenGLContext creation failed"));
        surface.destroy();
        return;
    }

    context.makeCurrent(&surface);
    QSurfaceFormat fmt = context.format();
    QString version = QString::number(fmt.majorVersion()) + QString(".") + QString::number(fmt.minorVersion());
    context.doneCurrent();
    surface.destroy();

    addResult(QString("opengl_context"), true, version);
}
