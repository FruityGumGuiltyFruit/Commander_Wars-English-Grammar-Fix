var Constructor = function()
{
    this.getUnitDamageID = function(unit)
    {
        return "LIGHT_TANK";
    };

    this.init = function(unit)
    {
        unit.setAmmo1(5);
        unit.setMaxAmmo1(5);
        unit.setWeapon1ID("WEAPON_LIGHT_TANK_GUN");

        unit.setAmmo2(10);
        unit.setMaxAmmo2(10);
        unit.setWeapon2ID("WEAPON_LIGHT_TANK_MG");

        unit.setFuel(70);
        unit.setMaxFuel(70);
        unit.setBaseMovementPoints(6);
        unit.setMinRange(1);
        unit.setMaxRange(1);
        unit.setVision(2);
    };
    
    this.loadSprites = function(unit)
    {
        unit.loadSpriteV2("auto_tank+mask", GameEnums.Recoloring_Matrix);
        unit.loadSpriteV2("auto_tank", GameEnums.Recoloring_None);
    };
    this.doWalkingAnimation = function(action, map)
    {
        var unit = action.getTargetUnit();
        var animation = GameAnimationFactory.createWalkingAnimation(map, unit, action);
        animation.loadSpriteV2("auto_tank+walk+mask", GameEnums.Recoloring_Matrix, 2);
        animation.loadSpriteV2("auto_tank+walk", GameEnums.Recoloring_None, 2);
        animation.setSound("moveheavytank.wav", -2);
        return animation;
    };
    this.getMovementType = function()
    {
        return "MOVE_TANK";
    };
    this.getBaseCost = function()
    {
        return 10000;
    };
    this.getName = function()
    {
        return qsTr("Auto Tank");
    };
    this.canMoveAndFire = function()
    {
        return true;
    };

    this.getDescription = function()
    {
        return qsTr("Light tank unit. Good mobility makes it useful on battlefield. Can attack a second time after performing an attack each turn.");
    };
    this.getUnitType = function()
    {
        return GameEnums.UnitType_Ground;
    };
    this.getCOSpecificUnit = function(building)
    {
        return true;
    };

    this.getCOSpecificUnit = function(building)
    {
        return true;
    };
    this.startOfTurn = function(unit, map)
    {
        var variables = unit.getVariables();
        var unitAttackVariable = variables.createVariable("SECOND_ATTACK");
        unitAttackVariable.writeDataInt32(0);
    };
    this.postBattleActions = function(unit, damage, otherUnit, gotAttacked, weapon, action)
    {
        if (gotAttacked === false && unit.getUnitID() === "ZCOUNIT_AUTO_TANK")
        {
            var variables = unit.getVariables();
            var unitAttackVariable = variables.createVariable("SECOND_ATTACK");
            var attackedOnce = unitAttackVariable.readDataInt32();
            if (attackedOnce === 0)
            {
                unit.setHasMoved(false);
                unit.addMovementBonus(-999, 1);
                unitAttackVariable.writeDataInt32(1);
            }
        }
    };
    this.getEditorPlacementSound = function()
    {
        return "moveheavytank.wav";
    };
}

Constructor.prototype = UNIT;
var ZCOUNIT_AUTO_TANK = new Constructor();
