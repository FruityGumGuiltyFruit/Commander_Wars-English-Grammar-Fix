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
        building.loadSpriteV2("minicannon+S", GameEnums.Recoloring_Matrix, 400, Qt.point(0,0), TERRAIN.getBuildingTerrainPalette(building));
        building.loadSpriteV2("minicannon+S+mask", GameEnums.Recoloring_Matrix);
    };
    this.getBaseIncome = function()
    {
        return 0;
    };
    this.getDefense = function(building)
    {
        return 0;
    };
    this.actionList = ["ACTION_CANNON_FIRE"];
    this.startOfTurn = function(building, map)
    {
        building.setFireCount(1);
    };
    this.getActionTargetFields = function(building)
    {
        return globals.getShotFields(1, 4, 0, 1);
    };
    this.getMiniMapIcon = function()
    {
        return "minimap_blackholebuilding";
    };
    this.getName = function()
    {
        return qsTr("Mini Cannon");
    };
    this.getDamage = function(building, unit)
    {
        return 3;
    };
    this.getBuildingTargets = function()
    {
        return GameEnums.BuildingTarget_Enemy;
    };
    this.onDestroyed = function(building, map)
    {
        // called when the terrain is destroyed and replacing of this terrain starts
        var x = building.getX();
        var y = building.getY();
        map.replaceTerrainOnly("PLAINS_DESTROYED", x, y);
        map.getTerrain(x, y).loadSprites();
        var animation = GameAnimationFactory.createAnimation(map, x, y);
        animation.addSprite("explosion+land", -map.getImageSize() / 2, -map.getImageSize(), 0, 2);
        animation.addScreenshake(30, 0.95, 1000, 200);
        animation.setSound("explosion+land.wav");
    };
    this.getShotAnimation = function(building, map)
    {
        var animation = GameAnimationFactory.createAnimation(map, building.getX(), building.getY(), 70);
        animation.addSprite("minicanon_north", -map.getImageSize() * 0.5, map.getImageSize() * 0.5, 0, 2);
        animation.setSound("minicanon_shot.wav");
        return animation;
    };
    this.getDescription = function()
    {
        return qsTr("Black Hole Minicannon that can deal 3 HP of damage to a single unit in a small range.");
    };
}

Constructor.prototype = BUILDING;
var ZMINICANNON_S = new Constructor();
