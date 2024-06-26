CO_YUKIO.superPowerBombDamage = 4;

CO_YUKIO.powerTrueDamageBonus = 30;
CO_YUKIO.powerTrueDefenseBonus = 30;
CO_YUKIO.powerMinTrueDamage = 10
CO_YUKIO.powerOffBonus = 0;
CO_YUKIO.powerDefBonus = 10;

CO_YUKIO.d2dCoZoneOffBonus = 0;
CO_YUKIO.d2dCoZoneDefBonus = 0;
CO_YUKIO.d2dCoZoneTrueDamageBonus = 0;
CO_YUKIO.d2dCoZoneTrueDefenseBonus = 0;
CO_YUKIO.d2dCoZoneMinTrueDamage = 0;

CO_YUKIO.d2dOffBonus = 0;
CO_YUKIO.d2dDefBonus = 0;
CO_YUKIO.d2dTrueDamageBonus = 15;
CO_YUKIO.d2dTrueDefenseBonus = 15;
CO_YUKIO.d2dMinTrueDamage = 10;


CO_YUKIO.init = function(co, map)
{
    co.setPowerStars(0);
    co.setSuperpowerStars(8);
};
CO_YUKIO.activateSuperpower = function(co, powerMode, map)
{
    CO_YUKIO.activatePower(co, map);
    co.setPowerMode(GameEnums.PowerMode_Power);
};
CO_YUKIO.getSuperPowerDescription = function()
{
    return CO_YUKIO.getPowerDescription();
};
CO_YUKIO.getSuperPowerName = function()
{
    return CO_YUKIO.getPowerName();
};
