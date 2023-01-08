PLAYER.startOfTurn = function(player, map)
{
    // moved all code to sub functions so it easier to cross mod against this vanilla mod
    PLAYER.coop_ModDoStartOfTurnRepairing(player, map);
    PLAYER.coop_ModApplyDefenceDebuf(player, map);
    PLAYER.coop_ModApplyRoyalGuardDebuf(player, map);
};

PLAYER.coop_ModDoStartOfTurnRepairing = function(player, map)
{
    if (!player.getIsDefeated())
    {
        var count = map.getPlayerCount();
        for (var i = 0; i < count; i++)
        {
            var otherPlayer = map.getPlayer(i);
            if (otherPlayer.isAlly(player) && otherPlayer !== player)
            {
                var buildings = otherPlayer.getBuildings();
                var size = buildings.size();
                for (var j = 0; j < size; j++)
                {
                    var building = buildings.at(j);
                    var constructionList = building.getConstructionList();
                    var repairList = building.getRepairTypes();
                    var unit = building.getTerrain().getUnit();
                    if ((unit !== null) &&
                            (unit.getOwner() === player) &&
                            ((repairList.indexOf(unit.getUnitType()) >= 0) ||
                             (constructionList.indexOf(unit.getUnitID()) >= 0)))
                    {
                        var x = unit.getX();
                        var y = unit.getY();
                        if (unit.canBeRepaired(Qt.point(x, y)))
                        {
                            unit.refill();
                            var repairAmount = 2 + unit.getRepairBonus(Qt.point(x, y));
                            UNIT.repairUnit(unit, repairAmount, map);
                            if (!unit.isStealthed(map.getCurrentViewPlayer()))
                            {
                                var animation = GameAnimationFactory.createAnimation(map, x, y);
                                var width = animation.addText(qsTr("REPAIR"), map.getImageSize() / 2 + 25, -2, 1);
                                animation.addBox("info", map.getImageSize() / 2, 0, width + 36, map.getImageSize(), 400);
                                animation.addSprite("repair", map.getImageSize() / 2 + 4, 4, 400, 2);
                            }
                        }
                    }
                }
            }
        }
    }
}

PLAYER.coop_ModApplyDefenceDebuf = function(player, map)
{
    var variables = player.getVariables();
    var boostVariable = variables.createVariable("COOP_MOD_DEFENSEBOOST_UNITS");
    var boosts = boostVariable.readDataListInt32();
    for (var i = 0; i < boosts.length; i++)
    {
        var unitId = boosts[i];
        var defenseUnit = map.getUnit(unitId);
        if (defenseUnit !== null)
        {
            // negate boost for the rest of the day
            defenseUnit.addDefensiveBonus(-200, 1);
        }
    }
    var empty = [];
    boostVariable.writeDataListInt32(empty);
};

PLAYER.coop_ModApplyRoyalGuardDebuf = function(player, map)
{
    var variables = player.getVariables();
    var boostVariable = variables.createVariable("COOP_MOD_ROYAL_GUARD_UNITS");
    var boosts = boostVariable.readDataListInt32();
    for (var i = 0; i < boosts.length; i++)
    {
        var unitId = boosts[i];
        var defenseUnit = map.getUnit(unitId);
        if (defenseUnit !== null)
        {
            // negate boost for the rest of the day
            defenseUnit.addDefensiveBonus(-ZCOUNIT_ROYAL_GUARD.defBonus, 1);
            defenseUnit.addOffensiveBonus(-ZCOUNIT_ROYAL_GUARD.offBonus, 1);
        }
    }
    var empty = [];
    boostVariable.writeDataListInt32(empty);
};
