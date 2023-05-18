#include <QFile>

#include "3rd_party/oxygine-framework/oxygine/actor/Stage.h"

#include "menue/mainwindow.h"
#include "menue/campaignmenu.h"
#include "menue/gamemenue.h"
#include "menue/editormenue.h"
#include "menue/optionmenue.h"
#include "menue/mapselectionmapsmenue.h"
#include "menue/creditsmenue.h"
#include "menue/wikimenu.h"
#include "menue/costylemenu.h"
#include "menue/replaymenu.h"
#include "menue/achievementmenu.h"
#include "menue/shopmenu.h"
#include "menue/generatormenu.h"

#include "coreengine/mainapp.h"
#include "coreengine/gameconsole.h"
#include "coreengine/audiomanager.h"
#include "coreengine/globalutils.h"

#include "resource_management/backgroundmanager.h"
#include "resource_management/fontmanager.h"
#include "resource_management/objectmanager.h"

#include "game/campaign.h"

#include "multiplayer/lobbymenu.h"

#include "objects/dialogs/filedialog.h"
#include "objects/dialogs/dialogtextinput.h"
#include "objects/dialogs/folderdialog.h"
#include "objects/dialogs/dialogmessagebox.h"
#include "objects/loadingscreen.h"

#include "ui_reader/uifactory.h"

Mainwindow::Mainwindow(const QString & initialView)
    : m_cheatTimeout(this)
{
#ifdef GRAPHICSUPPORT
    setObjectName("Mainwindow");
#endif
    Mainapp* pApp = Mainapp::getInstance();
    pApp->pauseRendering();
    Interpreter::setCppOwnerShip(this);
    CONSOLE_PRINT("Entering Main Menue", GameConsole::eDEBUG);
    BackgroundManager* pBackgroundManager = BackgroundManager::getInstance();
    // load background
    oxygine::spSprite sprite = oxygine::spSprite::create();
    addChild(sprite);
    oxygine::ResAnim* pBackground = pBackgroundManager->getResAnim("mainmenu");
    if (pBackground != nullptr)
    {
        sprite->setResAnim(pBackground);
        // background should be last to draw
        sprite->setPriority(static_cast<qint32>(Mainapp::ZOrder::Background));
        sprite->setScaleX(static_cast<float>(oxygine::Stage::getStage()->getWidth()) / static_cast<float>(pBackground->getWidth()));
        sprite->setScaleY(static_cast<float>(oxygine::Stage::getStage()->getHeight()) / static_cast<float>(pBackground->getHeight()));
    }

    pApp->getAudioManager()->clearPlayList();
    pApp->getAudioManager()->loadFolder("resources/music/hauptmenue");
    pApp->getAudioManager()->playRandom();

    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValue obj = pInterpreter->newQObject(this);
    pInterpreter->setGlobal("currentMenu", obj);
    if (initialView.isEmpty())
    {
        UiFactory::getInstance().createUi("ui/mainmenu.xml", this);
    }
    else
    {
        UiFactory::getInstance().createUi(initialView, this);
    }

    if (Settings::getUsername().isEmpty())
    {
        CONSOLE_PRINT("Showing initial username selection", GameConsole::eDEBUG);
        spDialogTextInput pDialogTextInput = spDialogTextInput::create(tr("Select Username"), false, "");
        addChild(pDialogTextInput);
        connect(pDialogTextInput.get(), &DialogTextInput::sigTextChanged, this, &Mainwindow::changeUsername, Qt::QueuedConnection);
    }

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;
    style.color = Qt::black;
    spLabel pTextfield = spLabel::create(300);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(Mainapp::getGameVersion());
    pTextfield->setPosition(oxygine::Stage::getStage()->getWidth() - 10 - pTextfield->getTextRect().width(), oxygine::Stage::getStage()->getHeight() - 10 - pTextfield->getTextRect().height());
    pTextfield->addClickListener([this](oxygine::Event*)
    {
        emit sigVersionClicked();
    });
    connect(this, &Mainwindow::sigVersionClicked, this, &Mainwindow::versionClicked, Qt::QueuedConnection);
    addChild(pTextfield);

    if (!Settings::getSmallScreenDevice())
    {
        // import
        oxygine::spButton pImport = ObjectManager::createButton(tr("Import"), 170, tr("Imports all data from an other Commander Wars release to the current release."));
        addChild(pImport);
        pImport->setPosition(10, oxygine::Stage::getStage()->getHeight() - 10 - pImport->getScaledHeight());
        pImport->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
        {
            emit sigImport();
        });
        connect(this, &Mainwindow::sigImport, this, &Mainwindow::import, Qt::QueuedConnection);
    }

    m_cheatTimeout.setSingleShot(true);
    connect(&m_cheatTimeout, &QTimer::timeout, this, &Mainwindow::cheatTimeout, Qt::QueuedConnection);

    pApp->continueRendering();
}

void Mainwindow::import()
{
    QString path = Settings::getUserPath();
    spFolderDialog folderDialog = spFolderDialog::create(path);
    addChild(folderDialog);
    connect(folderDialog.get(), &FolderDialog::sigFolderSelected, this, &Mainwindow::importFromDirectory, Qt::QueuedConnection);
}

void Mainwindow::importFromDirectory(QString folder)
{
    spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
    pLoadingScreen->show();
    QStringList filter;
    filter << "*.*";
    pLoadingScreen->setProgress("Importing data", 0);
    GlobalUtils::importFilesFromDirectory(folder + "/data", "data", filter, false);
    pLoadingScreen->setProgress("Importing maps", 20);
    GlobalUtils::importFilesFromDirectory(folder + "/maps", "maps", filter, false);
    pLoadingScreen->setProgress("Importing mods", 40);
    QStringList modExcludes;
    modExcludes << "aw_styled_icons" << "aw_unloading" << "aw2_damage_formula" << "aw2_image"
                << "awdc_co" << "awdc_flare" << "awdc_powergain" << "awdc_terrain"
                << "awdc_unit" << "awdc_unit_image" << "awdc_weather" << "awds_co"
                << "awds_unit" << "awds_weather" << "cow_debuff_perks" << "cow_resell"
                << "cow_transfer" << "cow_triangle_weapons" << "map_creator" << "coop_mod";
    GlobalUtils::importFilesFromDirectory(folder + "/mods", "mods", filter, false, modExcludes);
    pLoadingScreen->setProgress("Importing custom terrain images", 60);
    GlobalUtils::importFilesFromDirectory(folder + "/customTerrainImages", "customTerrainImages", filter, false);
    pLoadingScreen->setProgress("Importing ini file", 80);
    filter.clear();
    filter << "Commander_Wars.ini";
    GlobalUtils::importFilesFromDirectory(folder + "/", "", filter, true);
    Settings::loadSettings();
    pLoadingScreen->setProgress("Importing userdata", 90);
    filter.clear();
    filter << "*.dat";
    GlobalUtils::importFilesFromDirectory(folder + "/", "", filter, false);
    Userdata::getInstance()->changeUser();
    pLoadingScreen->hide();
}

void Mainwindow::changeUsername(QString name)
{
    Settings::setUsername(name);
    Userdata::getInstance()->changeUser();
    Settings::saveSettings();
}

bool Mainwindow::isValidSavegame()
{
    QString lastSaveGame = Settings::getLastSaveGame();
    if (!QFile::exists(lastSaveGame) ||
        !lastSaveGame.endsWith(".sav"))
    {
        return false;
    }
    return true;
}

void Mainwindow::enterSingleplayer(const QStringList & filter)
{    
    Mainapp::getInstance()->pauseRendering();
    auto view = spMapSelectionView::create(filter);
    auto window = spMapSelectionMapsMenue::create(view);
    oxygine::Stage::getStage()->addChild(window);
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::enterMultiplayer()
{
    Mainapp::getInstance()->pauseRendering();
    oxygine::Stage::getStage()->addChild(spLobbyMenu::create());
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::enterEditor()
{
    Mainapp::getInstance()->pauseRendering();
    auto window = spEditorMenue::create();
    oxygine::Stage::getStage()->addChild(window);
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::enterOptionmenue(const QString & xmlFile)
{
    Mainapp::getInstance()->pauseRendering();
    auto window = spOptionMenue::create(xmlFile);
    oxygine::Stage::getStage()->addChild(window);
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::enterWikimenue()
{
    Mainapp::getInstance()->pauseRendering();
    auto window = spWikimenu::create();
    oxygine::Stage::getStage()->addChild(window);
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::enterCreditsmenue()
{
    Mainapp::getInstance()->pauseRendering();
    auto window = spCreditsMenue::create();
    oxygine::Stage::getStage()->addChild(window);
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::enterGeneratorMenu(const QString & generatorUi)
{
    Mainapp::getInstance()->pauseRendering();
    auto window = spGeneratorMenu::create(generatorUi);
    oxygine::Stage::getStage()->addChild(window);
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::enterAchievementmenue()
{    
    Mainapp::getInstance()->pauseRendering();
    auto window = spAchievementmenu::create();
    oxygine::Stage::getStage()->addChild(window);
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::enterShopMenu()
{
    Mainapp::getInstance()->pauseRendering();
    auto window = spShopmenu::create();
    oxygine::Stage::getStage()->addChild(window);
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::enterLoadGame()
{    
    QStringList wildcards;
    wildcards.append("*.sav");
    QString path = Settings::getUserPath() + "savegames";
    spFileDialog saveDialog = spFileDialog::create(path, wildcards, false, "", false, tr("Load"));
    addChild(saveDialog);
    connect(saveDialog.get(), &FileDialog::sigFileSelected, this, &Mainwindow::loadGame, Qt::QueuedConnection);
}

void Mainwindow::enterLoadCampaign()
{    
    QStringList wildcards;
    wildcards.append("*.camp");
    QString path = Settings::getUserPath() + "savegames";
    spFileDialog saveDialog = spFileDialog::create(path, wildcards, false, "", false, tr("Load"));
    addChild(saveDialog);
    connect(saveDialog.get(), &FileDialog::sigFileSelected, this, &Mainwindow::loadCampaign, Qt::QueuedConnection);
}

void Mainwindow::loadCampaign(QString filename)
{
    if (filename.endsWith(".camp"))
    {
        QFile file(filename);
        if (file.exists())
        {
            spCampaign pCampaign = spCampaign::create();
            QDataStream stream(&file);
            file.open(QIODevice::ReadOnly);
            pCampaign->deserializeObject(stream);
            spCampaignMenu pMenu = spCampaignMenu::create(pCampaign, false);
            oxygine::Stage::getStage()->addChild(pMenu);
            leaveMenue();
        }
        else
        {
            CONSOLE_PRINT("Mainwindow::loadCampaign not existing savefile was selected", GameConsole::eDEBUG);
        }
    }
    else
    {
        oxygine::handleErrorPolicy(oxygine::ep_show_error, "Mainwindow::loadCampaign illegal savefile was selected");
    }
}

void Mainwindow::enterReplayGame()
{
    Mainapp::getInstance()->pauseRendering();
    QStringList wildcards;
    wildcards.append("*.rec");
    QString path = Settings::getUserPath() + "data/records";
    spFileDialog saveDialog = spFileDialog::create(path, wildcards, false, "", false, tr("Load"));
    addChild(saveDialog);
    connect(saveDialog.get(), &FileDialog::sigFileSelected, this, &Mainwindow::replayGame, Qt::QueuedConnection);
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::lastSaveGame()
{
    loadGame(Settings::getLastSaveGame());
}

void Mainwindow::loadGame(QString filename)
{
    if (filename.endsWith(".sav"))
    {
        QFile file(filename);
        if (file.exists())
        {
            spGameMenue pMenu = spGameMenue::create(filename, true);
            oxygine::Stage::getStage()->addChild(pMenu);
            Mainapp* pApp = Mainapp::getInstance();
            pApp->getAudioManager()->clearPlayList();
            pMenu->startGame();
            leaveMenue();
        }
        else
        {
            oxygine::handleErrorPolicy(oxygine::ep_show_error, "Mainwindow::loadGame not existing savefile was selected");
        }
    }
    else
    {
        oxygine::handleErrorPolicy(oxygine::ep_show_error, "Mainwindow::loadGame illegal savefile was selected");
    }
}

void Mainwindow::replayGame(QString filename)
{
    if (filename.endsWith(".rec"))
    {
        QFile file(filename);
        if (file.exists())
        {
            CONSOLE_PRINT("Leaving Main Menue", GameConsole::eDEBUG);
            Mainapp* pApp = Mainapp::getInstance();
            pApp->getAudioManager()->clearPlayList();
            spReplayMenu pMenu = spReplayMenu::create(filename);
            oxygine::Stage::getStage()->addChild(pMenu);
            if (!pMenu->getValid())
            {
                emit pMenu->sigShowRecordInvalid();
            }
            leaveMenue();
        }
        else
        {
            oxygine::handleErrorPolicy(oxygine::ep_show_error, "Mainwindow::replayGame not existing record file was selected");
        }
    }
    else
    {
        oxygine::handleErrorPolicy(oxygine::ep_show_error, "Mainwindow::replayGame illegal record file was selected");
    }
}

void Mainwindow::leaveMenue()
{    
    CONSOLE_PRINT("Leaving Main Menue", GameConsole::eDEBUG);
    m_onEnterTimer.stop();
    oxygine::Actor::detach();
}

void Mainwindow::enterCOStyleMenu()
{
    Mainapp::getInstance()->pauseRendering();
    auto window = spCOStyleMenu::create();
    oxygine::Stage::getStage()->addChild(window);
    leaveMenue();
    Mainapp::getInstance()->continueRendering();
}

void Mainwindow::quitGame()
{
    Mainapp* pApp = Mainapp::getInstance();
    pApp->quitGame();
}

void Mainwindow::restartGame()
{
    emit Mainapp::getInstance()->sigQuit(1);
}

void Mainwindow::onEnter()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString object = "Init";
    QString func = "main";
    if (pInterpreter->exists(object, func))
    {
        CONSOLE_PRINT("Executing:" + object + "." + func, GameConsole::eDEBUG);
        QJSValueList args({pInterpreter->newQObject(this)});
        pInterpreter->doFunction(object, func, args);
    }
}

void Mainwindow::cheatTimeout()
{
    m_cheatCounter = 0;
}

void Mainwindow::versionClicked()
{
    CONSOLE_PRINT("Mainwindow::versionClicked", GameConsole::eDEBUG);
    ++m_cheatCounter;
    m_cheatTimeout.stop();
    if (m_cheatCounter >= 10)
    {
        m_cheatCounter = 0;
        spDialogMessageBox pDialogMessageBox = spDialogMessageBox::create(tr("Do you want to enable all current items in the shop?"), true);
        connect(pDialogMessageBox.get(), &DialogMessageBox::sigOk, this, &Mainwindow::unlockAllShopItems, Qt::QueuedConnection);
        addChild(pDialogMessageBox);
    }
    else
    {
        m_cheatTimeout.start(400);
    }
}

void Mainwindow::unlockAllShopItems()
{
    Userdata::getInstance()->unlockAllShopItems(true);
}
