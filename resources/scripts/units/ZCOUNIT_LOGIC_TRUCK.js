var Constructor = function()
{
    this.getUnitDamageID = function(unit)
    {
        return "APC";
    };
    this.init = function(unit)
    {
        unit.setAmmo1(0);
        unit.setMaxAmmo1(0);
        unit.setWeapon1ID("");

        unit.setAmmo2(0);
        unit.setMaxAmmo2(0);
        unit.setWeapon2ID("");

        unit.setFuel(60);
        unit.setMaxFuel(60);
        unit.setBaseMovementPoints(5);
        unit.setMinRange(1);
        unit.setMaxRange(1);
        unit.setVision(1);
    };
    
    this.loadSprites = function(unit)
    {
        unit.loadSpriteV2("logi_truck+mask", GameEnums.Recoloring_Matrix);
    };
    this.getMovementType = function()
    {
        return "MOVE_TIRE_A";
    };
    this.actionList = ["ACTION_LOAD", "ACTION_SUPPORTALL_RATION_MONEY", "ACTION_JOIN", "ACTION_WAIT", "ACTION_CO_UNIT_0", "ACTION_CO_UNIT_1"];
    this.getBaseCost = function()
    {
        return 5000;
    };
    this.getName = function()
    {
        return qsTr("Logistic Truck");
    };
    this.doWalkingAnimation = function(action, map)
    {
        var unit = action.getTargetUnit();
        var animation = GameAnimationFactory.createWalkingAnimation(map, unit, action);
        animation.loadSpriteV2("logi_truck+walk+mask", GameEnums.Recoloring_Matrix, 2);
        animation.setSound("movetire.wav", -2);
        return animation;
    };
    this.getDescription = function()
    {
        return qsTr("The logistic truck is a defenceless unit that </r><div c='#00ff00'> supplies </div><r> adjacent units, and earns funds based on the amount of fuel and ammo resupplied.");
    };
    this.getUnitType = function()
    {
        return GameEnums.UnitType_Ground;
    };
    this.startOfTurn = function(unit, map)
    {
        // pay unit upkeep
        if (unit.getTerrain() !== null)
        {
            ACTION_SUPPORTALL_RATION_MONEY.giveRation(unit, map);
        }
    };
    this.getCOSpecificUnit = function(building)
    {
        return true;
    };
    this.getEditorPlacementSound = function()
    {
        return "movetire.wav";
    };
}

Constructor.prototype = UNIT;
var ZCOUNIT_LOGIC_TRUCK = new Constructor();
