var Constructor = function()
{
    
    this.loadPage = function(wikipage)
    {
        wikipage.loadHeadline(LOADEDWIKIPAGE.getName());
        wikipage.loadImage("icon_fire", 2);
        wikipage.loadText(qsTr("Orders a minicannon or black cannon to attack a single unit to deal some damage to it."));
    };

    this.getTags = function()
    {
        return [qsTr("Action")];
    };

    this.getName = function()
    {
        return qsTr("Fire with Cannon");
    };
};

Constructor.prototype = WIKIPAGE;
var LOADEDWIKIPAGE = new Constructor();
