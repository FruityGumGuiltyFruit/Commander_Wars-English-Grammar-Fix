#pragma once
#include "3rd_party/oxygine-framework/oxygine/oxygine-forwards.h"
#include "3rd_party/oxygine-framework/oxygine/Draggable.h"
#include "3rd_party/oxygine-framework/oxygine/math/Vector2.h"
#include "3rd_party/oxygine-framework/oxygine/actor/Actor.h"
#include "3rd_party/oxygine-framework/oxygine/actor/ClipRectActor.h"

namespace oxygine
{
    class SlidingActor;
    using spSlidingActor = intrusive_ptr<SlidingActor>;
    class SlidingActor final : public Actor
    {
    public:
        static void setDefaultTouchThreshold(float val);

        explicit SlidingActor();
        ~SlidingActor();
        spActor getContent() const
        {
            return m_content;
        }
        const RectF& getDragBounds() const
        {
            return m_drag.getDragBounds();
        }
        /**max allowed radius of touch move when content could be clicked*/
        void setTouchThreshold(float rad);
        /**sets actor with scrollable data*/
        void setContent(spActor content);
        /**lock for user scrolling*/
        void setLocked(bool locked);
        /**snaps content*/
        void snap();
        /**stop if sliding*/
        void stop();
        void updateDragBounds();

    protected:
        void destroy();
        virtual void handleEvent(Event* event) override;
        virtual void sizeChanged(const Point& size) override;
        virtual void doUpdate(const UpdateState& us) override;
        void _newEvent(Event* event);

    protected:
        bool m_sliding{false};
        float m_rad{15.0f};
        bool m_ignoreTouchUp{false};
        Vector2 m_downPos;
        Vector2 m_speed;
        Draggable m_drag;
        spActor m_content;
        spClipRectActor m_clip;
        spEventDispatcher m_holded;
        struct  iter
        {
            Vector2 pos;
            timeMS tm;
        };
        enum {NUM = 11};
        iter m_prev[NUM];
        qint32 m_current{0};
        timeMS m_lastIterTime{0};
        pointer_index m_finger{0};
        static float m_defaultTouchThreshold;
    };
}
