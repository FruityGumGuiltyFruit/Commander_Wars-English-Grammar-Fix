#ifndef CREATEOUTLINE_H
#define CREATEOUTLINE_H

#include "3rd_party/oxygine-framework/oxygine/actor/Actor.h"

class CreateOutline final
{
public:
    CreateOutline() = delete;
    ~CreateOutline() = delete;

    static void addCursorRangeOutline(oxygine::spActor pOutlineSprite, const QString & prefix, qint32 range, QColor color);
private:
    static void createOuterLeftRightOutline(oxygine::spActor pOutlineSprite, const QString & prefix, qint32 range, QColor color);
    static void createOuterTopBottomOutline(oxygine::spActor pOutlineSprite, const QString & prefix, qint32 range, QColor color);
    static void createOutline(oxygine::spActor pOutlineSprite, const QString & prefix, qint32 i, qint32 range, QColor color);
};

#endif // CREATEOUTLINE_H
