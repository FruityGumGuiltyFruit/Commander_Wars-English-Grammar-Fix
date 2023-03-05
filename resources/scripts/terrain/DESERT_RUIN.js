var Constructor = function()
{
    this.getTerrainGroup = function()
    {
        return 2;
    };
    this.getDefaultPalette = function()
    {
        return "palette_desert";
    };
    this.baseTerrainId = "DESERT";
    this.loadBaseSprite = function(terrain, map)
    {
        __BASERUIN.loadBase(terrain, "desert_ruin", map);
    };
    this.getTerrainSprites = function()
    {
        return __BASERUIN.getSprites("desert_ruin");
    };
};
Constructor.prototype = __BASERUIN;
var DESERT_RUIN = new Constructor();
