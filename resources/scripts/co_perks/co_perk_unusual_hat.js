var Constructor = function()
{
    this.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                       defender, defPosX, defPosY, isAttacker, action, luckmode, map)
    {
		if (CO_PERK.isActive(co))
		{
			var rangedAttacked = (Math.abs(atkPosX - defPosX) + Math.abs(atkPosY - defPosY) > 1);
			if (rangedAttacked)
			{
				return -10;
			}
		}
        return 0;
    };
	// Perk - Intel
    this.getDescription = function()
    {
        return qsTr("Decreases the indirect-fire defense by 10%.");
    };
    this.getIcon = function(map)
    {
        return "unusual_hat";
    };
    this.getName = function()
    {
        return qsTr("Unusual Hat -10%");
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
var CO_PERK_UNUSUAL_HAT = new Constructor();
