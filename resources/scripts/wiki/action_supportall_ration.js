var Constructor = function()
{
    
    this.loadPage = function(wikipage)
    {
        wikipage.loadHeadline(LOADEDWIKIPAGE.getName());
        wikipage.loadImage("ration", 2);
        wikipage.loadText(qsTr("Refills fuel and ammo to all units surrounding this unit."));
    };

    this.getTags = function()
    {
        return [qsTr("Action")];
    };

    this.getName = function()
    {
        return qsTr("Ration");
    };
};

Constructor.prototype = WIKIPAGE;
var LOADEDWIKIPAGE = new Constructor();
