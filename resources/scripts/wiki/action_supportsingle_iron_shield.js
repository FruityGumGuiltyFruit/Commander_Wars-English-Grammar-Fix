var Constructor = function()
{
    this.loadPage = function(wikipage)
    {
        wikipage.loadHeadline(LOADEDWIKIPAGE.getName());
        wikipage.loadImage("defenseStar", 2);
        wikipage.loadText(qsTr("Shields an allied unit. This highly increases the defense of the unit until the start of the next turn."));
    };

    this.getTags = function()
    {
        return [qsTr("Action")];
    };

    this.getName = function()
    {
        return qsTr("Iron Shield");
    };
};

Constructor.prototype = WIKIPAGE;
var LOADEDWIKIPAGE = new Constructor();
