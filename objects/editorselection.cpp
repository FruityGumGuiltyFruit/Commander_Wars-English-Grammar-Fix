#include "3rd_party/oxygine-framework/oxygine/actor/SlidingActor.h"

#include "objects/editorselection.h"
#include "objects/base/moveinbutton.h"
#include "objects/base/label.h"

#include "resource_management/objectmanager.h"
#include "resource_management/terrainmanager.h"
#include "resource_management/buildingspritemanager.h"
#include "resource_management/unitspritemanager.h"
#include "resource_management/movementtablemanager.h"
#include "resource_management/fontmanager.h"

#include "coreengine/mainapp.h"

#include "game/co.h"

const float EditorSelection::m_xFactor = 1.5f;
const float EditorSelection::m_yFactor = 2.5f;

EditorSelection::EditorSelection(qint32 width, bool smallScreen, GameMap* pMap)
    : m_pMap(pMap)
{
#ifdef GRAPHICSUPPORT
    setObjectName("EditorSelection");
#endif
    Interpreter::setCppOwnerShip(this);
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    setWidth(width);
    setHeight(Settings::getHeight() - 80);
    setPosition(Settings::getWidth() - width, 80);
    if (smallScreen)
    {
        m_BoxPlacementSize = createV9Box(0, m_startHPlacementSize, width / 2, m_selectionHeight);
        m_BoxSelectionType = createV9Box(0, m_startHSelectionType, width / 2, m_selectionHeight);
        m_BoxSelectedPlayer = createV9Box(0, m_startHSelectedPlayer, width / 2, m_selectionHeight + GameMap::getImageSize());
        m_BoxPlacementSelection = createV9Box(width / 2, m_startHPlacementSize, width / 2, Settings::getHeight() -  80);
    }
    else
    {
        m_BoxPlacementSize = createV9Box(0, m_startHPlacementSize, width, m_selectionHeight);
        m_BoxSelectionType = createV9Box(0, m_startHSelectionType, width, m_selectionHeight);
        m_BoxSelectedPlayer = createV9Box(0, m_startHSelectedPlayer, width, m_selectionHeight + GameMap::getImageSize());
        m_BoxPlacementSelection = createV9Box(0, m_startHTerrain, width, Settings::getHeight() - m_startHTerrain - 80);
    }

    m_PlacementSelectionClip = oxygine::spSlidingActor::create();
    m_PlacementSelectionClip->setPosition(10, 50);
    m_PlacementSelectionClip->setSize(m_BoxPlacementSelection->getScaledWidth() - 20,
                                        m_BoxPlacementSelection->getScaledHeight() - 100);
    m_BoxPlacementSelection->addChild(m_PlacementSelectionClip);
    m_PlacementActor = oxygine::spActor::create();
    m_PlacementActor->setWidth(m_PlacementSelectionClip->getScaledWidth());
    m_PlacementActor->setY(-GameMap::getImageSize());
    m_PlacementSelectionClip->setContent(m_PlacementActor);

    m_labelWidth = m_PlacementSelectionClip->getScaledWidth() - GameMap::getImageSize() - m_frameSize - m_frameSize;
    m_xCount = m_labelWidth / (GameMap::getImageSize() * m_xFactor) + 1;
    createBoxPlacementSize();
    createBoxSelectionMode();
    createPlayerSelection();

    m_CurrentSelector = oxygine::spSprite::create();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("editor+selector");
    m_CurrentSelector->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    m_CurrentSelector->setScale(GameMap::getImageSize() / pAnim->getWidth());
    if (pAnim->getTotalFrames() > 1)
    {
        oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), oxygine::timeMS(pAnim->getTotalFrames() * GameMap::frameTime * 2), -1);
        m_CurrentSelector->addTween(tween);
    }
    else
    {
        m_CurrentSelector->setResAnim(pAnim);
    }

    oxygine::spButton pButtonTop = oxygine::spButton::create();
    pButtonTop->setResAnim(ObjectManager::getInstance()->getResAnim("arrow+down"));
    pButtonTop->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    oxygine::Sprite* ptr = pButtonTop.get();
    pButtonTop->addEventListener(oxygine::TouchEvent::OVER, [ptr](oxygine::Event*)
    {
        ptr->addTween(oxygine::Sprite::TweenAddColor(QColor(16, 16, 16, 0)), oxygine::timeMS(300));
    });

    pButtonTop->addEventListener(oxygine::TouchEvent::OUTX, [ptr](oxygine::Event*)
    {
        ptr->addTween(oxygine::Sprite::TweenAddColor(QColor(0, 0, 0, 0)), oxygine::timeMS(300));
    });
    pButtonTop->setFlippedY(true);
    pButtonTop->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        emit sigChangeScrollValue(1);
    });
    pButtonTop->setPosition(m_BoxPlacementSelection->getScaledWidth() / 2 - pButtonTop->getScaledWidth() / 2, 15);
    m_BoxPlacementSelection->addChild(pButtonTop);

    oxygine::spButton pButtonDown = oxygine::spButton::create();
    pButtonDown->setResAnim(ObjectManager::getInstance()->getResAnim("arrow+down"));
    pButtonDown->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    ptr = pButtonDown.get();
    pButtonDown->addEventListener(oxygine::TouchEvent::OVER, [ptr](oxygine::Event*)
    {
        ptr->addTween(oxygine::Sprite::TweenAddColor(QColor(16, 16, 16, 0)), oxygine::timeMS(300));
    });

    pButtonDown->addEventListener(oxygine::TouchEvent::OUTX, [ptr](oxygine::Event*)
    {
        ptr->addTween(oxygine::Sprite::TweenAddColor(QColor(0, 0, 0, 0)), oxygine::timeMS(300));
    });
    pButtonDown->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        emit sigChangeScrollValue(-1);
    });
    m_BoxPlacementSelection->addEventListener(oxygine::TouchEvent::WHEEL_DIR, [this](oxygine::Event* pEvent)
    {
        oxygine::TouchEvent* pTouchEvent = oxygine::safeCast<oxygine::TouchEvent*>(pEvent);
        if (pTouchEvent != nullptr)
        {
           emit sigChangeScrollValue(pTouchEvent->wheelDirection.y);
           pTouchEvent->stopPropagation();
        }
    });
    pButtonDown->setPosition(m_BoxPlacementSelection->getScaledWidth() / 2 - pButtonTop->getScaledWidth() / 2,
                             m_BoxPlacementSelection->getScaledHeight() - pButtonDown->getScaledHeight() - 18);
    m_BoxPlacementSelection->addChild(pButtonDown);

    m_PlacementActor->addChild(m_CurrentSelector);
    m_CurrentSelector->setPosition(m_frameSize, m_startH);
    m_CurrentSelector->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));

    // create terrains
    TerrainManager* pTerrainManager = TerrainManager::getInstance();
    // reserve vector size for fun and speed :D
    m_Terrains.reserve(pTerrainManager->getCount());
    QStringList sortedTerrainIDs = pTerrainManager->getTerrainsSorted();
    for (const auto& terrainId : qAsConst(sortedTerrainIDs))
    {
        if (Terrain::getShowInEditor(terrainId))
        {
            spTerrain pTerrain = Terrain::createTerrain(terrainId, -10, -10, "", m_pMap);
            pTerrain->setTooltipText(pTerrain->getTerrainName());
            m_Terrains.append(pTerrain);
            m_Terrains[m_Terrains.size() - 1]->loadSprites();
            pTerrain->setPriority(static_cast<qint32>(Mainapp::ZOrder::Terrain));
            m_PlacementActor->addChild(m_Terrains[m_Terrains.size() - 1]);
        }
    }

    connect(this, &EditorSelection::sigUpdateSelectedPlayer, this, &EditorSelection::updateSelectedPlayer, Qt::QueuedConnection);
    connect(this, &EditorSelection::sigChangeSelectedPlayer, this, &EditorSelection::changeSelectedPlayer, Qt::QueuedConnection);
    connect(this, &EditorSelection::sigUpdateUnitView, this, &EditorSelection::updateUnitView, Qt::QueuedConnection);
    connect(this, &EditorSelection::sigUpdateTerrainView, this, &EditorSelection::updateTerrainView, Qt::QueuedConnection);
    connect(this, &EditorSelection::sigUpdateBuildingView, this, &EditorSelection::updateBuildingView, Qt::QueuedConnection);
    connect(this, &EditorSelection::sigChangeScrollValue, this, &EditorSelection::changeScrollValue, Qt::QueuedConnection);

    // load other sprites not shown in the starting screen
    BuildingSpriteManager* pBuildingSpriteManager = BuildingSpriteManager::getInstance();
    QStringList sortedBuildingIDs = pBuildingSpriteManager->getBuildingsSorted();
    for (const auto & buildingId : qAsConst(sortedBuildingIDs))
    {
        if (Building::getShowInEditor(buildingId))
        {
            spBuilding building = spBuilding::create(buildingId, m_pMap);
            building->setTooltipText(building->getName());
            qint32 width = building->getBuildingWidth();
            qint32 heigth = building->getBuildingHeigth();
            building->setScaleX(1.0f / static_cast<float>(width));
            building->setScaleY(1.0f / static_cast<float>(heigth));
            spTerrain pSprite = Terrain::createTerrain(building->getBaseTerrain()[0], -1, -1, "", m_pMap);
            pSprite->loadSprites();
            pSprite->setPriority(static_cast<qint32>(Mainapp::ZOrder::Terrain));
            pSprite->setScaleX(1 / building->getScaleX());
            pSprite->setScaleY(1 / building->getScaleY());
            if (width > 1)
            {
                pSprite->oxygine::Actor::setX(-GameMap::getImageSize() * (width - 1));
            }
            if (heigth > 1)
            {
                pSprite->oxygine::Actor::setY(-GameMap::getImageSize() * (heigth - 1));
            }
            m_Buildings.append(building);
            qint32 index = m_Buildings.size() - 1;
            m_Buildings[index]->updateBuildingSprites(false);
            m_Buildings[index]->addChild(pSprite);
            m_Buildings[index]->setVisible(false);
            m_PlacementActor->addChild(m_Buildings[index]);
        }
    }

    // load other sprites not shown in the starting screen
    UnitSpriteManager* pUnitSpriteManager = UnitSpriteManager::getInstance();
    MovementTableManager* pMovementTableManager = MovementTableManager::getInstance();

    spTerrain plains = Terrain::createTerrain(GameMap::PLAINS, -1, -1, "", m_pMap);
    spTerrain sea = Terrain::createTerrain("SEA", -1, -1, "", m_pMap);

    QStringList sortedUnits = pUnitSpriteManager->getUnitsSorted();
    for (const auto& unitId : sortedUnits)
    {
        if (Unit::getShowInEditor(unitId))
        {
            spUnit unit = spUnit::create(unitId, m_Players.at(1)->getOwner(), false, m_pMap);
            unit->setTooltipText(unit->getName());
            m_Units.append(unit);
            oxygine::spSprite pSprite = oxygine::spSprite::create();
            QString movementType = unit->getMovementType();
            if (pMovementTableManager->getBaseMovementPoints(movementType, plains.get(), plains.get(), unit.get()) > 0)
            {
                pAnim = pTerrainManager->getResAnim("plains+0");
                pSprite->setResAnim(pAnim);
            }
            else if (pMovementTableManager->getBaseMovementPoints(movementType, sea.get(), sea.get(), unit.get()) > 0)
            {
                pAnim = pTerrainManager->getResAnim("SEA");
                pSprite->setResAnim(pAnim);
            }
            else
            {
                pAnim = pTerrainManager->getResAnim("plains+0");
                pSprite->setResAnim(pAnim);
            }
            pSprite->setPriority(-100);
            pSprite->setScale(GameMap::getImageSize() / pAnim->getWidth());
            unit->addChild(pSprite);
            unit->setVisible(false);
            m_PlacementActor->addChild(unit);
        }
    }

    initSelection();
    // select terrains view
    updateTerrainView();

    if (Settings::getSmallScreenDevice())
    {
        setX(Settings::getWidth() - 1);
        addChild(spMoveInButton::create(this, getScaledWidth()));
    }
}

void EditorSelection::changeScrollValue(qint32 dir)
{
    if (dir > 0)
    {
        m_PlacementActor->setY(m_PlacementActor->getY() + GameMap::getImageSize());
        if (m_PlacementActor->getY() > -GameMap::getImageSize())
        {
            m_PlacementActor->setY(-GameMap::getImageSize());
        }
    }
    else
    {
        m_PlacementActor->setY(m_PlacementActor->getY() - GameMap::getImageSize());
        if (m_PlacementActor->getScaledHeight() < m_PlacementSelectionClip->getScaledHeight())
        {
            m_PlacementActor->setY(-GameMap::getImageSize());
        }
        else if (m_PlacementActor->getY() < -m_PlacementActor->getScaledHeight() + m_PlacementSelectionClip->getScaledHeight())
        {
            m_PlacementActor->setY(-m_PlacementActor->getScaledHeight() + m_PlacementSelectionClip->getScaledHeight());
        }
    }
}

void EditorSelection::createBoxPlacementSize()
{
    qint32 yStartPos = 10;
    qint32 xChange = 40;
    ObjectManager* pObjectManager = ObjectManager::getInstance();

    m_CurrentSelectorSize = oxygine::spSprite::create();
    oxygine::ResAnim* pAnimMarker = pObjectManager->getResAnim("editor+selector");
    m_CurrentSelectorSize->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    if (pAnimMarker->getTotalFrames() > 1)
    {
        oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnimMarker), oxygine::timeMS(pAnimMarker->getTotalFrames() * GameMap::frameTime * 2), -1);
        m_CurrentSelectorSize->addTween(tween);
    }
    else
    {
        m_CurrentSelectorSize->setResAnim(pAnimMarker);
    }
    m_BoxPlacementSize->addChild(m_CurrentSelectorSize);
    m_CurrentSelectorSize->setPosition(m_frameSize + xChange, yStartPos);

    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("editor+none");
    oxygine::spSprite pSpriteNone = oxygine::spSprite::create();
    pSpriteNone->setResAnim(pAnim);
    pSpriteNone->setPosition(m_frameSize, yStartPos);
    m_BoxPlacementSize->addChild(pSpriteNone);
    auto* pCurrentSelectorSize = m_CurrentSelectorSize.get();
    pSpriteNone->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentSelectorSize, yStartPos](oxygine::Event *)
    {
        m_SizeMode = PlacementSize::None;
        pCurrentSelectorSize->setPosition(m_frameSize, yStartPos);
        emit sigSelectionChanged();
    });
    // scale marker to correct size if needed
    m_CurrentSelectorSize->setScale(pAnim->getWidth() / pAnimMarker->getWidth());

    oxygine::spSprite pSpriteSmall = oxygine::spSprite::create();
    pAnim = pObjectManager->getResAnim("editor+small");
    pSpriteSmall->setResAnim(pAnim);
    pSpriteSmall->setPosition(m_frameSize + xChange, yStartPos);
    m_BoxPlacementSize->addChild(pSpriteSmall);
    pSpriteSmall->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentSelectorSize, yStartPos, xChange](oxygine::Event *)
    {
        m_SizeMode = PlacementSize::Small;
        pCurrentSelectorSize->setPosition(m_frameSize + xChange, yStartPos);
        emit sigSelectionChanged();
    });

    oxygine::spSprite pSpriteMedium = oxygine::spSprite::create();
    pAnim = pObjectManager->getResAnim("editor+medium");
    pSpriteMedium->setResAnim(pAnim);
    pSpriteMedium->setPosition(m_frameSize + xChange * 2, yStartPos);
    m_BoxPlacementSize->addChild(pSpriteMedium);
    pSpriteMedium->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentSelectorSize, yStartPos, xChange](oxygine::Event *)
    {
        m_SizeMode = PlacementSize::Medium;
        pCurrentSelectorSize->setPosition(m_frameSize + xChange * 2, yStartPos);
        emit sigSelectionChanged();
    });

    oxygine::spSprite pSpriteBigSquare = oxygine::spSprite::create();
    pAnim = pObjectManager->getResAnim("editor+big+square");
    pSpriteBigSquare->setResAnim(pAnim);
    pSpriteBigSquare->setPosition(m_frameSize + xChange * 3, yStartPos );
    m_BoxPlacementSize->addChild(pSpriteBigSquare);
    pSpriteBigSquare->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentSelectorSize, yStartPos, xChange](oxygine::Event *)
    {
        m_SizeMode = PlacementSize::BigSquare;
        pCurrentSelectorSize->setPosition(m_frameSize + xChange * 3, yStartPos);
        emit sigSelectionChanged();
    });

    oxygine::spSprite pSpriteBig = oxygine::spSprite::create();
    pAnim = pObjectManager->getResAnim("editor+big");
    pSpriteBig->setResAnim(pAnim);
    pSpriteBig->setPosition(m_frameSize + xChange * 4, yStartPos );
    m_BoxPlacementSize->addChild(pSpriteBig);
    pSpriteBig->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentSelectorSize, yStartPos, xChange](oxygine::Event *)
    {
        m_SizeMode = PlacementSize::Big;
        pCurrentSelectorSize->setPosition(m_frameSize + xChange * 4, yStartPos);
        emit sigSelectionChanged();
    });

    oxygine::spSprite pSpriteFill = oxygine::spSprite::create();
    pAnim = pObjectManager->getResAnim("editor+fill");
    pSpriteFill->setResAnim(pAnim);
    pSpriteFill->setPosition(m_frameSize + xChange * 5, yStartPos );
    m_BoxPlacementSize->addChild(pSpriteFill);
    pSpriteFill->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentSelectorSize, yStartPos, xChange](oxygine::Event *)
    {
        m_SizeMode = PlacementSize::Fill;
        pCurrentSelectorSize->setPosition(m_frameSize + xChange * 5, yStartPos);
        emit sigSelectionChanged();
    });
}

void EditorSelection::createPlayerSelection()
{
    // since we can get called when the player count changes :)
    m_BoxSelectedPlayer->removeChildren();
    m_Players.clear();

    oxygine::spButton pButtonLeft = oxygine::spButton::create();
    pButtonLeft->setResAnim(ObjectManager::getInstance()->getResAnim("arrow+right"));
    pButtonLeft->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    oxygine::Sprite* ptr = pButtonLeft.get();
    pButtonLeft->addEventListener(oxygine::TouchEvent::OVER, [=](oxygine::Event*)
    {
        ptr->addTween(oxygine::Sprite::TweenAddColor(QColor(16, 16, 16, 0)), oxygine::timeMS(300));
    });

    pButtonLeft->addEventListener(oxygine::TouchEvent::OUTX, [=](oxygine::Event*)
    {
        ptr->addTween(oxygine::Sprite::TweenAddColor(QColor(0, 0, 0, 0)), oxygine::timeMS(300));
    });
    pButtonLeft->setFlippedX(true);
    pButtonLeft->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        m_playerStartIndex -= 1;
        if (m_playerStartIndex < 0)
        {
            m_playerStartIndex = 0;
        }
        emit sigUpdateSelectedPlayer();
    });
    pButtonLeft->setPosition(10, 10);
    m_BoxSelectedPlayer->addChild(pButtonLeft);

    oxygine::spButton pButtonRight = oxygine::spButton::create();
    pButtonRight->setResAnim(ObjectManager::getInstance()->getResAnim("arrow+right"));
    pButtonRight->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    ptr = pButtonRight.get();
    pButtonRight->addEventListener(oxygine::TouchEvent::OVER, [ptr](oxygine::Event*)
    {
        ptr->addTween(oxygine::Sprite::TweenAddColor(QColor(16, 16, 16, 0)), oxygine::timeMS(300));
    });

    pButtonRight->addEventListener(oxygine::TouchEvent::OUTX, [ptr](oxygine::Event*)
    {
        ptr->addTween(oxygine::Sprite::TweenAddColor(QColor(0, 0, 0, 0)), oxygine::timeMS(300));
    });
    pButtonRight->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        m_playerStartIndex += 1;
        if (m_playerStartIndex >= m_Players.size() - calcMaxPlayerSelection())
        {
            m_playerStartIndex = m_Players.size() - calcMaxPlayerSelection();
            if (m_playerStartIndex < 0)
            {
                m_playerStartIndex = 0;
            }
        }
        emit sigUpdateSelectedPlayer();
    });
    pButtonRight->setPosition(m_BoxSelectedPlayer->getScaledWidth() - 30, 10);
    m_BoxSelectedPlayer->addChild(pButtonRight);
    TerrainManager* pTerrainManager = TerrainManager::getInstance();
    for (qint32 i = -1; i < m_pMap->getPlayerCount(); i++)
    {
        spBuilding pBuilding = spBuilding::create("HQ", m_pMap);
        oxygine::spSprite pSprite = oxygine::spSprite::create();
        oxygine::ResAnim* pAnim = pTerrainManager->getResAnim("plains+0");
        pSprite->setResAnim(pAnim);
        pSprite->setPriority(-100);
        pSprite->setWidth(GameMap::getImageSize());
        pBuilding->addChild(pSprite);
        m_Players.append(pBuilding);
        if (i >= 0)
        {
            pBuilding->setOwner(m_pMap->getPlayer(i));
        }
        else
        {
            pBuilding->setOwner(nullptr);
        }
        m_BoxSelectedPlayer->addChild(pBuilding);
        pBuilding->setVisible(true);
        pBuilding->setPosition(25 * i, 27);
        pBuilding->addEventListener(oxygine::TouchEvent::CLICK, [this, i](oxygine::Event*)
        {
            emit sigChangeSelectedPlayer(i);
        });
    }
    updateSelectedPlayer();
    changeSelectedPlayer(0);
}

void EditorSelection::changeSelectedPlayer(qint32 player)
{    
    // update buildings
    if (player < 0)
    {
        m_currentPlayer = nullptr;
    }
    else
    {
        m_currentPlayer = m_Players.at(player + 1)->getOwner();
    }
    for (qint32 i2 = 0; i2 < m_Buildings.size(); i2++)
    {
        if (player < 0)
        {
            m_Buildings.at(i2)->setOwner(nullptr);
        }
        else
        {
            m_Buildings.at(i2)->setOwner(m_Players.at(player + 1)->getOwner());
        }
    }
    // update units
    for (qint32 i2 = 0; i2 < m_Units.size(); i2++)
    {
        if (player < 0)
        {
            // do nothing :)
            m_Units.at(i2)->setOwner(m_Players.at(1)->getOwner());
        }
        else
        {
            m_Units.at(i2)->setOwner(m_Players.at(player + 1)->getOwner());
        }
    }    
}

void EditorSelection::updateSelectedPlayer()
{
    if (m_playerStartIndex >= m_Players.size() - calcMaxPlayerSelection())
    {
        m_playerStartIndex = m_Players.size() - calcMaxPlayerSelection();
        if (m_playerStartIndex < 0)
        {
            m_playerStartIndex = 0;
        }
    }
    for (qint32 i = 0; i < m_Players.size(); i++)
    {
        m_Players[i]->setVisible(false);
    }
    qint32 m_MaxPlayer = calcMaxPlayerSelection();
    for (qint32 i = m_playerStartIndex; i < m_playerStartIndex + m_MaxPlayer; i++)
    {
        if (i < m_Players.size())
        {
            m_Players[i]->setVisible(true);
            m_Players[i]->setPosition(40 + (m_Players[i]->getScaledWidth() + 5)  * (i - m_playerStartIndex), 40);
        }
    }
}

qint32 EditorSelection::calcMaxPlayerSelection()
{
    return (m_BoxSelectedPlayer->getScaledWidth() - 80) / (m_Players[0]->getScaledWidth() + 5);
}

void EditorSelection::createBoxSelectionMode()
{
    qint32 yStartPos = 10;
    qint32 xChange = 40;
    ObjectManager* pObjectManager = ObjectManager::getInstance();

    m_CurrentSelectorMode = oxygine::spSprite::create();
    oxygine::ResAnim* pAnimMarker = pObjectManager->getResAnim("editor+selector");
    m_CurrentSelectorMode->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    if (pAnimMarker->getTotalFrames() > 1)
    {
        oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnimMarker), oxygine::timeMS(pAnimMarker->getTotalFrames() * GameMap::frameTime * 2), -1);
        m_CurrentSelectorMode->addTween(tween);
    }
    else
    {
        m_CurrentSelectorMode->setResAnim(pAnimMarker);
    }
    m_BoxSelectionType->addChild(m_CurrentSelectorMode);
    m_CurrentSelectorMode->setPosition(m_frameSize, yStartPos);

    m_pSpriteTerrainMode = oxygine::spSprite::create();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("editor+terrain");
    m_pSpriteTerrainMode->setResAnim(pAnim);
    m_pSpriteTerrainMode->setPosition(m_frameSize, yStartPos);
    m_BoxSelectionType->addChild(m_pSpriteTerrainMode);
    auto* pCurrentSelectorMode = m_CurrentSelectorMode.get();
    m_pSpriteTerrainMode->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentSelectorMode, yStartPos](oxygine::Event *)
    {
        m_Mode = EditorMode::Terrain;
        pCurrentSelectorMode->setPosition(m_frameSize, yStartPos);
        selectTerrain(0);
        emit sigUpdateTerrainView();
    });
    // scale marker to correct size if needed
    m_CurrentSelectorMode->setScale(pAnim->getWidth() / pAnimMarker->getWidth());

    m_pSpriteBuildingMode = oxygine::spSprite::create();
    pAnim = pObjectManager->getResAnim("editor+building");
    m_pSpriteBuildingMode->setResAnim(pAnim);
    m_pSpriteBuildingMode->setPosition(m_frameSize + xChange, yStartPos);
    m_BoxSelectionType->addChild(m_pSpriteBuildingMode);
    m_pSpriteBuildingMode->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentSelectorMode, xChange, yStartPos](oxygine::Event *)
    {
        m_Mode = EditorMode::Building;
        pCurrentSelectorMode->setPosition(m_frameSize + xChange, yStartPos);
        selectBuilding(0);
        emit sigUpdateBuildingView();
    });

    m_pSpriteUnitMode = oxygine::spSprite::create();
    pAnim = pObjectManager->getResAnim("editor+unit");
    m_pSpriteUnitMode->setResAnim(pAnim);
    m_pSpriteUnitMode->setPosition(m_frameSize + xChange * 2, yStartPos);
    m_BoxSelectionType->addChild(m_pSpriteUnitMode);
    m_pSpriteUnitMode->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentSelectorMode, xChange, yStartPos](oxygine::Event *)
    {
        m_Mode = EditorMode::Unit;
        pCurrentSelectorMode->setPosition(m_frameSize + xChange * 2, yStartPos);
        selectUnit(0);
        emit sigUpdateUnitView();
    });

}

oxygine::spSprite EditorSelection::createV9Box(qint32 x, qint32 y, qint32 width, qint32 heigth)
{
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::spBox9Sprite pSprite = oxygine::spBox9Sprite::create();
    addChild(pSprite);
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("panel");
    pSprite->setResAnim(pAnim);
    pSprite->setSize(width, heigth);
    pSprite->setPosition(x, y);
    pSprite->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    return pSprite;
}

void EditorSelection::updateTerrainView()
{    
    hideSelection();
    for (auto & terrain : m_Terrains)
    {
        terrain->setVisible(true);
    }
    for (auto & item : m_terrainActors)
    {
        item->setVisible(true);
    }
    m_PlacementActor->setHeight(m_Terrains[m_Terrains.size() - 1]->oxygine::Actor::getY() + GameMap::getImageSize() + 5);
    m_PlacementActor->setY(-GameMap::getImageSize());
    m_PlacementSelectionClip->updateDragBounds();
    selectTerrain(0);    
}

void EditorSelection::updateBuildingView()
{    
    hideSelection();
    for (auto & building : m_Buildings)
    {
        building->setVisible(true);
    }
    for (auto & item : m_buildingActors)
    {
        item->setVisible(true);
    }
    m_PlacementActor->setHeight(m_Buildings[m_Buildings.size() - 1]->oxygine::Actor::getY() + GameMap::getImageSize() + 5);
    m_PlacementActor->setY(-GameMap::getImageSize());
    m_PlacementSelectionClip->updateDragBounds();
    selectBuilding(0);    
}

void EditorSelection::updateUnitView()
{    
    hideSelection();
    for (auto & unit : m_Units)
    {
        unit->setVisible(true);
    }
    for (auto & item : m_unitActors)
    {
        item->setVisible(true);
    }
    m_PlacementActor->setHeight(m_Units[m_Units.size() - 1]->oxygine::Actor::getY() + GameMap::getImageSize() + 5);
    m_PlacementActor->setY(-GameMap::getImageSize());
    m_PlacementSelectionClip->updateDragBounds();
    selectUnit(0);    
}

void EditorSelection::hideSelection()
{
    for (auto & item : m_Terrains)
    {
        item->setVisible(false);
    }
    for (auto & item : m_terrainActors)
    {
        item->setVisible(false);
    }
    for (auto & item : m_Buildings)
    {
        item->setVisible(false);
    }
    for (auto & item : m_buildingActors)
    {
        item->setVisible(false);
    }
    for (auto & item : m_Units)
    {
        item->setVisible(false);
    }
    for (auto & item : m_unitActors)
    {
        item->setVisible(false);
    }
}

void EditorSelection::initSelection()
{
    initBuildingSection();
    initTerrainSection();
    initUnitSelection();
}

void EditorSelection::initBuildingSection()
{
    qint32 posY = m_startH - GameMap::getImageSize();
    qint32 xCounter = 0;
    qint32 currentIdentifier = std::numeric_limits<qint32>::min();
    for (qint32 i = 0; i < m_Buildings.size(); i++)
    {
        createBuildingSectionLabel(i, currentIdentifier, xCounter, posY);
        qint32 posX = getPosX(xCounter);
        if (xCounter >= m_xCount)
        {
            posY += GameMap::getImageSize() * m_yFactor;
            xCounter = 0;
            posX = m_frameSize;
        }
        qint32 width = m_Buildings[i]->getBuildingWidth();
        qint32 heigth = m_Buildings[i]->getBuildingHeigth();
        m_Buildings[i]->setX(posX + GameMap::getImageSize() * (width - 1) / (width));
        m_Buildings[i]->setY(posY + GameMap::getImageSize() * (heigth - 1) / (heigth));
        m_Buildings[i]->setVisible(false);
        m_Buildings[i]->addEventListener(oxygine::TouchEvent::CLICK, [this, i](oxygine::Event*)
        {
            selectBuilding(i);
        });
        xCounter++;
    }
}

void EditorSelection::createBuildingSectionLabel(qint32 item, qint32 & currentIdentifier, qint32 & xCounter, qint32 & posY)
{
    BuildingSpriteManager* pBuildingSpriteManager = BuildingSpriteManager::getInstance();
    qint32 newIdentifier = m_Buildings[item]->getBuildingGroup();
    if (newIdentifier != currentIdentifier)
    {
        posY += GameMap::getImageSize();
        currentIdentifier = newIdentifier;
        xCounter = 0;
        oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
        style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
        style.multiline = false;

        spLabel pTextfield = spLabel::create(m_labelWidth);
        pTextfield->setStyle(style);
        pTextfield->setPosition(getPosX(xCounter), posY);
        pTextfield->setHtmlText(pBuildingSpriteManager->getBuildingGroupName(currentIdentifier));
        pTextfield->setVisible(false);
        m_PlacementActor->addChild(pTextfield);
        m_buildingActors.append(pTextfield);
        posY += GameMap::getImageSize() * m_yFactor;
    }
}

void EditorSelection::initTerrainSection()
{
    qint32 posY = m_startH - GameMap::getImageSize();
    qint32 xCounter = 0;
    qint32 currentIdentifier = std::numeric_limits<qint32>::min();
    for (qint32 i = 0; i < m_Terrains.size(); i++)
    {
        createTerrainSectionLabel(i, currentIdentifier, xCounter, posY);
        qint32 posX = getPosX(xCounter);
        if (xCounter >= m_xCount)
        {
            posY += GameMap::getImageSize() * m_yFactor;
            xCounter = 0;
            posX = m_frameSize;
        }
        m_Terrains[i]->setPosition(posX, posY);
        m_Terrains[i]->setVisible(false);
        m_Terrains[i]->addEventListener(oxygine::TouchEvent::CLICK, [this, i](oxygine::Event*)
        {
            selectTerrain(i);
        });
        xCounter++;
    }
}

void EditorSelection::createTerrainSectionLabel(qint32 item, qint32 & currentIdentifier, qint32 & xCounter, qint32 & posY)
{
    TerrainManager* pTerrainManager = TerrainManager::getInstance();
    qint32 newIdentifier = m_Terrains[item]->getTerrainGroup();
    if (newIdentifier != currentIdentifier)
    {
        posY += GameMap::getImageSize();
        currentIdentifier = newIdentifier;
        xCounter = 0;
        oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
        style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
        style.multiline = false;

        spLabel pTextfield = spLabel::create(m_labelWidth);
        pTextfield->setStyle(style);
        pTextfield->setPosition(getPosX(xCounter), posY);
        pTextfield->setHtmlText(pTerrainManager->getTerrainGroupName(currentIdentifier));
        pTextfield->setVisible(false);
        m_PlacementActor->addChild(pTextfield);
        m_terrainActors.append(pTextfield);
        posY += GameMap::getImageSize() * m_yFactor;
    }
}

void EditorSelection::initUnitSelection()
{
    qint32 posY = m_startH - GameMap::getImageSize();
    qint32 xCounter = 0;
    qint32 currentIdentifier = std::numeric_limits<qint32>::min();
    for (qint32 i = 0; i < m_Units.size(); i++)
    {
        createUnitSectionLabel(i, currentIdentifier, xCounter, posY);
        qint32 posX = getPosX(xCounter);
        if (xCounter >= m_xCount)
        {
            posY += GameMap::getImageSize() * 1.5f;
            xCounter = 0;
            posX = m_frameSize;
        }
        m_Units[i]->setPosition(posX, posY);
        m_Units[i]->setVisible(false);
        m_Units[i]->addEventListener(oxygine::TouchEvent::CLICK, [this, i](oxygine::Event*)
        {
            selectUnit(i);
        });
        xCounter++;
    }
}

void EditorSelection::createUnitSectionLabel(qint32 item, qint32 & currentIdentifier, qint32 & xCounter, qint32 & posY)
{
    qint32 newIdentifier = m_Units[item]->getUnitType();
    if (newIdentifier != currentIdentifier)
    {
        posY += GameMap::getImageSize();
        currentIdentifier = newIdentifier;
        xCounter = 0;
        oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
        style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
        style.multiline = false;

        spLabel pTextfield = spLabel::create(m_labelWidth);
        pTextfield->setStyle(style);
        pTextfield->setPosition(getPosX(xCounter), posY);
        pTextfield->setHtmlText(UnitSpriteManager::getUnitTypeText(currentIdentifier));
        pTextfield->setVisible(false);
        m_PlacementActor->addChild(pTextfield);
        m_unitActors.append(pTextfield);
        posY += GameMap::getImageSize() * 1.5f;
    }
}

qint32 EditorSelection::getPosX(qint32 xCounter)
{
    return m_frameSize + xCounter * GameMap::getImageSize() * m_xFactor;
}

void EditorSelection::selectBuilding(qint32 building)
{
    m_selectedItem = building;
    qint32 width = m_Buildings[building]->getBuildingWidth();
    qint32 heigth = m_Buildings[building]->getBuildingHeigth();
    qint32 x = GameMap::getImageSize() * (width - 1) / (width);
    qint32 y = GameMap::getImageSize() * (heigth - 1) / (heigth);
    m_CurrentSelector->setPosition(m_Buildings[building]->oxygine::Actor::getPosition() - oxygine::Point(x, y));
    emit sigSelectionChanged();
}

void EditorSelection::selectUnit(qint32 unit)
{
    m_selectedItem = unit;
    m_CurrentSelector->setPosition(m_Units[unit]->oxygine::Actor::getPosition());
    emit sigSelectionChanged();
}

void EditorSelection::selectTerrain(qint32 terrain)
{
    m_selectedItem = terrain;
    m_CurrentSelector->setPosition(m_Terrains[terrain]->oxygine::Actor::getPosition());
    emit sigSelectionChanged();
}

void EditorSelection::selectTerrain(QString terrainID)
{
    
    m_Mode = EditorMode::Terrain;
    m_CurrentSelectorMode->setPosition(m_pSpriteTerrainMode->getPosition());
    updateTerrainView();
    for (qint32 i = 0; i < m_Terrains.size(); i++)
    {
        if (m_Terrains[i]->getTerrainID() == terrainID)
        {
            selectTerrain(i);
        }
    }
    
}

void EditorSelection::selectBuilding(QString buildingID)
{
    
    m_Mode = EditorMode::Building;
    m_CurrentSelectorMode->setPosition(m_pSpriteBuildingMode->getPosition());
    updateBuildingView();
    for (qint32 i = 0; i < m_Buildings.size(); i++)
    {
        if (m_Buildings[i]->getBuildingID() == buildingID)
        {
            selectBuilding(i);
        }
    }
    
}

void EditorSelection::selectUnit(QString unitID)
{
    
    m_Mode = EditorMode::Unit;
    m_CurrentSelectorMode->setPosition(m_pSpriteUnitMode->getPosition());
    updateUnitView();
    for (qint32 i = 0; i < m_Units.size(); i++)
    {
        if (m_Units[i]->getUnitID() == unitID)
        {
            selectUnit(i);
        }
    }
    
}

void EditorSelection::KeyInput(Qt::Key cur)
{
    if (cur == Settings::getKey_EditorPlaceTerrain())
    {
        oxygine::TouchEvent event(oxygine::TouchEvent::CLICK, false);
        event.target = m_pSpriteTerrainMode;
        event.currentTarget = m_pSpriteTerrainMode;
        m_pSpriteTerrainMode->dispatchEvent(&event);
    }
    else if (cur  == Settings::getKey_EditorPlaceBuilding())
    {
        oxygine::TouchEvent event(oxygine::TouchEvent::CLICK, false);
        event.target = m_pSpriteBuildingMode;
        event.currentTarget = m_pSpriteBuildingMode;
        m_pSpriteBuildingMode->dispatchEvent(&event);
    }
    else if (cur  == Settings::getKey_EditorPlaceUnit())
    {
        oxygine::TouchEvent event(oxygine::TouchEvent::CLICK, false);
        event.target = m_pSpriteUnitMode;
        event.currentTarget = m_pSpriteUnitMode;
        m_pSpriteUnitMode->dispatchEvent(&event);
    }
    else if (cur  == Settings::getKey_EditorNextTeam())
    {
        
        qint32 player = 0;
        if (m_currentPlayer.get() != nullptr)
        {
            player = m_currentPlayer->getPlayerID() + 1;
            if (player >= m_pMap->getPlayerCount())
            {
                player = -1;
            }
        }
        else
        {
            player = 0;
        }
        changeSelectedPlayer(player);
    }
    else if (cur  == Settings::getKey_EditorPreviousTeam())
    {
        
        qint32 player = 0;
        if (m_currentPlayer.get() != nullptr)
        {
            player = m_currentPlayer->getPlayerID() - 1;
        }
        else
        {
            player = m_pMap->getPlayerCount() - 1;
        }
        changeSelectedPlayer(player);
    }
    else if (cur == Settings::getKey_EditorSelectionLeft())
    {
        qint32 item = m_selectedItem - 1;
        changeSelection(item);
    }
    else if (cur == Settings::getKey_EditorSelectionRight())
    {
        qint32 item = m_selectedItem + 1;
        changeSelection(item);
    }
}

void EditorSelection::changeSelection(qint32 item)
{
    if (item < 0)
    {
        item = 0;
    }
    switch (m_Mode)
    {
        case EditorMode::Unit:
        {
            if (item >= m_Units.size())
            {
                item = m_Units.size() - 1;
            }
            selectUnit(item);
            break;
        }
        case EditorMode::Terrain:
        {
            if (item >= m_Terrains.size())
            {
                item = m_Terrains.size() - 1;
            }
            selectTerrain(item);
            break;
        }
        case EditorMode::Building:
        {
            if (item >= m_Buildings.size())
            {
                item = m_Buildings.size() - 1;
            }
            selectBuilding(item);
            break;
        }
        case EditorMode::All:
        {
            break;
        }
    }
}

EditorSelection::PlacementSize EditorSelection::getSizeMode() const
{
    return m_SizeMode;
}

spBuilding EditorSelection::getCurrentSpBuilding()
{
    return  m_Buildings.at(m_selectedItem);
}

spUnit EditorSelection::getCurrentSpUnit()
{
    return  m_Units.at(m_selectedItem);
}
