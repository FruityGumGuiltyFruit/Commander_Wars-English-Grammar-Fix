MOVE_CRAWLER.getMovementpoints = function(terrain, unit, trapChecking = false)
{
    var currentUnit = terrain.getUnit();
    if ((currentUnit !== null && unit !== null) &&
        (unit.getOwner().isEnemy(currentUnit.getOwner())))
    {
        if (!currentUnit.isStealthed(unit.getOwner()) || trapChecking)
        {
            if (currentUnit.getUnitType() !== GameEnums.UnitType_Air)
            {
                return -1;
            }
        }
    }
    var id = terrain.getID();
    if ((id === "ZGATE_E_W" || id === "ZGATE_N_S") &&
            (unit !== null) &&
            (unit.getOwner().isAlly(terrain.getBuilding().getOwner())))
    {
        return 1;
    }
    return MOVEMENTTABLE.getMovementpointsFromTable(terrain, MOVE_CRAWLER.movementpointsTable);
};
