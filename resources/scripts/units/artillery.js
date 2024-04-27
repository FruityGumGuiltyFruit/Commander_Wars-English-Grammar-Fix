var Constructor = function()
{
    this.init = function(unit)
    {
        unit.setAmmo1(6);
        unit.setMaxAmmo1(6);
        unit.setWeapon1ID("WEAPON_CANNON");

        unit.setAmmo2(0);
        unit.setMaxAmmo2(0);
        unit.setWeapon2ID("");

        unit.setFuel(50);
        unit.setMaxFuel(50);
        unit.setBaseMovementPoints(5);
        unit.setMinRange(2);
        unit.setMaxRange(3);
        unit.setVision(1);

    };
    
    this.loadSprites = function(unit)
    {
        unit.loadSpriteV2("artillery+mask", GameEnums.Recoloring_Matrix);
    };
    this.getMovementType = function()
    {
        return "MOVE_TANK";
    };
    this.doWalkingAnimation = function(action, map)
    {
        var unit = action.getTargetUnit();
        var animation = GameAnimationFactory.createWalkingAnimation(map, unit, action);
        var unitID = unit.getUnitID().toLowerCase();
        animation.loadSpriteV2(unitID + "+walk+mask", GameEnums.Recoloring_Matrix, 1);
        animation.setSound("movetank.wav", -2);
        return animation;
    };
    this.getBaseCost = function()
    {
        return 6000;
    };
    this.getName = function()
    {
        return qsTr("Artillery");
    };

    this.getDescription = function()
    {
        return qsTr("Indirect-attack unit. Attack cannot be launched after unit has moved.");
    };
    this.getUnitType = function()
    {
        return GameEnums.UnitType_Ground;
    };

    this.getTypeOfWeapon1 = function(unit)
    {
        return GameEnums.WeaponType_Indirect;
    };
    this.getEditorPlacementSound = function()
    {
        return "movetank.wav";
    };
}

Constructor.prototype = UNIT;
var ARTILLERY = new Constructor();
