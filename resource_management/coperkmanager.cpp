#include "resource_management/coperkmanager.h"

COPerkManager::COPerkManager()
    : RessourceManagement<COPerkManager>("/images/co_perks/res.xml",
                                         "/scripts/co_perks")
{
    Interpreter::setCppOwnerShip(this);
#ifdef GRAPHICSUPPORT
    setObjectName("COPerkManager");
#endif
}

QString COPerkManager::getIcon(qint32 position)
{
    if ((position >= 0) && (position < m_loadedRessources.size()))
    {
        Interpreter* pInterpreter = Interpreter::getInstance();
        QJSValue value = pInterpreter->doFunction(m_loadedRessources[position], "getIcon");
        if (value.isString())
        {
            return value.toString();
        }
    }
    return "";
}

bool COPerkManager::isSelectable(qint32 position)
{
    if ((position >= 0) && (position < m_loadedRessources.size()))
    {
        return isSelectable(m_loadedRessources[position]);
    }
    return true;
}

bool COPerkManager::isSelectable(QString id)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValue value = pInterpreter->doFunction(id, "isSelectable");
    if (value.isBool())
    {
        return value.toBool();
    }
    return true;
}

QString COPerkManager::getDescription(qint32 position)
{
    if ((position >= 0) && (position < m_loadedRessources.size()))
    {
        Interpreter* pInterpreter = Interpreter::getInstance();
        QJSValue value = pInterpreter->doFunction(m_loadedRessources[position], "getDescription");
        if (value.isString())
        {
            return value.toString();
        }
    }
    return "";
}

QString COPerkManager::getGroup(qint32 position)
{
    if ((position >= 0) && (position < m_loadedRessources.size()))
    {
        Interpreter* pInterpreter = Interpreter::getInstance();
        QJSValue value = pInterpreter->doFunction(m_loadedRessources[position], "getGroup");
        if (value.isString())
        {
            return value.toString();
        }
    }
    return "";
}

void COPerkManager::removeRessource(QString id)
{
    for (qint32 i = 0; i < m_loadedRessources.size(); ++i)
    {
        if (m_loadedRessources[i] == id)
        {
            m_loadedRessources.removeAt(i);
            break;
        }
    }
}
