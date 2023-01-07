var Constructor = function()
{
    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
		if (CO_PERK.isActive(co))
		{
			if (map !== null)
			{
				if (map.getGameRules().getCurrentWeather().getWeatherId() === "WEATHER_SNOW")
                {
					return -20;
				}
			}
		}
        return 0;
    };
	// Perk - Intel
    this.getDescription = function()
    {
        return qsTr("Decreases the attack of units by 20% during snow.");
    };
    this.getIcon = function(map)
    {
        return "frostbitten";
    };
    this.getName = function()
    {
        return qsTr("Frostbitten");
    };
    this.getGroup = function()
    {
        return qsTr("Weather Debuff");
    };
    this.getCosts = function()
    {
        return -1;
    };
};

Constructor.prototype = CO_PERK;
var CO_PERK_FROSTBITTEN = new Constructor();
