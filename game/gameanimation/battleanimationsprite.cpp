#include <QColor>

#include "3rd_party/oxygine-framework/oxygine/tween/tweenshakey.h"
#include "3rd_party/oxygine-framework/oxygine/tween/tweentogglevisibility.h"
#include "3rd_party/oxygine-framework/oxygine/tween/tweenscreenshake.h"

#include "coreengine/interpreter.h"
#include "coreengine/audiomanager.h"
#include "coreengine/globalutils.h"
#include "coreengine/gameconsole.h"
#include "coreengine/settings.h"

#include "resource_management/battleanimationmanager.h"
#include "resource_management/cospritemanager.h"

#include "game/player.h"
#include "game/co.h"

#include "game/gameanimation/battleanimationsprite.h"
#include "game/gameanimation/battleanimation.h"

#include "3rd_party/oxygine-framework/oxygine/actor/slidingsprite.h"

const char* const BattleAnimationSprite::standingAnimation = "loadStandingAnimation";
const char* const BattleAnimationSprite::impactUnitOverlayAnimation = "loadImpactUnitOverlayAnimation";
const char* const BattleAnimationSprite::impactAnimation = "loadImpactAnimation";
const char* const BattleAnimationSprite::fireAnimation = "loadFireAnimation";
const char* const BattleAnimationSprite::moveInAnimation = "loadMoveInAnimation";
const char* const BattleAnimationSprite::standingFiredAnimation = "loadStandingFiredAnimation";
const char* const BattleAnimationSprite::dyingAnimation = "loadDyingAnimation";
const char* const BattleAnimationSprite::stopAnimation = "loadStopAnimation";

BattleAnimationSprite::BattleAnimationSprite(GameMap* pMap, spUnit pUnit, Terrain* pTerrain, QString animationType, qint32 hp, bool playSound)
    : m_pUnit(pUnit),
      m_pTerrain(pTerrain),
      m_hpRounded(hp),
      m_nextFrameTimer(this),
      m_playSound(playSound)
{
#ifdef GRAPHICSUPPORT
    setObjectName("BattleAnimationSprite");
#endif
    Interpreter::setCppOwnerShip(this);
    if (m_hpRounded < 0.0f)
    {
        m_hpRounded = pUnit->getHpRounded();
    }
    // setup next frame timer
    m_nextFrameTimer.setSingleShot(true);
    setUnitFrameDelay(75);

    m_Actor = oxygine::spClipRectActor::create();
    m_Actor->setSize(127, 192);
    setSize(m_Actor->getScaledWidth(), m_Actor->getScaledHeight());
    addChild(m_Actor);
    loadAnimation(animationType);
    connect(this, &BattleAnimationSprite::sigDetachChild, this, &BattleAnimationSprite::detachChild, Qt::QueuedConnection);
    connect(&m_nextFrameTimer, &QTimer::timeout, this, &BattleAnimationSprite::startNextUnitFrames, Qt::QueuedConnection);
}

BattleAnimationSprite::~BattleAnimationSprite()
{
    stopSound();
}

void BattleAnimationSprite::clear()
{
    m_Actor->removeChildren();
    m_nextFrames.clear();
    m_currentFrame.clear();
}

void BattleAnimationSprite::flipActorsX(bool flippedX)
{
    if (m_nextFrames.length() > 0)
    {
        for (auto & frame : m_nextFrames[m_nextFrames.length() - 1])
        {
            for (auto & sprite : frame)
            {
                sprite->flipActorsX(flippedX);
            }
        }
    }
    oxygine::Sprite::flipActorsX(flippedX);
}

void BattleAnimationSprite::loadAnimation(QString animationType)
{
    loadAnimation(animationType, m_pUnit.get());
}

void BattleAnimationSprite::loadAnimation(QString animationType, Unit* pUnit, Unit* pDefender, qint32 attackerWeapon, bool clearSprite, bool start)
{
    QVector<QVector<oxygine::spSprite>> buffer;
    if (!clearSprite && m_nextFrames.length() > 0)
    {
        for (auto & unitFrame : m_nextFrames[m_nextFrames.length() - 1])
        {
            buffer.append(QVector<oxygine::spSprite>());
            for (auto & sprite : unitFrame)
            {
                buffer[buffer.length() - 1].append(sprite);
            }
        }
        m_nextFrames.removeAt(m_nextFrames.length() - 1);
    }
    else if (!clearSprite)
    {
        for (auto & unitFrame : m_currentFrame)
        {
            buffer.append(QVector<oxygine::spSprite>());
            for (auto & sprite : unitFrame)
            {
                buffer[buffer.length() - 1].append(sprite);
            }
        }
    }
    bool startFrame = false;
    if (m_nextFrames.length() == 0)
    {
        startFrame = true;
        m_frameIterator = 0;
    }
    if ((m_nextFrames.length() == 0 ||
        m_nextFrames[m_nextFrames.length() - 1].length() > 0))
    {
        m_nextFrames.append(QVector<QVector<oxygine::spSprite>>());
    }
    if (!clearSprite)
    {
        for (qint32 i = 0; i <  buffer.size(); ++i)
        {
            if (i >= m_nextFrames[m_nextFrames.length() - 1].size())
            {
                m_nextFrames[m_nextFrames.length() - 1].append(QVector<oxygine::spSprite>());
            }
        }
        for (qint32 i = buffer.size() - 1; i >= 0 ; --i)
        {
            for (auto & sprite : buffer[i])
            {
                m_nextFrames[m_nextFrames.length() - 1][i].prepend(sprite);
            }
        }
    }
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = animationType;
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(pDefender),
                       attackerWeapon,
                       pInterpreter->newQObject(m_pMap)});
    pInterpreter->doFunction("BATTLEANIMATION_" + pUnit->getUnitID(), function1, args);
    if (m_nextFrames.length() > 0 && !clearSprite)
    {
        for (qint32 i = m_currentFrame.length() - 1; i >= 0; --i)
        {
            if (i >= m_nextFrames[0].size())
            {
                for (auto & sprite : m_currentFrame[i])
                {
                    sprite->detach();
                }
                m_currentFrame[i].clear();
            }
        }
    }
    if (startFrame && start)
    {
        startNextUnitFrames();
    }
}

qint32 BattleAnimationSprite::loadDyingFadeOutAnimation(qint32 fadeoutTime)
{
    qint32 maxUnitCount = getMaxUnitCount();
    qint32 startCount = getUnitCount(maxUnitCount, GlobalUtils::roundUp(m_dyingStartHp));
    qint32 endCount = getUnitCount(maxUnitCount, GlobalUtils::roundUp(m_dyingEndHp));
    qint32 frameDelay = 75;
    qint32 count = 0;

    QVector<QVector<oxygine::spSprite>>* frame;
    if (m_nextFrames.length() > 0 && m_nextFrames[m_nextFrames.length() - 1].length() > 0)
    {
        frame = &m_nextFrames[m_nextFrames.length() - 1];
    }
    else
    {
        frame = &m_currentFrame;
    }
    for (qint32 i = startCount; i > endCount; --i)
    {
        if (i - 1 < frame->size())
        {
            auto & sprites = (*frame)[i - 1];
            for (auto & sprite : sprites)
            {
                oxygine::spTween tween = oxygine::createTween(oxygine::Actor::TweenAlpha(0),
                                                              oxygine::timeMS(fadeoutTime  / static_cast<qint32>(Settings::getBattleAnimationSpeed())), 1, false,
                                                              oxygine::timeMS(count * frameDelay / static_cast<qint32>(Settings::getBattleAnimationSpeed())));
                sprite->addTween(tween);
            }
            loadSound("dying_fadeout.wav", 1, count * frameDelay);
            ++count;
        }
    }
    return count * frameDelay;
}

QPoint BattleAnimationSprite::getUnitPositionOffset(qint32 unitIdx)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getPositionOffset";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(m_pUnit.get()),
                       pInterpreter->newQObject(m_pTerrain),
                       unitIdx,
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + m_pUnit->getUnitID(), function1, args);
    return erg.toVariant().toPoint();
}

qint32 BattleAnimationSprite::getMaxUnitCount()
{
    if (m_maxUnitCount > 0)
    {
        return m_maxUnitCount;
    }
    return getAnimationUnitCount();
}

qint32 BattleAnimationSprite::getAnimationUnitCount()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getMaxUnitCount";
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + m_pUnit->getUnitID(), function1);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 1;
    }
}

qint32 BattleAnimationSprite::getImpactDurationMS(Unit* pUnit, Unit* pDefender, qint32 attackerWeapon)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getImpactDurationMS";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(pDefender),
                       attackerWeapon,
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + pUnit->getUnitID(), function1, args);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 1000;
    }
}

bool BattleAnimationSprite::hasMoveInAnimation(Unit* pUnit, Unit* pDefender, qint32 attackerWeapon)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "hasMoveInAnimation";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(pDefender),
                       attackerWeapon,
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + m_pUnit->getUnitID(), function1, args);
    if (erg.isBool())
    {
        return erg.toBool();
    }
    else
    {
        return false;
    }
}

qint32 BattleAnimationSprite::getDyingDurationMS(Unit* pUnit, Unit* pDefender, qint32 attackerWeapon)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getDyingDurationMS";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(pDefender),
                       attackerWeapon,
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + pUnit->getUnitID(), function1, args);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 0;
    }
}

bool BattleAnimationSprite::hasDyingAnimation()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "hasDyingAnimation";
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + m_pUnit->getUnitID(), function1);
    if (erg.isBool())
    {
        return erg.toBool();
    }
    else
    {
        return false;
    }
}

void BattleAnimationSprite::setMaxUnitCount(const qint32 &value)
{
    m_maxUnitCount = value;
}

qint32 BattleAnimationSprite::getFireDurationMS(Unit* pUnit, Unit* pDefender, qint32 attackerWeapon)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getFireDurationMS";
    QJSValueList args({pInterpreter->newQObject(this),
                          pInterpreter->newQObject(pUnit),
                          pInterpreter->newQObject(pDefender),
                          attackerWeapon,
                          pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + pUnit->getUnitID(), function1, args);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 500;
    }
}

qint32 BattleAnimationSprite::getFiredDurationMS(Unit* pUnit, Unit* pDefender, qint32 attackerWeapon)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getFiredDurationMS";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(pDefender),
                       attackerWeapon,
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + pUnit->getUnitID(), function1, args);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 500;
    }
}

qint32 BattleAnimationSprite::getMoveInDurationMS(Unit* pUnit, Unit* pDefender, qint32 attackerWeapon)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getMoveInDurationMS";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(pDefender),
                       attackerWeapon,
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + pUnit->getUnitID(), function1, args);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 500;
    }
}

qint32 BattleAnimationSprite::getStopDurationMS(Unit* pUnit, Unit* pDefender, qint32 attackerWeapon)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getStopDurationMS";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(pDefender),
                       attackerWeapon,
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction("BATTLEANIMATION_" + pUnit->getUnitID(), function1, args);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 500;
    }
}

QPoint BattleAnimationSprite::getUnitPosition(qint32 unitCount, qint32 maxUnitCount)
{
    QPoint ret = QPoint((unitCount * 70) % 100,
                        15 * (maxUnitCount - unitCount) + 8);
    return ret;
}

void BattleAnimationSprite::loadSprite(QString spriteID, bool addPlayerColor, qint32 maxUnitCount, QPoint offset,
                                       qint32 loops, float scale, short priority, qint32 showDelay,
                                       bool _invertFlipX, bool deleteAfter, qint32 frameTime, qint32 frames, qint32 startFrame)
{
    loadMovingSprite(spriteID, addPlayerColor, maxUnitCount, offset,
                     QPoint(0, 0), 0, deleteAfter,
                     loops, scale, priority, showDelay,
                     _invertFlipX, frameTime, frames, startFrame);
}

void BattleAnimationSprite::loadSpriteV2(QString spriteID, GameEnums::Recoloring mode, qint32 maxUnitCount, QPoint offset,
                                         qint32 loops, float scale, short priority, qint32 showDelay,
                                         bool _invertFlipX, bool deleteAfter, qint32 frameTime, qint32 frames, qint32 startFrame)
{
    loadMovingSpriteV2(spriteID, mode, maxUnitCount, offset,
                       QPoint(0, 0), 0, deleteAfter,
                       loops, scale, priority, showDelay,
                       _invertFlipX, frameTime, frames, startFrame);
}

qint32 BattleAnimationSprite::getUnitCount(qint32 maxUnitCount)
{
    return getUnitCount(maxUnitCount, m_hpRounded);
}

qint32 BattleAnimationSprite::getFireUnitCount(qint32 maxUnitCount)
{
    return getUnitCount(maxUnitCount, m_fireHp);
}

qint32 BattleAnimationSprite::getUnitCount(qint32 maxUnitCount, qint32 hp)
{
    qint32 count = GlobalUtils::roundUp(hp / Unit::MAX_UNIT_HP * maxUnitCount);
    if (count < 0)
    {
        count = 0;
    }
    return count;
}

void BattleAnimationSprite::loadMovingSprite(QString spriteID, bool addPlayerColor, qint32 maxUnitCount, QPoint offset,
                                             QPoint movement, qint32 moveTime, bool deleteAfter,
                                             qint32 loops, float scale, short priority, qint32 showDelay,
                                             bool _invertFlipX, qint32 frameTime, qint32 frames, qint32 startFrame)
{    
    if (addPlayerColor)
    {
        loadMovingSpriteV2(spriteID, GameEnums::Recoloring_Mask, maxUnitCount, offset, movement,
                           moveTime, deleteAfter, loops, scale, priority, showDelay, _invertFlipX, frameTime, frames, startFrame);
    }
    else
    {
        loadMovingSpriteV2(spriteID, GameEnums::Recoloring_None, maxUnitCount, offset, movement,
                           moveTime, deleteAfter, loops, scale, priority, showDelay, _invertFlipX, frameTime, frames, startFrame);
    }
}

void BattleAnimationSprite::loadMovingSpriteV2(QString spriteID, GameEnums::Recoloring mode, qint32 maxUnitCount, QPoint offset,
                                               QPoint movement, qint32 moveTime, bool deleteAfter,
                                               qint32 loops, float scale, short priority, qint32 showDelay,
                                               bool _invertFlipX, qint32 frameTime, qint32 frames, qint32 startFrame)
{
    qint32 value = getUnitCount(maxUnitCount);

    while (m_nextFrames[m_nextFrames.length() - 1].length() < value)
    {
        m_nextFrames[m_nextFrames.length() - 1].append(QVector<oxygine::spSprite>());
    }

    for (qint32 i = maxUnitCount; i >= maxUnitCount - value + 1; i--)
    {

        loadSingleMovingSpriteV2(spriteID, mode, offset + getUnitBasePosition(i, maxUnitCount, value), movement, moveTime, deleteAfter,
                                 loops, scale, i + priority, showDelay, _invertFlipX, frameTime, frames, startFrame);
        if (m_lastLoadedSprite.get() != nullptr)
        {
            m_nextFrames[m_nextFrames.length() - 1][maxUnitCount - i].append(m_lastLoadedSprite);
            m_lastLoadedSprite->detach();
        }
    }
}

void BattleAnimationSprite::loadDyingMovingSprite(QString livingSpriteId, QString dyingSpriteId, GameEnums::Recoloring mode, QPoint offset,
                           QPoint movement, float rotation, qint32 moveTime, short priority, qint32 firedFrame, qint32 maxUnitCount, qint32 showDelay)
{
    qint32 startUnits = getUnitCount(maxUnitCount, GlobalUtils::roundUp(m_dyingStartHp));
    qint32 endUnits = getUnitCount(maxUnitCount, GlobalUtils::roundUp(m_dyingEndHp));
    while (m_nextFrames[m_nextFrames.length() - 1].length() < startUnits)
    {
        m_nextFrames[m_nextFrames.length() - 1].append(QVector<oxygine::spSprite>());
    }
    for (qint32 i = maxUnitCount; i >= maxUnitCount - startUnits + 1; i--)
    {
        if (i < (maxUnitCount - endUnits + 1))
        {
            loadSingleMovingSpriteV2(dyingSpriteId, mode, offset + getUnitBasePosition(i, maxUnitCount, startUnits), movement, moveTime, true,
                                     1, 1.0f, i + priority, showDelay, false, GameMap::frameTime, -1, 0, rotation);
        }
        else
        {
            loadSingleMovingSpriteV2(livingSpriteId, mode, offset + getUnitBasePosition(i, maxUnitCount, startUnits), QPoint(0, 0), 0, false,
                                     1, 1.0f, i + priority, 0, false, GameMap::frameTime, firedFrame, firedFrame, 0);
        }

        if (m_lastLoadedSprite.get() != nullptr)
        {
            m_nextFrames[m_nextFrames.length() - 1][maxUnitCount - i].append(m_lastLoadedSprite);
            m_lastLoadedSprite->detach();
        }
    }
}


void BattleAnimationSprite::loadOnlyDyingMovingSprite(QString dyingSpriteId, GameEnums::Recoloring mode, QPoint offset, quint8 alpha,
                                                      QPoint movement, float rotation, qint32 moveTime, short priority,
                                                      qint32 firedFrame, qint32 maxUnitCount, qint32 showDelay)
{
    qint32 startUnits = getUnitCount(maxUnitCount, GlobalUtils::roundUp(m_dyingStartHp));
    qint32 endUnits = getUnitCount(maxUnitCount, GlobalUtils::roundUp(m_dyingEndHp));
    while (m_nextFrames[m_nextFrames.length() - 1].length() < startUnits)
    {
        m_nextFrames[m_nextFrames.length() - 1].append(QVector<oxygine::spSprite>());
    }
    for (qint32 i = maxUnitCount; i >= maxUnitCount - startUnits + 1; i--)
    {
        if (i < (maxUnitCount - endUnits + 1))
        {
            loadSingleMovingSpriteV2(dyingSpriteId, mode, offset + getUnitBasePosition(i, maxUnitCount, startUnits), movement, moveTime, true,
                                     1, 1.0f, i + priority, showDelay, false, GameMap::frameTime, -1, 0, rotation);
            if (m_lastLoadedSprite.get() != nullptr)
            {
                m_lastLoadedSprite->setAlpha(alpha);
                m_nextFrames[m_nextFrames.length() - 1][maxUnitCount - i].append(m_lastLoadedSprite);
                m_lastLoadedSprite->detach();
            }
        }
    }
}

QPoint BattleAnimationSprite::getUnitBasePosition(qint32 unit, qint32 maxUnitCount, qint32 unitsAlive)
{
    QPoint position(0, 0);
    if (maxUnitCount > 1)
    {
        qint32 offset = 0;
        if (m_invertStartPosition)
        {
            qint32 livingCount = getUnitCount(getAnimationUnitCount(), m_dyingStartHp);
            if (unitsAlive < livingCount)
            {
                offset = livingCount - unitsAlive;
            }
        }
        position = getUnitPosition(unit - offset, maxUnitCount);
    }
    QPoint posOffset = getUnitPositionOffset(unit);
    return position + posOffset;
}

void BattleAnimationSprite::loadSingleMovingSpriteV2(QString spriteID, GameEnums::Recoloring mode, QPoint offset,
                                                     QPoint movement, qint32 moveTime, bool deleteAfter,
                                                     qint32 loops, float scale, short priority, qint32 showDelay,
                                                     bool _invertFlipX, qint32 frameTime, qint32 frames, qint32 startFrame,
                                                     float rotation, quint8 alpha)
{    
    BattleAnimationManager* pBattleAnimationManager = BattleAnimationManager::getInstance();
    oxygine::ResAnim* pAnim = pBattleAnimationManager->getResAnim(spriteID, oxygine::ep_ignore_error);
    if (pAnim != nullptr)
    {
        loadSpriteInternal(pAnim, mode, offset, movement, moveTime, deleteAfter,
                           loops, scale, priority, showDelay, _invertFlipX, frameTime, frames, startFrame, rotation, alpha);
    }
    else
    {
        CONSOLE_PRINT("Unable to load battle sprite: " + spriteID, GameConsole::eDEBUG);
    }
}

void BattleAnimationSprite::loadSpriteInternal(oxygine::ResAnim* pAnim, GameEnums::Recoloring mode, QPoint offset,
                                               QPoint movement, qint32 moveTime, bool deleteAfter,
                                               qint32 loops, float scale, short priority, qint32 showDelay,
                                               bool _invertFlipX, qint32 frameTime, qint32 frames, qint32 startFrame,
                                               float rotation, quint8 alpha)
{
    oxygine::spSprite pSprite = oxygine::spSprite::create();
    if (pAnim != nullptr)
    {
        if (frames < 0)
        {
            frames = pAnim->getColumns() - 1;
        }
        if (frames > pAnim->getColumns() - 1)
        {
            frames = pAnim->getColumns() - 1;
        }
        if (startFrame < 0)
        {
            startFrame = 0;
        }
        if (startFrame > pAnim->getColumns() - 1)
        {
            startFrame = pAnim->getColumns() - 1;
        }
        oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim, startFrame, frames), oxygine::timeMS((frames - startFrame + 1) * frameTime), loops, false, oxygine::timeMS(static_cast<qint64>(showDelay / Settings::getBattleAnimationSpeed())));
        pSprite->addTween(tween);
        if (deleteAfter && moveTime <= 0)
        {
            tween->addDoneCallback([this](oxygine::Event * pEvent)
            {
                oxygine::spActor pTarget = oxygine::dynamic_pointer_cast<oxygine::Actor>(pEvent->target);
                if (pTarget.get() != nullptr)
                {
                    emit sigDetachChild(pTarget);
                }
            });
        }
        constexpr qint32 multiplier = 2;
        qint32 finalPriority = priority * multiplier;
        // repaint the unit?
        if (mode == GameEnums::Recoloring_Mask)
        {
            QColor color = m_pUnit->getOwner()->getColor();
            pSprite->setColor(color);
        }
        else if (mode == GameEnums::Recoloring_Table ||
                 mode == GameEnums::Recoloring_Matrix)
        {
            bool matrixMode = mode == GameEnums::Recoloring_Matrix;
            pSprite->setColorTable(m_pUnit->getOwner()->getColorTableAnim(), matrixMode);
        }
        else
        {
            finalPriority += 1;
        }
        pSprite->setPriority(finalPriority);
        pSprite->setScale(scale);
        pSprite->setSize(pAnim->getSize());
        pSprite->setInvertFlipX(_invertFlipX);
        pSprite->setAlpha(alpha);

        pSprite->setAnchor(0.5f, 0.5f);
        offset.setY(offset.y() - pAnim->getHeight() * 0.5f);

        qint32 xPos = 0;
        if (isFlippedX())
        {
            offset.setX(offset.x() - pAnim->getWidth() * 0.5f);
            xPos = offset.x();
            qint32 width = pAnim->getWidth() * scale;
            xPos = 127 - xPos - width;
        }
        else
        {
            offset.setX(offset.x() + pAnim->getWidth() * 0.5f);
            xPos = offset.x();
        }
        qint32 yPos = 192 - offset.y() - pAnim->getHeight() * scale;
        pSprite->setPosition(xPos , yPos);
        if (moveTime > 0)
        {
            qint32 endX = xPos + movement.x();
            if (isFlippedX())
            {
                endX = xPos - movement.x();
            }
            oxygine::spTween moveTween = oxygine::createTween(oxygine::Actor::TweenPosition(oxygine::Vector2(endX, yPos - movement.y())), oxygine::timeMS(static_cast<qint64>(moveTime / Settings::getBattleAnimationSpeed())), 1, false, oxygine::timeMS(static_cast<qint64>(showDelay / Settings::getBattleAnimationSpeed())));
            if (deleteAfter)
            {
                moveTween->addDoneCallback([this](oxygine::Event * pEvent)
                {
                    oxygine::spActor pTarget = oxygine::dynamic_pointer_cast<oxygine::Actor>(pEvent->target);
                    if (pTarget.get() != nullptr)
                    {
                        emit sigDetachChild(pTarget);
                    }
                });
            }
            pSprite->addTween(moveTween);
            if (rotation != 0)
            {
                if (isFlippedX())
                {
                    rotation = -rotation;
                }
                oxygine::spTween rotationTween = oxygine::createTween(oxygine::Actor::TweenRotation(rotation / 360.0f * 2.0f * M_PI), oxygine::timeMS(static_cast<qint64>(moveTime / Settings::getBattleAnimationSpeed())), 1, false, oxygine::timeMS(static_cast<qint64>(showDelay / Settings::getBattleAnimationSpeed())));
                pSprite->addTween(rotationTween);
            }
        }
        if (showDelay > 0)
        {
            oxygine::spTween visibileTween = oxygine::createTween(TweenToggleVisibility(0.96f, 1.0f), oxygine::timeMS(static_cast<qint64>(showDelay / Settings::getBattleAnimationSpeed())), 1);
            pSprite->addTween(visibileTween);
        }
    }
    m_Actor->addChild(pSprite);
    m_lastLoadedSprite = pSprite;
}

GameMap *BattleAnimationSprite::getMap() const
{
    return m_pMap;
}

void BattleAnimationSprite::loadCoMini(QString spriteID, GameEnums::Recoloring mode, QPoint offset,
                                       QPoint movement, qint32 moveTime, bool deleteAfter,
                                       qint32 loops, float scale, short priority, qint32 showDelay,
                                       bool _invertFlipX, qint32 frameTime, qint32 frames, qint32 startFrame,
                                       float rotation, quint8 alpha)
{
    COSpriteManager* pCOSpriteManager = COSpriteManager::getInstance();
    oxygine::ResAnim* pAnim = pCOSpriteManager->getResAnim(spriteID, oxygine::ep_ignore_error);
    if (pAnim != nullptr)
    {
        loadSpriteInternal(pAnim, mode, offset, movement, moveTime, deleteAfter,
                           loops, scale, priority, showDelay, _invertFlipX, frameTime, frames, startFrame, rotation, alpha);
    }
    else
    {
        CONSOLE_PRINT("Unable to load battle sprite: " + spriteID, GameConsole::eDEBUG);
    }
}

void BattleAnimationSprite::loadSingleMovingSprite(QString spriteID, bool addPlayerColor, QPoint offset,
                                                   QPoint movement, qint32 moveTime, bool deleteAfter,
                                                   qint32 loops, float scale, short priority, qint32 showDelay,
                                                   bool _invertFlipX, qint32 frameTime, qint32 frames, qint32 startFrame,
                                                   float rotation, quint8 alpha)
{
    if (addPlayerColor)
    {
        loadSingleMovingSpriteV2(spriteID, GameEnums::Recoloring_Mask, offset, movement,
                                 moveTime, deleteAfter, loops, scale, priority, showDelay,
                                 _invertFlipX, frameTime, frames, startFrame,
                                 rotation, alpha);
    }
    else
    {
        loadSingleMovingSpriteV2(spriteID, GameEnums::Recoloring_None, offset, movement,
                                 moveTime, deleteAfter, loops, scale, priority, showDelay,
                                 _invertFlipX, frameTime, frames, startFrame,
                                 rotation, alpha);
    }
}

bool BattleAnimationSprite::existResAnim(QString spriteID)
{
    BattleAnimationManager* pBattleAnimationManager = BattleAnimationManager::getInstance();
    oxygine::ResAnim* pAnim = pBattleAnimationManager->getResAnim(spriteID, oxygine::ep_ignore_error);
    if (pAnim == nullptr)
    {
        COSpriteManager* pCOSpriteManager = COSpriteManager::getInstance();
        oxygine::ResAnim* pAnim = pCOSpriteManager->getResAnim(spriteID, oxygine::ep_ignore_error);
        return (pAnim != nullptr);
    }
    return true;
}

void BattleAnimationSprite::addMoveTweenToLastLoadedSprites(qint32 deltaX, qint32 deltaY, qint32 moveTime, qint32 delayPerUnitMs, qint32 loops, bool scaleWithAnimationSpeed)
{
    if (m_nextFrames.size() > 0)
    {
        qint32 count = 0;
        for (auto & sprites : m_nextFrames[m_nextFrames.size() - 1])
        {
            if (sprites.size() > 0)
            {
                auto & sprite = sprites[sprites.size() - 1];
                qint64 time = moveTime;
                if (scaleWithAnimationSpeed)
                {
                    time /= Settings::getBattleAnimationSpeed();
                }
                oxygine::spTween moveTween = oxygine::createTween(oxygine::Actor::TweenPosition(oxygine::Vector2(sprite->getX() + deltaX, sprite->getY() + deltaY)),
                                                                  oxygine::timeMS(time),
                                                                  loops, true);
                sprite->addTween(moveTween);
                moveTween->setElapsed(oxygine::timeMS(delayPerUnitMs * count));
                ++count;
            }
        }
    }
}


void BattleAnimationSprite::loadColorOverlayForLastLoadedFrame(QColor color, qint32 time, qint32 loops, qint32 showDelayMs)
{
    QVector<QVector<oxygine::spSprite>>* frame;
    if (m_nextFrames.length() > 0 && m_nextFrames[m_nextFrames.length() - 1].length() > 0)
    {
        frame = &m_nextFrames[m_nextFrames.length() - 1];
    }
    else
    {
        frame = &m_currentFrame;
    }
    qint32 value = getUnitCount(m_maxUnitCount);
    for (qint32 i = m_maxUnitCount; i >= m_maxUnitCount - value + 1; i--)
    {
        if (i - 1 < frame->length() && i > 0)
        {
            for (auto & sprite : (*frame)[i - 1])
            {
                // add impact image
                oxygine::ColorRectSprite::TweenColor tweenColor2(color);
                oxygine::spTween colorTween2 = oxygine::createTween(tweenColor2, oxygine::timeMS(static_cast<qint64>(time / Settings::getBattleAnimationSpeed())), loops, true,
                                                                    oxygine::timeMS(static_cast<qint64>((showDelayMs + m_nextFrameTimer.interval() * (i - m_maxUnitCount)) / Settings::getBattleAnimationSpeed())));
                sprite->addTween(colorTween2);
            }
        }
    }
}

void BattleAnimationSprite::addUnitshakeToLastLoadedFrame(qint32 startIntensity, float decay, qint32 durationMs, qint32 delayMs, qint32 shakePauseMs)
{
    QVector<QVector<oxygine::spSprite>>* frame;
    if (m_nextFrames.length() > 0 && m_nextFrames[m_nextFrames.length() - 1].length() > 0)
    {
        frame = &m_nextFrames[m_nextFrames.length() - 1];
    }
    else
    {
        frame = &m_currentFrame;
    }
    qint32 value = getUnitCount(m_maxUnitCount);
    for (qint32 i = m_maxUnitCount; i >= m_maxUnitCount - value + 1; i--)
    {
        if (i - 1 < frame->length() && i > 0)
        {
            for (auto & sprite : (*frame)[i - 1])
            {
                // add impact image
                oxygine::spTween tween = oxygine::createTween(TweenScreenshake(startIntensity, decay / Settings::getBattleAnimationSpeed(), oxygine::timeMS(shakePauseMs)),
                                                                               oxygine::timeMS(static_cast<qint64>(durationMs / Settings::getBattleAnimationSpeed())), 1, false, oxygine::timeMS(static_cast<qint64>(delayMs / Settings::getBattleAnimationSpeed())));
                sprite->addTween(tween);
            }
        }
    }
}

void BattleAnimationSprite::detachChild(oxygine::spActor pActor)
{
    
    pActor->detach();
    
}

qint32 BattleAnimationSprite::getHpRounded() const
{
    return m_hpRounded;
}

void BattleAnimationSprite::setHpRounded(const qint32 &value)
{
    m_hpRounded = value;
}

void BattleAnimationSprite::loadSound(QString file, qint32 loops, qint32 delay, float volume, bool stopOldestSound)
{
    if (m_playSound)
    {
        Mainapp* pApp = Mainapp::getInstance();
        AudioManager* pAudio = pApp->getAudioManager();
        SoundData data;
        data.sound = file;
        data.loops = loops;
        m_Sounds.append(data);
        pAudio->playSound(file, loops, delay / static_cast<qint32>(Settings::getBattleAnimationSpeed()), volume, stopOldestSound);
    }
}

void BattleAnimationSprite::stopSound(bool forceStop)
{
    Mainapp* pApp = Mainapp::getInstance();
    AudioManager* pAudio = pApp->getAudioManager();
    qint32 i = 0;
    while (i < m_Sounds.size())
    {
        if (m_Sounds[i].loops < 0 || forceStop ||
            Settings::getBattleAnimationSpeedValue() > 10)
        {
            pAudio->stopSound(m_Sounds[i].sound);
            m_Sounds.removeAt(i);
        }
        else
        {
            ++i;
        }
    }
}

void BattleAnimationSprite::setUnitFrameDelay(qint32 delay)
{
    m_nextFrameTimer.setInterval(delay / static_cast<qint32>(Settings::getBattleAnimationSpeed()));
}

void BattleAnimationSprite::startNextFrame()
{
    startNextUnitFrames();
}

void BattleAnimationSprite::startNextUnitFrames()
{
    CONSOLE_PRINT("Progressing next battle frame current=" + QString::number(m_currentFrame.size()) +
                  " next frames=" + QString::number(m_nextFrames.length()) +
                  " frame iterator=" + QString::number(m_frameIterator) +
                  " owner=" + QString::number(m_pUnit->getOwner()->getPlayerID()), GameConsole::eDEBUG);
    if (m_currentFrame.size() == 0 && !m_startWithFraming)
    {
        // add initial frames
        for (auto & unitFrame : m_nextFrames[0])
        {
            m_currentFrame.append(QVector<oxygine::spSprite>());
            for (auto & sprite : unitFrame)
            {
                m_Actor->addChild(sprite);
                m_currentFrame[m_currentFrame.length() - 1].append(sprite);
            }
        }
        m_nextFrames.removeFirst();
    }
    else
    {
        if (m_frameIterator < m_currentFrame.size())
        {
            for (auto & sprite : m_currentFrame[m_frameIterator])
            {
                sprite->detach();
            }
            m_currentFrame[m_frameIterator].clear();
        }
        if (m_currentFrame.length() <= m_frameIterator)
        {
            m_currentFrame.append(QVector<oxygine::spSprite>());
        }

        if (m_nextFrames.length() > 0)
        {
            if (m_frameIterator < m_nextFrames[0].length())
            {
                for (auto & sprite : m_nextFrames[0][m_frameIterator])
                {
                    m_currentFrame[m_frameIterator].append(sprite);
                    m_Actor->addChild(sprite);
                }
                m_nextFrames[0][m_frameIterator].clear();
            }
        }
    }

    if (m_nextFrames.length() > 0)
    {
        ++m_frameIterator;
        if (m_frameIterator >= m_nextFrames[0].size())
        {
            CONSOLE_PRINT("Progressing next battle animation", GameConsole::eDEBUG);
            m_frameIterator = 0;
            m_nextFrames.removeFirst();
            if (m_nextFrames.size() > 0 && m_playNextFrame)
            {
                if (m_frameIterator < m_nextFrames[0].size())
                {
                    m_nextFrameTimer.start();
                }
            }
        }
        else if (m_nextFrames.size() > 0)
        {
            if (m_frameIterator < m_nextFrames[0].size())
            {
                m_nextFrameTimer.start();
            }
        }
        else
        {
            m_frameIterator = 0;
        }
    }
}

bool BattleAnimationSprite::getPlayNextFrame() const
{
    return m_playNextFrame;
}

void BattleAnimationSprite::setPlayNextFrame(bool newPlayNextFrame)
{
    m_playNextFrame = newPlayNextFrame;
}

bool BattleAnimationSprite::getHasFired() const
{
    return m_hasFired;
}

void BattleAnimationSprite::setHasFired(bool newHasFired)
{
    m_hasFired = newHasFired;
}

void BattleAnimationSprite::setBackgroundSprite(oxygine::spSprite newBackgroundSprite)
{
    m_pBackgroundSprite = newBackgroundSprite;
}

float BattleAnimationSprite::getBackgroundSpeed()
{
    return oxygine::safeSpCast<oxygine::SlidingSprite>(m_pBackgroundSprite->getFirstChild())->getSpeedX();
}

void BattleAnimationSprite::setBackgroundSpeed(float speed)
{
    m_backgroundSpeed = getBackgroundSpeed();
    auto & childs = m_pBackgroundSprite->getChildren();
    for (auto & child : childs)
    {
        auto sprite = oxygine::dynamic_pointer_cast<oxygine::SlidingSprite>(child);
        if (sprite.get() != nullptr)
        {
            sprite->setSpeedX(speed);
        }
    }
}

void BattleAnimationSprite::restoreBackgroundSpeed()
{
    if (m_backgroundSpeed != 0.0f)
    {
        float backup = m_backgroundSpeed;
        setBackgroundSpeed(m_backgroundSpeed);
        m_backgroundSpeed = backup;
    }
}

BattleAnimationSprite *BattleAnimationSprite::getEnemySprite() const
{
    return m_EnemySprite;
}

void BattleAnimationSprite::setEnemySprite(BattleAnimationSprite *newEnemySprite)
{
    m_EnemySprite = newEnemySprite;
}

bool BattleAnimationSprite::getInvertStartPosition() const
{
    return m_invertStartPosition;
}

void BattleAnimationSprite::setInvertStartPosition(bool invertStartPosition)
{
    m_invertStartPosition = invertStartPosition;
}

float BattleAnimationSprite::getDyingEndHp() const
{
    return m_dyingEndHp;
}

void BattleAnimationSprite::setDyingEndHp(float dyingEndHp)
{
    m_dyingEndHp = dyingEndHp;
}

float BattleAnimationSprite::getDyingStartHp() const
{
    return m_dyingStartHp;
}

void BattleAnimationSprite::setDyingStartHp(float dyingStartHp)
{
    m_dyingStartHp = dyingStartHp;
}

bool BattleAnimationSprite::getStartWithFraming() const
{
    return m_startWithFraming;
}

void BattleAnimationSprite::setStartWithFraming(bool startWithFraming)
{
    m_startWithFraming = startWithFraming;
}

void BattleAnimationSprite::addScreenshake(qint32 startIntensity, float decay, qint32 durationMs, qint32 delayMs, qint32 shakePauseMs)
{
    auto pOwner = dynamic_cast<BattleAnimation*>(getParent());
    if (pOwner != nullptr)
    {
        pOwner->addScreenshake(startIntensity, decay * Settings::getAnimationSpeed() / Settings::getBattleAnimationSpeed(),
                               durationMs  * Settings::getAnimationSpeed() / Settings::getBattleAnimationSpeed(),
                               delayMs  * Settings::getAnimationSpeed() / Settings::getBattleAnimationSpeed(), shakePauseMs);
    }
}

void BattleAnimationSprite::addSpriteScreenshake(qint32 startIntensity, float decay, qint32 durationMs, qint32 delayMs, qint32 shakePauseMs)
{
    oxygine::spTween tween = oxygine::createTween(TweenScreenshake(startIntensity, decay / Settings::getBattleAnimationSpeed(), oxygine::timeMS(shakePauseMs)),
                                                                   oxygine::timeMS(static_cast<qint64>(durationMs / Settings::getBattleAnimationSpeed())), 1, false, oxygine::timeMS(static_cast<qint64>(delayMs / Settings::getBattleAnimationSpeed())));
    getParent()->addTween(tween);
}

void BattleAnimationSprite::addSpriteShakeY(qint32 startIntensity, float startPercent, float endPercent, qint32 durationMs, qint32 delayMs, qint32 loops)
{
    oxygine::spTween tween = oxygine::createTween(TweenShakeY(startIntensity, startPercent, endPercent),
                                                                   oxygine::timeMS(static_cast<qint64>(durationMs / Settings::getBattleAnimationSpeed())), loops, false, oxygine::timeMS(static_cast<qint64>(delayMs / Settings::getBattleAnimationSpeed())));
    getParent()->addTween(tween);
}

void BattleAnimationSprite::addBattleViewScreenshake(qint32 startIntensity, float decay, qint32 durationMs, qint32 delayMs, qint32 shakePauseMs)
{
    auto pOwner = dynamic_cast<BattleAnimation*>(getParent());
    if (pOwner != nullptr)
    {
        pOwner->addBattleViewScreenshake(startIntensity, decay, durationMs, delayMs, shakePauseMs);
    }
}
