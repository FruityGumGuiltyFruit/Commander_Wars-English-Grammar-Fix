#include "ruleselection.h"

#include "coreengine/mainapp.h"
#include "coreengine/console.h"
#include "coreengine/globalutils.h"

#include "game/gamemap.h"

#include "resource_management/gamerulemanager.h"
#include "resource_management/fontmanager.h"
#include "resource_management/objectmanager.h"

#include "objects/base/checkbox.h"
#include "objects/base/spinbox.h"
#include "objects/base/dropdownmenu.h"
#include "objects/base/timespinbox.h"
#include "objects/dialogs/rules/cobannlistdialog.h"
#include "objects/dialogs/rules/perkselectiondialog.h"
#include "objects/dialogs/rules/actionlistdialog.h"
#include "objects/base/label.h"
#include "objects/dialogs/filedialog.h"
#include "objects/base/passwordbox.h"
#include "objects/base/textbox.h"

constexpr qint32 textWidth = 300;

RuleSelection::RuleSelection(GameMap* pMap, qint32 width, Mode mode, bool enabled)
    : m_mode(mode),
      m_ruleChangeEabled(enabled),
      m_pMap(pMap)
{
    setObjectName("RuleSelection");
    Interpreter::setCppOwnerShip(this);
    setWidth(width);
    showRuleSelection();
}

void RuleSelection::confirmRuleSelectionSetup()
{
    if (m_ruleChangeEabled)
    {
        CONSOLE_PRINT("Confirming rule selection and enabling/disabling rules for the map.", Console::eDEBUG);
        GameRuleManager* pGameRuleManager = GameRuleManager::getInstance();
        
        for (qint32 i = 0; i < pGameRuleManager->getVictoryRuleCount(); i++)
        {
            QString ruleID = pGameRuleManager->getVictoryRuleID(i);
            spVictoryRule pRule = spVictoryRule(m_pMap->getGameRules()->getVictoryRule(ruleID));
            if (pRule.get() != nullptr)
            {
                QStringList inputTypes = pRule->getRuleType();
                if (inputTypes[0] == VictoryRule::checkbox)
                {
                    qint32 ruleValue = pRule->getRuleValue(0);
                    if (ruleValue == 0)
                    {
                        CONSOLE_PRINT("Removing rule cause it's disabled: " + ruleID, Console::eDEBUG);
                        m_pMap->getGameRules()->removeVictoryRule(ruleID);
                    }
                    else
                    {
                        CONSOLE_PRINT("Rule is enabled: " + ruleID, Console::eDEBUG);
                    }
                }
                else if (inputTypes[0] == VictoryRule::spinbox)
                {
                    qint32 ruleValue = pRule->getRuleValue(0);
                    qint32 infiniteValue = pRule->getInfiniteValue(0);
                    if (ruleValue <= infiniteValue)
                    {
                        CONSOLE_PRINT("Removing rule cause it's disabled: " + ruleID, Console::eDEBUG);
                        m_pMap->getGameRules()->removeVictoryRule(ruleID);
                    }
                    else
                    {
                        CONSOLE_PRINT("Rule is enabled: " + ruleID, Console::eDEBUG);
                    }
                }
                else
                {
                    CONSOLE_PRINT("Removing rule cause it's in unsupported format: " + ruleID, Console::eERROR);
                    m_pMap->getGameRules()->removeVictoryRule(ruleID);
                }
            }
        }

        for (qint32 i = 0; i < pGameRuleManager->getGameRuleCount(); i++)
        {
            QString ruleID = pGameRuleManager->getGameRuleID(i);
            spGameRule pRule = spGameRule(m_pMap->getGameRules()->getGameRule(ruleID));
            if (pRule.get() != nullptr)
            {
                QStringList inputTypes = pRule->getRuleType();
                if (inputTypes[0] == VictoryRule::checkbox)
                {
                    qint32 ruleValue = pRule->getRuleValue(0);
                    if (ruleValue == 0)
                    {
                        CONSOLE_PRINT("Removing rule cause it's disabled: " + ruleID, Console::eDEBUG);
                        m_pMap->getGameRules()->removeGameRule(ruleID);
                    }
                    else
                    {
                        CONSOLE_PRINT("Rule is enabled: " + ruleID, Console::eDEBUG);
                    }
                }
                else if (inputTypes[0] == VictoryRule::spinbox)
                {
                    qint32 ruleValue = pRule->getRuleValue(0);
                    qint32 infiniteValue = pRule->getInfiniteValue(0);
                    if (ruleValue <= infiniteValue)
                    {
                        CONSOLE_PRINT("Removing rule cause it's disabled: " + ruleID, Console::eDEBUG);
                        m_pMap->getGameRules()->removeGameRule(ruleID);
                    }
                    else
                    {
                        CONSOLE_PRINT("Rule is enabled: " + ruleID, Console::eDEBUG);
                    }
                }
                else
                {
                    CONSOLE_PRINT("Removing rule cause it's in unsupported format: " + ruleID, Console::eERROR);
                    m_pMap->getGameRules()->removeGameRule(ruleID);
                }
            }
        }
    }
}

void RuleSelection::showRuleSelection(bool advanced)
{
    removeChildren();
    GameRuleManager* pGameRuleManager = GameRuleManager::getInstance();
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    qint32 y = 20;
    // font style
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;

    oxygine::TextStyle headerStyle = oxygine::TextStyle(FontManager::getMainFont48());
    headerStyle.color = FontManager::getFontColor();
    headerStyle.hAlign = oxygine::TextStyle::HALIGN_LEFT;

    QColor headerColor(0, 255, 0, 255);
    

    spLabel textField = spLabel::create(textWidth - 40);
    textField->setStyle(style);
    textField->setHtmlText(tr("Show Advanced: "));
    textField->setPosition(30, y);
    addChild(textField);
    spCheckbox pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("Check this to see all game rules."));
    pCheckbox->setPosition(textWidth, textField->getY());
    addChild(pCheckbox);
    pCheckbox->setChecked(advanced);
    connect(pCheckbox.get(), &Checkbox::checkChanged, this, &RuleSelection::showRuleSelection, Qt::QueuedConnection);
    y += 40;

    if (m_mode == Mode::Multiplayer)
    {
        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Game Description: "));
        textField->setPosition(30, y);
        addChild(textField);
        spTextbox pTexbox = spTextbox::create(Settings::getWidth() - 100 - textWidth);
        pTexbox->setPosition(textWidth, y);
        pTexbox->setCurrentText("");
        pTexbox->setTooltipText(tr("Map description shown for players who want to join. Keep it short here."));
        pTexbox->setEnabled(m_ruleChangeEabled);
        connect(pTexbox.get(), &Textbox::sigTextChanged, m_pMap->getGameRules(), &GameRules::setDescription, Qt::QueuedConnection);
        addChild(pTexbox);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Password: "));
        textField->setPosition(30, y);
        addChild(textField);

        spPasswordbox pPasswordbox = spPasswordbox::create(Settings::getWidth() - 100 - textWidth);
        pPasswordbox->setPosition(textWidth, y);
        pPasswordbox->setCurrentText("");
        m_pMap->getGameRules()->setPassword("");
        pPasswordbox->setTooltipText(tr("Map description shown for players who want to join. Keep it short here."));
        pPasswordbox->setEnabled(m_ruleChangeEabled);
        connect(pPasswordbox.get(), &Passwordbox::sigTextChanged, m_pMap->getGameRules(), &GameRules::setPassword, Qt::QueuedConnection);
        addChild(pPasswordbox);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Cosmetic Mods: "));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setPosition(textWidth, y);
        pCheckbox->setChecked(false);
        pCheckbox->setTooltipText(tr("If checked cosmetic mods can be different between host and players.\nWarning this may lead to desynced games or crashes if one of the mods is not a purely cosmetic mod."));
        pCheckbox->setEnabled(m_ruleChangeEabled);
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setCosmeticModsAllowed, Qt::QueuedConnection);
        addChild(pCheckbox);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Observer: "));
        textField->setPosition(30, y);
        addChild(textField);
        spSpinBox pSpinbox = spSpinBox::create(400, 0, 40, SpinBox::Mode::Int);
        pSpinbox->setTooltipText(tr("The amount of observers allowed to watch the game. No players can join a fog/shroud of war match"));
        pSpinbox->setInfinityValue(-1.0);
        pSpinbox->setSpinSpeed(1.0f);
        pSpinbox->setPosition(textWidth, textField->getY());
        pSpinbox->setEnabled(m_ruleChangeEabled);
        addChild(pSpinbox);
        pSpinbox->setCurrentValue(m_pMap->getGameRules()->getMultiplayerObserver());
        connect(pSpinbox.get(), &SpinBox::sigValueChanged, m_pMap->getGameRules(), &GameRules::setMultiplayerObserver, Qt::QueuedConnection);
        y += 40;
    }

    textField = spLabel::create(800);
    style.color = headerColor;
    textField->setStyle(headerStyle);
    style.color = FontManager::getFontColor();
    textField->setHtmlText(tr("Environment"));
    textField->setPosition(30, y);
    addChild(textField);
    y += 60;


    QStringList weatherStrings;
    QVector<qint32> weatherChances;
    for (qint32 i = 0; i < m_pMap->getGameRules()->getWeatherCount(); i++)
    {
        Weather* pWeather = m_pMap->getGameRules()->getWeather(i);
        weatherStrings.append(pWeather->getWeatherName());
        weatherChances.append(m_pMap->getGameRules()->getWeatherChance(pWeather->getWeatherId()));
    }
    m_pWeatherSlider = spMultislider::create(weatherStrings, Settings::getWidth() - 100, weatherChances);
    m_pWeatherSlider->setTooltipText(tr("The chance each weather can appear if random weather is active."));
    m_pWeatherSlider->setPosition(30, y);
    m_pWeatherSlider->setEnabled(m_ruleChangeEabled);
    addChild(m_pWeatherSlider);
    connect(m_pWeatherSlider.get(), &Multislider::signalSliderChanged, this, &RuleSelection::weatherChancesChanged, Qt::QueuedConnection);

    y += m_pWeatherSlider->getHeight();
    textField = spLabel::create(textWidth - 40);
    textField->setStyle(style);
    textField->setHtmlText(tr("Random Weather: "));
    textField->setPosition(30, y);
    addChild(textField);
    pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("If checked random weather appears during the game."));
    pCheckbox->setPosition(textWidth, textField->getY());
    pCheckbox->setEnabled(m_ruleChangeEabled);
    addChild(pCheckbox);
    pCheckbox->setChecked(m_pMap->getGameRules()->getRandomWeather());
    connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setRandomWeather, Qt::QueuedConnection);
    y += 40;

    textField = spLabel::create(textWidth - 40);
    textField->setStyle(style);
    textField->setHtmlText(tr("Start Weather: "));
    textField->setPosition(30, pCheckbox->getY() + 40);
    addChild(textField);

    spDropDownmenu startWeather = spDropDownmenu::create(400, weatherStrings);
    startWeather->setTooltipText(tr("The weather at the start of the game."));
    startWeather->setPosition(textWidth, textField->getY());
    startWeather->setCurrentItem(m_pMap->getGameRules()->getStartWeather());
    startWeather->setEnabled(m_ruleChangeEabled);
    connect(startWeather.get(), &DropDownmenu::sigItemChanged, this, &RuleSelection::startWeatherChanged, Qt::QueuedConnection);
    addChild(startWeather);
    startWeatherChanged(m_pMap->getGameRules()->getStartWeather());
    y += 50;
    if (advanced)
    {
        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Weather Forecast: "));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked you can see the weather for the next few turns for each player."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getWeatherPrediction());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setWeatherPrediction, Qt::QueuedConnection);
    }
    y = textField->getY() + 50;
    textField = spLabel::create(800);
    style.color = headerColor;
    textField->setStyle(headerStyle);
    style.color = FontManager::getFontColor();
    textField->setHtmlText(tr("Gameplay"));
    textField->setPosition(30, y);
    addChild(textField);
    y += 60;

    textField = spLabel::create(textWidth - 40);
    textField->setStyle(style);
    textField->setHtmlText(tr("Unit Ranking: "));
    textField->setPosition(30, y);
    addChild(textField);
    pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("If checked units can gain ranks by killing other units."));
    pCheckbox->setPosition(textWidth, textField->getY());
    pCheckbox->setEnabled(m_ruleChangeEabled);
    addChild(pCheckbox);
    pCheckbox->setChecked(m_pMap->getGameRules()->getRankingSystem());
    connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setRankingSystem, Qt::QueuedConnection);
    y += 40;

    textField = spLabel::create(textWidth - 40);
    textField->setStyle(style);
    textField->setHtmlText(tr("No CO Powers: "));
    textField->setPosition(30, y);
    addChild(textField);
    pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("If checked CO's can't use CO-Powers."));
    pCheckbox->setPosition(textWidth, textField->getY());
    pCheckbox->setEnabled(m_ruleChangeEabled);
    addChild(pCheckbox);
    pCheckbox->setChecked(m_pMap->getGameRules()->getNoPower());
    connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setNoPower, Qt::QueuedConnection);
    y += 40;

    textField = spLabel::create(textWidth - 40);
    textField->setStyle(style);
    textField->setHtmlText(tr("Day to Day Abilities: "));
    textField->setPosition(30, y);
    addChild(textField);
    pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("If checked CO's have their day to day abilities, when unchecked CO's only have perks active. This has no impact on co powers."));
    pCheckbox->setPosition(textWidth, textField->getY());
    pCheckbox->setEnabled(m_ruleChangeEabled);
    addChild(pCheckbox);
    pCheckbox->setChecked(m_pMap->getGameRules()->getEnableDayToDayCoAbilities());
    connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setEnableDayToDayCoAbilities, Qt::QueuedConnection);
    y += 40;

    if (advanced)
    {

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Single CO's: "));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked players may only select a single CO."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getSingleCo());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setSingleCo, Qt::QueuedConnection);
        y += 40;
    }

    if (advanced)
    {
        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("CO Specific Units: "));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If unchecked specific CO-Units can't be produced."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getCoUnits());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setCoUnits, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("CO Perks: "));
        textField->setPosition(30, y);
        addChild(textField);
        spSpinBox pSpinbox = spSpinBox::create(400, 0, 900);
        pSpinbox->setTooltipText(tr("Selects the amount of CO Perks that can be assigned per CO."));
        pSpinbox->setPosition(textWidth, textField->getY());
        pSpinbox->setInfinityValue(-1);
        pSpinbox->setEnabled(m_ruleChangeEabled);
        addChild(pSpinbox);
        pSpinbox->setCurrentValue(m_pMap->getGameRules()->getMaxPerkCount());
        connect(pSpinbox.get(), &SpinBox::sigValueChanged, m_pMap->getGameRules(), &GameRules::setMaxPerkCount, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Unit Limit: "));
        textField->setPosition(30, y);
        addChild(textField);
        pSpinbox = spSpinBox::create(400, 0, 9999);
        pSpinbox->setTooltipText(tr("The maximum amount of units a single player can own at any time."));
        pSpinbox->setInfinityValue(0.0);
        pSpinbox->setPosition(textWidth, textField->getY());
        pSpinbox->setEnabled(m_ruleChangeEabled);
        addChild(pSpinbox);
        pSpinbox->setCurrentValue(m_pMap->getGameRules()->getUnitLimit());
        connect(pSpinbox.get(), &SpinBox::sigValueChanged, m_pMap->getGameRules(), &GameRules::setUnitLimit, Qt::QueuedConnection);
        y += 40;
    }

    if (advanced)
    {
        textField = spLabel::create(800);
        style.color = headerColor;
        textField->setStyle(headerStyle);
        style.color = FontManager::getFontColor();
        textField->setHtmlText(tr("CO Powergain"));
        textField->setPosition(30, y);
        addChild(textField);
        y += 60;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("CO Powergain zone: "));
        textField->setPosition(30, y);
        addChild(textField);
        QStringList zoneModes = {tr("Global"), tr("Global and CO Zone Bonus"), tr("Only CO Zone")};
        spDropDownmenu pDropDownmenu = spDropDownmenu::create(400, zoneModes);
        pDropDownmenu->setTooltipText(tr("Defines where a player can gain power for his co powermeter."));
        pDropDownmenu->setPosition(textWidth, textField->getY());
        pDropDownmenu->setEnabled(m_ruleChangeEabled);
        addChild(pDropDownmenu);
        pDropDownmenu->setCurrentItem(static_cast<qint32>(m_pMap->getGameRules()->getPowerGainZone()));
        GameMap* pPtrMap = m_pMap;
        connect(pDropDownmenu.get(), &DropDownmenu::sigItemChanged, this, [=](qint32 value)
        {
            pPtrMap->getGameRules()->setPowerGainZone(static_cast<GameEnums::PowerGainZone>(value));
        }, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("CO Powergain mode: "));
        textField->setPosition(30, y);
        addChild(textField);
        QStringList gainModes = {tr("Money"), tr("Money only attacker"), tr("HP"), tr("HP only attacker")};
        pDropDownmenu = spDropDownmenu::create(400, gainModes);
        pDropDownmenu->setTooltipText(tr("Defines how the powermeter fills."));
        pDropDownmenu->setPosition(textWidth, textField->getY());
        pDropDownmenu->setEnabled(m_ruleChangeEabled);
        addChild(pDropDownmenu);
        pDropDownmenu->setCurrentItem(static_cast<qint32>(m_pMap->getGameRules()->getPowerGainMode()));
        connect(pDropDownmenu.get(), &DropDownmenu::sigItemChanged, this, [=](qint32 value)
        {
            pPtrMap->getGameRules()->setPowerGainMode(static_cast<GameEnums::PowerGainMode>(value));
        }, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Powergain reduction: "));
        textField->setPosition(30, y);
        addChild(textField);
        spSpinBox pSpinbox = spSpinBox::create(400, 0, 100, SpinBox::Mode::Float);
        pSpinbox->setTooltipText(tr("The reduction in powergain for each time a co powers was used. Making each consecutive power more expensive."));
        pSpinbox->setInfinityValue(-1);
        pSpinbox->setSpinSpeed(0.1);
        pSpinbox->setPosition(textWidth, textField->getY());
        pSpinbox->setEnabled(m_ruleChangeEabled);
        addChild(pSpinbox);
        pSpinbox->setCurrentValue(m_pMap->getGameRules()->getPowerUsageReduction());
        connect(pSpinbox.get(), &SpinBox::sigValueChanged, m_pMap->getGameRules(), &GameRules::setPowerUsageReduction, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Powergain Speed: "));
        textField->setPosition(30, y);
        addChild(textField);
        pSpinbox = spSpinBox::create(400, 0, 200, SpinBox::Mode::Float);
        pSpinbox->setTooltipText(tr("The speed at which the power bar fills up. This affects gains from HP damage or Funds damage based on the mode."));
        pSpinbox->setInfinityValue(-1.0);
        pSpinbox->setSpinSpeed(0.1f);
        pSpinbox->setPosition(textWidth, textField->getY());
        pSpinbox->setEnabled(m_ruleChangeEabled);
        addChild(pSpinbox);
        pSpinbox->setCurrentValue(m_pMap->getGameRules()->getPowerGainSpeed());
        connect(pSpinbox.get(), &SpinBox::sigValueChanged, m_pMap->getGameRules(), &GameRules::setPowerGainSpeed, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("CO Power loss: "));
        textField->setPosition(30, y);
        addChild(textField);
        pSpinbox = spSpinBox::create(400, 0, 1, SpinBox::Mode::Float);
        pSpinbox->setTooltipText(tr("The percentage of the CO gauge lost when the CO unit gets destroyed."));
        pSpinbox->setInfinityValue(-1.0);
        pSpinbox->setSpinSpeed(0.1f);
        pSpinbox->setPosition(textWidth, textField->getY());
        pSpinbox->setEnabled(m_ruleChangeEabled);
        addChild(pSpinbox);
        pSpinbox->setCurrentValue(m_pMap->getGameRules()->getPowerLoose());
        connect(pSpinbox.get(), &SpinBox::sigValueChanged, m_pMap->getGameRules(), &GameRules::setPowerLoose, Qt::QueuedConnection);
        y += 40;
    }
    textField = spLabel::create(800);
    style.color = headerColor;
    textField->setStyle(headerStyle);
    style.color = FontManager::getFontColor();
    textField->setHtmlText(tr("Fog of War"));
    textField->setPosition(30, y);
    addChild(textField);
    y += 60;

    textField = spLabel::create(textWidth - 40);
    textField->setStyle(style);
    textField->setHtmlText(tr("Fog Of War: "));
    textField->setPosition(30, y);
    addChild(textField);
    QStringList fogModes = {tr("Off"), tr("Mist of War"), tr("Fog of War"), tr("Shroud of War")};
    spDropDownmenu fogOfWar = spDropDownmenu::create(400, fogModes);
    fogOfWar->setTooltipText(tr("Select the fog of war rule for the current game. \n In Mist of War all units can be seen but not targeted without Vision. \n In Fog of War all units cannot be seen without vision. \n In Shroud of War none of the map can be seen until explored, all units cannot be seen without vision."));
    fogOfWar->setPosition(textWidth, textField->getY());
    auto fogMode = m_pMap->getGameRules()->getFogMode();
    if (fogMode == GameEnums::Fog_OfMist)
    {
        fogOfWar->setCurrentItem(1);
    }
    else if (fogMode == GameEnums::Fog_Off)
    {
        fogOfWar->setCurrentItem(0);
    }
    else
    {
        fogOfWar->setCurrentItem(fogMode + 1);
    }
    fogOfWar->setEnabled(m_ruleChangeEabled);
    GameMap* pPtrMap = m_pMap;
    connect(fogOfWar.get(), &DropDownmenu::sigItemChanged, m_pMap->getGameRules(), [=](qint32 value)
    {
        if (value == 1)
        {
            pPtrMap->getGameRules()->setFogMode(GameEnums::Fog_OfMist);
        }
        else if (value > 1)
        {
            pPtrMap->getGameRules()->setFogMode(static_cast<GameEnums::Fog>(value - 1));
        }
        else
        {
            pPtrMap->getGameRules()->setFogMode(static_cast<GameEnums::Fog>(value));
        }
    });
    addChild(fogOfWar);
    y += 40;

    if (advanced)
    {
        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Vision Block: "));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked units can't see over certain terrains, reducing their vision range. Air units are unaffected by this effect."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getVisionBlock());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setVisionBlock, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Building Hiding:"));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked most buildings deny vision. This means you can hide a unit in a building similar to a forest."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getBuildingVisionHide());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setBuildingVisionHide, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Day Screen: "));
        textField->setPosition(30, y);
        addChild(textField);
        QStringList dayModes = {tr("Default"), tr("Permanent")};
        spDropDownmenu pDropDownmenu = spDropDownmenu::create(400, dayModes);
        pDropDownmenu->setTooltipText(tr("If set to 'Permanent' the New Day screen for Human players will remain on screen until dismissed. If set to 'Default' this feature will be off except in Fog of War games. This setting only takes effect in games with more than 1 Human players on the Host's PC."));
        pDropDownmenu->setPosition(textWidth, textField->getY());
        pDropDownmenu->setEnabled(m_ruleChangeEabled);
        addChild(pDropDownmenu);
        pDropDownmenu->setCurrentItem(static_cast<qint32>(m_pMap->getGameRules()->getDayToDayScreen()));
        connect(pDropDownmenu.get(), &DropDownmenu::sigItemChanged, this, [=](qint32 item)
        {
            pPtrMap->getGameRules()->setDayToDayScreen(static_cast<GameRules::DayToDayScreen>(item));
        });
        y += 50;
    }
    if (advanced)
    {
        textField = spLabel::create(800);
        style.color = headerColor;
        textField->setStyle(headerStyle);
        style.color = FontManager::getFontColor();
        textField->setHtmlText(tr("Advanced"));
        textField->setPosition(30, y);
        addChild(textField);
        y += 60;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("CO Banlist: "));
        textField->setPosition(30, y);
        addChild(textField);
        oxygine::spButton coBannlist = ObjectManager::createButton(tr("Edit"), 150);
        coBannlist->setPosition(textWidth, y - 2);
        coBannlist->setEnabled(m_ruleChangeEabled);
        coBannlist->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
        {
            emit sigShowCOBannlist();
        });
        addChild(coBannlist);
        connect(this, &RuleSelection::sigShowCOBannlist, this, &RuleSelection::showCOBannlist, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Perk Banlist: "));
        textField->setPosition(30, y);
        addChild(textField);
        oxygine::spButton perkBannlist = ObjectManager::createButton(tr("Edit"), 150);
        perkBannlist->setPosition(textWidth, y - 2);
        perkBannlist->setEnabled(m_ruleChangeEabled);
        perkBannlist->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
        {
            emit sigShowPerkBannlist();
        });
        addChild(perkBannlist);
        connect(this, &RuleSelection::sigShowPerkBannlist, this, &RuleSelection::showPerkBannlist, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Action Banlist: "));
        textField->setPosition(30, y);
        addChild(textField);
        oxygine::spButton actionBannlist = ObjectManager::createButton(tr("Edit"), 150);
        actionBannlist->setPosition(textWidth, y - 2);
        actionBannlist->setEnabled(m_ruleChangeEabled);
        actionBannlist->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
        {
            emit sigShowActionBannlist();
        });
        addChild(actionBannlist);
        connect(this, &RuleSelection::sigShowActionBannlist, this, &RuleSelection::showActionBannlist, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Terrain Defense: "));
        textField->setPosition(30, y);
        addChild(textField);
        spSpinBox pSpinbox = spSpinBox::create(400, 0, 40, SpinBox::Mode::Int);
        pSpinbox->setTooltipText(tr("The defense each terrain star grants a unit."));
        pSpinbox->setInfinityValue(-1.0);
        pSpinbox->setSpinSpeed(1.0f);
        pSpinbox->setPosition(textWidth, textField->getY());
        pSpinbox->setEnabled(m_ruleChangeEabled);
        addChild(pSpinbox);
        pSpinbox->setCurrentValue(m_pMap->getGameRules()->getTerrainDefense());
        connect(pSpinbox.get(), &SpinBox::sigValueChanged, m_pMap->getGameRules(), &GameRules::setTerrainDefense, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("HP Defense Impact:"));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked: the impact of terrain defense stars is reduced the less hp a unit has."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getHpDefenseReduction());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setHpDefenseReduction, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Ship bridges:"));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked: ships and boats can move through bridges placed on sea tiles, like in Advance Wars Dark conflict."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getShipBridges());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setShipBridges, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Transporter refresh: "));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked: units can move after getting unloaded if the unit had remained in their transporter for more than one turn."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getTransporterRefresh());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setTransporterRefresh, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Round Time: "));
        textField->setPosition(30, y);
        addChild(textField);
        spTimeSpinBox pTimeSpinbox = spTimeSpinBox::create(400);
        pTimeSpinbox->setTooltipText(tr("The maximum amount of time in hh:mm::ss for each turn for each player."));
        pTimeSpinbox->setPosition(textWidth, textField->getY());
        pTimeSpinbox->setEnabled(m_ruleChangeEabled);
        addChild(pTimeSpinbox);
        pTimeSpinbox->setCurrentValue(m_pMap->getGameRules()->getRoundTimeMs());
        connect(pTimeSpinbox.get(), &TimeSpinBox::sigValueChanged, m_pMap->getGameRules(), &GameRules::setRoundTimeMs, Qt::QueuedConnection);
        y += 40;

        // Label
        if (m_mode != RuleSelection::Mode::Editor && m_ruleChangeEabled)
        {
            textField = spLabel::create(textWidth - 10);
            textField->setStyle(style);
            textField->setHtmlText(tr("Script:"));
            textField->setPosition(30, y);
            addChild(textField);
            oxygine::spButton pScriptButton = pObjectManager->createButton(tr("Select File"), 160);
            pScriptButton->setPosition(Settings::getWidth() - pScriptButton->getWidth() - 100, y);
            addChild(pScriptButton);
            m_MapScriptFile = spTextbox::create(pScriptButton->getX() - textField->getX() - textWidth);
            m_MapScriptFile->setTooltipText(tr("The relative path from the exe to the script associated with this map."));
            m_MapScriptFile->setPosition(textWidth, textField->getY());
            m_MapScriptFile->setCurrentText(m_pMap->getGameScript()->getScriptFile());
            m_MapScriptFile->setEnabled(m_ruleChangeEabled);
            addChild(m_MapScriptFile);
            pScriptButton->setEnabled(m_ruleChangeEabled);
            pScriptButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
            {
                emit sigShowSelectScript();
            });
            connect(this, &RuleSelection::sigShowSelectScript, this, &RuleSelection::showSelectScript, Qt::QueuedConnection);
            y += 40;
        }
    }
    if (advanced)
    {
        textField = spLabel::create(800);
        style.color = headerColor;
        textField->setStyle(headerStyle);
        style.color = FontManager::getFontColor();
        textField->setHtmlText(tr("Miscellaneous"));
        textField->setPosition(30, y);
        addChild(textField);
        y += 60;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Unique CO's: "));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked: CO's that are randomly selected are unique. Note: If not enough CO's are available this may select no CO for a player"));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getSingleRandomCO());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setSingleRandomCO, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("AI Attack Terrain: "));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked the AI attacks pipe seams and walls etc."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getAiAttackTerrain());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setAiAttackTerrain, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Unit sell value: "));
        textField->setPosition(30, y);
        addChild(textField);
        spSpinBox pSpinbox = spSpinBox::create(400, 0, 1, SpinBox::Mode::Float);
        pSpinbox->setTooltipText(tr("The amount of funds you are refunded for selling a unit. Selling is only possible if the 'Sell' action is active"));
        pSpinbox->setInfinityValue(-1.0);
        pSpinbox->setSpinSpeed(0.1f);
        pSpinbox->setUnit("%");
        pSpinbox->setPosition(textWidth, textField->getY());
        pSpinbox->setEnabled(m_ruleChangeEabled);
        addChild(pSpinbox);
        pSpinbox->setCurrentValue(m_pMap->getGameRules()->getResellValue());
        connect(pSpinbox.get(), &SpinBox::sigValueChanged, m_pMap->getGameRules(), &GameRules::setResellValue, Qt::QueuedConnection);
        y += 40;

        textField = spLabel::create(textWidth - 40);
        textField->setStyle(style);
        textField->setHtmlText(tr("Team Facing :"));
        textField->setPosition(30, y);
        addChild(textField);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("If checked units of the same team face the same direction rather than based on player order."));
        pCheckbox->setPosition(textWidth, textField->getY());
        pCheckbox->setEnabled(m_ruleChangeEabled);
        addChild(pCheckbox);
        pCheckbox->setChecked(m_pMap->getGameRules()->getTeamFacingUnits());
        connect(pCheckbox.get(), &Checkbox::checkChanged, m_pMap->getGameRules(), &GameRules::setTeamFacingUnits, Qt::QueuedConnection);
        y += 50;
        addCustomGamerules(y);
    }

    textField = spLabel::create(800);
    style.color = headerColor;
    textField->setStyle(headerStyle);
    style.color = FontManager::getFontColor();
    textField->setHtmlText(tr("Victory Rules"));
    textField->setPosition(30, y);
    addChild(textField);
    y += 60;

    qint32 initCount = m_pMap->getGameRules()->getVictoryRuleSize();
    CONSOLE_PRINT("Creating ruleset number of initial rules " + QString::number(initCount), Console::eDEBUG);
    for (qint32 i = 0; i < pGameRuleManager->getVictoryRuleCount(); i++)
    {
        qint32 xPos = 0;
        QString ruleID = pGameRuleManager->getVictoryRuleID(i);
        spVictoryRule pRule = spVictoryRule(m_pMap->getGameRules()->getVictoryRule(ruleID));
        if (pRule.get() == nullptr && m_ruleChangeEabled)
        {
            CONSOLE_PRINT("Creating default ruleset for " + ruleID, Console::eDEBUG);
            pRule = spVictoryRule::create(ruleID, m_pMap);
            QStringList types = pRule->getRuleType();
            for (qint32 i2 = 0; i2 < types.size(); i2++)
            {
                if (types[0] == VictoryRule::checkbox && i2 == 0)
                {
                    if (initCount == 0)
                    {
                        pRule->setRuleValue(pRule->getDefaultValue(0), 0);
                    }
                    else
                    {
                        pRule->setRuleValue(0, 0);
                    }
                }
                else
                {
                    pRule->setRuleValue(pRule->getDefaultValue(i2), i2);
                }
            }
            m_pMap->getGameRules()->addVictoryRule(pRule);
        }
        if (pRule.get() != nullptr)
        {
            QStringList inputTypes = pRule->getRuleType();
            VictoryRule* pPtrRule = pRule.get();
            for (qint32 i2 = 0; i2 < inputTypes.size(); i2++)
            {
                QString inputType = inputTypes[i2];
                QString descriptiopn = pRule->getRuleDescription(i2);
                // add a cool check box and a cool text
                QString labelName = pRule->getRuleName(i2);
                textField = spLabel::create(textWidth - 40);
                textField->setStyle(style);
                textField->setHtmlText(labelName);
                textField->setPosition(xPos + 30, y);
                addChild(textField);
                if (inputType == VictoryRule::checkbox)
                {
                    bool defaultValue = pRule->getRuleValue(i2);
                    spCheckbox pCheckbox = spCheckbox::create();
                    pCheckbox->setTooltipText(descriptiopn);
                    pCheckbox->setPosition(xPos + textWidth, textField->getY());
                    pCheckbox->setEnabled(m_ruleChangeEabled);
                    addChild(pCheckbox);
                    pCheckbox->setChecked(defaultValue);
                    connect(pCheckbox.get(), &Checkbox::checkChanged, this, [=](bool value)
                    {
                        pPtrRule->setRuleValue(value, i2);
                    });
                }
                else if (inputType == VictoryRule::spinbox)
                {
                    qint32 defaultValue = pRule->getRuleValue(i2);
                    qint32 startValue = pRule->getInfiniteValue(i2);
                    spSpinBox pSpinbox = spSpinBox::create(200, startValue, 9999);
                    pSpinbox->setTooltipText(descriptiopn);
                    pSpinbox->setPosition(xPos + textWidth, textField->getY());
                    pSpinbox->setInfinityValue(startValue);
                    pSpinbox->setEnabled(m_ruleChangeEabled);
                    addChild(pSpinbox);
                    pSpinbox->setCurrentValue(defaultValue);
                    connect(pSpinbox.get(), &SpinBox::sigValueChanged, this, [=](float value)
                    {
                        qint32 newValue = static_cast<qint32>(value);
                        pPtrRule->setRuleValue(newValue, i2);
                    });
                }
                xPos += textWidth * 2 * 0.85;
                if (xPos + textWidth * 2 * 0.85 + 80 > getWidth())
                {
                    setWidth(xPos + textWidth * 2 * 0.85 + 80);
                }
            }
            y += 50;
        }
    }
    setHeight(y + 50);
    emit sigSizeChanged();
}

void RuleSelection::addCustomGamerules(qint32 & y)
{
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    
    GameRuleManager* pGameRuleManager = GameRuleManager::getInstance();
    qint32 initCount = m_pMap->getGameRules()->getGameRuleSize();
    for (qint32 i = 0; i < pGameRuleManager->getGameRuleCount(); i++)
    {
        qint32 xPos = 0;
        QString ruleID = pGameRuleManager->getGameRuleID(i);
        spGameRule pRule = spGameRule(m_pMap->getGameRules()->getGameRule(ruleID));
        if (pRule.get() == nullptr)
        {
            pRule = spGameRule::create(ruleID);
            QStringList types = pRule->getRuleType();
            for (qint32 i2 = 0; i2 < types.size(); i2++)
            {
                if (types[0] == VictoryRule::checkbox && i2 == 0)
                {
                    if (initCount == 0)
                    {
                        pRule->setRuleValue(pRule->getDefaultValue(0), 0);
                    }
                    else
                    {
                        pRule->setRuleValue(0, 0);
                    }
                }
                else
                {
                    pRule->setRuleValue(pRule->getDefaultValue(i2), i2);
                }
            }
        }
        QStringList inputTypes = pRule->getRuleType();
        m_pMap->getGameRules()->addGameRule(pRule);
        GameRule* pPtrRule = pRule.get();
        for (qint32 i2 = 0; i2 < inputTypes.size(); i2++)
        {
            QString inputType = inputTypes[i2];
            QString descriptiopn = pRule->getRuleDescription(i2);
            // add a cool check box and a cool text
            QString labelName = pRule->getRuleName(i2);
            spLabel textField = spLabel::create(textWidth - 40);
            textField->setStyle(style);
            textField->setHtmlText(labelName);
            textField->setPosition(xPos + 30, i * 50 + y);
            addChild(textField);
            if (inputType == GameRule::checkbox)
            {
                bool defaultValue = pRule->getRuleValue(i2);
                spCheckbox pCheckbox = spCheckbox::create();
                pCheckbox->setTooltipText(descriptiopn);
                pCheckbox->setPosition(xPos + textWidth, textField->getY());
                pCheckbox->setEnabled(m_ruleChangeEabled);
                addChild(pCheckbox);
                pCheckbox->setChecked(defaultValue);
                connect(pCheckbox.get(), &Checkbox::checkChanged, this, [=](bool value)
                {
                    pPtrRule->setRuleValue(value, i2);
                });
            }
            else if (inputType == GameRule::spinbox)
            {
                qint32 defaultValue = pRule->getRuleValue(i2);
                qint32 startValue = pRule->getInfiniteValue(i2);
                spSpinBox pSpinbox = spSpinBox::create(200, startValue, 9999);
                pSpinbox->setTooltipText(descriptiopn);
                pSpinbox->setPosition(xPos + textWidth, textField->getY());
                pSpinbox->setInfinityValue(startValue);
                pSpinbox->setEnabled(m_ruleChangeEabled);
                addChild(pSpinbox);
                pSpinbox->setCurrentValue(defaultValue);
                connect(pSpinbox.get(), &SpinBox::sigValueChanged, this, [=](float value)
                {
                    qint32 newValue = static_cast<qint32>(value);
                    pPtrRule->setRuleValue(newValue, i2);
                });
            }
            xPos += textWidth * 2 * 0.85;
            if (xPos + textWidth * 2 * 0.85 + 80 > getWidth())
            {
                setWidth(xPos + textWidth * 2 * 0.85 + 80);
            }
        }
    }
    y += pGameRuleManager->getGameRuleCount() * 50;
}

GameMap *RuleSelection::getMap() const
{
    return m_pMap;
}

RuleSelection::Mode RuleSelection::getMode() const
{
    return m_mode;
}

void RuleSelection::startWeatherChanged(qint32 value)
{
    m_pMap->getGameRules()->setStartWeather(value);
}

void RuleSelection::weatherChancesChanged()
{
    for (qint32 i = 0; i < m_pMap->getGameRules()->getWeatherCount(); i++)
    {
        m_pMap->getGameRules()->changeWeatherChance(i, m_pWeatherSlider->getSliderValue(i));
    }
}

void RuleSelection::showCOBannlist()
{
    
    
    spCOBannListDialog pBannlist = spCOBannListDialog::create(m_pMap->getGameRules()->getCOBannlist());
    oxygine::Stage::getStage()->addChild(pBannlist);
    connect(pBannlist.get(), &COBannListDialog::editFinished, m_pMap->getGameRules(), &GameRules::setCOBannlist, Qt::QueuedConnection);
    
}

void RuleSelection::showPerkBannlist()
{
    
    spPerkSelectionDialog pBannlist = spPerkSelectionDialog::create(m_pMap, nullptr, -1, true, QStringList());
    oxygine::Stage::getStage()->addChild(pBannlist);
    connect(pBannlist.get(), &PerkSelectionDialog::editFinished, m_pMap->getGameRules(), &GameRules::setAllowedPerks, Qt::QueuedConnection);
}

void RuleSelection::showActionBannlist()
{
    
    spActionListDialog pBannlist = spActionListDialog::create(m_pMap->getGameRules()->getAllowedActions(), m_pMap);
    oxygine::Stage::getStage()->addChild(pBannlist);
    connect(pBannlist.get(), &ActionListDialog::editFinished, m_pMap->getGameRules(), &GameRules::setAllowedActions, Qt::QueuedConnection);
    
}

void RuleSelection::showSelectScript()
{
    QStringList wildcards;
    wildcards.append("*.js");
    QString path = Settings::getUserPath() + "maps";
    spFileDialog fileDialog = spFileDialog::create(path, wildcards, "", false, tr("Load"));
    oxygine::Stage::getStage()->addChild(fileDialog);
    connect(fileDialog.get(),  &FileDialog::sigFileSelected, this, &RuleSelection::scriptFileChanged, Qt::QueuedConnection);
    
}

void RuleSelection::scriptFileChanged(QString file)
{
    file = GlobalUtils::makePathRelative(file);
    m_MapScriptFile->setCurrentText(file);
    
    m_pMap->getGameScript()->setScriptFile(file);
    m_pMap->getGameScript()->init();
    
}
