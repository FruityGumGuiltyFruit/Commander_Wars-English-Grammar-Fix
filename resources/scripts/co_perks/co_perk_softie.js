var Constructor = function()
{
    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
		if (CO_PERK.isActive(co))
		{
			if (attacker.getBaseMaxRange() === 1)
			{
				return -10;
			}
		}
        return 0;
    };
	// Perk - Intel
    this.getDescription = function()
    {
        return qsTr("Decreases the direct attack of units by 10%.");
    };
    this.getIcon = function(map)
    {
        return "softie";
    };
    this.getName = function()
    {
        return qsTr("Softie -10%");
    };
    this.getGroup = function()
    {
        return qsTr("Offensive Debuff");
    };
    this.getCosts = function()
    {
        return -2;
    };
};

Constructor.prototype = CO_PERK;
var CO_PERK_SOFTIE = new Constructor();
