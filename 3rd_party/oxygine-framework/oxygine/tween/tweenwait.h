#pragma once

#include "3rd_party/oxygine-framework/oxygine/oxygine-forwards.h"

class TweenWait final
{
public:
    using TActor = oxygine::Actor;
    explicit TweenWait() = default;
    ~TweenWait() = default;
    void update(oxygine::Actor& actor, float p, const oxygine::UpdateState& us);

    void init(oxygine::Actor&){}
    void done(oxygine::Actor&){}
protected:
};
