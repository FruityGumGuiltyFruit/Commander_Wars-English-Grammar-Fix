var Constructor = function()
{
    this.getBuildingGroup = function()
    {
        return 1;
    };
    
    this.loadSprites = function(building, neutral, map)
    {
        if (building.getOwnerID() >= 0 && !neutral)
        {
            // none neutral player
            building.loadSprite("mine", false);
            building.loadSpriteV2("mine+mask", GameEnums.Recoloring_Matrix);
        }
        else
        {
            // neutral player
            building.loadSprite("mine+neutral", false);
        }
        building.loadSprite("mine+shadow+" + BUILDING.getBuildingBaseTerrain(building, map), false);
    };
    this.getBaseIncome = function()
    {
        return 3000;
    };
    this.getName = function()
    {
        return qsTr("Mine");
    };

    this.getDescription = function()
    {
        return qsTr("Ore mine. Once captured produces a lot of funds, but can't repair units.");
    };

    this.getVisionHide = function(building)
    {
        return true;
    };
}

Constructor.prototype = BUILDING;
var MINE = new Constructor();
