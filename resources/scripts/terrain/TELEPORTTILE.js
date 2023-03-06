var Constructor = function()
{
    this.getTerrainGroup = function()
    {
        return 1;
    };
    this.getDefaultPalette = function()
    {
        return "palette_clear";
    };
    this.init = function (terrain)
    {
        terrain.setTerrainName(TELEPORTTILE.getName());
    };

    this.getName = function()
    {
        return qsTr("Teleport Tile");
    };
    this.loadBaseTerrain = function(terrain, currentTerrainID, map)
    {
        if (currentTerrainID === "SNOW")
        {
            terrain.loadBaseTerrain("SNOW");
        }
        else if (currentTerrainID === "DESERT")
        {
            terrain.loadBaseTerrain("DESERT");
        }
        else if (currentTerrainID === "WASTE")
        {
            terrain.loadBaseTerrain("WASTE");
        }
        else if (currentTerrainID === "LAKE")
        {
            terrain.loadBaseTerrain("LAKE");
        }
        else if (currentTerrainID === "SEA")
        {
            terrain.loadBaseTerrain("SEA");
        }
        else
        {
            terrain.loadBaseTerrain("PLAINS");
        }
    };
    this.loadBaseSprite = function(terrain, map)
    {
        var surroundings = terrain.getSurroundings("TELEPORTTILE", false, false, GameEnums.Directions_Direct, false, true);
        terrain.loadBaseSprite("teleporttile" + surroundings);
    };
    this.getMiniMapIcon = function()
    {
        return "minimap_teleport";
    };
    this.getTerrainAnimationForeground = function(unit, terrain, defender, map)
    {
        return "fore_sea";
    };
    this.getTerrainAnimationBackground = function(unit, terrain, defender, map)
    {
        var weatherModifier = TERRAIN.getWeatherModifier(map);
        return "back_" + weatherModifier +"sea";
    };
    this.getDescription = function()
    {
        return qsTr("A mysterious tile that costs 0MP to move over.");
    };

    this.getTerrainSprites = function()
    {
        // array of sprites that can be selected as fix sprites for this terrain
        return ["teleporttile",
                "teleporttile+E",
                "teleporttile+E+S",
                "teleporttile+E+S+W",
                "teleporttile+E+W",
                "teleporttile+N",
                "teleporttile+N+E",
                "teleporttile+N+E+S",
                "teleporttile+N+E+S+W",
                "teleporttile+N+E+W",
                "teleporttile+N+S",
                "teleporttile+N+S+W",
                "teleporttile+N+W",
                "teleporttile+S",
                "teleporttile+S+W",
                "teleporttile+W"];
    };
};
Constructor.prototype = TERRAIN;
var TELEPORTTILE = new Constructor();
