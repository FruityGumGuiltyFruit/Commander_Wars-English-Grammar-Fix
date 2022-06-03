#ifndef PLAYER_H
#define PLAYER_H

#include <tuple>
#include <QColor>
#include <QObject>
#include <QVector>
#include <QVector3D>

#include "coreengine/mainapp.h"
#include "coreengine/qmlvector.h"
#include "coreengine/fileserializable.h"

#include "game/GameEnums.h"
#include "game/co.h"

#include "gameinput/basegameinputif.h"

class GameMap;
class Player;
using spPlayer = oxygine::intrusive_ptr<Player>;

class Player : public QObject, public oxygine::Actor, public FileSerializable
{
    Q_OBJECT
public:
    /**
     * @brief Player
     */
    explicit Player(GameMap* pMap);
    virtual ~Player() = default;
    /**
     * @brief init
     */
    void init();
    /**
     * @brief releaseStaticData
     */
    static void releaseStaticData();
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
     * @brief deserializeObject
     * @param pStream
     * @param fast
     */
    void deserializer(QDataStream& pStream, bool fast);
    /**
     * @brief getVersion version of the file
     * @return
     */
    inline virtual qint32 getVersion() const override
    {
        return 17;
    }
    /**
     * @brief setBaseGameInput sets the player input
     * @param pBaseGameInput
     */
    void setBaseGameInput(spBaseGameInputIF pBaseGameInput);
    /**
     * @brief getCO
     * @param id index of the co 0 or 1
     * @return pointer to the co
     */
    spCO getspCO(quint8 id);
    /**
     * @brief getSpEnemyUnits
     * @return
     */
    QVector<spUnit> getSpEnemyUnits();

    /**
     * @brief getBuildlistChanged
     * @return
     */
    bool getBuildlistChanged() const;
    /**
     * @brief setBuildlistChanged
     * @param BuildlistChanged
     */
    void setBuildlistChanged(bool BuildlistChanged);
    /**
     * @brief getSocketId
     * @return
     */
    quint64 getSocketId() const;
    /**
     * @brief setSocketId
     * @param socketId
     */
    void setSocketId(const quint64 &socketId);
    /**
     * @brief getColorTableAnim
     * @return
     */
    virtual oxygine::spResAnim getColorTableAnim() const;
    /**
     * @brief getColorTableAnim
     * @return
     */
    static oxygine::spResAnim getNeutralTableAnim();
    /**
     * @brief getColorTable
     * @return
     */
    const QImage &getColorTable() const;
    /**
     * @brief getNeutralTableImage
     * @return
     */
    static const QImage &getNeutralTableImage();
    /**
     * @brief setUniqueIdentifier
     * @param newUniqueIdentifier
     */
    void setUniqueIdentifier(const QString &newUniqueIdentifier);
    void setPlayerNameId(const QString &newDisplayName);
    void setMenu(GameMenue *newMenu);
    /**
     * @brief setControlType
     * @param newControlType
     */
    void setControlType(const GameEnums::AiTypes &newControlType);

public slots:
    /**
     * @brief getControlType
     * @return this is the ai type set during game creation this may differ to the instanciated ai type during multiplayer matches
     */
    GameEnums::AiTypes getControlType() const;
    /**
     * @brief getDisplayName
     * @return
     */
    QString getPlayerNameId() const;
    /**
     * @brief getUniqueIdentifier
     * @return
     */
    const QString getUniqueIdentifier() const;
    /**
     * @brief getMap
     * @return
     */
    GameMap *getMap() const;
    /**
     * @brief onUnitDeath
     * @param pUnit
     */
    void onUnitDeath(Unit* pUnit);
    /**
     * @brief postAction
     * @param pAction
     */
    void postAction(GameAction* pAction);
    /**
     * @brief setPlayerArmy
     * @param value
     */
    void setPlayerArmy(const QString &value);
    /**
     * @brief getPlayerArmySelected
     * @return
     */
    bool getPlayerArmySelected() const;
    /**
     * @brief setPlayerArmySelected
     * @param playerArmySelected
     */
    void setPlayerArmySelected(bool playerArmySelected);
    /**
     * @brief getFlipUnitSprites
     * @return
     */
    bool getFlipUnitSprites() const;
    /**
     * @brief getUnitBuildValue
     * @param unitID
     * @return
     */
    float getUnitBuildValue(const QString & unitID);
    /**
     * @brief getBaseGameInput pointer to the ai or human player interface
     * @return
     */
    BaseGameInputIF* getBaseGameInput();
    /**
     * @brief getIsDefeated
     * @return
     */
    bool getIsDefeated() const;
    /**
     * @brief setIsDefeated if true defeats the player but doesn't remove units or buildings
     * @param value
     */
    void setIsDefeated(bool value);
    /**
     * @brief swapCOs swaps co 0 and 1
     */
    void swapCOs();
    /**
     * @brief getColor the color of this player
     * @return
     */
    QColor getColor() const;
    /**
     * @brief setColor sets the color of this player. Note this want update existing sprites
     * @param Color
     */
    void setColor(QColor Color, qint32 table = -1);
    /**
     * @brief getPlayerID player id of this player from 0 to n
     * @return
     */
    qint32 getPlayerID() const;
    /**
     * @brief getArmy the army string id of this player.
     * @return
     */
    QString getArmy();
    /**
     * @brief isEnemy checks the alliance with this player
     * @param pPlayer the player we want to check if he's an enemy
     * @return the alliance of the player with us
     */
    GameEnums::Alliance checkAlliance(Player* pPlayer);
    /**
     * @brief isEnemyUnit checks if the given unit is an enemy
     * @param pUnit
     * @return
     */
    bool isEnemyUnit(Unit* pUnit);
    /**
     * @brief isEnemy
     * @param pOwner
     * @return
     */
    bool isEnemy(Player* pOwner);
    /**
     * @brief isPlayerIdEnemy
     * @param playerId
     */
    bool isPlayerIdEnemy(qint32 playerId);
    /**
     * @brief Player::isPlayerIdAlly
     * @param playerId
     * @return
     */
    bool isPlayerIdAlly(qint32 playerId);
    /**
     * @brief isAlly
     * @param pOwner
     * @return
     */
    bool isAlly(Player* pOwner);
    /**
     * @brief getFunds
     * @return
     */
    qint32 getFunds() const;
    /**
     * @brief addFunds increases the money of this player by the given value
     * @param value
     */
    void addFunds(const qint32 &value);
    /**
     * @brief setFunds
     * @param value
     */
    void setFunds(const qint32 &value);
    /**
     * @brief earnMoney earns money based on the buildings the player has
     * @param modifier multiplier additionaly modifying the income
     */
    void earnMoney(float modifier = 1.0f);
    /**
     * @brief calcIncome
     * @param modifier
     * @return
     */
    qint32 calcIncome(float modifier = 1.0f) const;
    /**
     * @brief calcArmyValue
     * @return
     */
    qint32 calcArmyValue();
    /**
     * @brief getBonusIncome
     * @param pBuilding
     * @param income
     * @return
     */
    qint32 getIncomeReduction(Building* pBuilding, qint32 income);
    /**
     * @brief getCO
     * @param id index of the co 0 or 1
     * @return pointer to the co
     */
    CO* getCO(quint8 id);
    /**
     * @brief getMaxCoCount
     * @return
     */
    qint32 getMaxCoCount() const;
    /**
     * @brief setCO
     * @param coId the co we want to load
     * @param id the index at which we want this co
     */
    void setCO(QString coId, quint8 idx);
    /**
     * @brief getCoCount
     * @return
     */
    qint32 getCoCount() const;
    /**
     * @brief getCostModifier
     * @param baseCost
     * @return
     */
    qint32 getCostModifier(const QString & id, qint32 baseCost, QPoint position);
    /**
     * @brief getCosts
     * @param id
     * @return
     */
    qint32 getCosts(const QString & id, QPoint position);
    /**
     * @brief gainPowerstar
     * @param fundsDamage
     */
    void gainPowerstar(qint32 fundsDamage, QPoint position, qint32 hpDamage, bool defender, bool counterAttack);
    /**
     * @brief startOfTurn
     */
    void startOfTurn();
    /**
     * @brief getUnits
     * @return
     */
    QmlVectorUnit* getUnits();
    /**
     * @brief getEnemyUnits
     * @return
     */
    QmlVectorUnit* getEnemyUnits();
    /**
     * @brief getEnemyBuildings
     * @return
     */
    QmlVectorBuilding* getEnemyBuildings();
    /**
     * @brief getEnemyCount
     * @return
     */
    qint32 getEnemyCount();
    /**
     * @brief getBuildings
     * @return
     */
    QmlVectorBuilding* getBuildings(const QString & id = "");
    /**
     * @brief updateCORange
     */
    void updateVisualCORange();
    /**
     * @brief getMovementcostModifier
     * @param pUnit
     * @param position
     * @return
     */
    qint32 getMovementcostModifier(Unit* pUnit, QPoint position);
    /**
     * @brief getWeatherMovementCostModifier
     * @param pUnit
     * @param position
     * @return
     */
    qint32 getWeatherMovementCostModifier(Unit* pUnit, QPoint position);
    /**
     * @brief getBonusMovementpoints
     * @param pUnit
     * @param position
     * @return
     */
    qint32 getBonusMovementpoints(Unit* pUnit, QPoint position);
    /**
     * @brief getRockettarget finds the most valuable target for damaging units in a diamond.
     * If multiple targets with the same value are found.
     * One is selected at random
     * @param radius
     * @param damage
     * @param ownUnitValue value of own or allied units compared to enemy ones.
     * @return -1, -1 for no target found
     */
    QPoint getRockettarget(qint32 radius, qint32 damage, float ownUnitValue = 1.2f, GameEnums::RocketTarget targetType = GameEnums::RocketTarget_Money);
    /**
     * @brief getSiloRockettarget
     * @param radius
     * @param damage
     * @param highestDamage
     * @param ownUnitValue
     * @param targetType
     * @return
     */
    QPoint getSiloRockettarget(qint32 radius, qint32 damage, qint32 & highestDamage, float ownUnitValue = 1.2f, GameEnums::RocketTarget targetType = GameEnums::RocketTarget_Money);
    /**
     * @brief getRocketTargetDamage
     * @param x
     * @param y
     * @param pPoints
     * @param damage
     * @param ownUnitValue
     * @param targetType
     * @return
     */
    qint32 getRocketTargetDamage(qint32 x, qint32 y, QmlVectorPoint* pPoints, qint32 damage, float ownUnitValue, GameEnums::RocketTarget targetType, bool ignoreStealthed);
    /**
     * @brief defineArmy defines our army sprites based on the current co at position 0
     */
    void defineArmy();
    /**
     * @brief getFundsModifier
     * @return
     */
    float getFundsModifier() const;
    /**
     * @brief setFundsModifier
     * @param value
     */
    void setFundsModifier(float value);
    /**
     * @brief calculatePlayerStrength
     * @return
     */
    qint32 calculatePlayerStrength() const;
    /**
     * @brief getBuildingCount
     * @return
     */
    qint32 getBuildingCount(const QString & buildingID = "");
    /**
     * @brief getBuildingListCount
     * @param list
     * @param whitelist
     * @return
     */
    qint32 getBuildingListCount(const QStringList & list, bool whitelist = true);
    /**
     * @brief getUnitCount
     * @return
     */
    qint32 getUnitCount(const QString & unitID = "") const;
    /**
     * @brief getUnitCount
     * @param pUnit
     * @param unitID
     * @return
     */
    qint32 getUnitCount(Unit* pUnit, const QString & unitID) const;
    /**
     * @brief getEnemyBonus
     * @param position
     * @param pUnit
     * @return
     */
    qint32 getCoBonus(QPoint position, Unit* pUnit, const QString & function);
    /**
     * @brief getTeam
     * @return
     */
    qint32 getTeam() const;
    /**
     * @brief setTeam
     * @param value
     */
    void setTeam(const qint32 &value);
    /**
     * @brief defeatPlayer defeats the player and removes him from further playing
     * @param pPLayer none nullptr will give all buildings to the selected player
     * @param units if units is true the player will also get the controll of the given player
     */
    void defeatPlayer(Player* pPlayer, bool units = false);
    /**
     * @brief postBattleActions
     * @param pAttacker
     * @param pDefender
     */
    void postBattleActions(Unit* pAttacker, float atkDamage, Unit* pDefender, bool gotAttacked, qint32 weapon, GameAction* pAction);
    /**
     * @brief updatePlayerVision updates the current vision of this player.
     * @brief reduces the timer for vision created for several turns
     */
    void updatePlayerVision(bool reduceTimer = false);
    /**
     * @brief addVisionField
     * @param x
     * @param y
     * @param duration
     */
    void addVisionField(qint32 x, qint32 y, qint32 duration = 1, bool directView = false);
    /**
     * @brief getFieldVisible
     * @param x
     * @param y
     */
    virtual bool getFieldVisible(qint32 x, qint32 y);
    /**
     * @brief getFieldVisibleLoaded
     * @return
     */
    inline bool getFieldVisibleLoaded()
    {
        return m_FogVisionFields.size() > 0;
    }
    /**
     * @brief getFieldVisibleType
     * @param x
     * @param y
     * @return
     */
    virtual GameEnums::VisionType getFieldVisibleType(qint32 x, qint32 y);
    /**
     * @brief getFieldDirectVisible
     * @param x
     * @param y
     * @return if true a stealthed unit will be made visible by this players view
     */
    virtual bool getFieldDirectVisible(qint32 x, qint32 y);
    /**
     * @brief loadVisionFields
     */
    void loadVisionFields();
    /**
     * @brief loadCOMusic
     */
    void loadCOMusic();
    /**
     * @brief buildedUnit called after a unit was created do whatever you want with this information
     * @param pUnit
     */
    void buildedUnit(Unit* pUnit);
    /**
     * @brief getWeatherImmune
     * @return
     */
    bool getWeatherImmune();
    /**
     * @brief getBuildList
     * @return
     */
    QStringList getBuildList() const;
    /**
     * @brief getCOUnits
     * @return
     */
    QStringList getCOUnits(Building* pBuilding);
    /**
     * @brief getTransportUnits
     * @param pUnit
     * @return
     */
    QStringList getTransportUnits(Unit* pUnit);
    /**
     * @brief setBuildList
     * @param BuildList
     */
    void setBuildList(const QStringList & BuildList);
    /**
     * @brief changeBuildlist
     * @param unitID
     * @param remove
     */
    void changeBuildlist(const QString& unitID, bool remove = false);
    /**
     * @brief getVariables
     * @return
     */
    inline ScriptVariables* getVariables()
    {
        return &m_Variables;
    }
private:
    /**
     * @brief loadTable
     * @param table
     * @return
     */
    bool loadTable(qint32 table);
    /**
     * @brief loadTableFromFile
     * @param tablename
     * @return
     */
    bool loadTableFromFile(const QString & tablename);
    /**
     * @brief colorToTable
     * @param baseColor
     * @return
     */
    bool colorToTable(QColor baseColor);
    /**
     * @brief colorToTableInTable
     * @param baseColor
     * @return
     */
    bool colorToTableInTable(QColor baseColor);
    /**
     * @brief createTable
     * @param baseColor
     */
    void createTable(QColor baseColor);
    /**
     * @brief getAverageCost
     * @return
     */
    qint32 getAverageCost();
    /**
     * @brief addVisionFieldInternal
     * @param x
     * @param y
     * @param duration
     * @param directView
     */
    void addVisionFieldInternal(qint32 x, qint32 y, qint32 duration, bool directView);
    /**
     * @brief calculatePlayerStrength
     * @param pUnit
     */
    qint32 calculatePlayerStrength(Unit* pUnit) const;
protected:
    GameMap* m_pMap{nullptr};

private:
    qint32 m_funds{0};
    float m_fundsModifier{1.0f};

    QColor m_Color;
    QImage m_colorTable;
    oxygine::spResAnim m_ColorTableAnim;
    QString m_playerArmy{""};
    qint32 m_team{0};
    std::array<spCO, 2> m_playerCOs;
    /**
     * @brief m_pBaseGameInput pointer to the ai or human player
     */
    spBaseGameInputIF m_pBaseGameInput{nullptr};
    GameEnums::AiTypes m_controlType;
    GameMenue* m_pMenu{nullptr};
    bool m_isDefeated{false};
    struct VisionFieldInfo
    {
        VisionFieldInfo()
        {
        }
        VisionFieldInfo(GameEnums::VisionType visionType, qint32 duration, bool directView)
            : m_visionType(visionType),
              m_duration(duration),
              m_directView(directView)
        {
        }
        GameEnums::VisionType m_visionType = GameEnums::VisionType_Shrouded;
        qint32 m_duration = 0;
        bool m_directView = false;
    };

    std::vector<std::vector<VisionFieldInfo>> m_FogVisionFields;
    /**
     * @brief m_BuildList contains all units we're allowed to build
     */
    QStringList m_BuildList;
    bool m_BuildlistChanged{false};
    ScriptVariables m_Variables;
    quint64 m_socketId{0};
    bool m_playerArmySelected{false};
    qint32 m_averageCosts{-1};
    /**
     * @brief m_uniqueIdentifier
     */
    QString m_uniqueIdentifier;
    QString m_playerNameId;
    static oxygine::spResAnim m_neutralTableAnim;
    static QImage m_neutralTableImage;
};

Q_DECLARE_INTERFACE(Player, "Player");

#endif // PLAYER_H
