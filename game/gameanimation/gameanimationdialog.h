#ifndef GAMEANIMATIONDIALOG_H
#define GAMEANIMATIONDIALOG_H

#include <QTimer>
#include <QElapsedTimer>
#include "3rd_party/oxygine-framework/oxygine/KeyEvent.h"

#include "game/gameanimation/gameanimation.h"
#include "game/GameEnums.h"

class GameAnimationDialog;
using spGameAnimationDialog = std::shared_ptr<GameAnimationDialog>;

class GameAnimationDialog final : public GameAnimation
{
    Q_OBJECT
public:
    static const qint32 dialogHeigth = 96;
    explicit GameAnimationDialog(quint32 frameTime, GameMap* pMap);
   virtual ~GameAnimationDialog() = default;
    virtual void restart() override;
    void finishDialog();
    void updateShownText();
signals:
    void sigStartFinishTimer();
    void sigRightClick();
    void sigLeftClick();
public slots:
    /**
     * @brief nextDialogStep
     */
    void nextDialogStep();
    /**
     * @brief keyInput
     * @param event
     */
    void keyInput(oxygine::KeyEvent event);
public:
    /**
     * @brief loadCoSprite
     * @param coid
     * @param offsetX
     * @param offsetY
     * @param scale
     * @param flippedX
     */
    Q_INVOKABLE void loadCoSprite(const QString coid, float offsetX, float offsetY, bool flippedX, float scale = 1.0f);
    /**
     * @brief onFinished
     * @param skipping
     * @return
     */
    Q_INVOKABLE virtual bool onFinished(bool skipping) override;
    /**
     * @brief setPositionTop wether dialog appears on top or bottom of the game
     * @param value
     */
    Q_INVOKABLE void setPositionTop(bool value);
    /**
     * @brief setColor
     */
    Q_INVOKABLE void setColor(QColor color);
    /**
     * @brief setDialog
     * @param text
     */
    Q_INVOKABLE void setDialog(const QString text);
    /**
     * @brief setCO
     * @param coid
     */
    Q_INVOKABLE void setCO(const QString coid, GameEnums::COMood mood);
    /**
     * @brief setPlayerCO
     * @param player
     * @param co
     * @param mood
     */
    Q_INVOKABLE void setPlayerCO(qint32 player, quint8 co, GameEnums::COMood mood);
    /**
     * @brief setFinishDelay sets if the dialog disappears automatically if all text was displayed -1 for infinite
     * @param value
     */
    Q_INVOKABLE void setFinishDelay(qint32 valueMs);
    /**
     * @brief setTextSpeed
     * @param speed
     */
    Q_INVOKABLE void setTextSpeed(qint32 speed);
    /**
     * @brief loadBackground
     * @param file path relative to the Commander Wars.exe
     */
    Q_INVOKABLE void loadBackground(const QString file);
protected:
    virtual void update(const oxygine::UpdateState& us) override;
    void startFinishTimer();
private:
    QString m_Text;
    qint32 m_writePosition{0};
    qint32 m_autoFinishMs{-1};
    QTimer m_finishTimer;

    oxygine::spSprite m_TextMask;
    oxygine::spSprite m_TextBackground;
    oxygine::spSprite m_COSprite;

    oxygine::spTextField m_TextField;
    QElapsedTimer m_textTimer;
    qint32 m_textSpeed;

    // background sprite handling
    oxygine::spSprite m_BackgroundSprite;
    oxygine::spResAnim m_BackgroundAnim;

    bool m_paused{false};
};

Q_DECLARE_INTERFACE(GameAnimationDialog, "GameAnimationDialog");

#endif // GAMEANIMATIONDIALOG_H
