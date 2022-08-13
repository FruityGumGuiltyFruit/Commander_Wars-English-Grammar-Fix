#pragma once
#include "3rd_party/oxygine-framework/oxygine/oxygine-forwards.h"
#include "3rd_party/oxygine-framework/oxygine/PointerState.h"

#ifdef GRAPHICSUPPORT
#include "3rd_party/oxygine-framework/oxygine/core/windowOpenglBase.h"
#else
#include "3rd_party/oxygine-framework/oxygine/core/windowBase.h"
#endif

#include <QMutex>
#include <QBasicTimer>
#include <QThread>
#include <QKeyEvent>
#include <QElapsedTimer>

namespace oxygine
{
    class GameWindow : public WindowBaseClass
    {
        Q_OBJECT
    public:
        explicit GameWindow();
        virtual ~GameWindow() = default;
#ifdef GRAPHICSUPPORT
        static QOpenGLContext* getGLContext();
#endif

        static GameWindow* getWindow();
        bool isReady2Render();
        /**
         * @brief quitGame quits this game
         */
        void quitGame()
        {
            m_quit = true;
        }
        void loadResAnim(oxygine::spResAnim pAnim, QImage & image, qint32 columns, qint32  rows, float scaleFactor, bool addTransparentBorder);
        virtual bool isWorker() = 0;
        /**
         * @brief isWorkerRunning
         * @return
         */
        virtual bool isWorkerRunning() = 0;
        /**
         * @brief isEvenScale
         * @param width1
         * @param width2
         * @return
         */
        static bool isEvenScale(qint32 width1, qint32 width2);
        /**
         * @brief pauseRendering
         */
        void pauseRendering()
        {
            if (!isWorker())
            {
                oxygine::handleErrorPolicy(oxygine::ep_show_error, "pauseRendering not started by worker thread");
            }
            if (m_pausedCounter == 0)
            {
                m_pauseMutex.lock();
            }
            ++m_pausedCounter;
        }
        /**
         * @brief continueRendering
         */
        void continueRendering()
        {
            if (!isWorker())
            {
                oxygine::handleErrorPolicy(oxygine::ep_show_error, "continueRendering not started by worker thread");
            }
            --m_pausedCounter;
            if (m_pausedCounter == 0)
            {
                m_pauseMutex.unlock();
            }
        }
        /**
         * @brief hasCursor
         * @return
         */
        bool hasCursor();
        virtual void shutdown();
        void setTimerCycle(qint32 newTimerCycle);
        qint32 getTimerCycle() const;
        bool getShuttingDown() const;
        void setShuttingDown(bool newShuttingDown);
        void redrawUi();
    signals:
        void sigLoadSingleResAnim(oxygine::spResAnim pAnim, QImage & image, qint32 columns, qint32 rows, float scaleFactor, bool addTransparentBorder);
        void sigLoadRessources();
        void sigMousePressEvent(oxygine::MouseButton button, qint32 x, qint32 y);
        void sigMouseReleaseEvent(oxygine::MouseButton button, qint32 x, qint32 y);
        void sigWheelEvent(qint32 x, qint32 y);
        void sigMouseMoveEvent(qint32 x, qint32 y);
        void sigStopUpdateTimer();
        void sigStartUpdateTimer();
        void sigQuit(qint32 exitCode);
        void sigShowKeyboard(bool visible);
    public slots:
        /**
         * @brief getBrightness
         * @return
         */
        float getBrightness() const;
        /**
         * @brief setBrightness
         * @param brightness
         */
        void setBrightness(float brightness);
        /**
         * @brief getGamma
         * @return
         */
        float getGamma() const;
        /**
         * @brief setGamma
         * @param gamma
         */
        void setGamma(float gamma);
#ifdef GRAPHICSUPPORT
        virtual void initializeGL() override;
#endif
        bool isMainThread() const
        {
            return QThread::currentThreadId() == m_mainHandle;
        }
        void launchGame();

    protected slots:
        void loadSingleResAnim(oxygine::spResAnim pAnim, QImage & image, qint32 columns, qint32 rows, float scaleFactor, bool addTransparentBorder);
        virtual void loadRessources(){}

        void quitApp();
        virtual void onQuit() = 0;
        void quit(qint32 exitCode);
        void showKeyboard(bool visible);
    protected:
        virtual void registerResourceTypes();
#ifdef GRAPHICSUPPORT
        virtual void timerEvent(QTimerEvent *) override;
        virtual void paintGL() override;
        virtual void resizeGL(qint32 w, qint32 h) override;
        // input events
        virtual void mousePressEvent(QMouseEvent *event) override;
        virtual void mouseReleaseEvent(QMouseEvent *event) override;
        virtual void wheelEvent(QWheelEvent *event) override;
        virtual void mouseMoveEvent(QMouseEvent *event)override;        
        virtual void touchEvent(QTouchEvent *event) override;
#endif

        void updateData();
        bool beginRendering();
        void swapDisplayBuffers();

        void handleZoomGesture(QList<QTouchEvent::TouchPoint> & touchPoints);
        bool sameTouchpoint(QPointF pos1, QPointF pos2) const;

    protected:
        bool m_renderEnabled = true;

        bool m_quit{false};
        QBasicTimer m_Timer;
        qint32 m_timerCycle{1};
        QElapsedTimer m_pressDownTime;
        bool m_pressDownTimeRunning{false};

        // touch handling
        bool m_longPressSent{false};
        bool m_touchMousePressSent{false};
        float m_lastZoomValue{1.0f};

        QMutex m_pauseMutex;
        qint32 m_pausedCounter{0};

        static GameWindow* m_window;
        float m_brightness{0.0f};
        float m_gamma{1.0f};
        Qt::HANDLE m_mainHandle{nullptr};

        bool m_shuttingDown{false};
        bool m_noUi{false};
        bool m_launched{false};
        qint32 m_repeatedFramesDropped{0};
    };
}
