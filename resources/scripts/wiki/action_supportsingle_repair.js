var Constructor = function()
{
    this.loadPage = function(wikipage)
    {
        wikipage.loadHeadline(LOADEDWIKIPAGE.getName());
        wikipage.loadImage("repair", 2);
        wikipage.loadText(qsTr("Repairs and refills the target unit. The player has to pay for the repair."));
    };

    this.getTags = function()
    {
        return [qsTr("Action")];
    };

    this.getName = function()
    {
        return qsTr("Repair");
    };
};

Constructor.prototype = WIKIPAGE;
var LOADEDWIKIPAGE = new Constructor();
