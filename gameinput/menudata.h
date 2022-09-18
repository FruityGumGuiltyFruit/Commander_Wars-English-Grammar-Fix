#ifndef MENUDATA_H
#define MENUDATA_H

#include "3rd_party/oxygine-framework/oxygine/actor/Actor.h"
#include <QObject>
#include <QStringList>
#include <QVector>

class Unit;
class GameMap;
class MenuData;
using spMenuData = oxygine::intrusive_ptr<MenuData>;

class MenuData final : public QObject, public oxygine::ref_counter
{
    Q_OBJECT
public:
    explicit MenuData(GameMap* pMap);
    ~MenuData();
    QStringList getTexts()
    {
        return m_texts;
    }
    QVector<oxygine::spActor> getIconList()
    {
        return m_iconList;
    }

public slots:
    /**
     * @brief getActionIDs
     * @return
     */
    QStringList getActionIDs()
    {
        return m_actionIDs;
    }
    /**
     * @brief getCostList
     * @return
     */
    QVector<qint32> getCostList()
    {
        return m_costList;
    }
    /**
     * @brief getEnabledList
     * @return
     */
    QVector<bool> getEnabledList()
    {
        return m_enabledList;
    }
    /**
     * @brief getMap
     * @return
     */
    GameMap *getMap() const;
    /**
     * @brief addData adds data to a later shown menu
     * @param text the text shown in the menu
     * @param actionID the id written into the action data to identify which menu item was selected
     * @param icon the unique string representing the icon to be shown currently units and normal icons are supported
     * @param costs the costs given to the action when executing the given action
     * @param enabled if the menu item is selectable or not
     */
    void addData(QString text, QString actionID, QString icon, qint32 costs = 0, bool enabled = true);
    /**
     * @brief addData adds data to a later shown menu
     * @param text the text shown in the menu
     * @param actionID the id written into the action data to identify which menu item was selected
     * @param pIcon the unit that will be shown including it's current ammo, fuel and hp stats
     * @param costs the costs given to the action when executing the given action
     * @param enabled if the menu item is selectable or not
     */
    void addUnitData(QString text, QString actionID, Unit* pIcon, qint32 costs = 0, bool enabled = true);
    /**
     * @brief validData verifies if the menu data is valid. In case of wrongly used javascript implementation
     * @return
     */
    bool validData();
    /**
     * @brief remove
     */
    void remove()
    {
        delete this;
    }
private:
    QStringList m_texts;
    QStringList m_actionIDs;
    QVector<qint32> m_costList;
    QVector<oxygine::spActor> m_iconList;
    QVector<bool> m_enabledList;
    GameMap* m_pMap{nullptr};
};

#endif // MENUDATA_H
