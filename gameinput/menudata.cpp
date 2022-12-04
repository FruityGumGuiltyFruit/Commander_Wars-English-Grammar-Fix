#include "gameinput/menudata.h"

#include "coreengine/gameconsole.h"

#include "resource_management/gamemanager.h"

#include "game/gamemap.h"
#include "game/unit.h"
#include "game/co.h"

MenuData::MenuData(GameMap* pMap)
    : m_pMap(pMap)
{
#ifdef GRAPHICSUPPORT
    setObjectName("MenuData");
#endif
    Interpreter::setCppOwnerShip(this);
    Interpreter::getInstance()->trackJsObject(this);
}

MenuData::~MenuData()
{
}

void MenuData::addData(QString text, QString actionID, QString icon, qint32 costs, bool enabled)
{
    m_texts.append(text);
    m_actionIDs.append(actionID);
    m_costList.append(costs);
    m_enabledList.append(enabled);
    GameManager* pGameManager = GameManager::getInstance();
    m_iconList.append(pGameManager->getIcon(m_pMap, icon));
}

void MenuData::addUnitData(QString text, QString actionID, Unit* pIcon, qint32 costs, bool enabled)
{
    m_texts.append(text);
    m_actionIDs.append(actionID);
    m_costList.append(costs);
    m_enabledList.append(enabled);
    GameManager* pGameManager = GameManager::getInstance();
    spUnit menuIcon = oxygine::dynamic_pointer_cast<Unit>(pGameManager->getIcon(m_pMap, pIcon->getUnitID()));
    if (menuIcon.get() != nullptr)
    {
        menuIcon->setHasMoved(pIcon->getHasMoved());
        menuIcon->setHp(pIcon->getHp());
        menuIcon->setAmmo1(pIcon->getAmmo1());
        menuIcon->setAmmo2(pIcon->getAmmo2());
        menuIcon->setFuel(pIcon->getFuel());
        menuIcon->setUnitRank(pIcon->getUnitRank());
        m_iconList.append(menuIcon);
    }
    else
    {
        m_iconList.append(oxygine::spSprite::create());
    }

}

bool MenuData::validData()
{
    if (m_texts.size() > 0 &&
        m_texts.size() == m_actionIDs.size() &&
        m_actionIDs.size() == m_costList.size() &&
        m_costList.size() == m_enabledList.size())
    {
        return true;
    }
    CONSOLE_PRINT("Menu data is inconsistent and considered broken. Game gets stucked now. This may stop the ai from working.", GameConsole::eERROR);
    return false;
}

GameMap *MenuData::getMap() const
{
    return m_pMap;
}
