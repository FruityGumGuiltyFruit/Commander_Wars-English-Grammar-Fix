var Constructor = function()
{
    this.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                       defender, defPosX, defPosY, isAttacker, action, luckmode, map)
    {
		if (CO_PERK.isActive(co))
		{
			if (defender.isTransporter())
			{
				return -20;
			}
		}
        return 0;
    };
	// Perk - Intel
    this.getDescription = function()
    {
        return qsTr("Decreases the transporter defense by 20%.");
    };
    this.getIcon = function(map)
    {
        return "classic_apc";
    };
    this.getName = function()
    {
        return qsTr("Classic APC -20%");
    };
    this.getGroup = function()
    {
        return qsTr("Defensive Debuff");
    };
    this.getCosts = function()
    {
        return -1;
    };
};

Constructor.prototype = CO_PERK;
var CO_PERK_CLASSIC_APC = new Constructor();
