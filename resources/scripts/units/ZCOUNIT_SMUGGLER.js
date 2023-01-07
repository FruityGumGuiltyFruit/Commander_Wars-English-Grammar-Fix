var Constructor = function()
{
    this.getUnitDamageID = function(unit)
    {
        return "RECON";
    };

    this.init = function(unit)
    {
        unit.setAmmo1(10);
        unit.setMaxAmmo1(10);
        unit.setWeapon1ID("WEAPON_RECON_MG");

        unit.setAmmo2(1);
        unit.setMaxAmmo2(1);
        unit.setWeapon2ID("WEAPON_CARGO");

        unit.setFuel(80);
        unit.setMaxFuel(80);
        unit.setBaseMovementPoints(8);
        unit.setMinRange(1);
        unit.setMaxRange(1);
        unit.setVision(2);
    };
    this.getBaseCost = function()
    {
        return 4500;
    };
    
    this.loadSprites = function(unit)
    {
        unit.loadSpriteV2("smuggler+mask", GameEnums.Recoloring_Matrix);
    };
    this.getMovementType = function()
    {
        return "MOVE_TIRE_A";
    };
    this.doWalkingAnimation = function(action, map)
    {
        var unit = action.getTargetUnit();
        var animation = GameAnimationFactory.createWalkingAnimation(map, unit, action);
        animation.loadSpriteV2("smuggler+walk+mask", GameEnums.Recoloring_Matrix, 2);
        animation.setSound("movetire.wav", -2);
        return animation;
    };
    this.getName = function()
    {
        return qsTr("Smuggler");
    };
    this.canMoveAndFire = function()
    {
        return true;
    };

    this.getDescription = function()
    {
        return qsTr("High mobility. Can resupply another unit. Gets a free repair and resupply on any building.");
    };
    this.getUnitType = function()
    {
        return GameEnums.UnitType_Ground;
    };
    this.startOfTurn = function(unit, map)
    {
        var terrain = unit.getTerrain();
        if (terrain !== null)
        {
            var building = terrain.getBuilding();
            if (building !== null)
            {
                var unitX = unit.getX();
                var unitY = unit.getY();
                var repairAmount = 2 + unit.getRepairBonus(Qt.point(unitX, unitY));
                unit.setHp(unit.getHpRounded() + repairAmount);
                unit.refill(false);
                if (!unit.isStealthed(map.getCurrentViewPlayer()))
                {
                    var animationCount = GameAnimationFactory.getAnimationCount();
                    var animation = GameAnimationFactory.createAnimation(map, unitX, unitY);
                    var width = animation.addText(qsTr("REPAIR"), map.getImageSize() / 2 + 25, 2, 1);
                    animation.addBox("info", map.getImageSize() / 2, 0, width + 36, map.getImageSize(), 400);
                    animation.addSprite("repair", map.getImageSize() / 2 + 4, 4, 400, 2);
                    animation.addSound("repair_2.wav");
                    if (animationCount > 0)
                    {
                        GameAnimationFactory.getAnimation(animationCount - 1).queueAnimation(animation);
                    }
                }
            }
        }
    };
    this.actionList = ["ACTION_FIRE", "ACTION_SUPPORTSINGLE_SUPPLY", "ACTION_JOIN", "ACTION_LOAD",
                       "ACTION_UNLOAD", "ACTION_WAIT", "ACTION_CO_UNIT_0", "ACTION_CO_UNIT_1"];
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
var ZCOUNIT_SMUGGLER = new Constructor();
