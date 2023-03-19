CO_COLIN.init = function(co, map)
{
    co.setPowerStars(0);
    co.setSuperpowerStars(2);
};
CO_COLIN.activateSuperpower = function(co, powerMode, map)
{
    CO_COLIN.activatePower(co, map);
};
CO_COLIN.getSuperPowerDescription = function()
{
    return CO_COLIN.getPowerDescription();
};
CO_COLIN.getSuperPowerName = function()
{
    return CO_COLIN.getPowerName();
};
CO_COLIN.globalBoost = 10;
CO_COLIN.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                      defender, defPosX, defPosY, isDefender, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        return -CO_COLIN.globalBoost;
    }
    return 0;
};

CO_COLIN.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                       defender, defPosX, defPosY, isAttacker, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        if (co.getPowerMode() > GameEnums.PowerMode_Off)
        {
            return 10;
        }
    }
    return 0;
};
CO_COLIN.getCostModifier = function(co, id, baseCost, posX, posY, map)
{
    if (co.getIsCO0() === true)
    {
        return -baseCost * CO_COLIN.costModifier / 100;
    }
    return 0;
};
