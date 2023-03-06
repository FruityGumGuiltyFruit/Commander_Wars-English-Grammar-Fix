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
        __BASEDESTROYED_WELD.loadBase(terrain, "desert_destroyedweld", map)
    };
    this.getTerrainSprites = function()
    {
        return __BASEDESTROYED_WELD.getSprites("desert_destroyedweld");
    };
};
Constructor.prototype = __BASEDESTROYED_WELD;
var DESERT_DESTROYEDWELD = new Constructor();
