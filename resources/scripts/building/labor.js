var Constructor = function()
{
    this.getBuildingGroup = function()
    {
        return 0;
    };
    
    this.loadSprites = function(building, neutral, map)
    {
        if (building.getOwnerID() >= 0 && !neutral)
        {
            // none neutral player
            building.loadSprite("labor", false);
            building.loadSpriteV2("labor+mask", GameEnums.Recoloring_Matrix);
        }
        else
        {
            // neutral player
            building.loadSprite("labor+neutral", false);
        }
        building.loadSprite("labor+shadow+" + BUILDING.getBuildingBaseTerrain(building, map), false);
    };
    this.addCaptureAnimationBuilding = function(animation, building, startPlayer, capturedPlayer)
    {
        animation.addBuildingSprite("labor+mask", startPlayer , capturedPlayer, GameEnums.Recoloring_Matrix);
    };
    this.getName = function()
    {
        return qsTr("Labor");
    };
    this.actionList = ["ACTION_BUILD_UNITS"];
    this.constructionList = ["HOELLIUM"];
    this.getConstructionList = function(building)
    {
        return LABOR.constructionList;
    };

    this.getDescription = function()
    {
        return qsTr("<r>Once captured can be used for </r><div c='#00ff00'>production</div><r> of </r><div c='#00ff00'>oozium</div><r>units.</r>");
    };

    this.getVisionHide = function(building)
    {
        return true;
    };
	this.onWeatherChanged = function(building, weather, map)
	{	
		var weatherId = weather.getWeatherId();
		if (weatherId === "WEATHER_SNOW")
		{
			building.loadWeatherOverlaySpriteV2("labor+snow", false);
		};
	};
}

Constructor.prototype = BUILDING;
var LABOR = new Constructor();
