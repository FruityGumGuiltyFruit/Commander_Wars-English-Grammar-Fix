var Constructor = function()
{
    this.loadPage = function(wikipage)
    {
        wikipage.loadHeadline(LOADEDWIKIPAGE.getName());
        wikipage.loadImage("icon_bh", 2);
        wikipage.loadText(qsTr("Black Hole is an aggressive force that seeks to dominate Wars World. Its COs have evil intents and are highly destructive, not caring about how many units they lose, as long as they can achieve their goal. In the first two games, Black Hole occupies some small islands in Cosmo Land and Macro Land respectively as their base of operation, before being driven out by the Allied Nations. "));
    };

    this.getTags = function()
    {
        return ["Army"];
    };

    this.getName = function()
    {
        return qsTr("Black Hole");
    };
};

Constructor.prototype = WIKIPAGE;
var LOADEDWIKIPAGE = new Constructor();
