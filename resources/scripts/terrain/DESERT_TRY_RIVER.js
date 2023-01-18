var Constructor = function()
{
    this.getTerrainGroup = function()
    {
        return 2;
    };
    // loader for stuff which needs C++ Support
    this.init = function (terrain)
    {
        terrain.setTerrainName(DESERT_TRY_RIVER.getName());
    };
    this.getName = function()
    {
        return qsTr("Dry River");
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
        else
        {
            terrain.loadBaseTerrain("PLAINS");
        }
    };
    this.getOffensiveFieldBonus = function(terrain, attacker, atkPosX, atkPosY,
                                           defender, defPosX, defPosY, isDefender, action, luckMode)
    {
        return -20;
    };
    this.loadBaseSprite = function(terrain, map)
    {
        var surroundings = terrain.getSurroundings("DESERT_TRY_RIVER,BRIDGE,BRIDGE1,BRIDGE2", false, false, GameEnums.Directions_Direct);
        terrain.loadBaseSprite("desert_try_river" + surroundings);
    };
    this.getMiniMapIcon = function()
    {
        return "minimap_desert_try_river";
    };
    this.getTerrainAnimationForeground = function(unit, terrain, defender, map)
    {
        return "fore_river";
    };
    this.getTerrainAnimationBackground = function(unit, terrain, defender, map)
    {
        return "back_river";
    };

    this.getDescription = function()
    {
        return qsTr("Once a river. Now an empty riverbed that can be crossed by most units doesn't provide cover. It reduces the firepower of units by 20%.");
    };

    this.getTerrainSprites = function()
    {
        // array of sprites that can be selected as fix sprites for this terrain
        return ["desert_try_river",
                "desert_try_river+E",
                "desert_try_river+E+S",
                "desert_try_river+E+S+W",
                "desert_try_river+E+W",
                "desert_try_river+N",
                "desert_try_river+N+E",
                "desert_try_river+N+E+S",
                "desert_try_river+N+E+S+W",
                "desert_try_river+N+E+W",
                "desert_try_river+N+S",
                "desert_try_river+N+S+W",
                "desert_try_river+N+W",
                "desert_try_river+S",
                "desert_try_river+S+W",
                "desert_try_river+W"];
    };
    this.getTerrainAnimationForeground = function(unit, terrain, defender, map)
    {
        return "";
    };

    this.getTerrainAnimationBackground = function(unit, terrain, defender, map)
    {
        if (unit !== null &&
            unit.getMovementType() === "MOVE_SMALL_BOAT")
        {
            return "back_desertriver+boat";
        }
        else
        {
            var variables = terrain.getVariables();
            var variable = variables.getVariable("BACKGROUND_ID");
            var rand = 0;
            if (variable === null)
            {
                rand = globals.randInt(0, 1);
                variable = variables.createVariable("BACKGROUND_ID");
                variable.writeDataInt32(rand);
            }
            else
            {
                rand = variable.readDataInt32();
            }
            return "back_desertriver+" + rand.toString();
        }
    };
};
Constructor.prototype = TERRAIN;
var DESERT_TRY_RIVER = new Constructor();
