var Constructor = function()
{
    this.init = function (terrain)
    {
        if (terrain.getPalette() === "")
        {
            terrain.setPalette(Global[terrain.getTerrainID()].getDefaultPalette());
        }
        terrain.setVisionHigh(2);
        terrain.setTerrainName(Global[terrain.getTerrainID()].getName(terrain));
        terrain.setHp(100);
    };
    this.baseTerrainId = "PLAINS";
    this.getName = function(terrain = null)
    {
        var baseTerrainId = "";
        if (terrain !== null)
        {
            baseTerrainId = Global[terrain.getTerrainID()].baseTerrainId
        }
        if (baseTerrainId === "WASTE")
        {
            return qsTr("Waste Weld");
        }
        else if (baseTerrainId === "SNOW")
        {
            return qsTr("Snowy Weld");
        }
        else if (baseTerrainId === "DESERT")
        {
            return qsTr("Desert Weld");
        }
        else
        {
            return qsTr("Weld");
        }
    };
    this.getOffensiveFieldBonus = function(terrain, attacker, atkPosX, atkPosY,
                                           defender, defPosX, defPosY, isDefender, action, luckMode)
    {
        var baseTerrainId = ""
        if (terrain !== null)
        {
            baseTerrainId = Global[terrain.getTerrainID()].baseTerrainId;
        }
        if (baseTerrainId === "DESERT")
        {
            return -20;
        }
        return 0;
    };
    this.getBonusVision = function(unit, terrain, map)
    {
        var baseTerrainId = ""
        if (terrain !== null)
        {
            baseTerrainId = Global[terrain.getTerrainID()].baseTerrainId;
        }
        if (baseTerrainId === "WASTE")
        {
            return 1;
        }
        return 0;
    };
    this.loadBaseTerrain = function(terrain, currentTerrainID, map, currentPalette)
    {
        if (currentTerrainID === "SNOW")
        {
            terrain.loadBaseTerrain("SNOW", currentPalette);
        }
        else if (currentTerrainID === "DESERT")
        {
            terrain.loadBaseTerrain("DESERT", currentPalette);
        }
        else if (currentTerrainID === "WASTE")
        {
            terrain.loadBaseTerrain("WASTE", currentPalette);
        }
        else if (currentTerrainID === "PLAINS")
        {
            terrain.loadBaseTerrain("PLAINS", currentPalette);
        }
        else
        {
            var baseTerrainId = ""
            if (terrain !== null)
            {
                baseTerrainId = Global[terrain.getTerrainID()].baseTerrainId;
            }
            terrain.loadBaseTerrain(baseTerrainId, currentPalette);
        }
    };
    this.loadBase = function(terrain, spriteId, map)
    {
        var surroundings = terrain.getSurroundings("PIPELINE,DESERT_PIPELINE,SNOW_PIPELINE,WASTE_PIPELINE", false, false, GameEnums.Directions_Direct, true);
        var x = terrain.getX();
        var y = terrain.getY();
        if (map !== null)
        {
            if (map.onMap(x, y + 1))
            {
                var building = map.getTerrain(x, y + 1).getBuilding();
                if (building !== null &&
                   (building.getBuildingID() === "ZBLACKHOLE_FACTORY" ||
                    building.getBuildingID() === "ZBLACKHOLE_FACTORYDESERT" ||
                    building.getBuildingID() === "ZBLACKHOLE_FACTORYWASTE" ||
                    building.getBuildingID() === "ZBLACKHOLE_FACTORYSNOW") &&
                    building.getX() - 1 === x && building.getY() - 4 === y)
                {
                    if (surroundings.indexOf("+W") >= 0)
                    {
                        surroundings = surroundings.replace("+W", "+S+W");
                    }
                    else
                    {
                        surroundings += "+S";
                    }
                }
            }
        }
        if ((surroundings === ""))
        {
            terrain.loadBaseSprite(spriteId + "+E+W");
        }
        else if ((surroundings === "+N+S"))
        {

            terrain.loadBaseSprite(spriteId + "+N+S");
        }
        else if ((surroundings === "+E+W"))
        {
            terrain.loadBaseSprite(spriteId + "+E+W");
        }
        else
        {
            terrain.loadBaseSprite(spriteId + "+E+W");
        }
    };
    this.canBePlaced = function(x, y, map)
    {
        var terrain = map.getTerrain(x, y);
        var surroundings = terrain.getSurroundings("PIPELINE,DESERT_PIPELINE,SNOW_PIPELINE,WASTE_PIPELINE", false, false, GameEnums.Directions_Direct, true);
        if ((surroundings === "+E+W") || (surroundings === "+N+S"))
        {
            return true;
        }
        else
        {
            return false;
        }
    };
    this.onBaseDestroyed = function(terrain, spriteId, map)
    {
        // called when the terrain is destroyed and replacing of this terrain starts
        var x = terrain.getX();
        var y = terrain.getY();
        map.replaceTerrainOnly(spriteId, x, y);
        map.getTerrain(x, y).loadSprites();
        var animation = GameAnimationFactory.createAnimation(map, x, y);
        animation.addSprite("explosion+land", -map.getImageSize() / 2, -map.getImageSize(), 0, 2);
        animation.addScreenshake(30, 0.95, 1000, 200);
        animation.setSound("pipe_destroyed.wav");
    };
    this.getMiniMapIcon = function()
    {
        return "minimap_weld";
    };
    this.getTerrainAnimationForeground = function(unit, terrain, defender, map)
    {
        return "fore_pipeline";
    };

    this.getTerrainAnimationBackground = function(unit, terrain, defender, map)
    {
        var id = TERRAIN.getTerrainAnimationId(terrain, map);
        var baseTerrainId = ""
        if (terrain !== null)
        {
            baseTerrainId = Global[terrain.getTerrainID()].baseTerrainId;
        }
        if (baseTerrainId === "WASTE")
        {
            return "back_wasteplains";
        }
        else if (baseTerrainId === "SNOW")
        {
            return TERRAIN.getTerrainBackgroundId(id, "snow", true);
        }
        else if (baseTerrainId === "DESERT")
        {
            return TERRAIN.getTerrainBackgroundId(id, "desert", true);
        }
        else
        {
            var weatherModifier = TERRAIN.getWeatherModifier(map);
            return TERRAIN.getTerrainBackgroundId(id, weatherModifier, true);
        }
    };

    this.getDescription = function(terrain)
    {
        var baseTerrainId = ""
        if (terrain !== null)
        {
            baseTerrainId = Global[terrain.getTerrainID()].baseTerrainId;
        }
        if (baseTerrainId === "WASTE")
        {
            return qsTr("Black Hole Pipeline Weld can be destroyed to cross the pipeline.");
        }
        else if (baseTerrainId === "SNOW")
        {
            return qsTr("Black Hole Pipeline Weld can be destroyed to cross the pipeline.");
        }
        else if (baseTerrainId === "DESERT")
        {
            return qsTr("Black Hole Pipeline Weld can be destroyed to cross the pipeline. It reduces the firepower of units by 20%.");
        }
        else
        {
            return qsTr("Black Hole Pipeline Weld can be destroyed to cross the pipeline.");
        }
    };

    this.getSprites = function(spriteId)
    {
        return [spriteId + "+E+W",
                spriteId + "+N+S"];
    };
    this.getEditorPlacementSound = function()
    {
        return "placeBuilding.wav";
    };
};
Constructor.prototype = TERRAIN;
var __BASEWELD = new Constructor();
