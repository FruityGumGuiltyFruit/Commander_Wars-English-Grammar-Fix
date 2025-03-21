#ifndef PATHFINDINGSYSTEM_H
#define PATHFINDINGSYSTEM_H

#include <QtGlobal>
#include <QObject>
#include <QVector>
#include <QPoint>
#include <QList>

#include "coreengine/qmlvector.h"

class PathFindingSystem;
using spPathFindingSystem = std::shared_ptr<PathFindingSystem>;

class PathFindingSystem : public QObject
{
    Q_OBJECT
public:
    enum Directions
    {
        West,
        East,
        South,
        North,
        Max,
        Unknown
    };
    static const qint32 infinite;
    /**
     * @brief node we want to check or checked already
     */
    struct Node
    {
        Node(qint32 x, qint32 y, qint32 index,
             qint32 totalCost, qint32 currentCosts,
             qint32 prevNodeX, qint32 prevNodeY, qint32 distance)
            : x(x),
              y(y),
              index(index),
              totalCost(totalCost),
              currentCosts(currentCosts),
              prevNodeX(prevNodeX),
              prevNodeY(prevNodeY),
              distance(distance)
        {
        }
        /**
         * @brief x
         */
        qint32 x;
        /**
         * @brief y
         */
        qint32 y;
        /**
         * @brief index
         */
        qint32 index;
        /**
         * @brief totalCost total estimated cost using this path
         */
        qint32 totalCost;
        /**
         * @brief currentCosts
         */
        qint32 currentCosts;
        /**
         * @brief prevNodeX
         */
        qint32 prevNodeX;
        /**
         * @brief prevNodeY
         */
        qint32 prevNodeY;
        /**
         * @brief distance
         */
        qint32 distance;
    };
    /**
     * @brief PathFindingSystem
     * @param startX start x for the node
     * @param startY start y for the node
     */
    explicit PathFindingSystem(qint32 startX, qint32 startY,
                               qint32 width, qint32 heigth);
    virtual ~PathFindingSystem() = default;
    /**
     * @brief setStartPoint
     * @param startX
     * @param startY
     */
    void setStartPoint(qint32 startX, qint32 startY);
    /**
     * @brief getRemainingCost
     * @param x
     * @param y
     * @return the remaining costs for this node to reach the target
     */
    virtual qint32 getRemainingCost(qint32 x, qint32 y, qint32 currentCost) = 0;
    /**
     * @brief finished checks if this would be the target node to reach
     * @param x
     * @param y
     * @param costs
     * @return
     */
    virtual bool finished(qint32 x, qint32 y, qint32 costs) = 0;
    /**
     * @brief getCosts for optimized speed this function should fill the movecosts array and return values from their if the field was called already
     * @param index
     * @param x
     * @param y
     * @return
     */
    virtual qint32 getCosts(qint32 index, qint32 x, qint32 y, qint32 curX, qint32 curY, qint32 currentCost) = 0;
    /**
     * @brief explores the map until the open list is empty or finished returned true.
     * This will also add the start point to the open list
     */
    void explore();
    /**
     * @brief setFinishNode
     */
    virtual void setFinishNode(qint32 x, qint32 y);
    /**
     * @brief get the Path to the given field as vector of qpoints. an empty vector means unreachable
     * @param x
     * @param y
     * @return the first point is the target and the last point is the start
     */
    std::vector<QPoint> getPathFast(qint32 x, qint32 y) const;
    /**
     * @brief getTargetPath
     * @return
     */
    std::vector<QPoint> getTargetPathFast() const;
    /**
     * @brief getFields searches for all fields in the range of min and max ignoring all movement costs
     * @param min minimum search range
     * @param max maximum search range
     * @return shared pointer to the points
     */
    static std::vector<QPoint> getFieldsFast(qint32 startX, qint32 startY, qint32 min, qint32 max);
    /**
     * @brief getAllNodePoints returns all reachable fields in a point vector
     * @param maxRange max costs of the target fields (costs need to be smaller than this value)
     * @return
     */
    std::vector<QPoint> getAllNodePointsFast(qint32 maxRange = infinite);
    /**
     * @brief remove
     */
    Q_INVOKABLE void remove()
    {
    }
    /**
     * @brief getTarget
     * @return
     */
    Q_INVOKABLE QPoint getTarget() const;
    /**
     * @brief getCosts
     * @param x
     * @param y
     * @return
     */
    Q_INVOKABLE qint32 getTargetCosts(qint32 x, qint32 y) const;
    /**
     * @brief isReachable
     * @param x
     * @param y
     * @return if the given field is reachable
     */
    Q_INVOKABLE bool isReachable(qint32 x, qint32 y) const;
    /**
     * @brief getAllNodePoints returns all reachable fields in a point vector
     * @return
     */
    Q_INVOKABLE QmlVectorPoint* getAllQmlVectorPoints();
    /**
     * @brief getIndex
     * @param x
     * @param y
     * @return
     */
    Q_INVOKABLE inline qint32 getIndex(qint32 x, qint32 y) const
    {
        return x + y * m_width;
    }
    /**
     * @brief get the Path to the given field as vector of qpoints. an empty vector means unreachable
     * @param x
     * @param y
     * @return the first point is the target and the last point is the start
     */
    Q_INVOKABLE QVector<QPoint> getPath(qint32 x, qint32 y) const;
    /**
     * @brief getTargetPath
     * @return
     */
    Q_INVOKABLE QVector<QPoint> getTargetPath() const;
    /**
     * @brief getFields searches for all fields in the range of min and max ignoring all movement costs
     * @param min minimum search range
     * @param max maximum search range
     * @return shared pointer to the points
     */
    Q_INVOKABLE static QVector<QPoint> getFields(qint32 startX, qint32 startY, qint32 min, qint32 max);
    /**
     * @brief getAllNodePoints returns all reachable fields in a point vector
     * @param maxRange max costs of the target fields (costs need to be smaller than this value)
     * @return
     */
    Q_INVOKABLE QVector<QPoint> getAllNodePoints(qint32 maxRange = infinite);

    Q_INVOKABLE bool hasPoints(qint32 startX, qint32 startY) const;
protected:
    inline qint32 getMoveDirection(const qint32& curX, const qint32& curY,
                                   const qint32& targetX, const qint32& targetY)
    {
        if (curX > targetX)
        {
            return Directions::West;
        }
        else if (curX < targetX)
        {
            return Directions::East;
        }
        else if (curY < targetY)
        {
            return Directions::South;
        }
        else if (curY > targetY)
        {
            return Directions::North;
        }
        else
        {
            return Directions::Unknown;
        }
    }
protected:
    QPoint m_StartPoint;
    qint32 m_width;
    qint32 m_heigth;
    std::vector<qint32> m_costs;
    std::vector<Directions> m_DirectionMap;
    std::vector<std::array<qint32, Directions::Max>> m_movecosts;
    std::list<Node> m_OpenList;
    qint32 m_FinishNode = -1;
    qint32 m_FinishNodeX = -1;
    qint32 m_FinishNodeY = -1;

};


#endif // PATHFINDINGSYSTEM_H
