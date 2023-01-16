var Constructor = function()
{
    this.getUnitDamageID = function(unit)
    {
        return "ARTILLERY";
    };

    this.init = function(unit)
    {
        unit.setAmmo1(4);
        unit.setMaxAmmo1(4);
        unit.setWeapon1ID("WEAPON_BATTLESHIP_CANNON");

        unit.setAmmo2(0);
        unit.setMaxAmmo2(0);
        unit.setWeapon2ID("");

        unit.setFuel(30);
        unit.setMaxFuel(30);
        unit.setBaseMovementPoints(3);
        unit.setMinRange(4);
        unit.setMaxRange(7);
        unit.setVision(1);
    };
    
    this.loadSprites = function(unit)
    {
        unit.loadSpriteV2("siege_cannon+mask", GameEnums.Recoloring_Matrix);
    };
    this.getMovementType = function()
    {
        return "MOVE_TANK";
    };
    this.getBaseCost = function()
    {
        return 20000;
    };
    this.getName = function()
    {
        return qsTr("Siege Cannon");
    };

    this.doWalkingAnimation = function(action, map)
    {
        var unit = action.getTargetUnit();
        var animation = GameAnimationFactory.createWalkingAnimation(map, unit, action);
        animation.loadSpriteV2("siege_cannon+walk+mask", GameEnums.Recoloring_Matrix, 2);
        animation.setSound("moveheavytank.wav", -2);
        return animation;
    };

    this.getDescription = function()
    {
        return qsTr("Long range indirect attack unit with low movement. Attack cannot be launched after unit has moved. The siege cannon gets a fire power boost for not moving more than one turn.");
    };
    this.getUnitType = function()
    {
        return GameEnums.UnitType_Ground;
    };
    this.postAction = function(unit, action)
    {
        var unitX = unit.getX();
        var unitY = unit.getY();
        var variables = unit.getVariables();
        var lastPositionX = variables.createVariable("LAST_POSITION_X");
        var lastPositionY = variables.createVariable("LAST_POSITION_Y");
        var turnCount = variables.createVariable("TURN_COUNT");
        var x = lastPositionX.readDataInt32();
        var y = lastPositionY.readDataInt32();
        var turns = turnCount.readDataInt32();
        if (x !== unitX &&
            y !== unitY)
        {
            lastPositionX.writeDataInt32(unitX);
            lastPositionY.writeDataInt32(unitY);
            turnCount.writeDataInt32(0);
            // remove buff if the unit moves and it had the buff
            if (turns >= 2)
            {
                unit.addOffensiveBonus(-ZCOUNIT_SIEGE_CANNON.offBonus, 1);
                unit.unloadIcon("arrow_up");
            }
        }
    };
    this.offBonus = 15;
    this.startOfTurn = function(unit, map)
    {
        var variables = unit.getVariables();
        var turnCount = variables.createVariable("TURN_COUNT");
        var turns = turnCount.readDataInt32();
        var posX = variables.getVariable("LAST_POSITION_X");
        if (posX === null)
        {
            var unitX = unit.getX();
            posX = variables.createVariable("LAST_POSITION_X");
            posX.writeDataInt32(unitX);
        }
        var posY = variables.getVariable("LAST_POSITION_Y");
        if (posX === null)
        {
            var unitY = unit.getY();
            posY = variables.createVariable("LAST_POSITION_Y");
            posY.writeDataInt32(unitY);
        }
        turns = turns + 1;
        turnCount.writeDataInt32(turns);
        if (turns >= 2)
        {
            var owner = unit.getOwner();
            unit.addOffensiveBonus(ZCOUNIT_SIEGE_CANNON.offBonus, 1);
            unit.loadIcon("arrow_up", map.getImageSize() / 2, map.getImageSize() / 2, 1, owner.getPlayerID());
        }
    };
    this.getCOSpecificUnit = function(building)
    {
        return true;
    };
    this.getTypeOfWeapon1 = function(unit)
    {
        return GameEnums.WeaponType_Indirect;
    };
    this.getEditorPlacementSound = function()
    {
        return "moveheavytank.wav";
    };
}

Constructor.prototype = UNIT;
var ZCOUNIT_SIEGE_CANNON = new Constructor();
