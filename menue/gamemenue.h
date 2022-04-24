#ifndef GAMEMENUE_H
#define GAMEMENUE_H

#include <QObject>
#include <QTimer>

#include <menue/ingamemenue.h>

#include "network/NetworkInterface.h"

#include "objects/base/chat.h"
#include "objects/base/closeablepopup.h"
#include "objects/base/label.h"

#include "game/gamerecording/replayrecorder.h"
#include "game/gameaction.h"
#include "game/gamemap.h"
#include "game/ui/playerinfo.h"
#include "game/ui/ingameinfobar.h"
#include "game/ui/humanquickbuttons.h"

#include "coreengine/LUPDATE_MACROS.h"

class GameMenue;
using spGameMenue = oxygine::intrusive_ptr<GameMenue>;

/**
 * @brief The GameMenue class handles the game :)
 */
class GameMenue : public InGameMenue
{
    Q_OBJECT
public:

    explicit GameMenue(spGameMap pMap, bool saveGame, spNetworkInterface pNetworkInterface);
    explicit GameMenue(QString map, bool saveGame);
    explicit GameMenue(spGameMap pMap);
    virtual ~GameMenue() = default;
    static spGameMenue getInstance();
    /**
     * @brief attachInterface
     * @param pNetworkInterface
     */
    void attachInterface(spNetworkInterface pNetworkInterface);

    /**
     * @brief getGameStarted
     * @return always true for singleplayer games turns true on multiplayer games once all clients have started the game
     */
    bool getGameStarted() const;
    /**
     * @brief getGameInfoBar
     * @return
     */
    IngameInfoBar* getGameInfoBar()
    {
        return m_IngameInfoBar.get();
    }
    Chat* getChat() const;
    /**
     * @brief getCurrentViewPlayer
     * @return
     */
    virtual Player* getCurrentViewPlayer();
    /**
     * @brief getSyncCounter
     * @return
     */
    qint64 getSyncCounter() const;
    /**
     * @brief autoScroll
     */
    virtual void autoScroll(QPoint cursorPosition) override;
    /**
     * @brief doTrapping
     * @param pGameAction
     */
    void doTrapping(spGameAction & pGameAction);
    bool getIsReplay() const;
    void setIsReplay(bool isReplay);

    bool getActionRunning() const;

signals:
    void sigActionPerformed();
    void sigGameStarted();
    void sigSyncFinished();
    void sigSaveGame();
    void sigExitGame();
    void sigShowExitGame();
    void sigShowSurrenderGame();
    void sigNicknameUnit(qint32 x, qint32 y, QString name);
public slots:
    /**
     * @brief centerMapOnAction
     * @param pGameAction
     */
    void centerMapOnAction(GameAction* pGameAction);
    /**
     * @brief actionPerformed
     */
    void actionPerformed();
    /**
     * @brief performAction performs the given action and deletes it afterwards.
     * @param pGameAction
     */
    void performAction(spGameAction pGameAction);
    /**
     * @brief isTrap
     * @param pAction
     * @param pMoveUnit
     * @param currentPoint
     * @param previousPoint
     * @return
     */
    bool isTrap(const QString & function, spGameAction pAction, Unit* pMoveUnit, QPoint currentPoint, QPoint previousPoint, qint32 moveCost);
    /**
     * @brief finsihActionPerformed
     */
    void finishActionPerformed();
    /**
     * @brief skipAnimations
     */
    void skipAnimations(bool postAnimation);
    /**
     * @brief doMultiTurnMovement
     * @param pGameAction
     */
    spGameAction doMultiTurnMovement(spGameAction pGameAction);
    /**
     * @brief updatePlayerinfo
     */
    void updatePlayerinfo();
    /**
     * @brief updateMinimap
     */
    void updateMinimap();
    /**
     * @brief cursorMoved
     * @param x
     * @param y
     */
    void cursorMoved(qint32 x, qint32 y);
    /**
     * @brief doPlayerInfoFlipping
     */
    void doPlayerInfoFlipping();
    /**
     * @brief victory
     * @param team
     */
    void victory(qint32 team);
    /**
     * @brief saveGame
     */
    void saveGame();
    /**
     * @brief victoryInfo
     */
    void victoryInfo();
    /**
     * @brief showCOInfo
     */
    void showCOInfo();
    /**
     * @brief showGameInfo
     */
    void showGameInfo(qint32 player);
    /**
     * @brief showOptions
     */
    void showOptions();
    /**
     * @brief showChangeSound
     */
    void showChangeSound();
    /**
     * @brief showAttackLog
     */
    void showAttackLog(qint32 player);
    /**
     * @brief showRuleSelection
     */
    void showRules();
    /**
     * @brief showAttackLog
     */
    void showUnitInfo(qint32 player);
    /**
     * @brief showUnitStatistics
     */
    void showUnitStatistics(qint32 player);
    /**
     * @brief showPlayerUnitStatistics
     * @param pPlayer
     */
    void showPlayerUnitStatistics(Player* pPlayer);
    /**
     * @brief saveMap
     * @param file
     */
    void saveMap(QString filename, bool skipAnimations = true);
    /**
     * @brief autoSaveMap
     */
    void autoSaveMap();
    /**
     * @brief saveMapAndExit
     * @param filename
     */
    void saveMapAndExit(QString filename);
    /**
     * @brief exitGame
     */
    void exitGame();
    /**
     * @brief surrenderGame
     */
    void surrenderGame();
    /**
     * @brief nicknameUnit
     */
    void showNicknameUnit(qint32 x, qint32 y);
    /**
     * @brief nicknameUnit
     * @param x
     * @param y
     * @param name
     */
    void nicknameUnit(qint32 x, qint32 y, QString name);
    /**
     * @brief keyInput
     * @param event
     */
    virtual void keyInput(oxygine::KeyEvent event) override;
    /**
     * @brief editFinishedCanceled
     */
    void editFinishedCanceled();
    /**
     * @brief isNetworkGame
     * @return
     */
    bool isNetworkGame();
    /**
     * @brief playerJoined
     * @param socketID
     */
    void playerJoined(quint64 socketID);
    /**
     * @brief continueAfterSyncGame
     */
    void continueAfterSyncGame();
    /**
     * @brief showExitGame
     */
    virtual void showExitGame();
    /**
     * @brief showSurrenderGame
     */
    void showSurrenderGame();
    /**
     * @brief startGame
     * @param startPlayer
     */
    void startGame();
    /**
     * @brief showSaveAndExitGame
     */
    void showSaveAndExitGame();
    /**
     * @brief showWiki
     */
    void showWiki();
    /**
     * @brief showDamageCalculator
     */
    void showDamageCalculator();
protected slots:
    /**
     * @brief recieveData
     * @param socketID
     * @param data
     * @param service
     */
    void recieveData(quint64 socketID, QByteArray data, NetworkInterface::NetworkSerives service);
    /**
     * @brief disconnected
     * @param socketID
     */
    void disconnected(quint64 socketID);
    /**
     * @brief joinAsObserver
     * @param stream
     * @param socketID
     */
    void joinAsObserver(QDataStream & stream, quint64 socketID);
    /**
     * @brief waitForPlayerJoinSyncFinished
     * @param stream
     * @param socketID
     */
    void waitForPlayerJoinSyncFinished(QDataStream & stream, quint64 socketID);
    /**
     * @brief waitingForPlayerJoinSyncFinished
     */
    void waitingForPlayerJoinSyncFinished(QDataStream & stream, quint64 socketID);
    /**
     * @brief removePlayerFromSyncWaitList
     * @param socketID
     */
    void removePlayerFromSyncWaitList(quint64 socketID);
    /**
     * @brief playerJoinedFinished
     */
    void playerJoinedFinished();
    /**
     * @brief updateTimer
     */
    void updateTimer();
    /**
     * @brief onEnter
     */
    virtual void onEnter() override;
    /**
     * @brief nextTurnPlayerTimeout
     */
    void nextTurnPlayerTimeout();
    /**
     * @brief delayedActionPerformed
     */
    void delayedActionPerformed();
protected:
    void loadUIButtons();
    void loadGameMenue();
    void connectMap();
    void keyInputAll(Qt::Key cur);
    QString getSaveFileEnding();
    void showChat();
    void doSaveMap();
    bool getIsMultiplayer(const spGameAction & pGameAction) const;
    bool requiresForwarding(const spGameAction & pGameAction) const;
protected:
    ReplayRecorder m_ReplayRecorder;
    spPlayerInfo m_pPlayerinfo;
    spIngameInfoBar m_IngameInfoBar;
    spLabel m_xyTextInfo;
    oxygine::spActor m_XYButtonBox;
    spHumanQuickButtons m_humanQuickButtons;
    spChat m_pChat{nullptr};
    oxygine::spButton m_ChatButton{nullptr};
    oxygine::spTween m_chatButtonShineTween{nullptr};
    spNetworkInterface m_pNetworkInterface;
    bool m_gameStarted{false};
    QVector<quint64> m_PlayerSockets;
    QVector<quint64> m_ReadySockets;
    oxygine::spTextField m_CurrentRoundTime;
    bool m_SaveGame{false};
    bool m_Multiplayer{false};

    QTimer m_UpdateTimer{this};
    QTimer m_delayedActionPerformedTimer{this};
    bool m_noTimeOut{false};
    spGameAction m_pStoredAction{nullptr};
    spGameAction m_pCurrentAction{nullptr};

    qint64 m_syncCounter{0};
    bool m_enabledAutosaving{false};

    QString m_saveFile;
    bool m_saveMap{false};
    bool m_exitAfterSave{false};
    bool m_saveAllowed{false};
    bool m_isReplay{false};
    bool m_actionRunning{false};

    struct
    {
        bool m_waitingForSyncFinished{false};
        spGameAction m_postSyncAction;
        QVector<bool> m_lockedPlayers;
        QVector<quint64> m_connectingSockets;
    } m_multiplayerSyncData;
};

Q_DECLARE_INTERFACE(GameMenue, "GameMenue");

#endif // GAMEMENUE_H
