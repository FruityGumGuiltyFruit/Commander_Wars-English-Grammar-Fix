var Constructor = function()
{
    this.getWeatherName = function()
    {

        return qsTr("Snow");
    };

    this.getDescription = function()
    {
        return qsTr("Snowy weather which reduces the movement of units.");
    };

    this.getWeatherTerrainSprite = function()
    {
        return "weather_snow";
    };

    this.getWeatherSymbol = function()
    {
        return "weather_symbol_snow_alt";
    };

    this.activate = function(weather, map)
    {
        var animationCount = GameAnimationFactory.getAnimationCount();
        var queueAnimation = null;
        if (animationCount > 0)
        {
            queueAnimation = GameAnimationFactory.getAnimation(animationCount - 1);
        }
        var animation = GameAnimationFactory.createAnimation(map, 0, 0);
        animation.addSprite2("white_pixel", 0, 0, 3200, map.getMapWidth(), map.getMapHeight());
        animation.addTweenColor(0, "#00FFFFFF", "#FFFFFFFF", 3000, true);
        animation.setSound("snow.wav");
        animation.setStartOfAnimationCall("ANIMATION", "preOnAnimationChangedAnimation");
        if (queueAnimation !== null)
        {
            queueAnimation.queueAnimation(animation);
        }
    };

    this.getMovementCostModifier = function(weather, unit, terrain, map)
    {
        var id = terrain.getID();
        if ((unit.getUnitType() === GameEnums.UnitType_Air) ||
            (id !== "STREET" &&
             id !== "STREET1" &&
             id !== "SNOW_STREET" &&
             id !== "BRIDGE" &&
             id !== "BRIDGE1" &&
             id !== "BRIDGE2" &&
             id !== "WASTE_PATH" &&
             id !== "DESERT_PATH" &&
             id !== "DESERT_PATH1" &&
             id !== "TELEPORTTILE" &&
            terrain.getBuilding() === null))
        {
            return 1;
        }
        return 0;
    };

    this.getDefaultWeatherChance = function()
    {
        return 10;
    };
}

Constructor.prototype = WEATHER;
var WEATHER_SNOW = new Constructor();
