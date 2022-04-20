#ifndef REPLAYMENU_H
#define REPLAYMENU_H

#include <QMutex>

#include "menue/gamemenue.h"

#include "objects/base/v_scrollbar.h"

#include "gameinput/humanplayerinput.h"
#include "game/viewplayer.h"
#include "game/GameEnums.h"

class ReplayMenu;
using spReplayMenu = oxygine::intrusive_ptr<ReplayMenu>;

class ReplayMenu : public GameMenue
{
    Q_OBJECT
    static constexpr qint32 actionPixelSize = 5;
public:
    ReplayMenu(QString filename);
    virtual ~ReplayMenu();
    /**
     * @brief getCurrentViewPlayer
     * @return
     */
    virtual Player* getCurrentViewPlayer() override;
    /**
     * @brief getValid
     * @return
     */
    bool getValid() const;

signals:
    void sigShowRecordInvalid();
    void sigExitReplay();
    void sigSwapPlay();
    void sigStartFastForward();
    void sigStopFastForward();
    void sigShowConfig();
    void sigOneStep();
    void sigRewindDay();
public slots:
    void showRecordInvalid();
    void exitReplay();
    void nextReplayAction();
    virtual void showExitGame() override;
    void swapPlay();
    void startFastForward();
    void stopFastForward();
    void showConfig();
    void setViewTeam(qint32 item);
    /**
     * @brief startSeeking
     */
    void startSeeking();
    /**
     * @brief seekRecord
     * @param value
     */
    void seekRecord(float value);
    /**
     * @brief seekChanged
     * @param value
     */
    void seekChanged(float value);
    /**
     * @brief togglePlayUi
     */
    void togglePlayUi();
    /**
     * @brief oneStep
     */
    void oneStep();
    /**
     * @brief rewind
     */
    void rewindDay();
protected slots:
    virtual void onEnter() override;
protected:
    /**
     * @brief loadUIButtons
     */
    void loadUIButtons();
    /**
     * @brief loadSeekUi
     */
    void loadSeekUi();
    /**
     * @brief seekToDay
     * @param day
     */
    void seekToDay(qint32 day);
private:
    bool m_paused{false};
    bool m_uiPause{false};
    bool m_pauseRequested{false};
    quint32 m_replayCounter{0};
    spV_Scrollbar m_progressBar;
    QRecursiveMutex m_replayMutex;
    oxygine::spButton m_playButton;
    oxygine::spButton m_pauseButton;
    oxygine::spButton m_fastForwardButton;
    oxygine::spButton m_oneStepButton;
    oxygine::spButton m_rewindDayButton;
    oxygine::spButton m_configButton;
    oxygine::spBox9Sprite m_taskBar;

    bool m_seekingOverworldAnimations = false;
    GameEnums::BattleAnimationMode m_seekingBattleAnimations = GameEnums::BattleAnimationMode_All;
    bool m_seekingDialog = false;
    bool m_seekingCapture = false;
    bool m_seekingMovement = false;
    bool m_seekingDay2Day = false;

    spHumanPlayerInput m_HumanInput;
    spViewplayer m_Viewplayer;

    bool m_storedOverworldAnimations = false;
    GameEnums::BattleAnimationMode m_storedBattleAnimMode;
    GameEnums::BattleAnimationType m_storedBatteAnimType;
    bool m_storedDialog = false;
    bool m_storedCaptureAnimation = false;
    bool m_storedMovementAnimation = false;
    bool m_storedDay2DayAnimation = false;
    quint32 m_storedDialogAnimationSpeed = 1;
    quint32 m_storedAnimationSpeed = 1;
    quint32 m_storedBattleAnimationSpeed = 1;
    quint32 m_storedCaptureAnimationSpeed = 1;
    qint64 m_lastRewind = 0;

    bool m_seeking{false};
    bool m_valid{false};
    oxygine::spActor m_seekActor;
    spLabel m_seekDayLabel;
};

#endif // REPLAYMENU_H
