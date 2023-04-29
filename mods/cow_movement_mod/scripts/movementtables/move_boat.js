MOVE_BOAT.getMovementpoints = function(terrain, unit, currentTerrain, trapChecking = false, map)
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
    if (terrain.getID() === "BRIDGE" &&
       (terrain.getBaseTerrainID() === "SEA" || terrain.getBaseTerrainID() === "LAKE"))
    {
        return 1;
    }
    return MOVEMENTTABLE.getMovementpointsFromTable(terrain, MOVE_BOAT.movementpointsTable);
};
