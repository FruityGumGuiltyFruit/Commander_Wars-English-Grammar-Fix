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
        __BASEPIPELINE.loadBase(terrain, "waste_pipeline", map)
    };
    this.getTerrainSprites = function()
    {
        return __BASEPIPELINE.getSprites("waste_pipeline")
    };
};
Constructor.prototype = __BASEPIPELINE;
var WASTE_PIPELINE = new Constructor();
