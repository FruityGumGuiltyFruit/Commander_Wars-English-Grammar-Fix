var Constructor = function()
{
    this.getBuildingGroup = function()
    {
        return 3;
    };
    this.init = function (building)
    {
        building.setHp(100);
        building.setAlwaysVisble(true);
    };
    
    this.loadSprites = function(building, neutral, map)
    {
        building.loadSpriteV2("deathray+E", GameEnums.Recoloring_Matrix, 400, Qt.point(0,0), TERRAIN.getBuildingTerrainPalette(building));
        building.loadSpriteV2("deathray+E+mask", GameEnums.Recoloring_Matrix);
    };
    this.getDefense = function(building)
    {
        return 0;
    };
    this.getBaseIncome = function()
    {
        return 0;
    };
    this.getDamage = function(building, unit)
    {
        return 8;
    };
    this.getBuildingTargets = function()
    {
        return GameEnums.BuildingTarget_Enemy;
    };
    this.actionList = ["ACTION_DEATHRAY_FIRE"];
    this.startOfTurn = function(building, map)
    {
        building.setFireCount(building.getFireCount() + 1);
    };
    this.getName = function()
    {
        return qsTr("Death Ray");
    };
    this.getActionTargetFields = function(building)
    {
		var targets = globals.getEmptyPointArray();
        // laser to not fire infinitly but the range is still fucking huge :)
        for (var i = 1; i < 80; i++)
        {
            targets.append(Qt.point(i, -1));
            targets.append(Qt.point(i, 0));
            targets.append(Qt.point(i, 1));
        }
        return targets;
    };
    this.getActionTargetOffset = function(building)
    {
        // offset for large buildings since there reference point is bound to the lower right corner.
        return Qt.point(0, -1);
    };
    this.getBuildingWidth = function()
    {
        // one field width default for most buildings
        return 3;
    };
    this.getBuildingHeigth = function()
    {
        // one field heigth default for most buildings
        return 3;
    };
    this.canBuildingBePlaced = function(terrain, building, map)
    {
        return BUILDING.canLargeBuildingPlaced(terrain, building, ZDEATHRAY_S.getBuildingWidth(), ZDEATHRAY_S.getBuildingHeigth(), map);
    };
    this.getMiniMapIcon = function()
    {
        return "minimap_blackholebuilding";
    };
    this.getIsAttackable = function(building, x, y)
    {
        var buildX = building.getX();
        var buildY = building.getY();
        if (y === buildY && buildX - 1 === x)
        {
            return true;
        }
        return false;
    };
    this.onDestroyed = function(building, map)
    {
        // called when the terrain is destroyed and replacing of this terrain starts
        var x = building.getX();
        var y = building.getY();
        var animation2 = GameAnimationFactory.createAnimation(map, 0, 0);
        animation2.addSprite2("white_pixel", 0, 0, 3200, map.getMapWidth(), map.getMapHeight());
        animation2.addTweenColor(0, "#00FFFFFF", "#FFFFFFFF", 3000, true);
        animation2.addScreenshake(45, 0.98, 3000, 200);
        animation2.setSound("black_canon_destroyed.wav");
        map.getTerrain(x, y).loadBuilding("ZBLACK_BUILDING_DESTROYED");
    };

    this.createRayAnimation = function(building, x, y, fields, map)
    {
        var animation = GameAnimationFactory.createAnimation(map, x, y + 2, 200);
        animation.addSprite("deathray_start_loading", 0, 0, 0, 2);
        animation.setRotation(270);
        animation.addSound("deathray_fire.wav");
        var animation2 = GameAnimationFactory.createAnimation(map, x, y + 2, 200);
        animation2.addSprite("deathray_start", 0, 0, 0, 2);
        animation2.setRotation(270);
        animation.queueAnimation(animation2);
        var size = fields.size();
        for (var i = 0; i < size; i++)
        {
            var point = fields.at(i);
            if (map.onMap(x + point.x, y + point.y))
            {
                if ((point.y === -1) && ((point.x) % 2 === 0))
                {
                    animation2 = GameAnimationFactory.createAnimation(map, x + point.x, y + point.y + 3, 200);
                    animation2.addSprite("deathray", 0, -map.getImageSize() * 0.085, 0, 2);
                    animation2.setRotation(270);
                    animation.queueAnimation(animation2);
                }
            }
        }
        return animation2;
    };

    this.getDescription = function()
    {
        return qsTr("Black Hole Deathray that deals 8 HP of damage to all enemie units in a large ray in front of it. It needs 7 days to reload after fire.");
    };
}

Constructor.prototype = BUILDING;
var ZDEATHRAY_E = new Constructor();
