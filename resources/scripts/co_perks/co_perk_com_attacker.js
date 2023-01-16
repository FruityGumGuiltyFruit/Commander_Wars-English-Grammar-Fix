var Constructor = function()
{
    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
		if (CO_PERK.isActive(co))
		{
			var towers = co.getOwner().getBuildingCount("TOWER");        
			return towers * 5;
		}
        return 0;
    };
	// Perk - Intel
    this.getDescription = function()
    {
        return qsTr("Increases the attack boost per tower by 5%.");
    };
    this.getIcon = function(map)
    {
        return "towerpower";
    };
    this.getName = function()
    {
        return qsTr("Com Attacker");
    };
    this.getGroup = function()
    {
        return qsTr("Offensive");
    };
    this.getCosts = function()
    {
        return 2;
    };
};

Constructor.prototype = CO_PERK;
var CO_PERK_COM_ATTACKER = new Constructor();
