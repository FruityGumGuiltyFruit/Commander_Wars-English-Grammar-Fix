var Constructor = function()
{
    this.loadPage = function(wikipage)
    {
        wikipage.loadHeadline(LOADEDWIKIPAGE.getName());
        wikipage.loadImage("icon_os", 2);
        wikipage.loadText(qsTr("The Orange Star army is lead by Nell and her Sister Rachel also  de-facto leads the Allied Nations. With the exception of Jake, the forces of Orange Star concentrates in direct warfare, excelling in frontal assaults. "));
    };

    this.getTags = function()
    {
        return [qsTr("Army")];
    };

    this.getName = function()
    {
        return qsTr("Orange Star");
    };
};

Constructor.prototype = WIKIPAGE;
var LOADEDWIKIPAGE = new Constructor();
