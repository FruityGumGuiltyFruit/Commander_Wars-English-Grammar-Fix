var Constructor = function()
{
    this.getUnitDamageID = function(unit)
    {
        return "HEAVY_TANK";
    };

    this.init = function(unit)
    {
        unit.setAmmo1(3);
        unit.setMaxAmmo1(3);
        unit.setWeapon1ID("WEAPON_HEAVY_TANK_GUN");

        unit.setAmmo2(5);
        unit.setMaxAmmo2(5);
        unit.setWeapon2ID("WEAPON_HEAVY_TANK_MG");

        unit.setFuel(50);
        unit.setMaxFuel(50);
        unit.setBaseMovementPoints(5);
        unit.setMinRange(1);
        unit.setMaxRange(1);
        unit.setVision(1);
    };
    
    this.loadSprites = function(unit)
    {
        unit.loadSpriteV2("crystal_tank+mask", GameEnums.Recoloring_Matrix);
    };
    this.doWalkingAnimation = function(action, map)
    {
        var unit = action.getTargetUnit();
        var animation = GameAnimationFactory.createWalkingAnimation(map, unit, action);
        animation.loadSpriteV2("crystal_tank+walk+mask", GameEnums.Recoloring_Matrix, 2);
        animation.setSound("moveheavytank.wav", -2);
        return animation;
    };
    this.getMovementType = function()
    {
        return "MOVE_TANK";
    };
    this.getBaseCost = function()
    {
        return 14000;
    };
    this.getName = function()
    {
        return qsTr("Crystal Tank");
    };
    this.canMoveAndFire = function()
    {
        return true;
    };

    this.getDescription = function()
    {
        return qsTr("Special heavy armored tank. The tank is healed by some fraction of the damage dealt.");
    };
    this.getUnitType = function()
    {
        return GameEnums.UnitType_Ground;
    };

    this.postBattleActions = function(unit, damage, otherUnit, gotAttacked, weapon, action)
    {
        if (!gotAttacked)
        {
            // damage can be negativ if we can't do a counter attack the damge is -1
            // avoid loosing hp cause of our passive or power
            var healPercent = 0.35;
            if (damage > 0)
            {
                unit.setHp(unit.getHp() + damage * healPercent);
            }
        }
    };
    this.getCOSpecificUnit = function(building)
    {
        return true;
    };
    this.getEditorPlacementSound = function()
    {
        return "moveheavytank.wav";
    };
}

Constructor.prototype = UNIT;
var ZCOUNIT_CRYSTAL_TANK = new Constructor();
