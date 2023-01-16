var Constructor = function()
{
    this.getMovementcostModifier = function(co, unit, posX, posY, map)
    {
        if (CO_PERK.isActive(co))
        {
            if (map !== null)
            {
                if (unit.getOwner() === co.getOwner())
                {
                    if (map.onMap(posX, posY))
                    {
                        var terrainID = map.getTerrain(posX, posY).getID();
                        var isSea = 	(terrainID === "SEA") ||
                                        (terrainID === "LAKE") ||
                                        (terrainID === "REAF") ||
                                        (terrainID === "ROUGH_SEA") ||
                                        (terrainID === "FOG");
                        if (isSea)
                        {
                            return -999;
                        }
                    }
                }
            }
        }
        return 0;
    };
    // Perk - Intel
    this.getDescription = function()
    {
        return qsTr("Sets movement costs on sea to 1.");
    };
    this.getIcon = function(map)
    {
        return "seamanship";
    };
    this.getName = function()
    {
        return qsTr("Seawalker");
    };
    this.getGroup = function()
    {
        return qsTr("Terrain");
    };
    this.getCosts = function()
    {
        return 1;
    };
};

Constructor.prototype = CO_PERK;
var CO_PERK_SEAWALKER = new Constructor();
