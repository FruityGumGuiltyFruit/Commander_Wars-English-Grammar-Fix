#include <QFileInfo>
#include <QFile>

#include "coreengine/console.h"
#include "coreengine/mainapp.h"

#include "resource_management/terrainmanager.h"
#include "resource_management/gameanimationmanager.h"

#include "game/terrain.h"
#include "game/gamemap.h"
#include "game/terrainfindingsystem.h"
#include "game/player.h"
#include "game/co.h"

spTerrain Terrain::createTerrain(const QString & terrainID, qint32 x, qint32 y, const QString & currentTerrainID, GameMap* pMap)
{
    spTerrain pTerrain = spTerrain::create(terrainID, x, y, pMap);
    pTerrain->setSize(GameMap::getImageSize(), GameMap::getImageSize());
    if (terrainID != "")
    {
        pTerrain->createBaseTerrain(currentTerrainID);
    }
    // check if the js-script was loaded already
    // otherwise do load it
    bool terrainExists = true;
    if (terrainID != "")
    {
        Interpreter* pInterpreter = Interpreter::getInstance();
        QJSValue obj = pInterpreter->getGlobal(terrainID);
        if (!obj.isObject())
        {
            terrainExists = false;
        }
        if (terrainExists)
        {
            pTerrain->init();
        }
        else
        {
            CONSOLE_PRINT("Unable to load Terrain " + terrainID, Console::eFATAL);
        }
    }
    return pTerrain;
}

Terrain::Terrain(QString terrainID, qint32 x, qint32 y, GameMap* pMap)
    : m_terrainID(terrainID),
      m_x(x),
      m_y(y),
      m_Building{nullptr},
      m_pMap(pMap)
{
    setObjectName("Terrain");
    Mainapp* pApp = Mainapp::getInstance();
    moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);
    setPriority(getMapTerrainDrawPriority());
    setSize(GameMap::getImageSize(),
            GameMap::getImageSize());
}

Terrain::~Terrain()
{
    if (m_Unit.get())
    {
        m_Unit->detach();
    }
    if (m_Building.get())
    {
        m_Building->detach();
    }
}

bool Terrain::getShowInEditor(QString terrainId)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getShowInEditor";

    QJSValue ret = pInterpreter->doFunction(terrainId, function1);
    if (ret.isBool())
    {
        return ret.toBool();
    }
    return true;
}

GameMap *Terrain::getMap() const
{
    return m_pMap;
}

bool Terrain::getHasFlowDirection() const
{
    return m_hasFlowDirection;
}

void Terrain::setHasFlowDirection(bool newHasFlowDirection)
{
    m_hasFlowDirection = newHasFlowDirection;
}

bool Terrain::getHasStartOfTurn() const
{
    return m_hasStartOfTurn;
}

void Terrain::setHasStartOfTurn(bool hasStartOfTurn)
{
    m_hasStartOfTurn = hasStartOfTurn;
}

QString Terrain::getTerrainDescription() const
{
    return m_terrainDescription;
}

void Terrain::setTerrainDescription(const QString &terrainDescription)
{
    m_terrainDescription = terrainDescription;
}

qint32 Terrain::getVisionHigh() const
{
    return m_VisionHigh;
}

void Terrain::setVisionHigh(const qint32 &VisionHigh)
{
    m_VisionHigh = VisionHigh;
}

qint32 Terrain::getTotalVisionHigh()
{
    qint32 high = m_VisionHigh;
    if (m_Building.get() != nullptr)
    {
        high += m_Building->getVisionHigh();
    }
    if (high < 0)
    {
        high = 0;
    }
    return high;
}

bool Terrain::isValid()
{
    return TerrainManager::getInstance()->exists(m_terrainID);
}

void Terrain::setSpriteVisibility(bool value)
{
    if (m_pBaseTerrain.get() != nullptr)
    {
        m_pBaseTerrain->setSpriteVisibility(value);
    }
    m_pTerrainSprite->setVisible(value);
    for (auto& sprite : m_pOverlaySprites)
    {
        sprite->setVisible(value);
    }
}

QString Terrain::getTerrainSpriteName() const
{
    return m_terrainSpriteName;
}

void Terrain::setTerrainSpriteName(const QString &terrainSpriteName)
{
    m_terrainSpriteName = terrainSpriteName;
}

bool Terrain::getFixedSprite() const
{
    return m_FixedSprite;
}

void Terrain::setFixedSprite(bool FixedSprite)
{
    m_FixedSprite = FixedSprite;
}

void Terrain::init()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    if (m_pBaseTerrain.get() != nullptr)
    {
        m_pBaseTerrain->init();
    }
    else
    {
        createBaseTerrain(GameMap::PLAINS);
    }
    QString function = "init";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(m_pMap)});
    pInterpreter->doFunction(m_terrainID, function, args);
}

qint32 Terrain::getHp() const
{
    return m_hp;
}

void Terrain::setHp(const qint32 &value)
{
    m_hp = value;
}

QString Terrain::getTerrainName() const
{
    return m_terrainName;
}

void Terrain::setTerrainName(const QString &value)
{
    m_terrainName = value;
}

void Terrain::syncAnimation(oxygine::timeMS syncTime)
{
    if (m_pTerrainSprite.get() != nullptr)
    {
        auto & tweens = m_pTerrainSprite->getTweens();
        for (auto & pTween : tweens)
        {
            pTween->setElapsed(syncTime);
        }
    }
    for (auto & sprites : m_pOverlaySprites)
    {
        auto & tweens = sprites->getTweens();
        for (auto & pTween : tweens)
        {
            pTween->setElapsed(syncTime);
        }
    }
    if (m_pBaseTerrain.get() != nullptr)
    {
        m_pBaseTerrain->syncAnimation(syncTime);
    }
}

Unit* Terrain::getUnit()
{
    return m_Unit.get();
}

Building* Terrain::getBuilding()
{
    return m_Building.get();
}

spBuilding Terrain::getSpBuilding()
{
    return m_Building;
}

void Terrain::createBaseTerrain(const QString & currentTerrainID)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValueList args({pInterpreter->newQObject(this),
                       currentTerrainID,
                       pInterpreter->newQObject(m_pMap)});
    // load sprite of the base terrain
    QString function = "loadBaseTerrain";
    pInterpreter->doFunction(m_terrainID, function, args);
    if (m_pBaseTerrain.get() != nullptr)
    {
        m_pBaseTerrain->createBaseTerrain(currentTerrainID);
    }
}

qint32 Terrain::getTerrainGroup()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getTerrainGroup";
    QJSValueList args({pInterpreter->newQObject(m_pMap)});
    QJSValue ret = pInterpreter->doFunction(m_terrainID, function1, args);
    if (ret.isNumber())
    {
        return ret.toInt();
    }
    return 0;
}

QString Terrain::getWeatherOverlayId()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    // load sprite of the base terrain
    QString function = "getWeatherOverlayId";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue ret = pInterpreter->doFunction(m_terrainID, function, args);
    if (ret.isString())
    {
        return ret.toString();
    }
    return "";
}

QPoint Terrain::getWeatherOverlaySpeed()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    // load sprite of the base terrain
    QString function = "getWeatherOverlaySpeed";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue ret = pInterpreter->doFunction(m_terrainID, function, args);
    return ret.toVariant().toPoint();
}

QString Terrain::getDescription()
{
    if (m_terrainDescription.isEmpty())
    {
        Interpreter* pInterpreter = Interpreter::getInstance();
        // load sprite of the base terrain
        QString function = "getDescription";
        QJSValueList args({pInterpreter->newQObject(this),
                          pInterpreter->newQObject(m_pMap)});
        QJSValue ret = pInterpreter->doFunction(m_terrainID, function, args);
        if (ret.isString())
        {
            return ret.toString();
        }
    }
    return m_terrainDescription;
}

void Terrain::setBaseTerrain(spTerrain terrain)
{
    if (m_pBaseTerrain.get() != nullptr)
    {
        m_pBaseTerrain->detach();
        m_pBaseTerrain = nullptr;
    }
    m_pBaseTerrain = terrain;
    m_pBaseTerrain->setPriority(static_cast<qint32>(DrawPriority::Terrain));
    m_pBaseTerrain->setPosition(0, 0);
    addChild(m_pBaseTerrain);
}

void Terrain::unloadSprites()
{
    // unload old stuff
    if (m_pTerrainSprite.get() != nullptr)
    {
        m_pTerrainSprite->detach();
        m_pTerrainSprite = nullptr;
    }
    if (m_pOverlaySprites.size() > 0)
    {
        for (qint32 i = 0; i < m_pOverlaySprites.size(); i++)
        {
            m_pOverlaySprites[i]->detach();
        }
        m_pOverlaySprites.clear();
    }
}

void Terrain::loadBaseTerrainSprites()
{
    // load sub terrain
    if (m_pBaseTerrain.get() != nullptr)
    {
        m_pBaseTerrain->loadSprites();
    }
}

void Terrain::loadSprites()
{
    unloadSprites();
    loadBaseTerrainSprites();
    Interpreter* pInterpreter = Interpreter::getInstance();
    if (m_FixedSprite && customSpriteExists())
    {
        loadBaseSprite(m_terrainSpriteName);
    }
    else
    {
        QString function1 = "loadBaseSprite";
        QJSValueList args({pInterpreter->newQObject(this),
                           pInterpreter->newQObject(m_pMap)});
        pInterpreter->doFunction(m_terrainID, function1, args);
    }
    // ony load this for valid positions
    if (m_x >= 0 && m_y >= 0)
    {
        // next call starting by 0 again
        QString function2 = "loadOverlaySprite";
        QJSValueList args({pInterpreter->newQObject(this),
                           pInterpreter->newQObject(m_pMap)});
        pInterpreter->doFunction(m_terrainID, function2, args);
    }
}

void Terrain::loadBaseTerrain(const QString & terrainID)
{
    m_pBaseTerrain = spTerrain::create(terrainID, m_x, m_y, m_pMap);
    m_pBaseTerrain->setPriority(static_cast<qint32>(DrawPriority::Terrain));
    m_pBaseTerrain->setPosition(0, 0);
    addChild(m_pBaseTerrain);
}

void Terrain::loadBaseSprite(const QString & spriteID, qint32 frameTime)
{
    TerrainManager* pTerrainManager = TerrainManager::getInstance();
    oxygine::ResAnim* pAnim = pTerrainManager->getResAnim(spriteID, oxygine::error_policy::ep_ignore_error);
    if (pAnim != nullptr)
    {
        oxygine::spSprite pSprite = oxygine::spSprite::create();
        if (pAnim->getTotalFrames() > 1)
        {
            oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), oxygine::timeMS(pAnim->getTotalFrames() * frameTime), -1);
            pSprite->addTween(tween);
        }
        else
        {
            pSprite->setResAnim(pAnim);
        }
        pSprite->setScale((GameMap::getImageSize()) / pAnim->getWidth());
        pSprite->setPosition(-(pSprite->getScaledWidth() - GameMap::getImageSize()) / 2, -(pSprite->getScaledHeight() - GameMap::getImageSize()));
        pSprite->setPriority(static_cast<qint32>(DrawPriority::Terrain));
        addChild(pSprite);
        m_terrainSpriteName = spriteID;
        m_pTerrainSprite = pSprite;
    }
    else if (customSpriteExists())
    {
        oxygine::spSprite pSprite = oxygine::spSprite::create();
        pSprite->setPosition(-(pSprite->getScaledWidth() - GameMap::getImageSize()) / 2, -(pSprite->getScaledHeight() - GameMap::getImageSize()));
        addChild(pSprite);
        m_terrainSpriteName = spriteID;
        QImage img;
        if (QFile::exists(Settings::getUserPath() + m_terrainSpriteName))
        {
            img = QImage(Settings::getUserPath() + m_terrainSpriteName);
        }
        else
        {
            img = QImage(oxygine::Resource::RCC_PREFIX_PATH + m_terrainSpriteName);
        }
        oxygine::spSingleResAnim pAnim = oxygine::spSingleResAnim::create();
        Mainapp::getInstance()->loadResAnim(pAnim, img, 1, 1, 1, false);
        m_SpriteAnim = pAnim;
        pSprite->setResAnim(pAnim.get());
        if (pAnim.get() != nullptr)
        {
            pSprite->setScale((GameMap::getImageSize()) / pAnim->getWidth() );
        }
        pSprite->setPosition(-(pSprite->getScaledWidth() - GameMap::getImageSize()) / 2, -(pSprite->getScaledHeight() - GameMap::getImageSize()));
        pSprite->setPriority(static_cast<qint32>(DrawPriority::Terrain));
        m_pTerrainSprite = pSprite;
    }
    else
    {
        CONSOLE_PRINT("Unable to load terrain sprite: " + spriteID, Console::eDEBUG);
    }
}

bool Terrain::customSpriteExists() const
{
    TerrainManager* pTerrainManager = TerrainManager::getInstance();
    oxygine::ResAnim* pAnim = pTerrainManager->getResAnim(m_terrainSpriteName, oxygine::error_policy::ep_ignore_error);
    return pAnim != nullptr ||
           QFile::exists(Settings::getUserPath() + m_terrainSpriteName) ||
           QFile::exists(oxygine::Resource::RCC_PREFIX_PATH + m_terrainSpriteName);
}

void Terrain::updateFlowSprites(TerrainFindingSystem* pPfs)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "updateFlowSprites";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pPfs),
                       pInterpreter->newQObject(m_pMap)});
    pInterpreter->doFunction(m_terrainID, function1, args);
}

QStringList Terrain::getFlowTiles()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getFlowTiles";
    QJSValueList args({pInterpreter->newQObject(m_pMap)});
    QJSValue ret = pInterpreter->doFunction(m_terrainID, function1, args);
    return ret.toVariant().toStringList();
}

bool Terrain::existsResAnim(const QString & spriteId) const
{
    TerrainManager* pTerrainManager = TerrainManager::getInstance();
    return pTerrainManager->getResAnim(spriteId, oxygine::error_policy::ep_ignore_error) != nullptr;
}

QString Terrain::getFittingResAnim(const QString & spriteIdStart, const QString & spriteIdEnd) const
{
    QString ret;
    TerrainManager* pTerrainManager = TerrainManager::getInstance();
    ret = pTerrainManager->getFittingResAnim(spriteIdStart, spriteIdEnd);
    return ret;
}

QString Terrain::getSurroundings(const QString & list, bool useBaseTerrainID, bool blacklist, qint32 searchType, bool useMapBorder, bool useBuildingID, qint32 recursionCount, bool inverted)
{
    QStringList searchList = list.split(",");
    QString ret = "";
    for (qint32 i = 0; i < 8; i++)
    {
        qint32 curX = m_x;
        qint32 curY = m_y;
        // get our x, y coordinates
        GameMap::getField(curX, curY, static_cast<GameEnums::Directions>(i));
        bool found = false;
        QString addString = "";
        // load compare value
        GameEnums::Directions compareValue = GameEnums::Directions_None;
        if (searchType == GameEnums::Directions_All)
        {
            compareValue = static_cast<GameEnums::Directions>(i);
        }
        else if (searchType == GameEnums::Directions_Direct)
        {
            switch (i)
            {
                case GameEnums::Directions_North:
                case GameEnums::Directions_East:
                case GameEnums::Directions_West:
                case GameEnums::Directions_South:
                {
                    compareValue = static_cast<GameEnums::Directions>(i);
                    break;
                }
                default:
                {
                    // do nothing
                    compareValue = GameEnums::Directions_None;
                    break;
                }
            }
        }
        else if (searchType == GameEnums::Directions_Diagnonal)
        {
            switch (i)
            {
                case GameEnums::Directions_NorthEast:
                case GameEnums::Directions_NorthWest:
                case GameEnums::Directions_SouthWest:
                case GameEnums::Directions_SouthEast:
                {
                    compareValue = static_cast<GameEnums::Directions>(i);
                    break;
                }
                default:
                {
                    compareValue = GameEnums::Directions_None;
                    // do nothing
                    break;
                }
            }
        }
        else if (searchType == i)
        {
            compareValue = static_cast<GameEnums::Directions>(i);
        }
        else
        {
            // you asshole reached unreachable code :D
        }
        // check for compare value to find string
        if (compareValue == GameEnums::Directions_North)
        {
            if (inverted)
            {
                addString = "+S";
            }
            else
            {
                addString = "+N";
            }
        }
        else if (compareValue == GameEnums::Directions_East)
        {
            if (inverted)
            {
                addString = "+W";
            }
            else
            {
                addString = "+E";
            }
        }
        else if (compareValue == GameEnums::Directions_South)
        {
            if (inverted)
            {
                addString = "+N";
            }
            else
            {
                addString = "+S";
            }
        }
        else if (compareValue == GameEnums::Directions_West)
        {
            if (inverted)
            {
                addString = "+E";
            }
            else
            {
                addString = "+W";
            }
        }
        else if (compareValue == GameEnums::Directions_NorthEast)
        {
            if (inverted)
            {
                addString = "+SW";
            }
            else
            {
                addString = "+NE";
            }
        }
        else if (compareValue == GameEnums::Directions_SouthEast)
        {
            if (inverted)
            {
                addString = "+NW";
            }
            else
            {
                addString = "+SE";
            }
        }
        else if (compareValue == GameEnums::Directions_SouthWest)
        {
            if (inverted)
            {
                addString = "+NE";
            }
            else
            {
                addString = "+SW";
            }
        }
        else if (compareValue == GameEnums::Directions_NorthWest)
        {
            if (inverted)
            {
                addString = "+SE";
            }
            else
            {
                addString = "+NW";
            }
        }
        else
        {
            // do nothing
        }
        if (m_pMap != nullptr && m_pMap->onMap(curX, curY))
        {
            QString terrainID = "";
            QString neighbourID = "";
            Terrain* pTerrain = m_pMap->getTerrain(curX, curY);
            terrainID = pTerrain->getTerrainID();
            if (useBuildingID && pTerrain->getBuilding() != nullptr)
            {
                terrainID = pTerrain->getID();
            }
            if (useBaseTerrainID)
            {
                if (recursionCount > 0)
                {
                    neighbourID = pTerrain->getBaseTerrainIDOfLevel(recursionCount);
                }
                else
                {
                    neighbourID = pTerrain->getBaseTerrainID();
                }
            }
            if (blacklist)
            {
                if (!searchList.contains(terrainID) &&
                    !searchList.contains(neighbourID))
                {
                    found = true;
                }
            }
            else
            {
                if (searchList.contains(terrainID) ||
                    searchList.contains(neighbourID))
                {
                    found = true;
                }
            }
        }
        else if (useMapBorder)
        {
            found = true;
        }
        else
        {
            found = false;
        }
        if (found)
        {
            ret += addString;
        }
    }
    return ret;
}

void Terrain::loadOverlaySprite(const QString & spriteID)
{
    TerrainManager* pTerrainManager = TerrainManager::getInstance();
    oxygine::ResAnim* pAnim = pTerrainManager->getResAnim(spriteID, oxygine::ep_ignore_error);
    oxygine::spSprite pSprite = oxygine::spSprite::create();
    if (pAnim != nullptr)
    {
        if (pAnim->getTotalFrames() > 1)
        {
            oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), oxygine::timeMS(pAnim->getTotalFrames() * GameMap::frameTime), -1);
            pSprite->addTween(tween);
        }
        else
        {
            pSprite->setResAnim(pAnim);
        }
        pSprite->setScale((GameMap::getImageSize()) / pAnim->getWidth());
    }
    else
    {
        CONSOLE_PRINT("Unable to load overlay sprite: " + spriteID, Console::eDEBUG);
    }
    pSprite->setPosition(-(pSprite->getScaledWidth() - GameMap::getImageSize()) / 2, -(pSprite->getScaledHeight() - GameMap::getImageSize()));
    pSprite->setPriority(static_cast<qint32>(DrawPriority::TerrainOverlay));
    addChild(pSprite);
    m_pOverlaySprites.append(pSprite);
}

qint32 Terrain::getBaseDefense()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getDefense";
    qint32 defense = 0;
    if (m_Building.get() == nullptr)
    {
        QJSValueList args({pInterpreter->newQObject(this),
                          pInterpreter->newQObject(m_pMap)});
        QJSValue ret = pInterpreter->doFunction(m_terrainID, function1, args);
        if (ret.isNumber())
        {
            defense = ret.toInt();
        }
    }
    else
    {
        QJSValueList args({pInterpreter->newQObject(m_Building.get()),
                           pInterpreter->newQObject(m_pMap)});
        QJSValue ret = pInterpreter->doFunction(m_Building->getBuildingID(), function1, args);
        if (ret.isNumber())
        {
            defense = ret.toInt();
        }
    }
    return defense;
}

QString Terrain::getMinimapIcon()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getMiniMapIcon";
    QJSValueList args({pInterpreter->newQObject(this),
                      pInterpreter->newQObject(m_pMap)});
    QJSValue ret = pInterpreter->doFunction(m_terrainID, function1, args);
    if (ret.isString())
    {
        return ret.toString();
    }
    else
    {
        return "";
    }
}

qint32 Terrain::getMovementcostModifier(Unit* pUnit, qint32 x, qint32 y, qint32 curX, qint32 curY)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getMovementcostModifier";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       x,
                       y,
                       curX,
                       curY,
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 0;
    }
}

qint32 Terrain::getFirerangeModifier(Unit* pUnit)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getFirerangeModifier";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 0;
    }
}

qint32 Terrain::getMinFirerangeModifier(Unit* pUnit)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getMinFirerangeModifier";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return 0;
    }
}

qint32 Terrain::getDefense(Unit* pUnit)
{
    qint32 defense = getBaseDefense();
    if (pUnit != nullptr)
    {
        if (!pUnit->useTerrainDefense())
        {
            return 0;
        }
        defense += pUnit->getTerrainDefenseModifier(QPoint(m_x, m_y));
        
        for (qint32 i = 0; i < m_pMap->getPlayerCount(); i++)
        {
            Player* pPlayer = m_pMap->getPlayer(i);
            if (pPlayer->isEnemyUnit(pUnit) && !pPlayer->getIsDefeated())
            {
                CO* pCO = pPlayer->getCO(0);
                if (pCO != nullptr)
                {
                    defense += pCO->getEnemyTerrainDefenseModifier(pUnit, QPoint(m_x, m_y));
                }
                pCO = pPlayer->getCO(1);
                if (pCO != nullptr)
                {
                    defense += pCO->getEnemyTerrainDefenseModifier(pUnit, QPoint(m_x, m_y));
                }
            }
        }
    }
    // defense can't be negative
    if (defense < 0)
    {
        defense = 0;
    }
    return defense;
}

void Terrain::setBuilding(spBuilding pBuilding)
{
    if (m_Building.get() != nullptr)
    {
        removeBuilding();
    }
    if (pBuilding.get() != nullptr)
    {
        m_Building = pBuilding;
        if (m_x >= 0 && m_y >= 0)
        {
            pBuilding->setTerrain(m_pMap->getTerrain(Terrain::m_x, Terrain::m_y));
        }
        addBuildingSprite(m_Building);
        if (m_x >= 0 && m_y >= 0)
        {
            createBuildingDownStream();
        }
    }
}

void Terrain::addBuildingSprite(spBuilding pBuilding)
{
    if (m_pMap != nullptr && m_Building->usesMapLayer())
    {
        m_Building->setPriority(getMapTerrainDrawPriority() + static_cast<qint32>(ExtraDrawPriority::BuildingLayer));
        m_Building->setPosition(Terrain::m_x * GameMap::getImageSize(), Terrain::m_y * GameMap::getImageSize());
        if (Terrain::m_y >= 0)
        {
            m_pMap->getRowActor(Terrain::m_y)->addChild(m_Building);
        }
    }
    else
    {
        m_Building->setPosition(0, 0);
        m_Building->setPriority(static_cast<qint32>(DrawPriority::Building));
        addChild(m_Building);
    }
}

void Terrain::removeBuilding()
{
    if (m_Building.get() != nullptr)
    {
        if (m_Building->getTerrain() == this)
        {
            // remove it
            m_Building->detach();
            qint32 width = m_Building->getBuildingWidth();
            qint32 heigth = m_Building->getBuildingHeigth();
            
            // remove pointers
            for (qint32 x1 = 0; x1 < width; x1++)
            {
                for (qint32 y1 = 0; y1 < heigth; y1++)
                {
                    // remove down stream on all other fields
                    if (!((x1 == 0) && (y1 == 0)) &&
                        m_pMap->onMap(m_x - x1, m_y - y1))
                    {
                        m_pMap->getTerrain(m_x - x1, m_y - y1)->removeDownstream();
                    }
                }
            }
            m_Building = nullptr;
        }
        else
        {
            // remove building from base terrain instead of us
            Terrain* pTerrain = m_Building->getTerrain();
            if (pTerrain != nullptr)
            {
                pTerrain->removeBuilding();
            }
        }
    }
}

void Terrain::setSpBuilding(spBuilding pBuilding, bool OnlyDownStream)
{
    if (m_Building.get() != nullptr)
    {
        removeBuilding();
    }
    if (pBuilding.get() != nullptr)
    {
        m_Building = pBuilding;
        if (!OnlyDownStream)
        {
            m_Building->setTerrain(m_pMap->getTerrain(Terrain::m_x, Terrain::m_y));
            addBuildingSprite(m_Building);
        }
    }
}

qint32 Terrain::getMapTerrainDrawPriority()
{
    return static_cast<qint32>(Mainapp::ZOrder::Terrain) + static_cast<qint32>(Terrain::m_y);
}

void Terrain::loadBuilding(const QString & buildingID)
{
    if (m_Building.get() != nullptr)
    {
        removeBuilding();
    }
    m_Building = spBuilding::create(buildingID, m_pMap);
    m_Building->updateBuildingSprites(false);
    m_Building->setTerrain(m_pMap->getTerrain(Terrain::m_x, Terrain::m_y));
    addBuildingSprite(m_Building);
    createBuildingDownStream();
}

void Terrain::setUnit(spUnit pUnit)
{
    // remove current unit on this field
    if (m_Unit.get() != nullptr)
    {
        m_Unit->setTerrain(nullptr);
        m_Unit->detach();
        m_Unit = nullptr;
    }
    if (pUnit.get() != nullptr)
    {
        // add unit to this terrain
        m_Unit = pUnit;
        // remove it from last terrain
        Terrain* pTerrain = m_Unit->getTerrain();
        if (pTerrain != nullptr)
        {
            pTerrain->setUnit(spUnit());
        }
        // add Terrain to unit and unit to drawing actor
        pUnit->setPriority(getMapTerrainDrawPriority() + static_cast<qint32>(ExtraDrawPriority::UnitLayer));
        pUnit->setTerrain(m_pMap->getTerrain(Terrain::m_x, Terrain::m_y));
        pUnit->setPosition(Terrain::m_x * GameMap::getImageSize(), Terrain::m_y * GameMap::getImageSize());

        
        if (m_pMap)
        {
            Player* pPlayer = m_pMap->getCurrentViewPlayer();
            Player* pCurrentPlayer = m_pMap->getCurrentPlayer();
            if (pPlayer != pCurrentPlayer &&
                pPlayer != nullptr &&
                pPlayer->getFieldVisibleLoaded() &&
                !pPlayer->getFieldVisible(Terrain::m_x, Terrain::m_y))
            {
                pUnit->setVisible(false);
            }
            m_pMap->addChild(pUnit);
        }
    }
}

spUnit Terrain::getSpUnit()
{
    return m_Unit;
}

QString Terrain::getID()
{
    if (m_Building.get() == nullptr)
    {
        return m_terrainID;
    }
    else
    {
        return m_Building->getBuildingID();
    }
}

QString Terrain::getTerrainAnimationBase()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getTerrainAnimationBase";
    QJSValueList args({pInterpreter->newQObject(nullptr),
                       pInterpreter->newQObject(this),
                       pInterpreter->newQObject(nullptr),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    if (erg.isString())
    {
        return erg.toString();
    }
    else
    {
        return "";
    }
}

QString Terrain::getTerrainAnimationForeground()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getTerrainAnimationForeground";
    QJSValueList args({pInterpreter->newQObject(nullptr),
                       pInterpreter->newQObject(this),
                       pInterpreter->newQObject(nullptr),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    if (erg.isString())
    {
        return erg.toString();
    }
    else
    {
        return "";
    }
}

QString Terrain::getTerrainAnimationBackground()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getTerrainAnimationBackground";
    QJSValueList args({pInterpreter->newQObject(nullptr),
                       pInterpreter->newQObject(this),
                       pInterpreter->newQObject(nullptr),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    if (erg.isString())
    {
        return erg.toString();
    }
    else
    {
        return "";
    }
}

float Terrain::getTerrainAnimationMoveSpeed()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getTerrainAnimationMoveSpeed";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    if (erg.isNumber())
    {
        return erg.toNumber();
    }
    else
    {
        return 0.0f;
    }
}

QStringList Terrain::getTerrainSprites()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getTerrainSprites";
    QJSValueList args({pInterpreter->newQObject(m_pMap)});
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    return erg.toVariant().toStringList();
}

QString Terrain::getTerrainID() const
{
    return m_terrainID;
}

qint32 Terrain::getX() const
{
    return m_x;
}

void Terrain::setX(const qint32 &value)
{
    m_x = value;
}

qint32 Terrain::getY() const
{
    return m_y;
}

void Terrain::setY(const qint32 &value)
{
    m_y = value;
}

qint32 Terrain::getVision(Player* pPlayer)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getVision";
    QJSValueList args({pInterpreter->newQObject(pPlayer),
                       pInterpreter->newQObject(this),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue ret = pInterpreter->doFunction(m_terrainID, function1, args);
    if (ret.isNumber())
    {
        return ret.toInt();
    }
    else
    {
        return 0;
    }
}

TerrainFindingSystem* Terrain::createTerrainFindingSystem()
{
    TerrainFindingSystem* pPfs = new TerrainFindingSystem(m_pMap, getID(), Terrain::getX(), Terrain::getY());
    pPfs->explore();
    return pPfs;
}

bool Terrain::getVisionHide(Player* pPlayer)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getVisionHide";
    QJSValueList args({pInterpreter->newQObject(this),
                      pInterpreter->newQObject(pPlayer),
                      pInterpreter->newQObject(m_pMap)});
    QJSValue ret = pInterpreter->doFunction(m_terrainID, function1, args);
    
    if (ret.isBool() && m_pMap != nullptr)
    {
        bool value = ret.toBool();
        if (!value &&
            m_Building.get() != nullptr &&
            pPlayer != nullptr &&
            pPlayer->isEnemy(m_Building->getOwner()) &&
            m_pMap->getGameRules()->getBuildingVisionHide())
        {
            value = m_Building->getVisionHide();
        }
        if (value && pPlayer != nullptr)
        {
            CO* pCO = pPlayer->getCO(0);
            if (pCO != nullptr)
            {
                if (pCO->getPerfectVision())
                {
                    return false;
                }
            }
            pCO = pPlayer->getCO(1);
            if (pCO != nullptr)
            {
                if (pCO->getPerfectVision())
                {
                    return false;
                }
            }
        }
        return value;
    }
    else
    {
        return false;
    }
}

qint32 Terrain::getBonusVision(Unit* pUnit)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getBonusVision";
    QJSValueList args({pInterpreter->newQObject(pUnit),
                       pInterpreter->newQObject(this),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue ret = pInterpreter->doFunction(m_terrainID, function1, args);
    if (ret.isNumber())
    {
        return ret.toInt();
    }
    else
    {
        return 0;
    }
}

bool Terrain::isLoadingTile()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "isLoadingTile";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(m_pMap)});
    QJSValue ret = pInterpreter->doFunction(m_terrainID, function1, args);
    if (ret.isBool())
    {
        return ret.toBool();
    }
    else
    {
        return false;
    }
}

void Terrain::startOfTurn()
{
    if (m_hasStartOfTurn)
    {
        Interpreter* pInterpreter = Interpreter::getInstance();
        QString function1 = "startOfTurn";
        QJSValueList args({pInterpreter->newQObject(this),
                          pInterpreter->newQObject(m_pMap)});
        pInterpreter->doFunction(m_terrainID, function1, args);
    }
    for (auto & item : m_terrainOverlay)
    {
        if (item.duration > 0)
        {
            item.duration -= 1;
            if (item.duration <= 0)
            {
                removeTerrainOverlay(item.resAnim);
            }
        }
    }
}

qint32 Terrain::getOffensiveFieldBonus(GameAction* pAction, Unit* pAttacker, QPoint atkPosition,Unit* pDefender,  QPoint defPosition, bool isDefender, GameEnums::LuckDamageMode luckMode)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getOffensiveFieldBonus";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pAttacker),
                       atkPosition.x(),
                       atkPosition.y(),
                       pInterpreter->newQObject(pDefender),
                       defPosition.x(),
                       defPosition.y(),
                       isDefender,
                       pInterpreter->newQObject(pAction),
                       luckMode,
                       pInterpreter->newQObject(m_pMap)});
    qint32 ergValue = 0;
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    if (erg.isNumber())
    {
        ergValue += erg.toInt();
    }
    return ergValue;
}

qint32 Terrain::getDeffensiveFieldBonus(GameAction* pAction, Unit* pAttacker, QPoint atkPosition, Unit* pDefender, QPoint defPosition, bool isAttacker, GameEnums::LuckDamageMode luckMode)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getDeffensiveFieldBonus";
    QJSValueList args({pInterpreter->newQObject(this),
                       pInterpreter->newQObject(pAttacker),
                       atkPosition.x(),
                       atkPosition.y(),
                       pInterpreter->newQObject(pDefender),
                       defPosition.x(),
                       defPosition.y(),
                       isAttacker,
                       pInterpreter->newQObject(pAction),
                       luckMode,
                       pInterpreter->newQObject(m_pMap)});
    qint32 ergValue = 0;
    QJSValue erg = pInterpreter->doFunction(m_terrainID, function1, args);
    if (erg.isNumber())
    {
        ergValue += erg.toInt();
    }
    return ergValue;
}

void Terrain::addTerrainOverlay(QString id, qint32 x, qint32 y, QColor color, qint32 duration, float scale)
{
    bool found = false;
    for (auto & item : m_terrainOverlay)
    {
        if (item.resAnim == id)
        {
            found = true;
            if ((item.duration < 0 && duration > 0) ||
                (duration < 0 && item.duration > 0) ||
                (duration > item.duration))
            {
                item.duration = duration;
            }
        }
    }
    if (!found)
    {
        TerrainOverlay item;
        item.duration = duration;
        item.resAnim = id;
        item.scale = scale;
        item.offset = QPoint(x, y);
        item.color = color;
        oxygine::spSprite pSprite = oxygine::spSprite::create();
        pSprite->setPosition(x, y);
        pSprite->setColor(color);
        oxygine::ResAnim* pAnim = GameAnimationManager::getInstance()->getResAnim(id, oxygine::ep_ignore_error);
        if (pAnim != nullptr)
        {
            if (pAnim->getTotalFrames() > 1)
            {
                oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), oxygine::timeMS(pAnim->getTotalFrames() * GameMap::frameTime), -1);
                pSprite->addTween(tween);
            }
            else
            {
                pSprite->setResAnim(pAnim);
            }
        }
        pSprite->setPriority(static_cast<qint32>(DrawPriority::TerrainMarker));
        pSprite->setScale(scale);
        pSprite->setResAnim(pAnim);
        item.sprite = pSprite;
        addChild(pSprite);
        m_terrainOverlay.append(item);
    }
}

void Terrain::removeTerrainOverlay(const QString & id)
{
    for (qint32 i = 0; i < m_terrainOverlay.size(); ++i)
    {
        if (m_terrainOverlay[i].resAnim == id)
        {
            m_terrainOverlay[i].sprite->detach();
            m_terrainOverlay.removeAt(i);
        }
    }
}

void Terrain::serializeObject(QDataStream& pStream) const
{
    pStream << getVersion();

    pStream << m_terrainSpriteName;
    pStream << m_FixedSprite;

    pStream << m_terrainID;
    if (m_pBaseTerrain.get() == nullptr)
    {
        pStream << false;
    }
    else
    {
        pStream << true;
        m_pBaseTerrain->serializeObject(pStream);
    }
    if (m_Building.get() == nullptr)
    {
        pStream << false;
    }
    else
    {
        if (m_Building->getTerrain() == this)
        {
            pStream << true;
            m_Building->serializeObject(pStream);
        }
        else
        {
            pStream << false;
        }
    }
    if (m_Unit.get() == nullptr)
    {
        pStream << false;
    }
    else
    {
        pStream << true;
        m_Unit->serializeObject(pStream);
    }
    pStream << m_hp;

    pStream << m_terrainName;
    pStream << m_terrainDescription;
    m_Variables.serializeObject(pStream);

    pStream << static_cast<qint32>(m_terrainOverlay.size());
    for (auto & item : m_terrainOverlay)
    {
        pStream << item.duration;
        pStream << item.resAnim;
        pStream << item.scale;
        pStream << static_cast<qint32>(item.offset.x());
        pStream << static_cast<qint32>(item.offset.y());
        quint32 color = item.color.rgba();
        pStream << color;
    }
}

void Terrain::deserializeObject(QDataStream& pStream)
{
    deserializer(pStream, false);
}

void Terrain::deserializer(QDataStream& pStream, bool fast)
{
    qint32 version = 0;
    pStream >> version;
    if (version > 3)
    {
        pStream >> m_terrainSpriteName;
        pStream >> m_FixedSprite;
    }
    if (version > 2)
    {
        pStream >> m_terrainID;
    }
    else
    {
        char* id;
        pStream >> id;
        m_terrainID = id;
    }
    if (!fast)
    {
        init();
    }
    bool hasBaseTerrain = false;
    pStream >> hasBaseTerrain;
    if (hasBaseTerrain)
    {        
        m_pBaseTerrain = createTerrain("", m_x, m_y, "", m_pMap);
        m_pBaseTerrain->deserializer(pStream, fast);
        if (!fast)
        {
            m_pBaseTerrain->setPriority(static_cast<qint32>(DrawPriority::Terrain));
            m_pBaseTerrain->setPosition(0, 0);
            if (m_pBaseTerrain->isValid())
            {
                addChild(m_pBaseTerrain);
            }
            else
            {
                m_pBaseTerrain = nullptr;
            }
        }
    }
    bool hasBuilding = false;
    pStream >> hasBuilding;
    if (hasBuilding)
    {
        m_Building = spBuilding::create("", m_pMap);
        m_Building->deserializer(pStream, fast);

        if (m_Building->isValid())
        {
            if (!fast)
            {
                m_Building->setPosition(Terrain::m_x * GameMap::getImageSize(), Terrain::m_y * GameMap::getImageSize());
                m_Building->setPriority(getMapTerrainDrawPriority() + static_cast<qint32>(ExtraDrawPriority::BuildingLayer));
                m_pMap->getRowActor(Terrain::m_y)->addChild(m_Building);
            }
            m_Building->setTerrain(m_pMap->getTerrain(Terrain::m_x, Terrain::m_y));
            createBuildingDownStream();
        }
        else
        {
            m_Building = nullptr;
        }
    }
    bool hasUnit = false;
    pStream >> hasUnit;
    if (hasUnit)
    {
        m_Unit = spUnit::create("", nullptr, false, m_pMap);
        m_Unit->deserializer(pStream, fast);
        if (m_Unit->isValid())
        {
            if (!fast)
            {
                setUnit(m_Unit);
            }
        }
        else
        {
            m_Unit = nullptr;
        }
    }
    if (version > 1)
    {
        qint32 newHp = 0;
        pStream >> newHp;
        if (newHp > 0 && m_hp > 0)
        {
            m_hp = newHp;
        }
    }
    if (version > 4)
    {
        pStream >> m_terrainName;
        pStream >> m_terrainDescription;
    }
    if (version > 5 && version < 9)
    {
        bool dummy;
        pStream >> dummy;
    }
    if (version > 6)
    {
        m_Variables.deserializeObject(pStream);
    }
    if (version > 7)
    {
        qint32 size = 0;
        pStream >> size;
        for (qint32 i = 0; i < size; ++i)
        {
            TerrainOverlay item;
            pStream >> item.duration;
            pStream >> item.resAnim;
            pStream >> item.scale;
            qint32 value = 0;
            pStream >> value;
            item.offset.setX(value);
            pStream >> value;
            item.offset.setY(value);
            quint32 color;
            pStream >> color;
            item.color = QColor::fromRgba(color);
            addTerrainOverlay(item.resAnim, item.offset.x(), item.offset.y(), item.color, item.duration, item.scale);
        }
    }
}

void Terrain::createBuildingDownStream()
{
    qint32 width = m_Building->getBuildingWidth();
    qint32 heigth = m_Building->getBuildingHeigth();
    
    // recreate pointers
    for (qint32 x1 = 0; x1 < width; x1++)
    {
        for (qint32 y1 = 0; y1 < heigth; y1++)
        {
            // create down stream on all other fields
            if (!((x1 == 0) && (y1 == 0)))
            {
                auto* pTerrain = m_pMap->getTerrain(m_x - x1, m_y - y1);
                if (pTerrain != nullptr)
                {
                    pTerrain->setSpBuilding(m_Building, true);
                }
            }
        }
    }
}

void Terrain::removeDownstream()
{
    m_Building = nullptr;
}
