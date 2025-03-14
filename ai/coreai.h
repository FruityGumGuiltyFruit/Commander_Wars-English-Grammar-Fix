#pragma once
#include <qtextstream.h>
#include <qvector.h>
#include <qvector3d.h>
#include <qvector4d.h>
#include <QRectF>

#include "gameinput/basegameinputif.h"
#include "game/gameaction.h"
#include "game/unitpathfindingsystem.h"

#include "ai/islandmap.h"
#include "ai/targetedunitpathfindingsystem.h"
#include "ai/productionSystem/simpleproductionsystem.h"

#include "coreengine/qmlvector.h"
#include "coreengine/pathfindingsystem.h"

#include <functional>

class GameMap;
class Unit;
class CO;
class UnitPathFindingSystem;
class QmlVectorUnit;
class QmlVectorBuilding;
class QmlVectorPoint;
class Building;
class Terrain;

class CoreAI : public BaseGameInputIF
{
    Q_OBJECT
public:
    enum class ThreadLevel
    {
        Normal,
        High,
        Hq,
        Max,
    };

    enum class CircleReturns
    {
        Stop,
        Fail,
        Success,
    };

    enum class TargetDistance
    {
        NoTarget,
        FarTarget,
        CloseTarget,
    };


    struct UnitCountData
    {
        qint32 infantryUnits{0};
        qint32 indirectUnits{0};
        qint32 directUnits{0};
        qint32 transporterUnits{0};
        qint32 supplyUnits{0};
        qint32 supplyNeededUnits{0};
        std::vector<std::tuple<Unit*, Unit*>> transportTargets;
    };

    struct IniData
    {
        IniData(const QString & name, const QString & group, double * value, double defaultValue, double minValue, double maxValue)
            : m_name(name),
              m_group(group),
              m_value(value),
              m_defaultValue(defaultValue),
              m_minValue(minValue),
              m_maxValue(maxValue)
        {
        }
        QString m_name;
        QString m_group;
        double * m_value;
        double m_defaultValue;
        double m_minValue;
        double m_maxValue;
    };

    struct DamageData
    {
        qint32 x{-1};
        qint32 y{-1};
        float fundsDamage{0};
        float hpDamage{0};
        float hpDamageDifference{0};
        float counterDamage{0.0f};
    };

    struct FundsDamageData
    {
        float atkHpDamage{0.0f};
        float fundsDamage{0.0f};
        float hpDamage{0.0f};
    };

    /**
     * @brief The AISteps enum
     */
    enum class AISteps
    {
        moveUnits = 0,
                moveToTargets,
                moveIndirectsToTargets,
                loadUnits,
                moveTransporters,
                moveSupportUnits,
                moveAway,
                buildUnits,
    };

    struct MoveUnitData
    {
        spUnit pUnit;
        spUnitPathFindingSystem pUnitPfs;
        QStringList actions;
        float virtualDamageData{0};
        ThreadLevel m_threadLevel{ThreadLevel::Normal};
        QPoint m_hqThread;
        QVector<QPoint> m_capturePoints;
        qint32 movementPoints{0};
        qint32 minFireRange{0};
        qint32 maxFireRange{0};
        qint32 unitCosts{0};
        // get filled by sortUnitsFarFromEnemyFirst
        bool canCapture{false};
        qint32 distanceToEnemy{0};
        qint32 nextAiStep{0};

        // infos for performing an action
        spGameAction m_action;
        float m_score{0};
        QPoint captureTarget{-1, -1};
    };
    Q_ENUM(AISteps)
    // static string list of actions so we only define them once
    static const char* const ACTION_WAIT;
    static const char* const ACTION_HOELLIUM_WAIT;
    static const char* const ACTION_SUPPORTSINGLE;
    static const char* const ACTION_SUPPORTSINGLE_REPAIR;
    static const char* const ACTION_SUPPORTSINGLE_FREEREPAIR;
    static const char* const ACTION_SUPPORTSINGLE_SUPPLY;
    static const char* const ACTION_SUPPORTALL;
    static const char* const ACTION_BUILD;
    static const char* const ACTION_SUPPORTALL_RATION;
    static const char* const ACTION_SUPPORTALL_RATION_MONEY;
    static const char* const ACTION_UNSTEALTH;
    static const char* const ACTION_PLACE;
    static const char* const ACTION_STEALTH;
    static const char* const ACTION_BUILD_UNITS;
    static const char* const ACTION_CAPTURE;
    static const char* const ACTION_MISSILE;
    static const char* const ACTION_FIRE;
    static const char* const ACTION_JOIN;
    static const char* const ACTION_UNLOAD;
    static const char* const ACTION_LOAD;
    static const char* const ACTION_NEXT_PLAYER;
    static const char* const ACTION_SWAP_COS;
    static const char* const ACTION_ACTIVATE_TAGPOWER;
    static const char* const ACTION_ACTIVATE_POWER_CO_0;
    static const char* const ACTION_ACTIVATE_POWER_CO_1;
    static const char* const ACTION_ACTIVATE_SUPERPOWER_CO_0;
    static const char* const ACTION_ACTIVATE_SUPERPOWER_CO_1;
    static const char* const ACTION_CO_UNIT_0;
    static const char* const ACTION_CO_UNIT_1;
    static const char* const ACTION_EXPLODE;
    static const char* const ACTION_FLARE;
    static const char* const ACTION_TRAP;
    static const char* const ACTION_BLACKHOLEFACTORY_DOOR1;
    static const char* const ACTION_BLACKHOLEFACTORY_DOOR2;
    static const char* const ACTION_BLACKHOLEFACTORY_DOOR3;
    static const char* const ACTION_NEST_FACTORY_DOOR;

    static const char* const BUILDING_HQ;
    static const char* const UNIT_INFANTRY;

    explicit CoreAI(GameMap* pMap, GameEnums::AiTypes aiType, QString jsName);
    virtual ~CoreAI() = default;
    /**
     * @brief init
     */
    virtual void init(BaseGamemenu* pMenu) override;
    /**
     * @brief onGameStart
     */
    virtual void onGameStart() override;
    /**
     * @brief contains
     * @param points
     * @param point
     * @return
     */
    static bool contains(std::vector<QVector3D>& points, const QPoint & point);
    /**
     * @brief index
     * @param points
     * @param point
     * @return
     */
    static qint32 index(std::vector<QVector3D>& points, const QPoint & point);
    /**
     * @brief serialize stores the object
     * @param pStream
     */
    virtual void serializeObject(QDataStream& pStream) const override;
    /**
     * @brief deserialize restores the object
     * @param pStream
     */
    virtual void deserializeObject(QDataStream& pStream) override;
    /**
     * @brief getVersion version of the file
     * @return
     */
    virtual qint32 getVersion() const override
    {
        return 10;
    }
    void addMenuItemData(spGameAction & pGameAction, const QString & itemID, qint32 cost);
    GameEnums::AiTurnMode getTurnMode() const;

    /**
     * @brief useCOPower
     * @param pUnits
     * @param pEnemyUnits
     * @return
     */
    bool useCOPower(spQmlVectorUnit & pUnits, spQmlVectorUnit & pEnemyUnits);
    /**
     * @brief createMovementMap
     */
    void createMovementMap(spQmlVectorBuilding & pBuildings, spQmlVectorBuilding & pEnemyBuildings);
    /**
     * @brief useBuilding
     * @param pBuildings
     * @return
     */
    bool useBuilding(spQmlVectorBuilding & pBuildings, spQmlVectorUnit & pUnits);
    /**
     * @brief moveOoziums moves all those sweet nice ooziums :)
     * @param pUnits
     * @param pEnemyUnits
     */
    bool moveOoziums(spQmlVectorUnit & pUnits, spQmlVectorUnit & pEnemyUnits);
    /**
     * @brief moveBlackBombs
     * @param pUnits
     * @param pEnemyUnits
     * @return
     */
    bool moveBlackBombs(spQmlVectorUnit & pUnits, spQmlVectorUnit & pEnemyUnits);
    /**
     * @brief moveSupport
     * @param pUnits
     * @return
     */
    bool moveSupport(CoreAI::AISteps step, spQmlVectorUnit & pUnits, bool useTransporters);
    /**
     * @brief moveFlares
     * @param pUnits
     * @return
     */
    bool moveFlares(spQmlVectorUnit & pUnits);
    /**
     * @brief calcUnitDamage
     * @param pUnit
     * @param position
     * @param target
     * @return x = unit damage and y = counter damage
     */
    QRectF calcUnitDamage(spGameAction & pAction, const QPoint & target) const;
    /**
     * @brief CoreAI::calcVirtuelUnitDamage
     * @param pAttacker
     * @param attackerTakenDamage
     * @param atkPos
     * @param defX
     * @param defY
     * @param defenderTakenDamage
     * @return
     */
    static QRectF calcVirtuelUnitDamage(GameMap* pMap,
                                        Unit* pAttacker, float attackerTakenDamage, const QPoint & atkPos, GameEnums::LuckDamageMode luckModeAtk,
                                        Unit* pDefender, float defenderTakenDamage, const QPoint & defPos, GameEnums::LuckDamageMode luckModeDef,
                                        bool ignoreOutOfVisionRange = false, bool fastInAccurate = true);
    /**
     * @brief getBestTarget
     * @param pUnit
     * @param pAction
     * @param pPfs
     * @return target unit x, y and z = fonddamage
     */
    void getBestTarget(Unit* pUnit, spGameAction & pAction, UnitPathFindingSystem* pPfs, std::vector<QVector3D>& ret, std::vector<QVector3D>& moveTargetFields, qint32 maxDistance = PathFindingSystem::infinite);
    /**
     * @brief getAttacksFromField
     * @param pUnit
     * @param pAction
     * @param ret
     */
    void getBestAttacksFromField(Unit* pUnit, spGameAction & pAction, std::vector<QVector3D>& ret, std::vector<QVector3D>& moveTargetFields);
    /**
     * @brief getAttackTargets
     * @param pUnit
     * @param pAction
     * @param pPfs
     * @param ret
     * @param moveTargetFields
     */
    void getAttackTargets(Unit* pUnit, spGameAction & pAction, UnitPathFindingSystem* pPfs, std::vector<CoreAI::DamageData>& ret, std::vector<QVector3D>& moveTargetFields, qint32 maxDistance = PathFindingSystem::infinite) const;
    /**
     * @brief getAttacksFromField
     * @param pUnit
     * @param pAction
     * @param ret
     * @param moveTargetFields
     */
    void getAttacksFromField(Unit* pUnit, spGameAction & pAction, std::vector<DamageData>& ret, std::vector<QVector3D>& moveTargetFields) const;
    /**
     * @brief moveAwayFromProduction
     * @param pUnits
     * @return
     */
    bool moveAwayFromProduction(spQmlVectorUnit & pUnits);
    /**
     * @brief CoreAI::calcFundsDamage
     * @param damage
     * @param pAtk
     * @param pDef
     * @return
     */
    FundsDamageData calcFundsDamage(const QRectF & damage, Unit* pAtk, Unit* pDef) const;
    /**
     * @brief appendAttackTargets
     * @param pUnit
     * @param pEnemyUnits
     * @param targets
     */
    void appendAttackTargets(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, std::vector<QVector3D>& targets, qint32 distanceModifier = 1);
    /**
     * @brief createTargetedPfs
     * @param pUnit
     * @param targets
     * @return
     */
    TargetedUnitPathFindingSystem* createTargetedPfs(Unit* pUnit, const QVector<QVector3D> & targets);

signals:
    /**
     * @brief performAction signal with an action to be performed the action has to be deleted by the reciever of this slot. Only one slot can be connected to this signal
     * @param pAction
     */
    void sigPerformAction(spGameAction pAction, bool fromAiPipe = false);
public slots:
    /**
     * @brief process
     */
    virtual void process() = 0;
public:
    /**
     * @brief nextAction
     */
    Q_INVOKABLE virtual void nextAction();
    Q_INVOKABLE virtual void resetToTurnStart();
    /**
     * @brief getProductionSystem
     * @return
     */
    Q_INVOKABLE SimpleProductionSystem* getSimpleProductionSystem();
    /**
     * @brief getAiName
     * @return
     */
    Q_INVOKABLE QString getAiName() const;
    /**
     * @brief getVariables
     * @return
     */
    Q_INVOKABLE inline ScriptVariables* getVariables()
    {
        return &m_Variables;
    }
    /**
     * @brief loadIni
     * @param file
     */
    Q_INVOKABLE void loadIni(QString file);
    /**
     * @brief readIni
     * @param name
     */
    Q_INVOKABLE virtual void readIni(QString name);
    /**
     * @brief saveIni
     * @param name
     */
    Q_INVOKABLE void saveIni(QString name) const;
    /**
     * @brief randomizeIni
     */
    Q_INVOKABLE void randomizeIni(QString name, float chance, float mutationRate = 0.1f);
    /**
     * @brief setInitValue changes an ini-file loaded parameter to the given value if the name doesn't exist nothing happens
     * @param name
     * @param newValue
     */
    Q_INVOKABLE void setInitValue(QString name, double newValue);
    /**
     * @brief getInitValue gets the current value of an ini-file loaded parameter if the name doesn't exist returns 0.
     * @param name
     * @return
     */
    Q_INVOKABLE double getInitValue(QString name) const;
    /**
     * @brief calcBuildingDamage
     * @param pUnit
     * @param pBuilding
     * @return
     */
    Q_INVOKABLE float calcBuildingDamage(Unit* pUnit, const QPoint newPosition, Building* pBuilding) const;
    /**
     * @brief addMovementMap
     * @param pBuilding
     * @param damage
     */
    Q_INVOKABLE void addMovementMap(Building* pBuilding, float damage);
    /**
     * @brief onSameIsland checks if unit1 can reach unit 2. This may be vice versa but isn't checked here
     * @param pUnit1
     * @param pUnit2
     * @return
     */
    Q_INVOKABLE bool onSameIsland(Unit* pUnit1, Unit* pUnit2) const;
    /**
     * @brief onSameIsland checks if unit1 can reach the building. This may be vice versa but isn't checked here
     * @param pUnit1
     * @param pBuilding
     * @return
     */
    Q_INVOKABLE bool onSameIsland(Unit* pUnit1, Building* pBuilding) const;
    /**
     * @brief onSameIsland
     * @param movemnetType
     * @param x
     * @param y
     * @param x1
     * @param y1
     * @return
     */
    Q_INVOKABLE bool onSameIsland(const QString & movemnetType, qint32 x, qint32 y, qint32 x1, qint32 y1) const;
    /**
     * @brief onSameIsland
     * @param islandIdx
     * @param x
     * @param y
     * @param x1
     * @param y1
     * @return
     */
    Q_INVOKABLE bool onSameIsland(qint32 islandIdx, qint32 x, qint32 y, qint32 x1, qint32 y1) const;
    /**
     * @brief getIsland
     * @param pUnit1
     * @return
     */
    Q_INVOKABLE qint32 getIsland(Unit* pUnit);
    /**
     * @brief getIslandSize
     * @param pUnit
     * @param x
     * @param y
     * @return
     */
    Q_INVOKABLE qint32 getIslandSize(Unit* pUnit, qint32 x, qint32 y) const;
    /**
     * @brief getIslandIndex
     * @param pUnit1
     * @return
     */
    Q_INVOKABLE qint32 getIslandIndex(Unit* pUnit);
    /**
     * @brief isUnloadTerrain
     * @param pUnit
     * @param pTerrain
     * @return
     */
    Q_INVOKABLE bool isUnloadTerrain(Unit* pUnit, Terrain* pTerrain);
    /**
     * @brief isLoadingTerrain
     * @param pTransporter
     * @param pTerrain
     * @return
     */
    Q_INVOKABLE bool isLoadingTerrain(Unit* pTransporter, Terrain* pTerrain);
    /**
     * @brief createIslandMap
     * @param movementType
     * @param unitID
     */
    Q_INVOKABLE void createIslandMap(const QString & movementType, const QString & unitID);
    /**
     * @brief needsRefuel
     * @param pUnit
     * @return
     */
    Q_INVOKABLE bool needsRefuel(const Unit* pUnit) const;
    /**
     * @brief isRefuelUnit
     * @param actionList
     * @return
     */
    Q_INVOKABLE bool isRefuelUnit(const QStringList & actionList) const;
    /**
     * @brief isMoveableBuilding
     * @param pBuilding
     * @return
     */
    Q_INVOKABLE bool isMoveableTile(Building* pBuilding, UnitPathFindingSystem &turnPfs) const;
    /**
     * @brief getAiCoUnitMultiplier
     * @param pCO
     * @param pUnit
     * @return
     */
    Q_INVOKABLE float getAiCoUnitMultiplier(CO* pCO, Unit* pUnit);
    /**
     * @brief getAiCoBuildRatioModifier
     * @param pCO
     * @return
     */
    Q_INVOKABLE float getAiCoBuildRatioModifier();
    /**
     * @brief getUnitCount
     * @param pUnits
     * @param unitIds
     * @return
     */
    Q_INVOKABLE qint32 getUnitCount(QmlVectorUnit * pUnits, const QStringList & unitIds, float minHp = 0.0f, qint32 minFuel = 0);
    /**
     * @brief getFilteredUnits
     * @param pUnits
     * @param unitIds
     * @param minHp
     * @param minFuel
     * @return
     */
    Q_INVOKABLE QmlVectorUnit* getFilteredUnits(QmlVectorUnit * pUnits, const QStringList & unitIds, float minHp = 0.0f, qint32 minFuel = 0);
    /**
     * @brief getEnemyUnitCountNearOwnUnits
     * @param pUnits
     * @param pEnemyUnits
     * @param unitIds
     * @param distance
     * @param minHp
     * @return
     */
    Q_INVOKABLE qint32 getEnemyUnitCountNearOwnUnits(QmlVectorUnit * pUnits, QmlVectorUnit * pEnemyUnits, const QStringList unitIds, qint32 distance, float minHp = 0.0f);
    /**
     * @brief getBuildingCountsOnEnemyIslands
     * @param pUnits
     * @param pEnemyBuildings
     * @return
     */
    Q_INVOKABLE qint32 getBuildingCountsOnEnemyIslands(QmlVectorUnit * pUnits, QmlVectorBuilding * pEnemyBuildings);
    /**
     * @brief hasTargets checks if a unit has anything to do on this island
     * @param transporterMovement movement points of the transporting unit
     * @param pLoadingUnit
     * @param canCapture
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @return
     */
    Q_INVOKABLE bool hasTargets(qint32 transporterMovement, Unit* pLoadingUnit, bool canCapture, QmlVectorUnit * pEnemyUnits, QmlVectorBuilding * pEnemyBuildings,
                    qint32 loadingIslandIdx, qint32 loadingIsland, bool allowFastUnit = true, bool onlyTrueIslands = false, bool useEnemyProductionBuildings = false);
    /**
     * @brief CoreAI::getIdleUnitCount
     * @param pUnits
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @return
     */
    Q_INVOKABLE qint32 getIdleUnitCount(QmlVectorUnit* pUnits, const QStringList & unitIds, QmlVectorUnit * pEnemyUnits, QmlVectorBuilding * pEnemyBuildings);
    /**
     * @brief shareIslandWithEnemy
     * @param pUnits
     * @param pBuildings
     * @param pEnemyBuildings
     * @return
     */
    Q_INVOKABLE bool shareIslandWithEnemy(QmlVectorUnit* pUnits, QmlVectorBuilding * pBuildings, QmlVectorBuilding * pEnemyBuildings);
    /**
     * @brief resetMoveMap
     */
    Q_INVOKABLE void resetMoveMap();
    qint32 getAiFunctionStep() const
    {
        return m_aiFunctionStep;
    }

    /**
     * @brief hasMissileTarget
     * @return
     */
    bool hasMissileTarget() const
    {
        return m_missileTarget;
    }
    void addSelectedFieldData(spGameAction & pGameAction, const QPoint & point);
protected:
    /**
     * @brief prepareEnemieData
     * @param pUnits
     * @param pEnemyUnits
     * @param pEnemyBuildings
     */
    void prepareEnemieData(spQmlVectorUnit & pUnits, spQmlVectorBuilding &pBuildings, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings);
    void sortUnitsFarFromEnemyFirst(std::vector<MoveUnitData> & pUnits, spQmlVectorUnit & pEnemyUnits);

    /**
     * @brief isAttackOnTerrainAllowed
     * @param pTerrain
     * @return
     */
    bool isAttackOnTerrainAllowed(Terrain* pTerrain, float damage) const;
    /**
     * @brief processPredefinedAi
     * @return
     */
    bool processPredefinedAi();
    bool processPredefinedAiHold(Unit* pUnit);
    bool processPredefinedAiDefensive(Unit* pUnit);
    bool processPredefinedAiOffensive(Unit* pUnit, spQmlVectorUnit & pEnemyUnits);
    bool processPredefinedAiPatrol(Unit* pUnit);
    bool processPredefinedAiAttack(Unit* pUnit, spGameAction & pAction, UnitPathFindingSystem & pfs);
    bool processPredefinedAiTargetEnemyHq(Unit* pUnit, spQmlVectorBuilding & pEnemyBuildings);
    bool processPredefinedGenericScripted(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings);
    bool processPredefinedMapScripted(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings);
    virtual void finishTurn();
    // helper functions to get targets for unit actions
    void appendSupportTargets(const QStringList & actions, Unit* pCurrentUnit, spQmlVectorUnit & pUnits, spQmlVectorUnit & pEnemyUnits, std::vector<QVector3D>& targets, qint32 distanceModifier = 1);
    void appendCaptureTargets(const QStringList & actions, Unit* pUnit, spQmlVectorBuilding & pEnemyBuildings,  std::vector<QVector3D>& targets, qint32 distanceModifier = 1);
    void appendAttackTargetsIgnoreOwnUnits(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, std::vector<QVector3D>& targets, qint32 distanceModifier = 1);
    void appendRepairTargets(Unit* pUnit, spQmlVectorBuilding & pBuildings, std::vector<QVector3D>& targets);
    void appendSupplyTargets(Unit* pUnit, spQmlVectorUnit & pUnits, std::vector<QVector3D>& targets);
    void appendTransporterTargets(Unit* pUnit, spQmlVectorUnit & pUnits, std::vector<QVector3D>& targets);
    void appendCaptureTransporterTargets(Unit* pUnit, spQmlVectorUnit & pUnits,
                                         spQmlVectorBuilding & pEnemyBuildings, std::vector<QVector3D>& targets, qint32 distanceModifier = 1);
    std::vector<Unit*> appendLoadingTargets(Unit* pUnit, spQmlVectorUnit & pUnits,
                                            spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings,
                                            bool addCaptureTargets, bool virtualLoading, std::vector<QVector3D>& targets,
                                            bool all = false, qint32 distanceModifier = 1, bool onlyTrueIslands = false);
    CircleReturns doExtendedCircleAction(qint32 currentX, qint32 currentY, qint32 x, qint32 y, qint32 min, qint32 max, std::function<CircleReturns(qint32, qint32)> functor);
    /**
     * @brief hasCaptureTarget
     * @param pLoadingUnit
     * @param canCapture
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @param loadingIslandIdx
     * @param loadingIsland
     * @return
     */
    TargetDistance hasCaptureTarget(Unit* pLoadingUnit, bool canCapture, QmlVectorBuilding * pEnemyBuildings,
                                    qint32 loadingIslandIdx, qint32 loadingIsland, bool onlyTrueIslands = false);
    /**
     * @brief appendNearestUnloadTargets searches for unload fields closest to our current position
     * @param pUnit
     * @param pEnemyUnits
     * @param pEnemyBuildings
     */
    void appendNearestUnloadTargets(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings, std::vector<QVector3D>& targets, qint32 distanceModifier = 1);
    /**
     * @brief appendUnloadTargetsForCapturing searches unload fields near enemy buildings
     * @param pUnit
     * @param pEnemyBuildings
     */
    void appendUnloadTargetsForCapturing(Unit* pUnit, spQmlVectorUnit & ownUnits, spQmlVectorBuilding & pEnemyBuildings, std::vector<QVector3D>& targets, qint32 distanceModifier = 1);
    /**
     * @brief appendUnloadTargetsForAttacking
     * @param pUnit
     * @param pEnemyUnits
     * @param targets
     */
    void appendUnloadTargetsForAttacking(Unit* pUnit, spQmlVectorUnit & pEnemyUnits, std::vector<QVector3D>& targets, qint32 rangeMultiplier, qint32 distanceModifier = 1);
    /**
     * @brief appendTerrainBuildingAttackTargets
     * @param pUnit
     * @param pEnemyBuildings
     * @param targets
     */
    void appendTerrainBuildingAttackTargets(Unit* pUnit, spQmlVectorBuilding & pEnemyBuildings, std::vector<QVector3D>& targets, qint32 distanceModifier = 1);
    /**
     * @brief checkIslandForUnloading
     * @param pUnit
     * @param pLoadedUnit
     * @param checkedIslands
     * @param unitIslandIdx
     * @param unitIsland
     * @param loadedUnitIslandIdx
     * @param targetIsland
     * @param pUnloadArea
     * @param targets
     */
    void checkIslandForUnloading(Unit* pUnit, Unit* pLoadedUnit, std::vector<qint32>& checkedIslands,
                                 qint32 unitIslandIdx, qint32 unitIsland,
                                 qint32 loadedUnitIslandIdx, qint32 targetIsland, qint32 islandX, qint32 islandY,
                                 QmlVectorPoint* pUnloadArea, std::vector<QVector3D>& targets, qint32 distanceModifier = 1);
    /**
     * @brief getBestFlareTarget
     * @param pUnit
     * @param pAction
     * @param pPfs
     * @param flareTarget
     * @param moveTargetField
     */
    void getBestFlareTarget(Unit* pUnit, spGameAction & pAction, UnitPathFindingSystem* pPfs, QPoint& flareTarget, QPoint& moveTargetField);
    /**
     * @brief getFlareTargetScore
     * @param moveTarget
     * @param flareTarget
     * @param pUnfogCircle
     * @return
     */
    qint32 getFlareTargetScore(const QPoint& moveTarget, const QPoint& flareTarget, const spQmlVectorPoint& pUnfogCircle);

    /**
     * @brief createIslandMap
     * @param pUnits
     */
    void rebuildIsland(spQmlVectorUnit & pUnits);
    /**
     * @brief GetUnitCounts
     * @param pUnits
     * @param infantryUnits
     * @param indirectUnits
     * @param directUnits
     * @param transportTargets
     */
    void GetOwnUnitCounts(std::vector<MoveUnitData> & units, spQmlVectorUnit & pOwnUnits, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings,
                          UnitCountData & countData);
    /**
     * @brief buildCOUnit
     * @return
     */
    bool buildCOUnit(spQmlVectorUnit & pUnits);
    /**
     * @brief canTransportToEnemy
     * @param pUnit
     * @param pLoadedUnit
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @return
     */
    bool canTransportToEnemy(Unit* pUnit, Unit* pLoadedUnit, spQmlVectorUnit & pEnemyUnits, spQmlVectorBuilding & pEnemyBuildings);
    /**
     * @brief getBuildingPointFromScript
     * @param pData
     */
    bool getBuildingTargetPointFromScript(spGameAction & pAction, const spMarkedFieldData & pData, QPoint & target);
    /**
     * @brief getBuildingMenuItemFromScript
     * @param pAction
     * @param pData
     * @param index
     * @return
     */
    bool getBuildingMenuItemFromScript(spGameAction & pAction, spQmlVectorUnit & pUnits, spQmlVectorBuilding & pBuildings, const spMenuData & pData, qint32 & index);
    /**
     * @brief deserializeObjectVersion
     * @param stream
     * @param version
     */
    void deserializeObjectVersion(QDataStream &stream, qint32 version);
    /**
     * @brief getAttackTargetsFast
     * @param pUnit
     * @param minFirerange
     * @param maxFirerange
     * @param pPfs
     * @param ret
     * @param moveTargetFields
     * @param maxDistance
     */
    void getAttackTargetsFast(Unit* pUnit, QmlVectorPoint & firePoints, UnitPathFindingSystem* pPfs, std::vector<DamageData>& ret, qint32 maxDistance = PathFindingSystem::infinite) const;
    /**
     * @brief getAttacksFromFieldFast
     * @param pUnit
     * @param minFirerange
     * @param maxFirerange
     * @param ret
     * @param moveTargetFields
     */
    void getAttacksFromFieldFast(Unit* pUnit, QPoint position, QmlVectorPoint & firePoints, std::vector<DamageData>& ret) const;
    /**
     * @brief calcUnitDamageFast
     * @param pAttacker
     * @param pDefender
     * @return
     */
    static QRectF calcUnitDamageFast(Unit* pAttacker, Unit* pDefender);
    /**
     * @brief getBaseDamage
     * @param pAttacker
     * @param pDefender
     * @return
     */
    static QPointF getBaseDamage(Unit* pAttacker, Unit* pDefender);
    /**
     * @brief getBaseDamage
     * @param weaponID
     * @param pDefender
     * @return
     */
    static float getBaseDamage(const QString & weaponID, Unit* pDefender);
protected:
    std::vector<spIslandMap> m_IslandMaps;
    double m_buildingValue{1.0f};
    double m_ownUnitValue{1.0f};
    GameEnums::AiTurnMode m_turnMode{GameEnums::AiTurnMode_StartOfDay};
    AISteps m_aiStep{AISteps::moveUnits};
    qint32 m_aiFunctionStep{0};
    bool m_usedTransportSystem{false};
    bool m_usedPredefinedAi{false};
    bool m_missileTarget{false};
    double m_fuelResupply{0.33f};
    double m_ammoResupply{0.25f};
    double m_enemyPruneRange{3.0};
    double m_ownBuildingPruneRange{10};

    double m_minCoUnitScore{5000.0f};
    double m_coUnitValue{6000};
    double m_coUnitRankReduction{1000.0f};
    double m_coUnitScoreMultiplier{1.1f};
    double m_minCoUnitCount{5};
    double m_minSameIslandDistance{2.5};
    double m_slowUnitSpeed{2};
    double m_minTerrainDamage{20.0f};
    double m_minHpDamage{-2};

    QVector<IniData> m_iniData;
    QString m_aiName{"CoreAI"};
    ScriptVariables m_Variables;
    SimpleProductionSystem m_productionSystem;
    static std::map<QString, float> m_baseDamageTable;

private:
    bool finish{false};
    struct FlareInfo
    {
        qint32 minRange{0};
        qint32 maxRange{0};
        qint32 unfogRange{0};
    };
    FlareInfo m_flareInfo;
    struct ExplodeInfo
    {
        qint32 range{0};
        qint32 damage{0};
    };
    ExplodeInfo m_explodeInfo;
    QStringList m_iniFiles;
};
