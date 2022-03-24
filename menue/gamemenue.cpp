#include <QFile>
#include <QTime>
#include <qguiapplication.h>

#include "menue/gamemenue.h"
#include "menue/victorymenue.h"
#include "menue/optionmenue.h"

#include "coreengine/console.h"
#include "coreengine/audiothread.h"
#include "coreengine/globalutils.h"
#include "coreengine/settings.h"

#include "ai/proxyai.h"

#include "gameinput/humanplayerinput.h"
#include "game/player.h"
#include "game/co.h"
#include "game/gameanimation/gameanimationfactory.h"
#include "game/unitpathfindingsystem.h"

#include "resource_management/objectmanager.h"
#include "resource_management/fontmanager.h"
#include "resource_management/achievementmanager.h"

#include "objects/base/tableview.h"
#include "objects/base/moveinbutton.h"
#include "objects/dialogs/filedialog.h"
#include "objects/dialogs/ingame/coinfodialog.h"
#include "objects/dialogs/ingame/dialogvictoryconditions.h"
#include "objects/dialogs/dialogconnecting.h"
#include "objects/dialogs/dialogmessagebox.h"
#include "objects/dialogs/dialogtextinput.h"
#include "objects/dialogs/ingame/dialogattacklog.h"
#include "objects/dialogs/ingame/dialogunitinfo.h"
#include "objects/dialogs/rules/ruleselectiondialog.h"
#include "objects/gameplayandkeys.h"
#include "objects/unitstatisticview.h"

#include "ingamescriptsupport/genericbox.h"

#include "multiplayer/networkcommands.h"

#include "network/tcpserver.h"

#include "wiki/fieldinfo.h"
#include "wiki/wikiview.h"

#include "ingamescriptsupport/genericbox.h"

#include "ui_reader/uifactory.h"

#include "game/ui/damagecalculator.h"

GameMenue::GameMenue(spGameMap pMap, bool saveGame, spNetworkInterface pNetworkInterface)
    : InGameMenue(pMap),
      m_ReplayRecorder(m_pMap.get()),
      m_SaveGame(saveGame)
{
    setObjectName("GameMenue");
    CONSOLE_PRINT("Creating game menu singleton", Console::eDEBUG);
    Interpreter::setCppOwnerShip(this);
    loadHandling();
    m_pNetworkInterface = pNetworkInterface;
    loadGameMenue();
    loadUIButtons();
    if (m_pNetworkInterface.get() != nullptr)
    {
        
        for (qint32 i = 0; i < m_pMap->getPlayerCount(); i++)
        {
            Player* pPlayer = m_pMap->getPlayer(i);
            auto* baseGameInput = pPlayer->getBaseGameInput();
            if (baseGameInput != nullptr &&
                baseGameInput->getAiType() == GameEnums::AiTypes_ProxyAi)
            {
                dynamic_cast<ProxyAi*>(baseGameInput)->connectInterface(m_pNetworkInterface.get());
            }
        }
        connect(m_pNetworkInterface.get(), &NetworkInterface::sigDisconnected, this, &GameMenue::disconnected, Qt::QueuedConnection);
        connect(m_pNetworkInterface.get(), &NetworkInterface::recieveData, this, &GameMenue::recieveData, Qt::QueuedConnection);
        if (m_pNetworkInterface->getIsServer())
        {
            m_PlayerSockets = m_pNetworkInterface->getConnectedSockets();
            connect(m_pNetworkInterface.get(), &NetworkInterface::sigConnected, this, &GameMenue::playerJoined, Qt::QueuedConnection);
        }
        spDialogConnecting pDialogConnecting = spDialogConnecting::create(tr("Waiting for Players"), 1000 * 60 * 5);
        addChild(pDialogConnecting);
        connect(pDialogConnecting.get(), &DialogConnecting::sigCancel, this, &GameMenue::exitGame, Qt::QueuedConnection);
        if (pNetworkInterface->getIsObserver())
        {
            connect(this, &GameMenue::sigSyncFinished, pDialogConnecting.get(), &DialogConnecting::connected, Qt::QueuedConnection);
        }
        else
        {
            connect(this, &GameMenue::sigGameStarted, pDialogConnecting.get(), &DialogConnecting::connected, Qt::QueuedConnection);
            connect(this, &GameMenue::sigGameStarted, this, &GameMenue::startGame, Qt::QueuedConnection);
        }

        m_pChat = spChat::create(pNetworkInterface, QSize(Settings::getWidth(), Settings::getHeight() - 100), NetworkInterface::NetworkSerives::GameChat);
        m_pChat->setPriority(static_cast<qint32>(Mainapp::ZOrder::Dialogs));
        m_pChat->setVisible(false);
        addChild(m_pChat);
        emit m_pNetworkInterface->sigContinueListening();
    }
    else
    {
        startGame();
    }
    if (Settings::getAutoSavingCycle() > 0)
    {
        m_enabledAutosaving = true;
    }
    Mainapp* pApp = Mainapp::getInstance();
    pApp->continueRendering();
}

GameMenue::GameMenue(QString map, bool saveGame)
    : InGameMenue(-1, -1, map, saveGame),
      m_ReplayRecorder(m_pMap.get()),
      m_gameStarted(false),
      m_SaveGame(saveGame)

{
    setObjectName("GameMenue");
    Interpreter::setCppOwnerShip(this);
    loadHandling();
    loadGameMenue();
    loadUIButtons();
    if (Settings::getAutoSavingCycle() > 0)
    {
        m_enabledAutosaving = true;
    }
    Mainapp* pApp = Mainapp::getInstance();
    pApp->continueRendering();
}

GameMenue::GameMenue(spGameMap pMap)
    : InGameMenue(pMap),
      m_ReplayRecorder(m_pMap.get())
{
    setObjectName("GameMenue");
    CONSOLE_PRINT("Creating game menu singleton", Console::eDEBUG);
    Interpreter::setCppOwnerShip(this);
    Mainapp* pApp = Mainapp::getInstance();
    pApp->continueRendering();
}

spGameMenue GameMenue::getInstance()
{
    return oxygine::dynamic_pointer_cast<GameMenue>(m_pInstance);
}

void GameMenue::onEnter()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString object = "Init";
    QString func = "gameMenu";
    if (pInterpreter->exists(object, func))
    {
        CONSOLE_PRINT("Executing:" + object + "." + func, Console::eDEBUG);
        QJSValueList args({pInterpreter->newQObject(this)});
        pInterpreter->doFunction(object, func, args);
    }
    
    if (m_pMap.get() != nullptr &&
        m_pMap->getGameScript() != nullptr)
    {
        m_pMap->getGameScript()->onGameLoaded(this);
    }
}

void GameMenue::recieveData(quint64 socketID, QByteArray data, NetworkInterface::NetworkSerives service)
{
    if (service == NetworkInterface::NetworkSerives::Multiplayer)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);
        QString messageType;
        stream >> messageType;
        CONSOLE_PRINT("Local Network Command received: " + messageType + " for socket " + QString::number(socketID), Console::eDEBUG);
        if (messageType == NetworkCommands::CLIENTINITGAME)
        {
            if (m_pNetworkInterface->getIsServer())
            {
                // the given client is ready
                quint64 socket = 0;
                stream >> socket;
                CONSOLE_PRINT("socket game ready " + QString::number(socket), Console::eDEBUG);
                m_ReadySockets.append(socket);
                QVector<quint64> sockets;
                if (dynamic_cast<TCPServer*>(m_pNetworkInterface.get()))
                {
                    sockets = dynamic_cast<TCPServer*>(m_pNetworkInterface.get())->getConnectedSockets();
                }
                bool ready = true;
                for (qint32 i = 0; i < sockets.size(); i++)
                {
                    if (!m_ReadySockets.contains(sockets[i]))
                    {
                        ready = false;
                        CONSOLE_PRINT("Still waiting for socket game " + QString::number(sockets[i]), Console::eDEBUG);
                    }
                    else
                    {
                        CONSOLE_PRINT("Socket ready: " + QString::number(sockets[i]), Console::eDEBUG);
                    }
                }
                if (ready)
                {
                    CONSOLE_PRINT("All players are ready starting game", Console::eDEBUG);
                    QString command = QString(NetworkCommands::STARTGAME);
                    CONSOLE_PRINT("Sending command " + command, Console::eDEBUG);
                    QByteArray sendData;
                    QDataStream sendStream(&sendData, QIODevice::WriteOnly);
                    sendStream << command;
                    quint32 seed = QRandomGenerator::global()->bounded(std::numeric_limits<quint32>::max());
                    GlobalUtils::seed(seed);
                    GlobalUtils::setUseSeed(true);
                    sendStream << seed;
                    emit m_pNetworkInterface->sig_sendData(0, sendData, NetworkInterface::NetworkSerives::Multiplayer, false);
                    emit sigGameStarted();
                }
            }
        }
        else if (messageType == NetworkCommands::STARTGAME)
        {
            if (!m_pNetworkInterface->getIsServer())
            {
                quint32 seed = 0;
                stream >> seed;
                GlobalUtils::seed(seed);
                GlobalUtils::setUseSeed(true);
                emit sigGameStarted();
            }
        }
        else if (messageType == NetworkCommands::JOINASOBSERVER)
        {
            joinAsObserver(stream, socketID);
        }
        else if (messageType == NetworkCommands::JOINASPLAYER)
        {
            // todo receive join data and go from there
            emit m_pNetworkInterface.get()->sigDisconnectClient(socketID);
        }
        else if (messageType == NetworkCommands::WAITFORPLAYERJOINSYNCFINISHED)
        {
            waitForPlayerJoinSyncFinished(stream, socketID);
        }
        else if (messageType == NetworkCommands::WAITINGFORPLAYERJOINSYNCFINISHED)
        {
            waitingForPlayerJoinSyncFinished(stream, socketID);
        }
        else if (messageType == NetworkCommands::RECEIVEDCURRENTGAMESTATE)
        {
            removePlayerFromSyncWaitList(socketID);
        }
        else if (messageType == NetworkCommands::PLAYERJOINEDFINISHED)
        {
            playerJoinedFinished();
        }
    }    
    else if (service == NetworkInterface::NetworkSerives::GameChat)
    {
        if (m_pChat->getVisible() == false)
        {
            if (m_chatButtonShineTween.get())
            {
                m_chatButtonShineTween->removeFromActor();
            }
            m_chatButtonShineTween = oxygine::createTween(oxygine::VStyleActor::TweenAddColor(QColor(50, 50, 50, 0)), oxygine::timeMS(500), -1, true);
            m_ChatButton->addTween(m_chatButtonShineTween);
        }
    }
}

Player* GameMenue::getCurrentViewPlayer()
{
    
    spPlayer pCurrentPlayer = spPlayer(m_pMap->getCurrentPlayer());
    if (pCurrentPlayer.get() != nullptr)
    {
        qint32 currentPlayerID = pCurrentPlayer->getPlayerID();
        for (qint32 i = currentPlayerID; i >= 0; i--)
        {
            if (m_pMap->getPlayer(i)->getBaseGameInput() != nullptr &&
                m_pMap->getPlayer(i)->getBaseGameInput()->getAiType() == GameEnums::AiTypes_Human &&
                !m_pMap->getPlayer(i)->getIsDefeated())
            {
                return m_pMap->getPlayer(i);
            }
        }
        for (qint32 i = m_pMap->getPlayerCount() - 1; i > currentPlayerID; i--)
        {
            if (m_pMap->getPlayer(i)->getBaseGameInput() != nullptr &&
                m_pMap->getPlayer(i)->getBaseGameInput()->getAiType() == GameEnums::AiTypes_Human &&
                !m_pMap->getPlayer(i)->getIsDefeated())
            {
                return m_pMap->getPlayer(i);
            }
        }
        return pCurrentPlayer.get();
    }
    return nullptr;
}

void GameMenue::joinAsObserver(QDataStream & stream, quint64 socketID)
{
    if (m_pNetworkInterface->getIsServer())
    {
        auto* gameRules = m_pMap->getGameRules();
        auto & observer = gameRules->getObserverList();
        if (observer.size() < gameRules->getMultiplayerObserver())
        {
            observer.append(socketID);
            auto server = oxygine::dynamic_pointer_cast<TCPServer>(m_pNetworkInterface);
            if (server.get())
            {
                auto client = server->getClient(socketID);
                client->setIsObserver(true);
            }
            m_multiplayerSyncData.m_connectingSockets.append(socketID);
            for (qint32 i = 0; i < m_pMap->getPlayerCount(); ++i)
            {
                Player* pPlayer = m_pMap->getPlayer(i);
                BaseGameInputIF* pInput = pPlayer->getBaseGameInput();
                bool locked = pPlayer->getIsDefeated() ||
                              pInput == nullptr ||
                              pInput->getAiType() != GameEnums::AiTypes_ProxyAi;
                if (i < m_multiplayerSyncData.m_lockedPlayers.size())
                {
                    m_multiplayerSyncData.m_lockedPlayers[i] = locked;
                }
                else
                {
                    m_multiplayerSyncData.m_lockedPlayers.append(locked);
                }
            }
            QString command = NetworkCommands::WAITFORPLAYERJOINSYNCFINISHED;
            QByteArray sendData;
            QDataStream sendStream(&sendData, QIODevice::WriteOnly);
            sendStream << command;
            CONSOLE_PRINT("Sending command " + command, Console::eDEBUG);
            emit m_pNetworkInterface->sig_sendData(0, sendData, NetworkInterface::NetworkSerives::Multiplayer, true);
            m_multiplayerSyncData.m_waitingForSyncFinished = true;
        }
        else
        {
            emit m_pNetworkInterface.get()->sigDisconnectClient(socketID);
        }
    }
}

void GameMenue::waitForPlayerJoinSyncFinished(QDataStream & stream, quint64 socketID)
{
    if (!m_pNetworkInterface->getIsServer() &&
        !m_pNetworkInterface->getIsObserver())
    {
        QString command = NetworkCommands::WAITINGFORPLAYERJOINSYNCFINISHED;
        QByteArray sendData;
        QDataStream sendStream(&sendData, QIODevice::WriteOnly);
        sendStream << command;
        CONSOLE_PRINT("Sending command " + command, Console::eDEBUG);
        emit m_pNetworkInterface->sig_sendData(0, sendData, NetworkInterface::NetworkSerives::Multiplayer, false);
        m_multiplayerSyncData.m_waitingForSyncFinished = true;
    }
}

void GameMenue::waitingForPlayerJoinSyncFinished(QDataStream & stream, quint64 socketID)
{
    if (m_pNetworkInterface->getIsServer())
    {
        for (qint32 i = 0; i < m_pMap->getPlayerCount(); i++)
        {
            Player* pPlayer = m_pMap->getPlayer(i);
            quint64 playerSocketID = pPlayer->getSocketId();
             if (socketID == playerSocketID)
             {
                 m_multiplayerSyncData.m_lockedPlayers[i] = true;
             }
        }
        bool ready = true;
        for (qint32 i = 0; i < m_pMap->getPlayerCount(); i++)
        {
            if (!m_multiplayerSyncData.m_lockedPlayers[i])
            {
                ready = false;
            }
        }
        if (ready)
        {
            QString command = NetworkCommands::SENDCURRENTGAMESTATE;
            QByteArray sendData;
            QDataStream sendStream(&sendData, QIODevice::WriteOnly);
            sendStream << command;
            m_pMap->serializeObject(sendStream);
            CONSOLE_PRINT("Sending command " + command, Console::eDEBUG);
            for (auto & socketId : m_multiplayerSyncData.m_connectingSockets)
            {
                emit m_pNetworkInterface->sig_sendData(socketId, sendData, NetworkInterface::NetworkSerives::Multiplayer, false);
            }
        }
    }
}

void GameMenue::removePlayerFromSyncWaitList(quint64 socketID)
{
    if (m_pNetworkInterface->getIsServer())
    {
        m_multiplayerSyncData.m_connectingSockets.removeAll(socketID);
        continueAfterSyncGame();
    }
}

void GameMenue::playerJoinedFinished()
{
    emit sigSyncFinished();
    continueAfterSyncGame();
}

void GameMenue::continueAfterSyncGame()
{
    if (m_pNetworkInterface.get() != nullptr &&
        m_multiplayerSyncData.m_connectingSockets.size() <= 0 &&
        m_multiplayerSyncData.m_waitingForSyncFinished)
    {
        m_multiplayerSyncData.m_waitingForSyncFinished = false;
        if (m_pNetworkInterface->getIsServer())
        {
            QString command = NetworkCommands::PLAYERJOINEDFINISHED;
            QByteArray sendData;
            QDataStream sendStream(&sendData, QIODevice::WriteOnly);
            sendStream << command;
            CONSOLE_PRINT("Sending command " + command, Console::eDEBUG);
            emit m_pNetworkInterface->sig_sendData(0, sendData, NetworkInterface::NetworkSerives::Multiplayer, true);
        }
        if (m_multiplayerSyncData.m_postSyncAction.get() != nullptr)
        {
            performAction(m_multiplayerSyncData.m_postSyncAction);
            m_multiplayerSyncData.m_postSyncAction = nullptr;
        }
    }
}

void GameMenue::playerJoined(quint64 socketID)
{
    if (m_pNetworkInterface->getIsServer())
    {
        CONSOLE_PRINT("Player joined with socket: " + QString::number(socketID), Console::eDEBUG);
        QString command = QString(NetworkCommands::REQUESTJOINREASON);
        CONSOLE_PRINT("Sending command " + command, Console::eDEBUG);
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << command;
        emit m_pNetworkInterface->sig_sendData(socketID, data, NetworkInterface::NetworkSerives::Multiplayer, false);
    }
}

void GameMenue::disconnected(quint64 socketID)
{
    if (m_pNetworkInterface.get() != nullptr)
    {
        CONSOLE_PRINT("Handling player GameMenue::disconnect()", Console::eDEBUG);
        bool showDisconnect = !m_pNetworkInterface->getIsServer();
        m_multiplayerSyncData.m_connectingSockets.removeAll(socketID);
        auto & observer = m_pMap->getGameRules()->getObserverList();
        bool observerDisconnect = observer.contains(socketID);
        if (observerDisconnect)
        {
            observer.removeAll(socketID);
        }
        else
        {
            for (qint32 i = 0; i < m_pMap->getPlayerCount(); i++)
            {
                Player* pPlayer = m_pMap->getPlayer(i);
                quint64 playerSocketID = pPlayer->getSocketId();
                if (socketID == playerSocketID &&
                    !pPlayer->getIsDefeated())
                {
                    showDisconnect = true;
                    break;
                }
            }
            if (m_pNetworkInterface.get() != nullptr)
            {
                m_pNetworkInterface = nullptr;
            }
            if (showDisconnect)
            {
                m_gameStarted = false;
                spDialogMessageBox pDialogMessageBox = spDialogMessageBox::create(tr("A player has disconnected from the game! The game will now be stopped. You can save the game and reload the game to continue playing this map."));
                addChild(pDialogMessageBox);
            }
            if (Mainapp::getSlave())
            {
                CONSOLE_PRINT("Closing slave cause a player has disconnected.", Console::eDEBUG);
                QCoreApplication::exit(0);
            }
        }
        continueAfterSyncGame();
    }
}

bool GameMenue::isNetworkGame()
{
    if (m_pNetworkInterface.get() != nullptr)
    {
        return true;
    }
    return false;
}

void GameMenue::loadGameMenue()
{
    Mainapp* pApp = Mainapp::getInstance();
    moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);
    if (m_pNetworkInterface.get() != nullptr)
    {
        m_Multiplayer = true;
    }
    
    for (qint32 i = 0; i < m_pMap->getPlayerCount(); i++)
    {
        auto* input = m_pMap->getPlayer(i)->getBaseGameInput();
        if (input != nullptr)
        {
            input->init();
        }
    }
    // back to normal code
    m_pPlayerinfo = spPlayerInfo::create(m_pMap.get());
    m_pPlayerinfo->updateData();
    addChild(m_pPlayerinfo);

    m_IngameInfoBar = spIngameInfoBar::create(m_pMap.get());
    m_IngameInfoBar->updateMinimap();
    addChild(m_IngameInfoBar);
    if (Settings::getSmallScreenDevice())
    {
        m_IngameInfoBar->setX(Settings::getWidth() - 1);
        auto moveButton = spMoveInButton::create(m_IngameInfoBar.get(), m_IngameInfoBar->getScaledWidth());
        connect(moveButton.get(), &MoveInButton::sigMoved, this, &GameMenue::doPlayerInfoFlipping, Qt::QueuedConnection);
        m_IngameInfoBar->addChild(moveButton);
    }

    float scale = m_IngameInfoBar->getScaleX();
    m_autoScrollBorder = QRect(100, 140, m_IngameInfoBar->getScaledWidth(), 100);
    initSlidingActor(100, 165,
                     Settings::getWidth() - m_IngameInfoBar->getScaledWidth() - m_IngameInfoBar->getDetailedViewBox()->getScaledWidth() * scale - 150,
                     Settings::getHeight() - m_IngameInfoBar->getDetailedViewBox()->getScaledHeight() * scale - 175);
    m_mapSlidingActor->addChild(m_pMap);
    m_pMap->centerMap(m_pMap->getMapWidth() / 2, m_pMap->getMapHeight() / 2);

    m_delayedActionPerformedTimer.setSingleShot(true);
    connect(&m_delayedActionPerformedTimer, &QTimer::timeout, this, &GameMenue::delayedActionPerformed, Qt::QueuedConnection);

    connect(&m_UpdateTimer, &QTimer::timeout, this, &GameMenue::updateTimer, Qt::QueuedConnection);
    connectMap();
    connect(this, &GameMenue::sigExitGame, this, &GameMenue::exitGame, Qt::QueuedConnection);
    connect(this, &GameMenue::sigShowExitGame, this, &GameMenue::showExitGame, Qt::QueuedConnection);
    connect(this, &GameMenue::sigShowSurrenderGame, this, &GameMenue::showSurrenderGame, Qt::QueuedConnection);
    connect(this, &GameMenue::sigSaveGame, this, &GameMenue::saveGame, Qt::QueuedConnection);
    connect(this, &GameMenue::sigNicknameUnit, this, &GameMenue::nicknameUnit, Qt::QueuedConnection);

    connect(GameAnimationFactory::getInstance(), &GameAnimationFactory::animationsFinished, this, &GameMenue::actionPerformed, Qt::QueuedConnection);
    connect(m_Cursor.get(), &Cursor::sigCursorMoved, m_IngameInfoBar.get(), &IngameInfoBar::updateCursorInfo, Qt::QueuedConnection);
    connect(m_Cursor.get(), &Cursor::sigCursorMoved, this, &GameMenue::cursorMoved, Qt::QueuedConnection);

    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValue obj = pInterpreter->newQObject(this);
    pInterpreter->setGlobal("currentMenu", obj);
    UiFactory::getInstance().createUi("ui/gamemenu.xml", this);
}

void GameMenue::connectMap()
{    
    connect(m_pMap->getGameRules(), &GameRules::sigVictory, this, &GameMenue::victory, Qt::QueuedConnection);
    connect(m_pMap->getGameRules()->getRoundTimer(), &Timer::timeout, this, &GameMenue::nextTurnPlayerTimeout, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::signalExitGame, this, &GameMenue::showExitGame, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigSurrenderGame, this, &GameMenue::showSurrenderGame, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::signalSaveGame, this, &GameMenue::saveGame, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowGameInfo, this, &GameMenue::showGameInfo, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigVictoryInfo, this, &GameMenue::victoryInfo, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::signalShowCOInfo, this, &GameMenue::showCOInfo, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowAttackLog, this, &GameMenue::showAttackLog, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowUnitInfo, this, &GameMenue::showUnitInfo, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigQueueAction, this, &GameMenue::performAction, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowNicknameUnit, this, &GameMenue::showNicknameUnit, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowOptions, this, &GameMenue::showOptions, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowChangeSound, this, &GameMenue::showChangeSound, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowWiki, this, &GameMenue::showWiki, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowRules, this, &GameMenue::showRules, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowUnitStatistics, this, &GameMenue::showUnitStatistics, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigShowDamageCalculator, this, &GameMenue::showDamageCalculator, Qt::QueuedConnection);
    connect(m_pMap.get(), &GameMap::sigMovedMap, m_IngameInfoBar.get(), &IngameInfoBar::syncMinimapPosition, Qt::QueuedConnection);
    connect(m_IngameInfoBar->getMinimap(), &Minimap::clicked, m_pMap.get(), &GameMap::centerMap, Qt::QueuedConnection);
}

void GameMenue::loadUIButtons()
{    
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("panel");
    oxygine::spBox9Sprite pButtonBox = oxygine::spBox9Sprite::create();
    pButtonBox->setResAnim(pAnim);
    qint32 roundTime = m_pMap->getGameRules()->getRoundTimeMs();
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_TOP;
    style.hAlign = oxygine::TextStyle::HALIGN_MIDDLE;
    style.multiline = false;
    m_CurrentRoundTime = oxygine::spTextField::create();
    m_CurrentRoundTime->setSize(120, 30);
    m_CurrentRoundTime->setStyle(style);
    if (roundTime > 0)
    {
        pButtonBox->setSize(286 + 120, 50);
        m_CurrentRoundTime->setPosition(138 + 5, 10);
        pButtonBox->addChild(m_CurrentRoundTime);
        updateTimer();
    }
    else
    {
        pButtonBox->setSize(286, 50);
    }
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;

    pButtonBox->setPosition((Settings::getWidth() - m_IngameInfoBar->getScaledWidth()) / 2 - pButtonBox->getWidth() / 2, Settings::getHeight() - pButtonBox->getHeight() + 6);
    pButtonBox->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    addChild(pButtonBox);
    oxygine::spButton saveGame = pObjectManager->createButton(tr("Save"), 130);
    saveGame->setPosition(8, 4);
    saveGame->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigSaveGame();
    });
    pButtonBox->addChild(saveGame);

    oxygine::spButton exitGame = pObjectManager->createButton(tr("Exit"), 130);
    exitGame->setPosition(pButtonBox->getWidth() - 138, 4);
    exitGame->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigShowExitGame();
    });
    pButtonBox->addChild(exitGame);

    pAnim = pObjectManager->getResAnim("panel");
    pButtonBox = oxygine::spBox9Sprite::create();
    pButtonBox->setResAnim(pAnim);
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_TOP;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;
    m_xyTextInfo = spLabel::create(180);
    m_xyTextInfo->setStyle(style);
    m_xyTextInfo->setHtmlText("X: 0 Y: 0");
    m_xyTextInfo->setPosition(8, 8);
    pButtonBox->addChild(m_xyTextInfo);
    pButtonBox->setSize(200, 50);
    pButtonBox->setPosition((Settings::getWidth() - m_IngameInfoBar->getScaledWidth())  - pButtonBox->getWidth(), 0);
    m_XYButtonBox = pButtonBox;
    m_XYButtonBox->setVisible(Settings::getShowIngameCoordinates() && !Settings::getSmallScreenDevice());
    addChild(pButtonBox);
    m_UpdateTimer.setInterval(500);
    m_UpdateTimer.setSingleShot(false);
    m_UpdateTimer.start();
    bool loadQuickButtons = true;
    if (m_pNetworkInterface.get() != nullptr)
    {
        pButtonBox = oxygine::spBox9Sprite::create();
        pButtonBox->setResAnim(pAnim);
        pButtonBox->setSize(144, 50);
        pButtonBox->setPosition(0, Settings::getHeight() - pButtonBox->getHeight());
        pButtonBox->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
        addChild(pButtonBox);
        m_ChatButton = pObjectManager->createButton(tr("Show Chat"), 130);
        m_ChatButton->setPosition(8, 4);
        m_ChatButton->addClickListener([this](oxygine::Event*)
        {
            showChat();
        });
        pButtonBox->addChild(m_ChatButton);
        loadQuickButtons = !m_pNetworkInterface->getIsObserver();
    }
    if (loadQuickButtons)
    {
        m_humanQuickButtons = spHumanQuickButtons::create(this);
        m_humanQuickButtons->setEnabled(false);
        addChild(m_humanQuickButtons);
    }
}

void GameMenue::showChat()
{
    m_pChat->setVisible(!m_pChat->getVisible());
    setFocused(!m_pChat->getVisible());
    m_pChat->removeTweens();
    if (m_chatButtonShineTween.get())
    {
        m_chatButtonShineTween->removeFromActor();
    }
    m_ChatButton->setAddColor(0, 0, 0, 0);
}

void GameMenue::updateTimer()
{
    if (m_pMap.get() != nullptr)
    {
        QTimer* pTimer = m_pMap->getGameRules()->getRoundTimer();
        qint32 roundTime = pTimer->remainingTime();
        if (!pTimer->isActive())
        {
            roundTime = pTimer->interval();
        }
        if (roundTime < 0)
        {
            roundTime = 0;
        }
        m_CurrentRoundTime->setHtmlText(QTime::fromMSecsSinceStartOfDay(roundTime).toString("hh:mm:ss"));
    }
}

bool GameMenue::getGameStarted() const
{
    return m_gameStarted;
}

void GameMenue::editFinishedCanceled()
{
    setFocused(true);
}

spGameAction GameMenue::doMultiTurnMovement(spGameAction pGameAction)
{
    if (m_gameStarted &&
        pGameAction.get() != nullptr &&
        (pGameAction->getActionID() == CoreAI::ACTION_NEXT_PLAYER ||
         pGameAction->getActionID() == CoreAI::ACTION_SWAP_COS))
    {
        CONSOLE_PRINT("Check and update multiTurnMovement", Console::eDEBUG);
        
        // check for units that have a multi turn available
        qint32 heigth = m_pMap->getMapHeight();
        qint32 width = m_pMap->getMapWidth();
        Player* pPlayer = m_pMap->getCurrentPlayer();
        for (qint32 y = 0; y < heigth; y++)
        {
            for (qint32 x = 0; x < width; x++)
            {
                Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
                if (pUnit != nullptr)
                {
                    if ((pUnit->getOwner() == pPlayer) &&
                        (pUnit->getHasMoved() == false))
                    {
                        QVector<QPoint> currentMultiTurnPath = pUnit->getMultiTurnPath();
                        if (currentMultiTurnPath.size() > 0)
                        {
                            // replace current action with auto moving none moved units
                            m_pStoredAction = pGameAction;
                            spGameAction multiTurnMovement = spGameAction::create(CoreAI::ACTION_WAIT, m_pMap.get());
                            if (pUnit->getActionList().contains(CoreAI::ACTION_HOELLIUM_WAIT))
                            {
                                multiTurnMovement->setActionID(CoreAI::ACTION_HOELLIUM_WAIT);
                            }
                            multiTurnMovement->setTarget(pUnit->getPosition());
                            UnitPathFindingSystem pfs(m_pMap.get(), pUnit, pPlayer);
                            pfs.setMovepoints(pUnit->getFuel());
                            pfs.explore();
                            qint32 movepoints = pUnit->getMovementpoints(multiTurnMovement->getTarget());
                            // shorten path
                            auto newPath = pfs.getClosestReachableMovePath(currentMultiTurnPath, movepoints);
                            multiTurnMovement->setMovepath(newPath, pfs.getCosts(newPath));
                            QVector<QPoint> multiTurnPath;
                            // still some path ahead?
                            if (newPath.size() == 0)
                            {
                                multiTurnPath = currentMultiTurnPath;
                            }
                            else if (currentMultiTurnPath.size() > newPath.size())
                            {
                                for (qint32 i = 0; i <= currentMultiTurnPath.size() - newPath.size(); i++)
                                {
                                    multiTurnPath.append(currentMultiTurnPath[i]);
                                }
                            }
                            multiTurnMovement->setMultiTurnPath(multiTurnPath);
                            return multiTurnMovement;
                        }
                        else if (pUnit->getActionList().contains(CoreAI::ACTION_CAPTURE))
                        {
                            spGameAction multiTurnMovement = spGameAction::create(CoreAI::ACTION_CAPTURE, m_pMap.get());
                            multiTurnMovement->setTarget(pUnit->getPosition());
                            if (multiTurnMovement->canBePerformed())
                            {
                                m_pStoredAction = pGameAction;
                                return multiTurnMovement;
                            }
                        }
                    }
                }
            }
        }
    }
    return pGameAction;
}

bool GameMenue::getIsMultiplayer(const spGameAction & pGameAction) const
{
    return !pGameAction->getIsLocal() &&
            m_pNetworkInterface.get() != nullptr &&
            m_gameStarted;
}

bool GameMenue::requiresForwarding(const spGameAction & pGameAction) const
{
    Player* pCurrentPlayer = m_pMap->getCurrentPlayer();
    auto* baseGameInput = pCurrentPlayer->getBaseGameInput();
    return getIsMultiplayer(pGameAction) &&
           baseGameInput != nullptr &&
           baseGameInput->getAiType() != GameEnums::AiTypes_ProxyAi;
}

void GameMenue::nextTurnPlayerTimeout()
{
    auto* input = m_pMap->getCurrentPlayer()->getBaseGameInput();
    if (input != nullptr)
    {
        if (input->getAiType() == GameEnums::AiTypes_Human)

        {
            if (m_pCurrentAction.get() == nullptr)
            {
                spGameAction pAction = spGameAction::create(CoreAI::ACTION_NEXT_PLAYER, m_pMap.get());
                performAction(pAction);
            }
            else
            {
                m_noTimeOut = true;
            }
        }
        else if (input->getAiType() != GameEnums::AiTypes_Human &&
                 input->getAiType() != GameEnums::AiTypes_ProxyAi &&
                 input->getAiType() != GameEnums::AiTypes_Closed)
        {
            m_noTimeOut = true;
        }
    }
}

void GameMenue::performAction(spGameAction pGameAction)
{
    if (m_pCurrentAction.get() != nullptr)
    {
        CONSOLE_PRINT("Ignoring action request cause an action is currently performed", Console::eWARNING);
        return;
    }
    m_saveAllowed = false;
    if (m_multiplayerSyncData.m_waitingForSyncFinished)
    {
        m_multiplayerSyncData.m_postSyncAction = pGameAction;
        spDialogConnecting pDialogConnecting = spDialogConnecting::create(tr("Waiting for Players/Observers to join..."), 1000 * 60 * 5);
        addChild(pDialogConnecting);
        connect(pDialogConnecting.get(), &DialogConnecting::sigCancel, this, &GameMenue::exitGame, Qt::QueuedConnection);
        connect(this, &GameMenue::sigSyncFinished, pDialogConnecting.get(), &DialogConnecting::connected, Qt::QueuedConnection);
    }
    else if (pGameAction.get() != nullptr)
    {
        CONSOLE_PRINT("GameMenue::performAction " + pGameAction->getActionID() + " at X: " + QString::number(pGameAction->getTarget().x())
                       + " at Y: " + QString::number(pGameAction->getTarget().y()), Console::eDEBUG);
        
        Mainapp::getInstance()->pauseRendering();
        bool multiplayer = getIsMultiplayer(pGameAction);
        Player* pCurrentPlayer = m_pMap->getCurrentPlayer();
        auto* baseGameInput = pCurrentPlayer->getBaseGameInput();
        if (multiplayer &&
            baseGameInput != nullptr &&
            baseGameInput->getAiType() == GameEnums::AiTypes_ProxyAi &&
            m_syncCounter + 1 != pGameAction->getSyncCounter())
        {
            m_gameStarted = false;
            spDialogMessageBox pDialogMessageBox = spDialogMessageBox::create(tr("The game is out of sync and can't be continued. The game has been stopped. You can save the game and restart."));
            addChild(pDialogMessageBox);
        }
        else
        {
            if (multiplayer &&
                baseGameInput != nullptr &&
                baseGameInput->getAiType() == GameEnums::AiTypes_ProxyAi)
            {
                m_syncCounter = pGameAction->getSyncCounter();
            }
            m_pStoredAction = nullptr;
            m_pMap->getGameRules()->pauseRoundTime();
            if (!pGameAction->getIsLocal() &&
                (baseGameInput != nullptr &&
                 baseGameInput->getAiType() != GameEnums::AiTypes_ProxyAi))
            {
                pGameAction = doMultiTurnMovement(pGameAction);
            }
            Unit * pMoveUnit = pGameAction->getTargetUnit();
            doTrapping(pGameAction);
            // send action to other players if needed
            if (requiresForwarding(pGameAction))
            {
                CONSOLE_PRINT("Sending action to other players", Console::eDEBUG);
                m_syncCounter++;
                pGameAction->setSyncCounter(m_syncCounter);
                pGameAction->setRoundTimerTime(m_pMap->getGameRules()->getRoundTimer()->remainingTime());
                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);
                stream << m_pMap->getCurrentPlayer()->getPlayerID();
                pGameAction->serializeObject(stream);
                emit m_pNetworkInterface->sig_sendData(0, data, NetworkInterface::NetworkSerives::Game, true);
            }
            else if (multiplayer)
            {
                m_pMap->getGameRules()->getRoundTimer()->setInterval(pGameAction->getRoundTimerTime());
            }
            // record action if required
            m_ReplayRecorder.recordAction(pGameAction);
            // perform action
            GlobalUtils::seed(pGameAction->getSeed());
            GlobalUtils::setUseSeed(true);
            if (pMoveUnit != nullptr)
            {
                pMoveUnit->setMultiTurnPath(pGameAction->getMultiTurnPath());
            }

            if (baseGameInput != nullptr)
            {
                baseGameInput->centerCameraOnAction(pGameAction.get());
            }
            pGameAction->perform();
            // clean up the action
            m_pCurrentAction = pGameAction;
            pGameAction = nullptr;
            skipAnimations(false);
            if (!m_pMap->anyPlayerAlive())
            {
                CONSOLE_PRINT("Forcing exiting the game cause no player is alive", Console::eDEBUG);
                emit sigExitGame();
            }
            else if (m_pMap->getCurrentPlayer()->getIsDefeated())
            {
                CONSOLE_PRINT("Triggering next player cause current player is defeated", Console::eDEBUG);
                spGameAction pAction = spGameAction::create(m_pMap.get());
                pAction->setActionID(CoreAI::ACTION_NEXT_PLAYER);
                performAction(pAction);
            }
        }
        if (pCurrentPlayer != m_pMap->getCurrentPlayer())
        {
            auto* baseGameInput = m_pMap->getCurrentPlayer()->getBaseGameInput();
            if (baseGameInput != nullptr &&
                baseGameInput->getAiType() == GameEnums::AiTypes_Human)
            {
                autoSaveMap();
            }
        }
        Mainapp::getInstance()->continueRendering();
    }
}

void GameMenue::doTrapping(spGameAction & pGameAction)
{
    CONSOLE_PRINT("GameMenue::doTrapping", Console::eDEBUG);
    QVector<QPoint> path = pGameAction->getMovePath();
    
    Unit * pMoveUnit = pGameAction->getTargetUnit();
    if (path.size() > 1 && pMoveUnit != nullptr)
    {
        if (pGameAction->getRequiresEmptyField())
        {
            QVector<QPoint> trapPathNotEmptyTarget = path;
            qint32 trapPathCostNotEmptyTarget = pGameAction->getCosts();
            QPoint trapPoint = path[0];
            for (qint32 i = 0; i < path.size() - 1; i++)
            {
                QPoint point = path[i];
                QPoint prevPoint = path[i + 1];
                Unit* pUnit = m_pMap->getTerrain(point.x(), point.y())->getUnit();
                if (pUnit == nullptr || pMoveUnit->getOwner()->isAlly(pUnit->getOwner()))
                {
                    if (i > 0)
                    {
                        spGameAction pTrapAction = spGameAction::create(CoreAI::ACTION_TRAP, m_pMap.get());
                        pTrapAction->setMovepath(trapPathNotEmptyTarget, trapPathCostNotEmptyTarget);
                        pTrapAction->writeDataInt32(trapPoint.x());
                        pTrapAction->writeDataInt32(trapPoint.y());
                        pTrapAction->setTarget(pGameAction->getTarget());
                        pGameAction = pTrapAction;
                    }
                    break;
                }
                else
                {
                    trapPoint = point;
                    qint32 moveCost = pMoveUnit->getMovementCosts(point.x(), point.y(),
                                                                  prevPoint.x(), prevPoint.y(), true);
                    trapPathCostNotEmptyTarget -= moveCost;
                    trapPathNotEmptyTarget.removeFirst();
                }
            }
        }
        path = pGameAction->getMovePath();
        QVector<QPoint> trapPath;
        qint32 trapPathCost = 0;
        for (qint32 i = path.size() - 2; i >= 0; i--)
        {
            // check the movepath for a trap
            QPoint point = path[i];
            QPoint prevPoint = path[i];
            if (i > 0)
            {
                prevPoint = path[i - 1];
            }
            qint32 moveCost = pMoveUnit->getMovementCosts(point.x(), point.y(),
                                                          prevPoint.x(), prevPoint.y(), true);
            if (isTrap("isTrap", pGameAction, pMoveUnit, point, prevPoint, moveCost))
            {
                while (trapPath.size() > 1)
                {
                    QPoint currentPoint = trapPath[0];
                    QPoint previousPoint = trapPath[1];
                    moveCost = pMoveUnit->getMovementCosts(currentPoint.x(), currentPoint.y(),
                                                           previousPoint.x(), previousPoint.y());
                    if (isTrap("isStillATrap", pGameAction, pMoveUnit, currentPoint, previousPoint, moveCost))
                    {
                        trapPathCost -= moveCost;
                        trapPath.pop_front();
                        if (m_pMap->getTerrain(point.x(), point.y())->getUnit() != nullptr)
                        {
                            point = currentPoint;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                spGameAction pTrapAction = spGameAction::create(CoreAI::ACTION_TRAP, m_pMap.get());
                pTrapAction->setMovepath(trapPath, trapPathCost);
                pMoveUnit->getOwner()->addVisionField(point.x(), point.y(), 1, true);
                pTrapAction->writeDataInt32(point.x());
                pTrapAction->writeDataInt32(point.y());
                pTrapAction->setTarget(pGameAction->getTarget());
                pGameAction = pTrapAction;
                break;
            }
            else
            {
                trapPath.push_front(point);
                qint32 x = pMoveUnit->Unit::getX();
                qint32 y = pMoveUnit->Unit::getY();
                if (point.x() != x ||
                    point.y() != y)
                {
                    QPoint previousPoint = path[i + 1];
                    trapPathCost += pMoveUnit->getMovementCosts(point.x(), point.y(), previousPoint.x(), previousPoint.y());
                }
            }
        }
    }
}

bool GameMenue::isTrap(const QString & function, spGameAction pAction, Unit* pMoveUnit, QPoint currentPoint, QPoint previousPoint, qint32 moveCost)
{
    
    Unit* pUnit = m_pMap->getTerrain(currentPoint.x(), currentPoint.y())->getUnit();

    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValueList args({pInterpreter->newQObject(pAction.get()),
                       pInterpreter->newQObject(pMoveUnit),
                       pInterpreter->newQObject(pUnit),
                       currentPoint.x(),
                       currentPoint.y(),
                       previousPoint.x(),
                       previousPoint.y(),
                       moveCost,
                       pInterpreter->newQObject(m_pMap.get()),
                      });
    QJSValue erg = pInterpreter->doFunction("ACTION_TRAP", function, args);
    if (erg.isBool())
    {
        return erg.toBool();
    }
    return false;
}

void GameMenue::centerMapOnAction(GameAction* pGameAction)
{    
    CONSOLE_PRINT("centerMapOnAction()", Console::eDEBUG);
    Unit* pUnit = pGameAction->getTargetUnit();
    Player* pPlayer = getCurrentViewPlayer();
    
    QPoint target = pGameAction->getTarget();
    if (pUnit != nullptr &&
        !pUnit->isStealthed(pPlayer))
    {
        const auto & path = pGameAction->getMovePath();
        for (const auto & point : path)
        {
            if (pPlayer->getFieldVisible(point.x(), point.y()))
            {
                target = point;
                break;
            }
        }
    }

    if (m_pMap->onMap(target.x(), target.y()) &&
        pPlayer->getFieldVisible(target.x(), target.y()) &&
        (pUnit == nullptr ||
         !pUnit->isStealthed(pPlayer)))
    {
        m_pMap->centerMap(target.x(), target.y());
    }
    
}

void GameMenue::skipAnimations(bool postAnimation)
{
    CONSOLE_PRINT("skipping Animations", Console::eDEBUG);
    Mainapp::getInstance()->pauseRendering();
    if (GameAnimationFactory::getAnimationCount() > 0)
    {
        GameAnimationFactory::skipAllAnimations();
    }
    if (GameAnimationFactory::getAnimationCount() == 0 && !postAnimation)
    {
        CONSOLE_PRINT("GameMenue -> emitting animationsFinished()", Console::eDEBUG);
        emit GameAnimationFactory::getInstance()->animationsFinished();
    }
    Mainapp::getInstance()->continueRendering();
}

void GameMenue::finishActionPerformed()
{
    CONSOLE_PRINT("Doing post action update", Console::eDEBUG);
    
    if (m_pCurrentAction.get() != nullptr)
    {
        Unit* pUnit = m_pCurrentAction->getMovementTarget();
        if (pUnit != nullptr)
        {
            pUnit->postAction(m_pCurrentAction);
        }
        m_pMap->getCurrentPlayer()->postAction(m_pCurrentAction.get());
        m_pMap->getGameScript()->actionDone(m_pCurrentAction);
        m_pCurrentAction = nullptr;
    }
    m_pMap->killDeadUnits();
    m_pMap->getGameRules()->checkVictory();
    skipAnimations(true);
    m_pMap->getGameRules()->createFogVision();
    if (m_humanQuickButtons.get() != nullptr)
    {
        if (!m_pMap->getCurrentPlayer()->getIsDefeated() &&
            m_pMap->getCurrentPlayer()->getBaseGameInput()->getAiType() == GameEnums::AiTypes_Human)
        {
            m_humanQuickButtons->setEnabled(true);
        }
        else
        {
            m_humanQuickButtons->setEnabled(false);
        }
    }
}

void GameMenue::actionPerformed()
{
    if (getParent() != nullptr &&
        m_pMap.get() != nullptr)
    {
        CONSOLE_PRINT("Action performed", Console::eDEBUG);
        finishActionPerformed();
        if (Settings::getSyncAnimations())
        {
            m_pMap->syncUnitsAndBuildingAnimations();
        }
        m_IngameInfoBar->updateTerrainInfo(m_Cursor->getMapPointX(), m_Cursor->getMapPointY(), true);
        m_IngameInfoBar->updateMinimap();
        m_IngameInfoBar->updatePlayerInfo();
        if (GameAnimationFactory::getAnimationCount() == 0 &&
            !m_pMap->getGameRules()->getVictory())
        {
            if (!m_pMap->anyPlayerAlive())
            {
                CONSOLE_PRINT("Forcing exiting the game cause no player is alive", Console::eDEBUG);
                emit sigExitGame();
            }
            else if (m_pMap->getCurrentPlayer()->getIsDefeated())
            {
                CONSOLE_PRINT("Triggering next player cause current player is defeated", Console::eDEBUG);
                spGameAction pAction = spGameAction::create(CoreAI::ACTION_NEXT_PLAYER, m_pMap.get());
                performAction(pAction);
            }
            else if (m_pStoredAction.get() != nullptr)
            {
                performAction(m_pStoredAction);
            }
            else
            {
                GlobalUtils::setUseSeed(false);
                if (m_pMap->getCurrentPlayer()->getBaseGameInput()->getAiType() != GameEnums::AiTypes_ProxyAi)
                {
                    m_pMap->getGameRules()->resumeRoundTime();
                }
                if (m_noTimeOut)
                {
                    spGameAction pAction = spGameAction::create(CoreAI::ACTION_NEXT_PLAYER, m_pMap.get());
                    performAction(pAction);
                    m_noTimeOut = false;
                }
                else
                {
                    CONSOLE_PRINT("emitting sigActionPerformed()", Console::eDEBUG);
                    quint32 delay = Settings::getPauseAfterAction();
                    if (delay == 0)
                    {
                        emit sigActionPerformed();
                    }
                    else
                    {
                        m_delayedActionPerformedTimer.start(std::chrono::seconds(delay));
                    }
                }
            }
        }
    }
    else
    {
        CONSOLE_PRINT("Skipping action performed", Console::eDEBUG);
    }

    m_saveAllowed = true;
    if (m_saveMap)
    {
        doSaveMap();
    }
}

void GameMenue::delayedActionPerformed()
{
    if (m_pCurrentAction.get() == nullptr)
    {
        emit sigActionPerformed();
    }
}

void GameMenue::autoScroll(QPoint cursorPosition)
{
    Mainapp* pApp = Mainapp::getInstance();
    if (QGuiApplication::focusWindow() == pApp &&
        m_Focused &&
        Settings::getAutoScrolling())
    {
        
        if (m_pMap.get() != nullptr && m_IngameInfoBar.get() != nullptr &&
            m_pMap->getCurrentPlayer() != nullptr &&
            m_pMap->getCurrentPlayer()->getBaseGameInput()->getAiType() == GameEnums::AiTypes_Human)
        {
            qint32 moveX = 0;
            qint32 moveY = 0;
            auto bottomRightUi = m_IngameInfoBar->getDetailedViewBox()->getScaledSize() * m_IngameInfoBar->getScaleX();
            if ((cursorPosition.x() < m_IngameInfoBar->getX() - bottomRightUi.x &&
                 (cursorPosition.x() > m_IngameInfoBar->getX() - bottomRightUi.x - 50) &&
                 (m_pMap->getX() + m_pMap->getMapWidth() * m_pMap->getZoom() * GameMap::getImageSize() > m_IngameInfoBar->getX() - bottomRightUi.x - 50)) &&
                cursorPosition.y() > Settings::getHeight() - bottomRightUi.y)
            {

                moveX = -GameMap::getImageSize() * m_pMap->getZoom();
            }
            if ((cursorPosition.y() > Settings::getHeight() - m_autoScrollBorder.height() - bottomRightUi.y) &&
                (m_pMap->getY() + m_pMap->getMapHeight() * m_pMap->getZoom() * GameMap::getImageSize() > Settings::getHeight() - m_autoScrollBorder.height() - bottomRightUi.y) &&
                cursorPosition.x() > m_IngameInfoBar->getX() - bottomRightUi.x)
            {
                moveY = -GameMap::getImageSize() * m_pMap->getZoom();
            }
            if (moveX != 0 || moveY != 0)
            {
                MoveMap(moveX , moveY);
            }
            else
            {
                m_autoScrollBorder.setWidth(Settings::getWidth() - m_IngameInfoBar->getX());
                InGameMenue::autoScroll(cursorPosition);
            }
        }
    }
}

void GameMenue::cursorMoved(qint32 x, qint32 y)
{
    if (m_xyTextInfo.get() != nullptr)
    {
        m_xyTextInfo->setHtmlText("X: " + QString::number(x) + " Y: " + QString::number(y));
        doPlayerInfoFlipping();
    }
}

void GameMenue::doPlayerInfoFlipping()
{
    qint32 x = m_Cursor->getMapPointX();
    qint32 y = m_Cursor->getMapPointY();
    QPoint pos = getMousePos(x, y);
    bool flip = m_pPlayerinfo->getFlippedX();
    qint32 screenWidth = m_IngameInfoBar->getX();
    const qint32 diff = screenWidth / 8;
    if (Settings::getCoInfoPosition() == GameEnums::COInfoPosition_Left)
    {
        m_pPlayerinfo->setX(0);
        flip = false;
        if (m_XYButtonBox.get() != nullptr)
        {
            m_XYButtonBox->setX(screenWidth - m_XYButtonBox->getScaledWidth());
        }
    }
    else if (Settings::getCoInfoPosition() == GameEnums::COInfoPosition_Right)
    {
        flip = true;
        m_pPlayerinfo->setX(screenWidth);
        if (m_XYButtonBox.get() != nullptr)
        {
            m_XYButtonBox->setX(0);
        }
    }
    else if (Settings::getCoInfoPosition() == GameEnums::COInfoPosition_Flipping)
    {
        if ((pos.x() < (screenWidth) / 2 - diff))
        {
            flip = true;
            m_pPlayerinfo->setX(screenWidth);
            if (m_XYButtonBox.get() != nullptr)
            {
                m_XYButtonBox->setX(0);
            }
        }
        else if (pos.x() > (screenWidth) / 2 + diff)
        {
            m_pPlayerinfo->setX(0);
            flip = false;
            if (m_XYButtonBox.get() != nullptr)
            {
                m_XYButtonBox->setX(screenWidth - m_XYButtonBox->getScaledWidth());
            }
        }
    }
    if (flip != m_pPlayerinfo->getFlippedX())
    {
        m_pPlayerinfo->setFlippedX(flip);
        m_pPlayerinfo->updateData();
    }
}

void GameMenue::updatePlayerinfo()
{
    Mainapp::getInstance()->pauseRendering();
    CONSOLE_PRINT("GameMenue::updatePlayerinfo", Console::eDEBUG);
    if (m_pPlayerinfo.get() != nullptr)
    {
        m_pPlayerinfo->updateData();
    }
    if (m_IngameInfoBar.get() != nullptr)
    {
        m_IngameInfoBar->updatePlayerInfo();
    }
    
    for (qint32 i = 0; i < m_pMap->getPlayerCount(); i++)
    {
        m_pMap->getPlayer(i)->updateVisualCORange();
    }
    emit sigOnUpdate();
    Mainapp::getInstance()->continueRendering();
}

void GameMenue::updateMinimap()
{
    Mainapp::getInstance()->pauseRendering();
    if (m_IngameInfoBar.get() != nullptr)
    {
        m_IngameInfoBar->updateMinimap();
    }
    Mainapp::getInstance()->continueRendering();
}

void GameMenue::victory(qint32 team)
{
    if (m_pInstance.get() != nullptr)
    {
        CONSOLE_PRINT("GameMenue::victory for team " + QString::number(team), Console::eDEBUG);
        
        bool exit = true;
        bool humanWin = false;
        // create victorysd
        if (team >= 0)
        {
            for (qint32 i = 0; i < m_pMap->getPlayerCount(); i++)
            {
                Player* pPlayer = m_pMap->getPlayer(i);
                if (pPlayer->getTeam() != team)
                {
                    CONSOLE_PRINT("Defeating player " + QString::number(i) + " cause team " + QString::number(team) + " is set to win the game", Console::eDEBUG);
                    pPlayer->defeatPlayer(nullptr);
                }
                if (pPlayer->getIsDefeated() == false && pPlayer->getBaseGameInput()->getAiType() == GameEnums::AiTypes_Human)
                {
                    humanWin = true;
                }
            }
            if (humanWin)
            {
                Mainapp::getInstance()->getAudioThread()->playSound("victory.wav");
            }
            m_pMap->getGameScript()->victory(team);
            if (GameAnimationFactory::getAnimationCount() == 0)
            {
                exit = true;
            }
            else
            {
                exit = false;
            }
        }
        if (exit == true && !m_isReplay)
        {
            if (m_pNetworkInterface.get() != nullptr)
            {
                m_pChat->detach();
                m_pChat = nullptr;
            }
            if (m_pMap->getCampaign() != nullptr)
            {
                CONSOLE_PRINT("Informing campaign about game result. That human player game result is: " + QString::number(humanWin), Console::eDEBUG);
                m_pMap->getCampaign()->mapFinished(m_pMap.get(), humanWin);
            }
            AchievementManager::getInstance()->onVictory(team, humanWin, m_pMap.get());
            CONSOLE_PRINT("Leaving Game Menue", Console::eDEBUG);
            auto window = spVictoryMenue::create(m_pMap, m_pNetworkInterface);
            oxygine::Stage::getStage()->addChild(window);
            deleteMenu();
        }
    }
}

void GameMenue::showAttackLog(qint32 player)
{    
    m_Focused = false;
    CONSOLE_PRINT("showAttackLog() for player " + QString::number(player), Console::eDEBUG);
    spDialogAttackLog pAttackLog = spDialogAttackLog::create(m_pMap.get(), m_pMap->getPlayer(player));
    connect(pAttackLog.get(), &DialogAttackLog::sigFinished, this, [this]()
    {
        m_Focused = true;
    });
    addChild(pAttackLog);
}

void GameMenue::showRules()
{
    m_Focused = false;
    CONSOLE_PRINT("showRuleSelection()", Console::eDEBUG);
    spRuleSelectionDialog pRuleSelection = spRuleSelectionDialog::create(m_pMap.get(), RuleSelection::Mode::Singleplayer, false);
    connect(pRuleSelection.get(), &RuleSelectionDialog::sigOk, this, [this]()
    {
        m_Focused = true;
    });
    addChild(pRuleSelection);
}

void GameMenue::showUnitInfo(qint32 player)
{    
    m_Focused = false;
    CONSOLE_PRINT("showUnitInfo() for player " + QString::number(player), Console::eDEBUG);
    spDialogUnitInfo pDialogUnitInfo = spDialogUnitInfo::create(m_pMap->getPlayer(player));
    connect(pDialogUnitInfo.get(), &DialogUnitInfo::sigFinished, this, [this]()
    {
        m_Focused = true;
    });
    addChild(pDialogUnitInfo);
}

void GameMenue::showUnitStatistics(qint32 player)
{
    showPlayerUnitStatistics(m_pMap->getPlayer(player));
}

void GameMenue::showPlayerUnitStatistics(Player* pPlayer)
{
    m_Focused = false;
    CONSOLE_PRINT("showUnitStatistics()", Console::eDEBUG);
    spGenericBox pBox = spGenericBox::create();
    spUnitStatisticView view = spUnitStatisticView::create(m_pMap->getGameRecorder()->getPlayerDataRecords()[pPlayer->getPlayerID()],
            Settings::getWidth() - 60, Settings::getHeight() - 100, pPlayer);
    view->setPosition(30, 30);
    pBox->addItem(view);
    connect(pBox.get(), &GenericBox::sigFinished, this, [this]()
    {
        m_Focused = true;
    });
    addChild(pBox);
}

void GameMenue::showOptions()
{    
    m_Focused = false;
    CONSOLE_PRINT("showOptions()", Console::eDEBUG);
    spGenericBox pDialogOptions = spGenericBox::create();
    spGameplayAndKeys pGameplayAndKeys = spGameplayAndKeys::create(Settings::getHeight() - 80);
    pGameplayAndKeys->setY(0);
    pDialogOptions->addItem(pGameplayAndKeys);
    connect(pDialogOptions.get(), &GenericBox::sigFinished, this, [this]()
    {
        Settings::saveSettings();
        m_Focused = true;
    });
    addChild(pDialogOptions);
}

void GameMenue::showChangeSound()
{
    m_Focused = false;
    CONSOLE_PRINT("showChangeSound()", Console::eDEBUG);
    spGenericBox pDialogOptions = spGenericBox::create();
    QSize size(Settings::getWidth() - 20,
               Settings::getHeight() - 100);
    spPanel pPanel = spPanel::create(true, size, size);
    qint32 y = 10;
    qint32 sliderOffset = 400;
    OptionMenue::showSoundOptions(pPanel, sliderOffset, y, this);
    pPanel->setContentHeigth(y + 40);
    pPanel->setPosition(10, 10);
    pDialogOptions->addItem(pPanel);
    connect(pDialogOptions.get(), &GenericBox::sigFinished, this, [this]()
    {
        Settings::saveSettings();
        m_Focused = true;
    });
    addChild(pDialogOptions);
}

void GameMenue::showGameInfo(qint32 player)
{
    CONSOLE_PRINT("showGameInfo() for player " + QString::number(player), Console::eDEBUG);
    QStringList header = {tr("Player"),
                          tr("Produced"),
                          tr("Lost"),
                          tr("Killed"),
                          tr("Army Value"),
                          tr("Income"),
                          tr("Funds"),
                          tr("Bases")};
    QVector<QStringList> data;
    
    qint32 totalBuildings = m_pMap->getBuildingCount("");
    Player* pViewPlayer = m_pMap->getPlayer(player);
    if (pViewPlayer != nullptr)
    {
        m_Focused = false;
        for (qint32 i = 0; i < m_pMap->getPlayerCount(); i++)
        {
            QString funds = QString::number(m_pMap->getPlayer(i)->getFunds());
            QString armyValue = QString::number(m_pMap->getPlayer(i)->calcArmyValue());
            QString income = QString::number(m_pMap->getPlayer(i)->calcIncome());
            qint32 buildingCount = m_pMap->getPlayer(i)->getBuildingCount();
            QString buildings = QString::number(buildingCount);
            if (pViewPlayer->getTeam() != m_pMap->getPlayer(i)->getTeam() &&
                m_pMap->getGameRules()->getFogMode() != GameEnums::Fog_Off &&
                m_pMap->getGameRules()->getFogMode() != GameEnums::Fog_OfMist)
            {
                funds = "?";
                armyValue = "?";
                income = "?";
                buildings = "?";
            }
            data.append({tr("Player ") + QString::number(i + 1),
                         QString::number(m_pMap->getGameRecorder()->getBuildedUnits(i)),
                         QString::number(m_pMap->getGameRecorder()->getLostUnits(i)),
                         QString::number(m_pMap->getGameRecorder()->getDestroyedUnits(i)),
                         armyValue,
                         income,
                         funds,
                         buildings});
            totalBuildings -= buildingCount;
        }
        data.append({tr("Neutral"), "", "", "", "", "", "", QString::number(totalBuildings)});

        spGenericBox pGenericBox = spGenericBox::create();
        QSize size(Settings::getWidth() - 40, Settings::getHeight() - 80);
        qint32 width = (Settings::getWidth() - 20) / header.size();
        if (width < 150)
        {
            width = 150;
        }
        QSize contentSize(header.size() * width + 40, size.height());
        spPanel pPanel = spPanel::create(true, size, contentSize);
        pPanel->setPosition(20, 20);
        QVector<qint32> widths;
        for (qint32 i = 0; i < header.size(); ++i)
        {
            widths.append(width);
        }
        spTableView pTableView = spTableView::create(widths, data, header, false);
        pTableView->setPosition(20, 20);
        pPanel->addItem(pTableView);
        pPanel->setContentHeigth(pTableView->getHeight() + 40);
        pGenericBox->addItem(pPanel);
        addChild(pGenericBox);
        connect(pGenericBox.get(), &GenericBox::sigFinished, this, [this]()
        {
            m_Focused = true;
        });
    }
}

void GameMenue::showCOInfo()
{    
    CONSOLE_PRINT("showCOInfo()", Console::eDEBUG);
    
    spCOInfoDialog pCOInfoDialog = spCOInfoDialog::create(m_pMap->getCurrentPlayer()->getspCO(0), m_pMap->getspPlayer(m_pMap->getCurrentPlayer()->getPlayerID()), [this](spCO& pCurrentCO, spPlayer& pPlayer, qint32 direction)
    {
        if (direction > 0)
        {
            if (pCurrentCO.get() == pPlayer->getCO(1) ||
                pPlayer->getCO(1) == nullptr)
            {
                // roll over case
                if (pPlayer->getPlayerID() == m_pMap->getPlayerCount() - 1)
                {
                    pPlayer = m_pMap->getspPlayer(0);
                    pCurrentCO = pPlayer->getspCO(0);
                }
                else
                {
                    pPlayer = m_pMap->getspPlayer(pPlayer->getPlayerID() + 1);
                    pCurrentCO = pPlayer->getspCO(0);
                }
            }
            else
            {
                pCurrentCO = pPlayer->getspCO(1);
            }
        }
        else
        {
            if (pCurrentCO.get() == pPlayer->getCO(0) ||
                pPlayer->getCO(0) == nullptr)
            {
                // select player
                if (pPlayer->getPlayerID() == 0)
                {
                    pPlayer = m_pMap->getspPlayer(m_pMap->getPlayerCount() - 1);
                }
                else
                {
                    pPlayer = m_pMap->getspPlayer(pPlayer->getPlayerID() - 1);
                }
                // select co
                if ( pPlayer->getCO(1) != nullptr)
                {
                    pCurrentCO = pPlayer->getspCO(1);
                }
                else
                {
                    pCurrentCO = pPlayer->getspCO(0);
                }
            }
            else
            {
                pCurrentCO = pPlayer->getspCO(0);
            }
        }
    }, true);
    addChild(pCOInfoDialog);
    setFocused(false);
    connect(pCOInfoDialog.get(), &COInfoDialog::quit, this, &GameMenue::editFinishedCanceled, Qt::QueuedConnection);
    
}

void GameMenue::saveGame()
{    
    QStringList wildcards;
    wildcards.append("*" + getSaveFileEnding());
    QString path = Settings::getUserPath() + "savegames";
    spFileDialog saveDialog = spFileDialog::create(path, wildcards, m_pMap->getMapName());
    addChild(saveDialog);
    connect(saveDialog.get(), &FileDialog::sigFileSelected, this, [this](QString filename)
    {
        saveMap(filename);
    }, Qt::QueuedConnection);
    setFocused(false);
    connect(saveDialog.get(), &FileDialog::sigCancel, this, &GameMenue::editFinishedCanceled, Qt::QueuedConnection);
}

QString GameMenue::getSaveFileEnding()
{
    if (m_pNetworkInterface.get() != nullptr ||
        m_Multiplayer)
    {
        return ".msav";
    }
    else
    {
        return ".sav";
    }
}

void GameMenue::showSaveAndExitGame()
{    
    CONSOLE_PRINT("showSaveAndExitGame()", Console::eDEBUG);
    QStringList wildcards;
    if (m_pNetworkInterface.get() != nullptr ||
        m_Multiplayer)
    {
        wildcards.append("*.msav");
    }
    else
    {
        wildcards.append("*.sav");
    }
    QString path = Settings::getUserPath() + "savegames";
    spFileDialog saveDialog = spFileDialog::create(path, wildcards, m_pMap->getMapName());
    addChild(saveDialog);
    connect(saveDialog.get(), &FileDialog::sigFileSelected, this, &GameMenue::saveMapAndExit, Qt::QueuedConnection);
    setFocused(false);
    connect(saveDialog.get(), &FileDialog::sigCancel, this, &GameMenue::editFinishedCanceled, Qt::QueuedConnection);
}

void GameMenue::victoryInfo()
{    
    CONSOLE_PRINT("victoryInfo()", Console::eDEBUG);
    spDialogVictoryConditions pVictoryConditions = spDialogVictoryConditions::create(m_pMap.get());
    addChild(pVictoryConditions);
    setFocused(false);
    connect(pVictoryConditions.get(), &DialogVictoryConditions::sigFinished, this, &GameMenue::editFinishedCanceled, Qt::QueuedConnection);
}

void GameMenue::autoSaveMap()
{
    if (Settings::getAutoSavingCycle() > 0)
    {
        CONSOLE_PRINT("GameMenue::autoSaveMap()", Console::eDEBUG);
        QString path = GlobalUtils::getNextAutosavePath(Settings::getUserPath() + "savegames/" + m_pMap->getMapName() + "_autosave_", getSaveFileEnding(), Settings::getAutoSavingCycle());
        saveMap(path, false);
    }
}

void GameMenue::saveMap(QString filename, bool skipAnimations)
{
    CONSOLE_PRINT("GameMenue::saveMap() " + filename, Console::eDEBUG);
    m_saveFile = filename;
    if (!m_saveFile.isEmpty())
    {
        m_saveMap = true;
        if (m_saveAllowed)
        {
            doSaveMap();
        }
        else if (skipAnimations)
        {
            GameAnimationFactory::skipAllAnimations();
        }
    }
    else
    {
        CONSOLE_PRINT("Trying to save empty map name saving ignored.", Console::eWARNING);
    }
    setFocused(true);
}

void GameMenue::saveMapAndExit(QString filename)
{
    m_exitAfterSave = true;
    saveMap(filename);
}

void GameMenue::doSaveMap()
{
    CONSOLE_PRINT("Saving map under " + m_saveFile, Console::eDEBUG);
    if (m_saveAllowed)
    {
        if (m_saveFile.endsWith(".sav") || m_saveFile.endsWith(".msav"))
        {
            QFile file(m_saveFile);
            file.open(QIODevice::WriteOnly | QIODevice::Truncate);
            QDataStream stream(&file);
            
            m_pMap->serializeObject(stream);
            file.close();
            Settings::setLastSaveGame(m_saveFile);
        }
        m_saveMap = false;
        m_saveFile = "";
        if (m_exitAfterSave)
        {
            exitGame();
        }
    }
    else
    {
        CONSOLE_PRINT("Save triggered while no saving is allowed. Game wasn't saved", Console::eERROR);
    }
}

void GameMenue::exitGame()
{    
    CONSOLE_PRINT("Finishing running animations and exiting game", Console::eDEBUG);
    m_gameStarted = false;
    while (GameAnimationFactory::getAnimationCount() > 0)
    {
        GameAnimationFactory::finishAllAnimations();
    }
    victory(-1);
}

void GameMenue::startGame()
{
    CONSOLE_PRINT("GameMenue::startGame", Console::eDEBUG);
    Mainapp* pApp = Mainapp::getInstance();
    GameAnimationFactory::clearAllAnimations();
    
    if (!m_SaveGame)
    {
        m_pMap->startGame();
        m_pMap->setCurrentPlayer(m_pMap->getPlayerCount() - 1);
        if (m_pNetworkInterface.get() == nullptr)
        {
            qint32 count = m_pMap->getPlayerCount();
            bool humanAlive = false;
            for (qint32 i = 0; i < count; i++)
            {
                Player* pPlayer = m_pMap->getPlayer(i);
                if (pPlayer->getBaseGameInput()->getAiType() == GameEnums::AiTypes_Human && !pPlayer->getIsDefeated())
                {
                    humanAlive = true;
                    break;
                }
            }
            m_pMap->setIsHumanMatch(humanAlive);
        }
        GameRules* pRules = m_pMap->getGameRules();
        pRules->init();
        updatePlayerinfo();
        m_ReplayRecorder.startRecording();
        CONSOLE_PRINT("Triggering action next player in order to start the game.", Console::eDEBUG);
        spGameAction pAction = spGameAction::create(CoreAI::ACTION_NEXT_PLAYER, m_pMap.get());
        if (m_pNetworkInterface.get() != nullptr)
        {
            pAction->setSeed(GlobalUtils::getSeed());
        }
        performAction(pAction);
    }
    else
    {
        pApp->getAudioThread()->clearPlayList();
        m_pMap->playMusic();
        m_pMap->updateUnitIcons();
        m_pMap->getGameRules()->createFogVision();
        pApp->getAudioThread()->playRandom();
        updatePlayerinfo();
        m_ReplayRecorder.startRecording();
        if ((m_pNetworkInterface.get() == nullptr ||
             m_pNetworkInterface->getIsServer()) &&
            !m_gameStarted)
        {
            CONSOLE_PRINT("emitting sigActionPerformed()", Console::eDEBUG);
            emit sigActionPerformed();
        }
    }
    m_pMap->setVisible(true);
    m_gameStarted = true;
    
}

void GameMenue::keyInput(oxygine::KeyEvent event)
{
    if (!event.getContinousPress())
    {
        // for debugging
        Qt::Key cur = event.getKey();
        if (m_Focused && m_pNetworkInterface.get() == nullptr)
        {
            if (cur == Settings::getKey_quicksave1())
            {
                saveMap("savegames/quicksave1.sav");
            }
            else if (cur == Settings::getKey_quicksave2())
            {
                saveMap("savegames/quicksave2.sav");
            }
            else if (cur == Settings::getKey_quickload1())
            {
                if (QFile::exists("savegames/quicksave1.sav"))
                {
                    Mainapp* pApp = Mainapp::getInstance();
                    CONSOLE_PRINT("Leaving Game Menue", Console::eDEBUG);
                    spGameMenue pMenue = spGameMenue::create("savegames/quicksave1.sav", true);
                    oxygine::Stage::getStage()->addChild(pMenue);
                    pApp->getAudioThread()->clearPlayList();
                    pMenue->startGame();
                    oxygine::Actor::detach();
                }
            }
            else if (cur == Settings::getKey_quickload2())
            {
                if (QFile::exists("savegames/quicksave2.sav"))
                {
                    CONSOLE_PRINT("Leaving Game Menue", Console::eDEBUG);
                    spGameMenue pMenue = spGameMenue::create("savegames/quicksave1.sav", true);
                    oxygine::Stage::getStage()->addChild(pMenue);
                    Mainapp* pApp = Mainapp::getInstance();
                    pApp->getAudioThread()->clearPlayList();
                    pMenue->startGame();
                    oxygine::Actor::detach();
                }
            }
            else
            {
                keyInputAll(cur);
            }
        }
        else if (m_Focused)
        {
            keyInputAll(cur);
        }
    }
    InGameMenue::keyInput(event);
}

void GameMenue::keyInputAll(Qt::Key cur)
{
    if (cur == Qt::Key_Escape)
    {
        emit sigShowExitGame();
    }
    else if (cur == Settings::getKey_information() ||
             cur == Settings::getKey_information2())
    {
        
        Player* pPlayer = m_pMap->getCurrentViewPlayer();
        GameEnums::VisionType visionType = pPlayer->getFieldVisibleType(m_Cursor->getMapPointX(), m_Cursor->getMapPointY());
        if (m_pMap->onMap(m_Cursor->getMapPointX(), m_Cursor->getMapPointY()) &&
            visionType != GameEnums::VisionType_Shrouded)
        {
            Terrain* pTerrain = m_pMap->getTerrain(m_Cursor->getMapPointX(), m_Cursor->getMapPointY());
            Unit* pUnit = pTerrain->getUnit();
            if (pUnit != nullptr && pUnit->isStealthed(pPlayer))
            {
                pUnit = nullptr;
            }
            spFieldInfo fieldinfo = spFieldInfo::create(pTerrain, pUnit);
            addChild(fieldinfo);
            connect(fieldinfo.get(), &FieldInfo::sigFinished, this, [this]
            {
                setFocused(true);
            });
            setFocused(false);
        }
        
    }
}

qint64 GameMenue::getSyncCounter() const
{
    return m_syncCounter;
}

Chat* GameMenue::getChat() const
{
    return m_pChat.get();
}

void GameMenue::showExitGame()
{    
    CONSOLE_PRINT("showExitGame()", Console::eDEBUG);
    m_Focused = false;
    spDialogMessageBox pExit = spDialogMessageBox::create(tr("Do you want to exit the current game?"), true);
    connect(pExit.get(), &DialogMessageBox::sigOk, this, &GameMenue::exitGame, Qt::QueuedConnection);
    connect(pExit.get(), &DialogMessageBox::sigCancel, this, [this]()
    {
        m_Focused = true;
    });
    addChild(pExit);
}

void GameMenue::showWiki()
{
    CONSOLE_PRINT("showWiki()", Console::eDEBUG);
    m_Focused = false;
    spGenericBox pBox = spGenericBox::create(false);
    spWikiView pView = spWikiView::create(Settings::getWidth() - 40, Settings::getHeight() - 60);
    pView->setPosition(20, 20);
    pBox->addItem(pView);
    connect(pBox.get(), &GenericBox::sigFinished, this, [this]()
    {
        m_Focused = true;
    });
    addChild(pBox);
}

void GameMenue::showSurrenderGame()
{
    
    if (m_pMap->getCurrentPlayer()->getBaseGameInput()->getAiType() == GameEnums::AiTypes::AiTypes_Human)
    {
        CONSOLE_PRINT("showSurrenderGame()", Console::eDEBUG);
        m_Focused = false;
        spDialogMessageBox pSurrender = spDialogMessageBox::create(tr("Do you want to surrender the current game?"), true);
        connect(pSurrender.get(), &DialogMessageBox::sigOk, this, &GameMenue::surrenderGame, Qt::QueuedConnection);
        connect(pSurrender.get(), &DialogMessageBox::sigCancel, this, [this]()
        {
            m_Focused = true;
        });
        addChild(pSurrender);
        
    }
}

void GameMenue::surrenderGame()
{    
    CONSOLE_PRINT("GameMenue::surrenderGame", Console::eDEBUG);
    spGameAction pAction = spGameAction::create(m_pMap.get());
    pAction->setActionID("ACTION_SURRENDER_INTERNAL");
    performAction(pAction);
    m_Focused = true;
}

void GameMenue::showNicknameUnit(qint32 x, qint32 y)
{    
    spUnit pUnit = spUnit(m_pMap->getTerrain(x, y)->getUnit());
    if (pUnit.get() != nullptr)
    {
        CONSOLE_PRINT("showNicknameUnit()", Console::eDEBUG);
        spDialogTextInput pDialogTextInput = spDialogTextInput::create(tr("Nickname for the Unit:"), true, pUnit->getName());
        connect(pDialogTextInput.get(), &DialogTextInput::sigTextChanged, this, [this, x, y](QString value)
        {
            emit sigNicknameUnit(x, y, value);
        });
        connect(pDialogTextInput.get(), &DialogTextInput::sigCancel, this, [this]()
        {
            m_Focused = true;
        });
        addChild(pDialogTextInput);
        m_Focused = false;
    }
}

void GameMenue::nicknameUnit(qint32 x, qint32 y, QString name)
{
    CONSOLE_PRINT("GameMenue::nicknameUnit", Console::eDEBUG);
    spGameAction pAction = spGameAction::create(m_pMap.get());
    pAction->setActionID("ACTION_NICKNAME_UNIT_INTERNAL");
    pAction->setTarget(QPoint(x, y));
    pAction->writeDataString(name);
    performAction(pAction);
    m_Focused = true;
}

void GameMenue::showDamageCalculator()
{
    addChild(spDamageCalculator::create());
}

bool GameMenue::getIsReplay() const
{
    return m_isReplay;
}

void GameMenue::setIsReplay(bool isReplay)
{
    m_isReplay = isReplay;
}
