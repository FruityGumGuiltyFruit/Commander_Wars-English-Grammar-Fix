var Constructor = function()
{
    this.getTerrainGroup = function()
    {
        return 4;
    };
    this.getDefaultPalette = function()
    {
        return "palette_waste";
    };
    this.baseTerrainId = "WASTE";
    this.loadBaseSprite = function(terrain, map)
    {
        __BASERUIN.loadBase(terrain, "waste_ruin", map);
    };
    this.getTerrainSprites = function()
    {
        return __BASERUIN.getSprites("waste_ruin");
    };
};
Constructor.prototype = __BASERUIN;
var WASTE_RUIN = new Constructor();
