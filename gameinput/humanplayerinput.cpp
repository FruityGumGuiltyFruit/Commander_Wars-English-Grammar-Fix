#include "gameinput/humanplayerinput.h"
#include "gameinput/markedfielddata.h"

#include "game/gamemap.h"
#include "game/gameaction.h"
#include "game/terrain.h"
#include "game/building.h"
#include "game/player.h"
#include "game/co.h"
#include "game/actionperformer.h"
#include "game/gameanimation/gameanimationfactory.h"

#include "resource_management/gamemanager.h"
#include "resource_management/fontmanager.h"
#include "resource_management/objectmanager.h"

#include "coreengine/mainapp.h"
#include "coreengine/audiothread.h"
#include "coreengine/interpreter.h"
#include "coreengine/globalutils.h"

#include "ai/coreai.h"

HumanPlayerInput::HumanPlayerInput(GameMap* pMap)
    : BaseGameInputIF(pMap, GameEnums::AiTypes_Human)
{    
    setObjectName("HumanPlayerInput");
    Mainapp* pApp = Mainapp::getInstance();
    moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);
}

void HumanPlayerInput::init()
{
    Mainapp* pApp = Mainapp::getInstance();
    spGameMenue pMenu = GameMenue::getInstance();
    
    if (pMenu.get() != nullptr)
    {
        connect(pMenu.get(), &GameMenue::sigRightClickDown, this, &HumanPlayerInput::rightClickDown, Qt::QueuedConnection);
        connect(pMenu.get(), &GameMenue::sigRightClickUp, this, &HumanPlayerInput::rightClickUp, Qt::QueuedConnection);
        connect(pMenu.get(), &GameMenue::sigLeftClick, this, &HumanPlayerInput::leftClick, Qt::QueuedConnection);
        connect(&pMenu->getActionPerformer(), &ActionPerformer::sigActionPerformed, this, &HumanPlayerInput::autoEndTurn, Qt::QueuedConnection);
        connect(m_pMap, &GameMap::sigZoomChanged, this, &HumanPlayerInput::zoomChanged, Qt::QueuedConnection);
        connect(pApp, &Mainapp::sigKeyDown, this, &HumanPlayerInput::keyDown, Qt::QueuedConnection);
        connect(pMenu->getCursor(), &Cursor::sigCursorMoved, this, &HumanPlayerInput::cursorMoved, Qt::QueuedConnection);
        connect(this, &HumanPlayerInput::performAction, &pMenu->getActionPerformer(), &ActionPerformer::performAction, Qt::QueuedConnection);
        connect(this, &HumanPlayerInput::sigNextTurn, this, &HumanPlayerInput::nextTurn, Qt::QueuedConnection);
        m_Fields.reserve(m_pMap->getMapWidth() * m_pMap->getMapHeight() / 4);
        m_FieldPoints.reserve(m_pMap->getMapWidth() * m_pMap->getMapHeight() / 4);
    }
}

HumanPlayerInput::~HumanPlayerInput()
{
    m_pGameAction = nullptr;
    m_pUnitPathFindingSystem = nullptr;
}

void HumanPlayerInput::rightClickUp(qint32, qint32)
{
    // if (m_pMap->getCurrentPlayer() == m_pPlayer ||
    //     m_pPlayer == nullptr)
    // {
    //     if (m_FieldPoints.size() > 0 && m_pGameAction.get() == nullptr)
    //     {
    //         cleanUpInput();
    //     }
    //
    // }
}

void HumanPlayerInput::rightClickDown(qint32 x, qint32 y)
{
    bool isViewPlayer = (m_pMap->getCurrentViewPlayer() == m_pPlayer);

    if (m_pMap->getCurrentPlayer() == m_pPlayer ||
        m_pPlayer == nullptr)
    {
        if (GameAnimationFactory::getAnimationCount() > 0)
        {
            GameAnimationFactory::finishAllAnimations();
            if (GameAnimationFactory::getAnimationCount() == 0)
            {
                emit GameAnimationFactory::getInstance()->animationsFinished();
            }
        }
        else if (m_pGameAction.get() != nullptr)
        {
            Mainapp::getInstance()->getAudioThread()->playSound("cancel.wav");
            if ((m_pGameAction->getInputStep() > 0) ||
                (m_pGameAction->getActionID() != ""))
            {
                cancelActionInput();
            }
            else if (m_CurrentMenu.get() == nullptr)
            {
                cancelSelection(x, y);
            }
            else
            {
                cancelActionInput();
            }
            
        }
        else if (m_pMap->onMap(x, y))
        {
            if (m_FieldPoints.size() == 0 && m_pGameAction.get() == nullptr)
            {
                showAttackableFields(x, y);
            }
            else if (m_FieldPoints.size() > 0 && !m_showVisionFields)
            {
                showVisionFields(x, y);
            }
            else
            {
                cleanUpInput();
            }
        }
        else
        {
            cleanUpInput();
        }
    }
    else if (isViewPlayer)
    {
        if (GameAnimationFactory::getAnimationCount() > 0)
        {
            GameAnimationFactory::finishAllAnimations();
            if (GameAnimationFactory::getAnimationCount() == 0)
            {
                emit GameAnimationFactory::getInstance()->animationsFinished();
            }
        }
        else
        {
            cancelActionInput();
        }
    }
    else
    {
        // do nothing
    }
}

void HumanPlayerInput::showVisionFields(qint32 x, qint32 y)
{
    Mainapp::getInstance()->pauseRendering();
    clearMarkedFields();
    // try to show fire ranges :)
    
    Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
    if ((pUnit != nullptr) &&
        (!pUnit->isStealthed(m_pPlayer)))
    {
        m_showVisionFields = true;
        Mainapp::getInstance()->getAudioThread()->playSound("selectunit.wav");
        auto points = pUnit->getVisionFields(pUnit->Unit::getPosition());
        for (auto & point : qAsConst(points))
        {
            if (!GlobalUtils::contains(m_FieldPoints, QPoint(point.x(), point.y())))
            {
                Interpreter* pInterpreter = Interpreter::getInstance();
                QColor viewColor = QColor(255, 127, 39, 255);
                QJSValue ret = pInterpreter->doFunction("Global", "getVisionFieldColor");
                if (ret.isString())
                {
                    viewColor = QColor(ret.toString());
                }
                createMarkedField(point.toPoint(), viewColor);
            }
        }
    }
    else
    {
        cleanUpInput();
    }
    syncMarkedFields();
    Mainapp::getInstance()->continueRendering();
}

void HumanPlayerInput::cancelSelection(qint32 x, qint32 y)
{
    CONSOLE_PRINT("HumanPlayerInput::cancelSelection", Console::eDEBUG);
    Unit* pUnit = m_pGameAction->getTargetUnit();
    if (pUnit != nullptr && !pUnit->getHasMoved() &&
        m_pUnitPathFindingSystem.get() != nullptr &&
        !m_pMap->onMap(x, y))
    {
        qint32 costs = m_pUnitPathFindingSystem->getTargetCosts(x, y);
        if (m_pUnitPathFindingSystem->getCosts(m_ArrowPoints) != costs &&
            costs > 0)
        {
            m_ArrowPoints = m_pUnitPathFindingSystem->getPathFast(x, y);
            createCursorPath(x, y);
        }
        else
        {
            cleanUpInput();
        }
    }
    else
    {
        cleanUpInput();
    }
}

void HumanPlayerInput::cancelActionInput()
{
    CONSOLE_PRINT("HumanPlayerInput::cancelActionInput", Console::eDEBUG);
    Unit* pUnit = nullptr;
    if (m_pGameAction.get() != nullptr)
    {
        pUnit = m_pGameAction->getTargetUnit();
    }
    spGameMenue pMenu = GameMenue::getInstance();
    if (pMenu.get() != nullptr)
    {
        pMenu->getCursor()->changeCursor("cursor+default");
    }
    if ((pUnit != nullptr) &&
        (!pUnit->isStealthed(m_pPlayer)))
    {
        // go one step back :)
        clearMenu();
        createMarkedMoveFields();
        // reset action
        m_pGameAction->reset();
    }
    else
    {
        cleanUpInput();
    }
}

void HumanPlayerInput::showAttackableFields(qint32 x, qint32 y)
{
    Mainapp::getInstance()->pauseRendering();
    clearMarkedFields();
    // try to show fire ranges :)
    
    Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
    if ((pUnit != nullptr) &&
        (!pUnit->isStealthed(m_pPlayer)))
    {
        if (pUnit->hasAmmo1() || pUnit->hasAmmo2())
        {
            qint32 maxRange = pUnit->getMaxRange(pUnit->getPosition());
            qint32 minRange = pUnit->getMinRange(pUnit->getPosition());
            spQmlVectorPoint pPoints = spQmlVectorPoint(GlobalUtils::getCircle(minRange, maxRange));
            Mainapp::getInstance()->getAudioThread()->playSound("selectunit.wav");
            UnitPathFindingSystem pfs(m_pMap, m_pMap->getTerrain(x, y)->getUnit(), m_pPlayer);
            pfs.explore();
            auto points = pfs.getAllNodePointsFast();
            for (auto & point : points)
            {
                if (pUnit->canMoveAndFire(QPoint(x, y)) ||
                    (point.x() == x && point.y() == y))
                {

                    for (auto & point2 : pPoints->getVector())
                    {
                        QPoint target = point2 + point;
                        if (!GlobalUtils::contains(m_FieldPoints, target))
                        {
                            createMarkedField(target, QColor(255, 0, 0));
                        }
                    }
                }
            }
        }
    }
    else
    {
        Building* pBuilding = m_pMap->getTerrain(x, y)->getBuilding();
        if (pBuilding != nullptr)
        {
            spQmlVectorPoint pPoints = spQmlVectorPoint(pBuilding->getActionTargetFields());
            QPoint targetOffset = pBuilding->getActionTargetOffset();
            QPoint buildingPos(pBuilding->Building::getX(), pBuilding->Building::getY());
            if (pPoints.get() != nullptr && pPoints->size() > 0)
            {
                Mainapp::getInstance()->getAudioThread()->playSound("selectunit.wav");
                for (auto & point : pPoints->getVector())
                {
                    createMarkedField(buildingPos + targetOffset + point, QColor(255, 0, 0));
                }
            }
        }
    }
    syncMarkedFields();
    Mainapp::getInstance()->continueRendering();
}

void HumanPlayerInput::syncMarkedFields()
{
    for (auto & field : m_Fields)
    {
        auto & tweens = field->getTweens();
        for (auto & pTween : tweens)
        {
            pTween->reset();
            pTween->start(*field);
        }
    }
}

void HumanPlayerInput::cleanUpInput()
{
    CONSOLE_PRINT("HumanPlayerInput::cleanUpInput", Console::eDEBUG);
    clearMenu();
    m_pGameAction = nullptr;
    m_pUnitPathFindingSystem = nullptr;
    m_showVisionFields = false;
    clearMarkedFields();
    deleteArrow();
    spGameMenue pMenue = GameMenue::getInstance();
    if (pMenue.get() != nullptr)
    {
        Cursor* pCursor = pMenue->getCursor();
        pCursor->changeCursor("cursor+default");
        pCursor->resetCursorRangeOutline();
        cursorMoved(pCursor->getMapPointX(), pCursor->getMapPointY());
    }
}

void HumanPlayerInput::clearMenu()
{
    if (m_CurrentMenu.get() != nullptr)
    {
        spGameMenue pMenue = GameMenue::getInstance();
        if (pMenue.get() != nullptr)
        {
            pMenue->setFocused(true);
        }
        m_CurrentMenu->detach();
        m_CurrentMenu = nullptr;
    }
}

void HumanPlayerInput::clearMarkedFields()
{
    Mainapp::getInstance()->pauseRendering();
    for (auto & field : m_Fields)
    {
        field->detach();
    }
    m_FieldPoints.clear();
    m_Fields.clear();
    m_pMarkedFieldData = nullptr;
    if (m_ZInformationLabel.get() != nullptr)
    {
        m_ZInformationLabel->detach();
        m_ZInformationLabel = nullptr;
    }
    for (auto & fields : m_InfoFields)
    {
        fields->detach();
    }
    m_InfoFields.clear();
    Mainapp::getInstance()->continueRendering();
}

void HumanPlayerInput::leftClick(qint32 x, qint32 y)
{
    CONSOLE_PRINT("humanplayer input leftClick() with X " + QString::number(x) + " Y " + QString::number(y), Console::eDEBUG);
    spGameMenue pMenu = GameMenue::getInstance();
    Cursor* pCursor = pMenu->getCursor();
    if (pMenu.get() != nullptr &&
        GameAnimationFactory::getAnimationCount() == 0)
    {
        
        bool isViewPlayer = (m_pMap->getCurrentViewPlayer() == m_pPlayer);
        if (!m_pMap->onMap(x, y))
        {
            // do nothing
        }
        else if (!pMenu->getFocused())
        {
            if (m_CurrentMenu.get() != nullptr && Settings::getSimpleDeselect())
            {
                Mainapp::getInstance()->getAudioThread()->playSound("cancel.wav");
                cancelActionInput();
            }
        }
        else if (m_pMap->getCurrentPlayer() == m_pPlayer ||
                 m_pPlayer == nullptr)
        {
            if (m_pMarkedFieldData.get() != nullptr &&
                (!m_pMarkedFieldData->getShowZData() ||
                 (!Settings::getTouchScreen() ||
                  (m_lastClickPoint.x() == x &&
                   m_lastClickPoint.y() == y))))
            {
                // did we select a marked field?
                if (m_pMarkedFieldData->getAllFields())
                {
                    markedFieldSelected(QPoint(x, y));
                }
                else
                {
                    QVector<QPoint> & pFields = *m_pMarkedFieldData->getPoints();
                    for (auto field : pFields)
                    {
                        if ((field.x() == x) &&
                            (field.y() == y))
                        {
                            markedFieldSelected(QPoint(x, y));
                            break;
                        }
                    }
                }
            }
            // no action selected
            else if (m_Fields.size() > 0 && m_pGameAction.get() == nullptr)
            {
                // do nothing
                // some one spawned them to give some info hints player needs to remove them by canceling.
            }
            else if (m_pGameAction.get() == nullptr &&
                     m_pPlayer != nullptr)
            {
                // prepare action
                m_pGameAction = spGameAction::create(m_pMap);
                if (m_pPlayer != nullptr)
                {
                    m_pGameAction->setPlayer(m_pPlayer->getPlayerID());
                }
                m_pGameAction->setTarget(QPoint(x, y));
                
                Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
                if ((pUnit != nullptr) &&
                    (!pUnit->isStealthed(m_pPlayer)))
                {
                    selectUnit(x, y);
                }
                else if (m_pPlayer != nullptr)
                {
                    Building* pBuilding = m_pMap->getTerrain(x, y)->getBuilding();
                    QStringList actions;
                    QStringList possibleActions;
                    if ((pBuilding != nullptr) &&
                        (pBuilding->getOwner() == m_pPlayer))
                    {
                        actions = pBuilding->getActionList();
                        for (auto & action : actions)
                        {
                            if (m_pGameAction->canBePerformed(action))
                            {
                                possibleActions.append(action);
                            }
                        }
                    }
                    if (possibleActions.size() > 0)
                    {
                        Mainapp::getInstance()->getAudioThread()->playSound("selectunit.wav");
                        if ((possibleActions.size() == 1) &&
                            (!m_pGameAction->isFinalStep(possibleActions[0])))
                        {
                            // skip show select action menu
                            m_pGameAction->setActionID(possibleActions[0]);
                            getNextStepData();
                        }
                        else
                        {
                            if (possibleActions.size() > 0)
                            {
                                createActionMenu(possibleActions, x, y);
                            }
                        }
                    }
                    else
                    {
                        std::chrono::milliseconds time = std::chrono::milliseconds(m_doubleClickTime.elapsed());
                        if ((time < std::chrono::milliseconds(1000) && m_doubleClickTime.isValid())
                            || !Settings::getTouchScreen())
                        {
                            actions = getEmptyActionList();
                            possibleActions.clear();
                            for (auto & action : actions)
                            {
                                if (m_pGameAction->canBePerformed(action, true))
                                {
                                    possibleActions.append(action);
                                }
                            }
                            if (possibleActions.size() > 0)
                            {
                                Mainapp::getInstance()->getAudioThread()->playSound("selectunit.wav");
                                createActionMenu(possibleActions, x, y);
                            }
                            else
                            {
                                cleanUpInput();
                            }
                            m_doubleClickTime.invalidate();
                        }
                        else
                        {
                            m_doubleClickTime.start();
                            m_pGameAction = nullptr;
                        }
                    }
                }
                else
                {
                    m_pGameAction = nullptr;
                }
            }
            // we want to select an action
            else if (m_pPlayer != nullptr &&
                     m_pGameAction.get() != nullptr &&
                     m_pGameAction->getActionID() == "")
            {
                if ((m_pUnitPathFindingSystem.get() != nullptr) &&
                    (m_pUnitPathFindingSystem->isReachable(x, y)) &&
                    (m_CurrentMenu.get() == nullptr))
                {
                    Unit* pUnit = m_pGameAction->getTargetUnit();
                    if (pUnit != nullptr)
                    {
                        m_pGameAction->setMovepath(m_ArrowPoints, m_pGameAction->getCosts());
                        QStringList actions;
                        QStringList possibleActions;
                        if (m_pUnitPathFindingSystem->getCosts(m_ArrowPoints) > pUnit->getMovementpoints(m_pGameAction->getTarget()))
                        {
                            if (pUnit->getActionList().contains(CoreAI::ACTION_HOELLIUM_WAIT))
                            {
                                actions.append(CoreAI::ACTION_HOELLIUM_WAIT);
                            }
                            else if (pUnit->getActionList().contains(CoreAI::ACTION_WAIT))
                            {
                                actions.append(CoreAI::ACTION_WAIT);
                            }
                        }
                        else
                        {
                            // we want to do something with this unit :)
                            actions = pUnit->getActionList();
                        }
                        for (auto & action : actions)
                        {
                            if (m_pGameAction->canBePerformed(action))
                            {
                                possibleActions.append(action);
                            }
                        }
                        if (possibleActions.size() > 0)
                        {
                            createActionMenu(possibleActions, x, y);
                        }
                    }
                }
                else
                {
                    Mainapp::getInstance()->getAudioThread()->playSound("cancel.wav");
                    cleanUpInput();
                }
            }
            else if (m_pPlayer == nullptr)
            {
                showInfoMenu(x, y);
            }
            else
            {
                //cleanUpInput();
            }
            
        }
        else if (isViewPlayer)
        {
            showInfoMenu(x, y);
        }
        else
        {
            // do nothing
        }
        m_lastClickPoint = QPoint(pCursor->getMapPointX(), pCursor->getMapPointY());
    }
}

void HumanPlayerInput::showInfoMenu(qint32 x, qint32 y)
{
    // prepare action
    m_pGameAction = spGameAction::create(m_pMap);
    m_pGameAction->setTarget(QPoint(x, y));
    if (m_pPlayer != nullptr)
    {
        m_pGameAction->setPlayer(m_pPlayer->getPlayerID());
    }
    else
    {
        m_pGameAction->setPlayer(m_pMap->getCurrentPlayer()->getPlayerID());
    }
    QStringList actions = getViewplayerActionList();
    QStringList possibleActions;
    for (auto & action : actions)
    {
        if (m_pGameAction->canBePerformed(action, true))
        {
            possibleActions.append(action);
        }
    }
    if (possibleActions.size() > 0)
    {
        Mainapp::getInstance()->getAudioThread()->playSound("selectunit.wav");
        createActionMenu(possibleActions, x, y);
    }

}

void HumanPlayerInput::markedFieldSelected(QPoint point)
{    
    m_pGameAction->writeDataInt32(point.x());
    m_pGameAction->writeDataInt32(point.y());
    clearMarkedFields();
    m_pGameAction->setInputStep(m_pGameAction->getInputStep() + 1);
    if (m_pGameAction->isFinalStep())
    {
        // if not perform action
        finishAction();
    }
    else
    {
        // else introduce next step
        getNextStepData();
    }
    
}

void HumanPlayerInput::menuItemSelected(const QString & itemID, qint32 cost)
{
    CONSOLE_PRINT("HumanPlayerInput::menuItemSelected", Console::eDEBUG);
    if (m_pGameAction.get() != nullptr)
    {
        // we're currently selecting the action for this action
        if (m_pGameAction->getActionID() == "")
        {
            // set the id
            m_pGameAction->setActionID(itemID);
            m_pGameAction->setCosts(m_pGameAction->getCosts() + cost);
        }
        // we want to append some data to the action
        else
        {
            m_pGameAction->writeDataString(itemID);
            // increase costs and input step
            m_pGameAction->setCosts(m_pGameAction->getCosts() + cost);
            m_pGameAction->setInputStep(m_pGameAction->getInputStep() + 1);
        }
        // check if the action needs further information
        if (m_pGameAction->isFinalStep())
        {
            // if not perform action
            finishAction();
        }
        else
        {
            // else introduce next step
            getNextStepData();
        }
    }
}

void HumanPlayerInput::getNextStepData()
{
    Mainapp::getInstance()->pauseRendering();
    CONSOLE_PRINT("HumanPlayerInput::getNextStepData", Console::eDEBUG);
    clearMenu();
    clearMarkedFields();
    spGameMenue pMenu = GameMenue::getInstance();
    if (pMenu.get() != nullptr)
    {
        Cursor* pCursor = pMenu->getCursor();
        pCursor->changeCursor("cursor+default");
        pCursor->resetCursorRangeOutline();
        QString stepType = m_pGameAction->getStepInputType();
        if (stepType.toUpper() == "MENU")
        {
            CONSOLE_PRINT("HumanPlayerInput::getNextStepData show menu", Console::eDEBUG);
            spMenuData pData = m_pGameAction->getMenuStepData();
            if (pData->validData())
            {
                m_CurrentMenu = spHumanPlayerInputMenu::create(m_pMap, pData->getTexts(), pData->getActionIDs(), pData->getIconList(), pData->getCostList(), pData->getEnabledList());
                attachActionMenu(m_pGameAction->getActionTarget().x(), m_pGameAction->getActionTarget().y());
            }
        }
        else if (stepType.toUpper() == "FIELD")
        {
            CONSOLE_PRINT("HumanPlayerInput::getNextStepData show fields", Console::eDEBUG);
            spMarkedFieldData pData = m_pGameAction->getMarkedFieldStepData();
            QVector<QPoint> & pFields = *pData->getPoints();
            for (auto & field : pFields)
            {
                createMarkedField(field, pData->getColor());
            }
            syncMarkedFields();
            m_pMarkedFieldData = pData;
            spCursorData pCursordata = m_pGameAction->getStepCursor();
            pMenu->getCursor()->changeCursor(pCursordata->getCursor(), pCursordata->getXOffset(), pCursordata->getYOffset(), pCursordata->getScale());
            if (!m_pMarkedFieldData->getAllFields())
            {
                nextMarkedField();
            }
        }
        else
        {
            CONSOLE_PRINT("Unknown step type detected. This will lead to an undefined behaviour. Action " + m_pGameAction->getActionID() + " at step " + QString::number(m_pGameAction->getInputStep()), Console::eERROR);
        }
    }
    Mainapp::getInstance()->continueRendering();
}

void HumanPlayerInput::finishAction()
{
    CONSOLE_PRINT("HumanPlayerInput::finishAction", Console::eDEBUG);
    if (m_pGameAction.get() != nullptr)
    {
        Unit* pUnit = m_pGameAction->getTargetUnit();
        if (pUnit != nullptr && m_pUnitPathFindingSystem.get() != nullptr)
        {
            qint32 movepoints = pUnit->getMovementpoints(m_pGameAction->getTarget());
            if (m_pUnitPathFindingSystem->getCosts(m_ArrowPoints) > movepoints)
            {
                // shorten path
                auto newPath = m_pUnitPathFindingSystem->getClosestReachableMovePath(m_ArrowPoints, movepoints);
                m_pGameAction->setMovepath(newPath, m_pUnitPathFindingSystem->getCosts(newPath));
                QVector<QPoint> multiTurnPath;
                for (qint32 i = 0; i <= m_ArrowPoints.size() - newPath.size(); i++)
                {
                    multiTurnPath.append(m_ArrowPoints[i]);
                }
                m_pGameAction->setMultiTurnPath(multiTurnPath);
            }
            else
            {
                // reset path in all other cases
                pUnit->setMultiTurnPath(QVector<QPoint>());
            }
        }

        
        bool isViewPlayer = (m_pMap->getCurrentViewPlayer() == m_pPlayer);
        if (m_pMap->getCurrentPlayer() == m_pPlayer)
        {
            emit performAction(m_pGameAction);
        }
        else if (isViewPlayer ||
                 m_pPlayer == nullptr)
        {
            if (m_pGameAction->getIsLocal())
            {
                m_pGameAction->perform();
            }
            m_pGameAction = nullptr;
        }
        else
        {
            // do nothing
        }
        m_pGameAction = nullptr;
    }
    cleanUpInput();
}

void HumanPlayerInput::createActionMenu(const QStringList & actionIDs, qint32 x, qint32 y)
{
    CONSOLE_PRINT("HumanPlayerInput::createActionMenu", Console::eDEBUG);
    clearMarkedFields();
    MenuData data(m_pMap);
    for (auto & action : actionIDs)
    {
        data.addData(GameAction::getActionText(m_pMap, action), action, GameAction::getActionIcon(m_pMap, action));
    }
    m_CurrentMenu = spHumanPlayerInputMenu::create(m_pMap, data.getTexts(), actionIDs, data.getIconList());
    attachActionMenu(x, y);
}

void HumanPlayerInput::attachActionMenu(qint32 x, qint32 y)
{
    spGameMenue pMenu = GameMenue::getInstance();
    if (pMenu.get() != nullptr)
    {
        
        oxygine::spSlidingActorNoClipRect pMapSliding = pMenu->getMapSliding();
        oxygine::spActor pMapSlidingActor = pMenu->getMapSlidingActor();
        float posX = x * GameMap::getImageSize() * m_pMap->getZoom() + m_pMap->getX() + pMapSliding->getX() + pMapSlidingActor->getX();
        if (posX + m_CurrentMenu->getWidth() > Settings::getWidth() - 40 - pMenu->getGameInfoBar()->getWidth())
        {
            posX = Settings::getWidth() - m_CurrentMenu->getWidth() - 40 - pMenu->getGameInfoBar()->getWidth();
        }
        if (posX < 10)
        {
            posX = 10;
        }
        float posY = y * GameMap::getImageSize() * m_pMap->getZoom() + m_pMap->getY() + pMapSliding->getY() + pMapSlidingActor->getY();
        if (posY < 10)
        {
            posY = 10;
        }
        else if (posY + m_CurrentMenu->getHeight() > Settings::getHeight())
        {
            posY = Settings::getHeight() - m_CurrentMenu->getHeight() - 10;
        }
        m_CurrentMenu->setPosition(posX, posY);
        pMenu->addChild(m_CurrentMenu);
        m_CurrentMenu->moveMouseToItem(0, 0);
        pMenu->setFocused(false);
        connect(m_CurrentMenu.get(), &HumanPlayerInputMenu::sigItemSelected, this, &HumanPlayerInput::menuItemSelected, Qt::QueuedConnection);
        connect(m_CurrentMenu.get(), &HumanPlayerInputMenu::sigCanceled, this, &HumanPlayerInput::rightClickDown, Qt::QueuedConnection);
    }
}

void HumanPlayerInput::selectUnit(qint32 x, qint32 y)
{
    CONSOLE_PRINT("Selecting unit", Console::eDEBUG);
    Mainapp::getInstance()->getAudioThread()->playSound("selectunit.wav");
    
    Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
    m_pUnitPathFindingSystem = spUnitPathFindingSystem::create(m_pMap, pUnit, m_pPlayer);
    if ((pUnit->getOwner() == m_pPlayer) &&
        pUnit->getActionList().contains(CoreAI::ACTION_WAIT))
    {
        qint32 points = pUnit->getMovementpoints(QPoint(x, y)) * static_cast<qint32>(Settings::getMultiTurnCounter());
        if (pUnit->getFuel() < points)
        {
            points = pUnit->getFuel();
        }
        m_pUnitPathFindingSystem->setMovepoints(points);
    }
    else
    {
        m_pUnitPathFindingSystem->setMovepoints(pUnit->getMovementpoints(QPoint(x, y)));
    }
    qint32 maxRange = pUnit->getMaxRange(pUnit->getPosition());
    if (maxRange > 1)
    {
        spGameMenue pMenue = GameMenue::getInstance();
        qint32 minRange = pUnit->getMinRange(pUnit->getPosition());
        Cursor* pCursor = pMenue->getCursor();
        if (minRange > 1)
        {
            pCursor->addCursorRangeOutline(minRange - 1, Qt::green);
        }
        pCursor->addCursorRangeOutline(maxRange, Qt::red);
    }
    qint32 infoRange = pUnit->getCursorInfoRange();
    if (infoRange >= 1)
    {
        spGameMenue pMenue = GameMenue::getInstance();
        Cursor* pCursor = pMenue->getCursor();
        pCursor->addCursorRangeOutline(infoRange, Qt::white);
    }
    m_pUnitPathFindingSystem->explore();
    createMarkedMoveFields();
}

void HumanPlayerInput::createMarkedField(QPoint point, QColor color)
{    
    if (m_pMap->onMap(point.x(), point.y()))
    {
        oxygine::spSprite pSprite = createMarkedFieldActor(point, color);
        m_FieldPoints.push_back(QPoint(point.x(), point.y()));
        m_Fields.push_back(pSprite);
    }
}

oxygine::spSprite HumanPlayerInput::createMarkedFieldActor(QPoint point, QColor color)
{    
    GameManager* pGameManager = GameManager::getInstance();
    oxygine::spSprite pSprite = oxygine::spSprite::create();
    oxygine::ResAnim* pAnim = pGameManager->getResAnim("marked+field");
    if (pAnim != nullptr)
    {
        if (pAnim->getTotalFrames() > 1 && !Settings::getStaticMarkedFields())
        {
            float initFrame = 0;
            oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim, initFrame, 0), oxygine::timeMS(static_cast<qint32>(pAnim->getTotalFrames() * GameMap::frameTime)), -1);
            pSprite->addTween(tween);
        }
        else
        {
            pSprite->setResAnim(pAnim);
        }
        pSprite->setColor(color);
        pSprite->setScale(static_cast<float>(GameMap::getImageSize()) / static_cast<float>(pAnim->getWidth()));
        pSprite->setPosition(point.x() * GameMap::getImageSize(), point.y() * GameMap::getImageSize());        
        m_pMap->getMarkedFieldsLayer()->addChild(pSprite);
    }
    return pSprite;
}

void HumanPlayerInput::createMarkedMoveFields()
{
    CONSOLE_PRINT("createMarkedMoveFields()", Console::eDEBUG);
    Mainapp::getInstance()->pauseRendering();
    clearMarkedFields();
    if (m_pUnitPathFindingSystem.get() != nullptr)
    {
        Interpreter* pInterpreter = Interpreter::getInstance();
        QColor multiTurnColor = QColor(128, 0, 255, 255);
        QJSValue ret = pInterpreter->doFunction("Global", "getMultiturnFieldColor");
        if (ret.isString())
        {
            multiTurnColor = QColor(ret.toString());
        }
        QColor turnColor = QColor(128, 255, 0, 255);
        ret = pInterpreter->doFunction("Global", "getTurnFieldColor");
        if (ret.isString())
        {
            turnColor = QColor(ret.toString());
        }
        qint32 movementpoints = m_pGameAction->getTargetUnit()->getMovementpoints(m_pGameAction->getTarget());
        auto points = m_pUnitPathFindingSystem->getAllNodePointsFast();
        for (auto & point : points)
        {
            if (m_pUnitPathFindingSystem->getTargetCosts(point.x(), point.y()) > movementpoints)
            {
                createMarkedField(point, multiTurnColor);
            }
            else
            {
                createMarkedField(point, turnColor);
            }
        }
        syncMarkedFields();
    }
    Mainapp::getInstance()->continueRendering();
}

void HumanPlayerInput::cursorMoved(qint32 x, qint32 y)
{
    spGameMenue pMenu = GameMenue::getInstance();
    if (pMenu.get() != nullptr)
    {
        
        auto mapPos = pMenu->getMapSlidingActor()->getPosition();
        m_lastMapView = QPoint(mapPos.x, mapPos.y);
        if (x != m_lastCursorPosition.x() ||
            y != m_lastCursorPosition.y())
        {
            if ((m_pMap->getCurrentPlayer() == m_pPlayer ||
                 m_pPlayer == nullptr) &&
                m_pMap->onMap(x, y))
            {
                CONSOLE_PRINT("HumanPlayerInput::cursorMoved" , Console::eDEBUG);
                if (m_pMarkedFieldData.get() != nullptr)
                {
                    if (m_pMarkedFieldData->getShowZData())
                    {
                        // marked field?
                        if (m_pMarkedFieldData->getPoints()->contains(QPoint(x, y)))
                        {
                            if (m_ZInformationLabel.get() != nullptr)
                            {
                                m_ZInformationLabel->detach();
                                m_ZInformationLabel = nullptr;
                            }
                            QPoint field(x, y);
                            const MarkedFieldData::ZInformation* pData = nullptr;
                            for (qint32 i = 0; i < m_pMarkedFieldData->getPoints()->size(); i++)
                            {
                                if (m_pMarkedFieldData->getPoints()->at(i) == field)
                                {
                                    const auto* info = m_pMarkedFieldData->getZInformation();
                                    pData = &info->at(i);
                                    break;
                                }
                            }
                            if (pData != nullptr)
                            {
                                if (pData->valueNames.size() == 0)
                                {
                                    createSimpleZInformation(x, y, pData);
                                }
                                else
                                {
                                    createComplexZInformation(x, y, pData);
                                }
                            }
                        }
                        else
                        {
                            if (m_ZInformationLabel.get() != nullptr)
                            {
                                m_ZInformationLabel->detach();
                                m_ZInformationLabel = nullptr;
                            }
                        }
                    }
                }
                else if (m_pUnitPathFindingSystem.get() != nullptr)
                {
                    if ((m_CurrentMenu.get() == nullptr) && m_pGameAction->getActionID() == "")
                    {
                        createCursorPath(x, y);
                    }
                }
                else
                {
                    Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
                    deleteArrow();
                    if (pUnit != nullptr)
                    {
                        auto multiTurnPath = pUnit->getMultiTurnPath();
                        std::vector<QPoint> path;
                        path.reserve(multiTurnPath.size());
                        for (auto & point : multiTurnPath)
                        {
                            path.push_back(point);
                        }
                        if (pUnit->getOwner() == m_pPlayer &&
                            multiTurnPath.size() > 0)
                        {
                            createArrow(path);
                        }
                    }
                }

            }
        }
        m_lastCursorPosition = QPoint(x, y);
    }
}

void HumanPlayerInput::createSimpleZInformation(qint32 x, qint32 y, const MarkedFieldData::ZInformation* pData)
{
    CONSOLE_PRINT("HumanPlayerInput::createSimpleZInformation " + QString::number(pData->singleValue) , Console::eDEBUG);
    
    QString labelText = "";
    labelText = QString::number(pData->singleValue) + " %";
    m_ZInformationLabel = oxygine::spActor::create();
    GameManager* pGameManager = GameManager::getInstance();
    oxygine::spSprite pSprite = oxygine::spSprite::create();
    oxygine::ResAnim* pAnim = pGameManager->getResAnim("z_information_label");
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
        oxygine::spSprite pSprite2 = oxygine::spSprite::create();
        oxygine::ResAnim* pAnim2 = pGameManager->getResAnim("z_information_label+mask");

        if (pAnim2->getTotalFrames() > 1)
        {
            oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim2), oxygine::timeMS(pAnim2->getTotalFrames() * GameMap::frameTime), -1);
            pSprite2->addTween(tween);
        }
        else
        {
            pSprite2->setResAnim(pAnim2);
        }
        QColor color = m_pMarkedFieldData->getZLabelColor();
        pSprite2->setColor(color.red(), color.green(), color.blue(), color.alpha());
        constexpr float baseScale = 4.0f;
        pSprite->setScale(baseScale);
        pSprite2->setScale(baseScale);
        m_ZInformationLabel->setSize(pAnim->getSize() * baseScale);
        m_ZInformationLabel->addChild(pSprite2);
        m_ZInformationLabel->addChild(pSprite);
        // add text to the label
        oxygine::spClipRectActor clipRec = oxygine::spClipRectActor::create();
        clipRec->setX(4);
        clipRec->setY(0);
        clipRec->setSize(28 * 4, 40);
        oxygine::spTextField textField = oxygine::spTextField::create();
        oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont72());
        style.color = FontManager::getFontColor();
        style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
        style.vAlign = oxygine::TextStyle::VALIGN_TOP;
        textField->setStyle(style);
        textField->setScale(32.0f / 72.0f);
        textField->setHtmlText(m_pMarkedFieldData->getZLabelText());
        textField->setSize(clipRec->getSize());
        clipRec->addChild(textField);
        m_ZInformationLabel->addChild(clipRec);

        oxygine::spTextField textField2 = oxygine::spTextField::create();
        textField2->setStyle(style);
        textField2->setY(44);
        textField2->setX(10);
        textField2->setSize(clipRec->getSize());
        textField2->setScale(32.0f / 72.0f);
        textField2->setHtmlText(labelText);
        m_ZInformationLabel->addChild(textField2);

        m_ZInformationLabel->setPriority(static_cast<qint32>(Mainapp::ZOrder::FocusedObjects));
        m_pMap->addChild(m_ZInformationLabel);
        zoomChanged(m_pMap->getZoom());
    }
}

bool HumanPlayerInput::inputAllowed()
{
    spGameMenue pMenu = GameMenue::getInstance();
    if (pMenu.get() != nullptr &&
        GameAnimationFactory::getAnimationCount() == 0 &&
        pMenu->getFocused())
    {
        
        if (m_pMap->getCurrentPlayer() == m_pPlayer &&
            m_pGameAction.get() == nullptr)
        {
            return true;
        }
    }
    return false;
}

void HumanPlayerInput::nextTurn()
{
    CONSOLE_PRINT("HumanPlayerInput::nextTurn()", Console::eDEBUG);
    spGameMenue pMenu = GameMenue::getInstance();
    if (inputAllowed())
    {
        spGameAction pAction = spGameAction::create(CoreAI::ACTION_NEXT_PLAYER, m_pMap);
        if (pAction->canBePerformed())
        {
            emit performAction(pAction);
        }
    }
}

void HumanPlayerInput::createComplexZInformation(qint32 x, qint32 y, const MarkedFieldData::ZInformation* pData)
{
    
    QString attackInfo = "Info: ";
    for (qint32 i = 0; i < pData->valueNames.size(); ++i)
    {
        attackInfo += " " + pData->valueNames[i] + " ";
        if (i <pData->ownUnitValues.size())
        {
            attackInfo += " own: " + QString::number(pData->ownUnitValues[i]);
        }
        if (i <pData->enemyUnitValues.size())
        {
            attackInfo += " enemy: " + QString::number(pData->enemyUnitValues[i]);
        }
    }
    CONSOLE_PRINT("HumanPlayerInput::createComplexZInformation " + attackInfo, Console::eDEBUG);
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::spBox9Sprite pBox = oxygine::spBox9Sprite::create();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("panel");
    pBox->setResAnim(pAnim);
    constexpr qint32 baseWidth = 90;
    constexpr qint32 textWidth = 60;
    qint32 itemWidth = textWidth;
    if (pData->ownUnitValues.size() > 0)
    {
        itemWidth += baseWidth;
    }
    if (pData->enemyUnitValues.size() > 0)
    {
        itemWidth += baseWidth;
    }
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.hAlign = oxygine::TextStyle::HALIGN_MIDDLE;
    style.vAlign = oxygine::TextStyle::VALIGN_TOP;
    pBox->setSize(itemWidth, 10 + 30 * pData->valueNames.size());
    for (qint32 i = 0; i < pData->valueNames.size(); ++i)
    {
        spLabel pLabel = spLabel::create(textWidth);
        pLabel->setStyle(style);
        pLabel->setHtmlText(pData->valueNames[i]);
        pLabel->setPosition(baseWidth, 30 * i + 5);
        pBox->addChild(pLabel);
    }
    if (pData->ownUnitValues.size() > 0)
    {
        oxygine::spColorRectSprite pRect = oxygine::spColorRectSprite::create();
        pRect->setPosition(7, 4);
        pRect->setSize(baseWidth, pBox->getHeight() - 10);
        if (m_pPlayer != nullptr)
        {
            pRect->setColor(m_pPlayer->getColor());
        }
        pRect->setAlpha(200);
        pBox->addChild(pRect);
        for (qint32 i = 0; i < pData->valueNames.size(); ++i)
        {
            spLabel pLabel = spLabel::create(baseWidth);
            pLabel->setStyle(style);
            if (pData->ownUnitValues[i] < 0)
            {
                pLabel->setHtmlText("-");
            }
            else
            {
                pLabel->setHtmlText(QString::number(pData->ownUnitValues[i]) + " %");
            }
            pLabel->setPosition(7, 30 * i + 5);
            pBox->addChild(pLabel);
        }
    }
    if (pData->enemyUnitValues.size() > 0)
    {
        oxygine::spColorRectSprite pRect = oxygine::spColorRectSprite::create();
        pRect->setSize(baseWidth, pBox->getHeight() - 10);
        pRect->setPosition(pBox->getWidth() - 4 - pRect->getWidth(), 4);
        pRect->setColor(pData->enemyColor);
        pRect->setAlpha(200);
        pBox->addChild(pRect);
        for (qint32 i = 0; i < pData->enemyUnitValues.size(); ++i)
        {
            spLabel pLabel = spLabel::create(baseWidth);
            pLabel->setStyle(style);
            if (pData->enemyUnitValues[i] < 0)
            {
                pLabel->setHtmlText("-");
            }
            else
            {
                pLabel->setHtmlText(QString::number(pData->enemyUnitValues[i]) + " %");
            }
            pLabel->setPosition(pRect->getX(), 30 * i + 5);
            pBox->addChild(pLabel);
        }
    }
    m_ZInformationLabel = pBox;
    m_ZInformationLabel->setPriority(static_cast<qint32>(Mainapp::ZOrder::FocusedObjects));
    m_pMap->addChild(m_ZInformationLabel);
    zoomChanged(m_pMap->getZoom());
}

void HumanPlayerInput::zoomChanged(float zoom)
{
    spGameMenue pMenu = GameMenue::getInstance();
    if (m_ZInformationLabel.get() != nullptr &&
        pMenu.get() != nullptr)
    {
        auto* pCursor = pMenu->getCursor();
        m_ZInformationLabel->setScale(1.0f / zoom);
        m_ZInformationLabel->setPosition(pCursor->getMapPointX() * GameMap::getImageSize() + GameMap::getImageSize() / 2 - m_ZInformationLabel->getScaledWidth() / 2,
                                         pCursor->getMapPointY() * GameMap::getImageSize() - 5 - m_ZInformationLabel->getScaledHeight());
    }
}

void HumanPlayerInput::createCursorPath(qint32 x, qint32 y)
{
    CONSOLE_PRINT("HumanPlayerInput::createCursorPath", Console::eDEBUG);
    auto points = m_ArrowPoints;
    QPoint lastPoint = QPoint(-1, -1);
    if (points.size() > 0)
    {
        lastPoint = points[0];
    }
    deleteArrow();
    if (m_pGameAction->getTarget() != QPoint(x, y) &&
        m_pUnitPathFindingSystem.get() != nullptr &&
        !m_pGameAction->getTargetUnit()->getHasMoved() &&
        GlobalUtils::contains(m_FieldPoints, QPoint(x, y)))
    {
        if (m_pUnitPathFindingSystem->getCosts(m_pUnitPathFindingSystem->getIndex(x, y), x, y, x, y, 0) >= 0)
        {
            // is it a neighbour field to the last target?
            if (((points.size() > 0) && ((points[0].x() - x + points[0].y() - y) != 0)))
            {
                if ((points.size() > 0) && ((qAbs(points[0].x() - x) + qAbs(points[0].y() - y)) == 1))
                {
                    if (GlobalUtils::contains(points, QPoint(x, y)))
                    {
                        points = m_pUnitPathFindingSystem->getPathFast(x, y);
                    }
                    else if (m_pUnitPathFindingSystem->getCosts(m_pUnitPathFindingSystem->getIndex(x, y), x, y, points[0].x(), points[0].y(), 0) >= 0)
                    {
                        points.insert(points.cbegin(), QPoint(x, y));
                        qint32 movepoints = m_pGameAction->getTargetUnit()->getMovementpoints(QPoint(x, y));
                        if ((m_pUnitPathFindingSystem->getTargetCosts(x, y)  <= movepoints) &&
                            (m_pUnitPathFindingSystem->getCosts(points)  > movepoints))
                        {
                            // not reachable this way get the ideal path
                            points = m_pUnitPathFindingSystem->getPathFast(x, y);
                        }
                    }
                    else
                    {
                        points = m_pUnitPathFindingSystem->getPathFast(x, y);
                    }
                }
                else
                {
                    points = m_pUnitPathFindingSystem->getPathFast(x, y);
                }
            }
            else if (points.size() == 0)
            {
                points = m_pUnitPathFindingSystem->getPathFast(x, y);
            }
            else
            {
                // do nothing
            }
            if ((points.size() == 0) ||
                (points[0].x() != x) ||
                (points[0].y() != y))
            {
                points = m_pUnitPathFindingSystem->getPathFast(x, y);
            }
            m_pGameAction->setCosts(m_pUnitPathFindingSystem->getCosts(points));
            m_ArrowPoints = points;
            createArrow(m_ArrowPoints);
        }
    }
}

QStringList HumanPlayerInput::getEmptyActionList()
{
    CONSOLE_PRINT("HumanPlayerInput::getEmptyActionList", Console::eDEBUG);
    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValue value = pInterpreter->doFunction("ACTION", "getEmptyFieldActions");
    if (value.isString())
    {
        return value.toString().split(",");
    }
    else
    {
        return value.toVariant().toStringList();
    }
}

QStringList HumanPlayerInput::getViewplayerActionList()
{
    CONSOLE_PRINT("HumanPlayerInput::getViewplayerActionList", Console::eDEBUG);
    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValue value = pInterpreter->doFunction("ACTION", "getViewplayerActionList");
    if (value.isString())
    {
        return value.toString().split(",");
    }
    else
    {
        return value.toVariant().toStringList();
    }
}

void HumanPlayerInput::createArrow(std::vector<QPoint>& points)
{
    
    GameManager* pGameManager = GameManager::getInstance();
    for (qint32 i = 0; i < points.size() - 1; i++)
    {
        oxygine::spSprite pSprite = oxygine::spSprite::create();
        oxygine::ResAnim* pAnim = pGameManager->getResAnim("arrow+unit");
        if (pAnim != nullptr)
        {
            pSprite->setResAnim(pAnim);
            pSprite->setScale((GameMap::getImageSize()) / pAnim->getWidth());
            pSprite->setPosition(points[i].x() * GameMap::getImageSize() -(pSprite->getScaledWidth() - GameMap::getImageSize()) / 2,  points[i].y() * GameMap::getImageSize() -(pSprite->getScaledHeight() - GameMap::getImageSize()));
            m_pMap->getMoveArrowLayer()->addChild(pSprite);
            m_Arrows.push_back(pSprite);

            if (i > 0)
            {
                // select arrow
                if (((points[i].x() < points[i + 1].x()) && (points[i].x() > points[i - 1].x())) ||
                    ((points[i].x() < points[i - 1].x()) && (points[i].x() > points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::LeftRight));
                }
                else if (((points[i].y() < points[i + 1].y()) && (points[i].y() > points[i - 1].y())) ||
                         ((points[i].y() < points[i - 1].y()) && (points[i].y() > points[i + 1].y())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::UpDown));
                }
                else if (((points[i].y() < points[i + 1].y()) && (points[i].x() < points[i - 1].x())) ||
                         ((points[i].y() < points[i - 1].y()) && (points[i].x() < points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::DownRight));
                }
                else if (((points[i].y() < points[i + 1].y()) && (points[i].x() > points[i - 1].x())) ||
                         ((points[i].y() < points[i - 1].y()) && (points[i].x() > points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::DownLeft));
                }
                else if (((points[i].y() > points[i + 1].y()) && (points[i].x() < points[i - 1].x())) ||
                         ((points[i].y() > points[i - 1].y()) && (points[i].x() < points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::UpRight));
                }
                else if (((points[i].y() > points[i + 1].y()) && (points[i].x() > points[i - 1].x())) ||
                         ((points[i].y() > points[i - 1].y()) && (points[i].x() > points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::UpLeft));
                }
            }
            else
            {
                // final arrow
                if (points[i].x() < points[i + 1].x())
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::Right));
                }
                else if (points[i].x() > points[i + 1].x())
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::Left));
                }
                else if (points[i].y() < points[i + 1].y())
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::Down));
                }
                else if (points[i].y() > points[i + 1].y())
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::Up));
                }
            }
        }
    }
}

void HumanPlayerInput::deleteArrow()
{
    for (qint32 i = 0; i < m_Arrows.size(); i++)
    {
        m_Arrows[i]->detach();
    }
    m_ArrowPoints.clear();
    m_Arrows.clear();
}

void HumanPlayerInput::gotoNext()
{
    if (inputAllowed())
    {
        if (m_FieldPoints.size() > 0)
        {
            nextMarkedField();
        }
        else
        {
            nextSelectOption();
        }
    }
}

void HumanPlayerInput::performBasicAction(QString action)
{
    if (inputAllowed())
    {
        spGameAction pAction = spGameAction::create(action, m_pMap);
        if (pAction->canBePerformed())
        {
            emit performAction(pAction);
        }
    }
}

void HumanPlayerInput::gotoPrevious()
{
    if (inputAllowed())
    {
        if (m_FieldPoints.size() > 0)
        {
            previousMarkedField();
        }
        else
        {
            previousSelectOption();
        }
    }
}

void HumanPlayerInput::keyDown(oxygine::KeyEvent event)
{
    if (!event.getContinousPress())
    {
        spGameMenue pMenu = GameMenue::getInstance();
        bool canInput = inputAllowed();
        // for debugging
        Qt::Key cur = event.getKey();
        if ((cur == Settings::getKey_next() ||
             cur == Settings::getKey_next2()) &&
            canInput)
        {
            gotoNext();
        }
        else if ((cur == Settings::getKey_previous() ||
                  cur == Settings::getKey_previous2()) &&
                 canInput)
        {
            gotoPrevious();
        }
        else if (cur == Settings::getKey_ShowAttackFields() ||
                 cur == Settings::getKey_ShowAttackFields2())
        {
            showSelectedUnitAttackableFields(true);
        }
        else if (cur == Settings::getKey_ShowIndirectAttackFields() ||
                 cur == Settings::getKey_ShowIndirectAttackFields2())
        {
            showSelectedUnitAttackableFields(false);
        }
    }
}

void HumanPlayerInput::showSelectedUnitAttackableFields(bool all)
{
    CONSOLE_PRINT("HumanPlayerInput::showSelectedUnitAttackableFields", Console::eDEBUG);
    spGameMenue pMenu = GameMenue::getInstance();
    if (m_pUnitPathFindingSystem.get() != nullptr &&
        m_pGameAction.get() != nullptr &&
        m_CurrentMenu.get() == nullptr &&
        m_pMap->getCurrentPlayer() == m_pPlayer &&
        pMenu.get() != nullptr &&
        GameAnimationFactory::getAnimationCount() == 0 &&
        pMenu->getFocused())
    {
        if (m_InfoFields.size() > 0)
        {
            for (auto & fields : m_InfoFields)
            {
                fields->detach();
            }
            m_InfoFields.clear();
            for (auto & fields : m_Fields)
            {
                fields->setVisible(true);
            }
        }
        else
        {
            Mainapp::getInstance()->getAudioThread()->playSound("selectunit.wav");
            for (auto & fields : m_Fields)
            {
                fields->setVisible(false);
            }
            Unit* currentUnit = m_pGameAction->getTargetUnit();
            QPoint position = currentUnit->getPosition();
            qint32 distance = currentUnit->getMovementpoints(position);
            distance += currentUnit->getMaxRange(position);

            std::vector<QPoint> usedFields;
            
            for (qint32 x = 0; x < m_pMap->getMapWidth(); x++)
            {
                for (qint32 y = 0; y < m_pMap->getMapHeight(); y++)
                {
                    Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
                    if ((pUnit != nullptr) &&
                        m_pPlayer->isEnemyUnit(pUnit) &&
                        (!pUnit->isStealthed(m_pPlayer)))
                    {
                        QPoint unitPositionpUnit(x, y);
                        qint32 currentMaxDistance = distance;
                        currentMaxDistance += pUnit->getMovementpoints(unitPositionpUnit);
                        currentMaxDistance += pUnit->getMaxRange(unitPositionpUnit);
                        if (GlobalUtils::getDistance(unitPositionpUnit, position) <= currentMaxDistance)
                        {
                            if(all || pUnit->getBaseMaxRange() > 1)
                            {
                                showUnitAttackFields(pUnit, usedFields);
                            }
                        }
                    }
                }
            }
        }
    }
    
}

void HumanPlayerInput::showUnitAttackFields(Unit* pUnit, std::vector<QPoint> & usedFields)
{
    Mainapp::getInstance()->pauseRendering();
    UnitPathFindingSystem pfs(m_pMap, pUnit, m_pPlayer);
    std::vector<QPoint> points;
    QPoint position = pUnit->getPosition();
    bool canMoveAndFire = pUnit->canMoveAndFire(position);
    if (canMoveAndFire)
    {
        pfs.explore();
        points = pfs.getAllNodePointsFast();
    }
    else
    {
        points.push_back(position);
    }
    
    qint32 maxRange = pUnit->getMaxRange(position);
    qint32 minRange = pUnit->getMinRange(position);
    spQmlVectorPoint pPoints = spQmlVectorPoint(GlobalUtils::getCircle(minRange, maxRange));
    for (auto & point : points)
    {
        if (canMoveAndFire ||
            (point.x() == position.x() && point.y() == position.y()))
        {
            for (auto & rangePos : pPoints->getVector())
            {
                QPoint target = rangePos + point;
                if (m_pMap->onMap(target.x(), target.y()) &&
                    !GlobalUtils::contains(usedFields, QPoint(target.x(), target.y())))
                {
                    usedFields.push_back(target);
                    m_InfoFields.push_back(createMarkedFieldActor(target, QColor(255, 0, 0)));
                }
            }
        }
    }
    syncMarkedFields();
    Mainapp::getInstance()->continueRendering();
}

void HumanPlayerInput::nextMarkedField()
{
    
    spGameMenue pGameMenue = GameMenue::getInstance();
    bool center = Settings::getCenterOnMarkedField();
    if (pGameMenue.get() != nullptr)
    {
        qint32 width = m_pMap->getMapWidth();
        qint32 heigth = m_pMap->getMapHeight();
        qint32 startX = pGameMenue->getCursor()->getMapPointX();
        qint32 startY = pGameMenue->getCursor()->getMapPointY();
        qint32 x = startX + 1;
        qint32 y = startY;
        bool found = false;
        while (y  < heigth && !found)
        {
            while (x < width && !found)
            {
                for (auto & field : m_FieldPoints)
                {
                    if (x == static_cast<qint32>(field.x()) &&
                       (y == static_cast<qint32>(field.y())))
                    {
                        if (center)
                        {
                            m_pMap->centerMap(x, y);
                        }
                        pGameMenue->calcNewMousePosition(x, y);
                        found = true;
                        break;
                    }
                }
                x++;
            }
            y++;
            x = 0;
        }
        x = 0;
        y = 0;
        while (y  < heigth && !found )
        {
            while (x < width && !found)
            {
                for (auto & field : m_FieldPoints)
                {
                    if (x == static_cast<qint32>(field.x()) &&
                        (y == static_cast<qint32>(field.y())))
                    {
                        if (center)
                        {
                            m_pMap->centerMap(x, y);
                        }
                        pGameMenue->calcNewMousePosition(x, y);
                        found = true;
                        break;
                    }
                }
                x++;
                if ((y == startY && x == startX))
                {
                    found = true;
                }
            }
            y++;
            x = 0;
        }
    }
}

void HumanPlayerInput::previousMarkedField()
{
    
    spGameMenue pGameMenue = GameMenue::getInstance();
    if (pGameMenue.get() != nullptr)
    {
        qint32 width = m_pMap->getMapWidth();
        qint32 heigth = m_pMap->getMapHeight();
        qint32 startX = pGameMenue->getCursor()->getMapPointX();
        qint32 startY = pGameMenue->getCursor()->getMapPointY();
        qint32 x = startX - 1;
        qint32 y = startY;
        bool found = false;
        bool center = Settings::getCenterOnMarkedField();
        while (y  >= 0 && !found)
        {
            while (x >= 0 && !found)
            {
                for (auto & field : m_FieldPoints)
                {
                    if (x == static_cast<qint32>(field.x()) &&
                        (y == static_cast<qint32>(field.y())))
                    {
                        if (center)
                        {
                            m_pMap->centerMap(x, y);
                        }
                        pGameMenue->calcNewMousePosition(x, y);
                        found = true;
                        break;
                    }
                }
                x--;
            }
            y--;
            x = width - 1;
        }
        x = width - 1;
        y = heigth - 1;
        while (y  >= 0 && !found )
        {
            while (x >= 0 && !found)
            {
                for (auto & field : m_FieldPoints)
                {
                    if (x == static_cast<qint32>(field.x()) &&
                        (y == static_cast<qint32>(field.y())))
                    {
                        if (center)
                        {
                            m_pMap->centerMap(x, y);
                        }
                        pGameMenue->calcNewMousePosition(x, y);
                        found = true;
                        break;
                    }
                }
                x--;
                if ((y == startY && x == startX))
                {
                    found = true;
                }
            }
            y--;
            x = width - 1;
        }
    }
}

void HumanPlayerInput::nextSelectOption()
{
    
    spGameMenue pGameMenue = GameMenue::getInstance();
    if (pGameMenue.get() != nullptr)
    {
        qint32 width = m_pMap->getMapWidth();
        qint32 heigth = m_pMap->getMapHeight();
        qint32 startX = pGameMenue->getCursor()->getMapPointX();
        qint32 startY = pGameMenue->getCursor()->getMapPointY();
        qint32 x = startX + 1;
        qint32 y = startY;
        bool found = false;
        bool center = Settings::getCenterOnMarkedField();
        if (m_pMap->onMap(startX, startY))
        {
            while (y  < heigth && !found)
            {
                while (x < width && !found)
                {
                    Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
                    Building* pBuilding = m_pMap->getTerrain(x, y)->getBuilding();
                    if (pUnit != nullptr &&
                        pUnit->getOwner() == m_pPlayer &&
                        !pUnit->getHasMoved())
                    {
                        if (center)
                        {
                            m_pMap->centerMap(x, y);
                        }
                        pGameMenue->calcNewMousePosition(x, y);
                        found = true;
                    }
                    else if ((pBuilding != nullptr) &&
                             (pBuilding->getOwner() == m_pPlayer))
                    {
                        GameAction action(m_pMap);
                        action.setTarget(QPoint(x, y));
                        QStringList actions = pBuilding->getActionList();
                        for (auto & actionId : actions)
                        {
                            if (action.canBePerformed(actionId))
                            {
                                if (center)
                                {
                                    m_pMap->centerMap(x, y);
                                }
                                pGameMenue->calcNewMousePosition(x, y);
                                found = true;
                                break;
                            }
                        }
                    }
                    x++;
                }
                y++;
                x = 0;
            }
            x = 0;
            y = 0;
            while (y  < heigth && !found )
            {
                while (x < width && !found)
                {
                    Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
                    Building* pBuilding = m_pMap->getTerrain(x, y)->getBuilding();
                    if (pUnit != nullptr &&
                        pUnit->getOwner() == m_pPlayer &&
                        !pUnit->getHasMoved())
                    {
                        if (center)
                        {
                            m_pMap->centerMap(x, y);
                        }
                        pGameMenue->calcNewMousePosition(x, y);
                        found = true;
                    }
                    else if ((pBuilding != nullptr) &&
                             (pBuilding->getOwner() == m_pPlayer))
                    {
                        GameAction action(m_pMap);
                        action.setTarget(QPoint(x, y));
                        QStringList actions = pBuilding->getActionList();
                        for (auto & actionId : actions)
                        {
                            if (action.canBePerformed(actionId))
                            {
                                if (center)
                                {
                                    m_pMap->centerMap(x, y);
                                }
                                pGameMenue->calcNewMousePosition(x, y);
                                found = true;
                                break;
                            }
                        }
                    }
                    x++;
                    if ((y == startY && x == startX))
                    {
                        found = true;
                    }
                }
                y++;
                x = 0;
            }
        }
    }
}

void HumanPlayerInput::previousSelectOption()
{
    
    spGameMenue pGameMenue = GameMenue::getInstance();
    if (pGameMenue.get() != nullptr)
    {
        qint32 width = m_pMap->getMapWidth();
        qint32 heigth = m_pMap->getMapHeight();
        qint32 startX = pGameMenue->getCursor()->getMapPointX();
        qint32 startY = pGameMenue->getCursor()->getMapPointY();
        qint32 x = startX - 1;
        qint32 y = startY;
        bool found = false;
        bool center = Settings::getCenterOnMarkedField();
        if (m_pMap->onMap(startX, startY))
        {
            while (y  >= 0 && !found)
            {
                while (x >= 0 && !found)
                {
                    Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
                    Building* pBuilding = m_pMap->getTerrain(x, y)->getBuilding();
                    if (pUnit != nullptr &&
                        pUnit->getOwner() == m_pPlayer &&
                        !pUnit->getHasMoved())
                    {
                        if (center)
                        {
                            m_pMap->centerMap(x, y);
                        }
                        pGameMenue->calcNewMousePosition(x, y);
                        found = true;
                    }
                    else if ((pBuilding != nullptr) &&
                             (pBuilding->getOwner() == m_pPlayer))
                    {
                        GameAction action(m_pMap);
                        action.setTarget(QPoint(x, y));
                        QStringList actions = pBuilding->getActionList();
                        for (auto & actionId : actions)
                        {
                            if (action.canBePerformed(actionId))
                            {
                                if (center)
                                {
                                    m_pMap->centerMap(x, y);
                                }
                                pGameMenue->calcNewMousePosition(x, y);
                                found = true;
                                break;
                            }
                        }
                    }
                    x--;
                }
                y--;
                x = width - 1;
            }
            x = width - 1;
            y = heigth - 1;
            while (y  >= 0 && !found )
            {
                while (x >= 0 && !found)
                {
                    Unit* pUnit = m_pMap->getTerrain(x, y)->getUnit();
                    Building* pBuilding = m_pMap->getTerrain(x, y)->getBuilding();
                    if (pUnit != nullptr &&
                        pUnit->getOwner() == m_pPlayer &&
                        !pUnit->getHasMoved())
                    {
                        if (center)
                        {
                            m_pMap->centerMap(x, y);
                        }
                        pGameMenue->calcNewMousePosition(x, y);
                        found = true;
                    }
                    else if ((pBuilding != nullptr) &&
                             (pBuilding->getOwner() == m_pPlayer))
                    {
                        GameAction action(m_pMap);
                        action.setTarget(QPoint(x, y));
                        QStringList actions = pBuilding->getActionList();
                        for (auto & actionId : actions)
                        {
                            if (action.canBePerformed(actionId))
                            {
                                if (center)
                                {
                                    m_pMap->centerMap(x, y);
                                }
                                pGameMenue->calcNewMousePosition(x, y);
                                found = true;
                                break;
                            }
                        }
                    }
                    if ((y == startY && x == startX))
                    {
                        found = true;
                    }
                    x--;
                }
                y--;
                x = width - 1;
            }
        }
    }
}

void HumanPlayerInput::autoEndTurn()
{
    
    if (m_pPlayer != nullptr &&
        m_pMap != nullptr &&
        m_pMap->getCurrentPlayer() == m_pPlayer)
    {
        CONSOLE_PRINT("HumanPlayerInput::autoEndTurn", Console::eDEBUG);
        CO* pCO0 = m_pPlayer->getCO(0);
        CO* pCO1 = m_pPlayer->getCO(1);
        if (Settings::getAutoEndTurn() &&
            GameAnimationFactory::getAnimationCount() == 0 &&
            (pCO0 == nullptr || (!pCO0->canUsePower() && !pCO0->canUseSuperpower())) &&
            (pCO1 == nullptr || (!pCO1->canUsePower() && !pCO1->canUseSuperpower())))
        {
            qint32 heigth = m_pMap->getMapHeight();
            qint32 width = m_pMap->getMapWidth();
            for (qint32 y = 0; y < heigth; y++)
            {
                for (qint32 x = 0; x < width; x++)
                {
                    Terrain* pTerrain = m_pMap->getTerrain(x, y);
                    Unit* pUnit = pTerrain->getUnit();
                    Building* pBuilding = pTerrain->getBuilding();
                    if (pUnit != nullptr && pUnit->getOwner() == m_pPlayer && !pUnit->getHasMoved())
                    {
                        return;
                    }
                    if (pBuilding != nullptr && pBuilding->getOwner() == m_pPlayer)
                    {
                        GameAction action(m_pMap);
                        action.setTarget(QPoint(x, y));
                        QStringList actions = pBuilding->getActionList();
                        for (auto & actionId : actions)
                        {
                            action.setActionID(actionId);
                            if (action.canBePerformed())
                            {
                                return;
                            }
                        }
                    }
                }
            }
            CONSOLE_PRINT("Auto triggering next player cause current player can't input any actions.", Console::eDEBUG);
            spGameAction pAction = spGameAction::create(CoreAI::ACTION_NEXT_PLAYER, m_pMap);
            emit performAction(pAction);
        }
    }
}

void HumanPlayerInput::serializeObject(QDataStream& stream) const
{
    stream << getVersion();
}

void HumanPlayerInput::deserializeObject(QDataStream& stream)
{
    qint32 version;
    stream >> version;
    if (version == 2)
    {
        QString dummy;
        stream >> dummy;
    }
}

void HumanPlayerInput::centerCameraOnAction(GameAction* pAction)
{
    
    spGameMenue pGameMenue = GameMenue::getInstance();
    if (m_pMap != nullptr && pGameMenue.get() != nullptr &&
        (m_pMap->getCurrentPlayer() == m_pPlayer ||
         m_pPlayer == nullptr))
    {
        if (Settings::getAutoCamera() && pAction == nullptr &&
            m_lastMapView.x() != std::numeric_limits<qint32>::min())
        {
            switch (Settings::getAutoFocusing())
            {
                case GameEnums::AutoFocusing_Owned:
                {
                    m_pMap->centerOnPlayer(m_pPlayer);
                    break;
                }
                case GameEnums::AutoFocusing_LastPos:
                {
                    pGameMenue->getMapSlidingActor()->setPosition(m_lastMapView.x(), m_lastMapView.y());
                    break;
                }
            }
        }
    }
}
