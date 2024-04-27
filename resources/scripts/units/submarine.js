var Constructor = function()
{
    this.init = function(unit)
    {
        unit.setAmmo1(6);
        unit.setMaxAmmo1(6);
        unit.setWeapon1ID("WEAPON_TORPEDO");

        unit.setAmmo2(0);
        unit.setMaxAmmo2(0);
        unit.setWeapon2ID("");

        unit.setFuel(70);
        unit.setMaxFuel(70);
        unit.setBaseMovementPoints(6);
        unit.setMinRange(1);
        unit.setMaxRange(1);
		unit.setVision(5);
    };
    this.getBaseCost = function()
    {
        return 14000;
    };
    
    this.loadSprites = function(unit)
    {
        unit.loadSpriteV2("submarine+mask", GameEnums.Recoloring_Matrix);
    };
    this.getMovementType = function()
    {
        return "MOVE_SHIP";
    };
    this.getName = function()
    {
        return qsTr("Submarine");
    };
    this.startOfTurn = function(unit, map)
    {
        if (unit.getTerrain() !== null)
        {
            // pay unit upkeep
            var fuelCosts = 1 + unit.getFuelCostModifier(Qt.point(unit.getX(), unit.getY()), 1);
            if (unit.getHidden())
            {
                fuelCosts += 4;
            }
            if (fuelCosts < 0)
            {
                fuelCosts = 0;
            }
            unit.setFuel(unit.getFuel() - fuelCosts);
        }
    };
    this.createExplosionAnimation = function(x, y, unit, map)
    {
        var animation = GameAnimationFactory.createAnimation(map, x, y);
        animation.addSprite("explosion+water", -map.getImageSize() / 2, -map.getImageSize(), 0, 2);
        animation.setSound("explosion+water.wav");
        return animation;
    };
    this.doWalkingAnimation = function(action, map)
    {
        var unit = action.getTargetUnit();
        var animation = GameAnimationFactory.createWalkingAnimation(map, unit, action);
        var unitID = unit.getUnitID().toLowerCase();
        animation.loadSpriteV2(unitID + "+walk+mask", GameEnums.Recoloring_Matrix, 1);
        animation.setSound("movesub.wav", -2);
        return animation;
    };
    this.actionList = ["ACTION_FIRE", "ACTION_STEALTH", "ACTION_UNSTEALTH", "ACTION_JOIN", "ACTION_WAIT", "ACTION_CO_UNIT_0", "ACTION_CO_UNIT_1"];
    this.canMoveAndFire = function()
    {
        return true;
    };
    this.getTerrainAnimationBase = function(unit, terrain, defender, map)
    {
        var weatherModifier = TERRAIN.getWeatherModifier(map);
        return "base_" + weatherModifier + "air";
    };

    this.getTerrainAnimationForeground = function(unit, terrain, defender, map)
    {
        return "";
    };

    this.getTerrainAnimationBackground = function(unit, terrain, defender, map)
    {
        var weatherModifier = TERRAIN.getWeatherModifier(map);
        return "back_" + weatherModifier +"sea";
    };

    this.getTerrainAnimationMoveSpeed = function()
    {
        return 1;
    };

    this.getDescription = function()
    {
        return qsTr("<r>Naval unit. After </r><div c='00ff00'>diving </div><r>, it remains hidden unless an enemy unit is adjacent.</r>");
    };
    this.getUnitType = function()
    {
        return GameEnums.UnitType_Naval;
    };
    this.buildedUnit = function(unit, player, map)
    {
        unit.setHidden(true);
    };
    this.getEditorPlacementSound = function()
    {
        return "movesub.wav";
    };
}

Constructor.prototype = UNIT;
var SUBMARINE = new Constructor();
