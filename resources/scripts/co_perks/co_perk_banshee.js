var Constructor = function()
{
    this.getFuelCostModifier = function(co, unit, x, y, costs, map)
    {
		if (CO_PERK.isActive(co))
		{
			if (unit.getHidden())
			{
				return 2;
			}
		}
        return 0;
    };
	// Perk - Intel
    this.getDescription = function()
    {
        return qsTr("Dive/Hide fuel cost +2.");
    };
    this.getIcon = function(map)
    {
        return "banshee";
    };
    this.getName = function()
    {
        return qsTr("Banshee");
    };
    this.getGroup = function()
    {
        return qsTr("General Debuff");
    };
    this.getCosts = function()
    {
        return -1;
    };
};

Constructor.prototype = CO_PERK;
var CO_PERK_BANSHEE = new Constructor();
