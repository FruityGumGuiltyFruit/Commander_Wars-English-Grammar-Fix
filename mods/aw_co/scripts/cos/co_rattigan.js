CO_RATTIGAN.init = function(co, map)
{
    co.setPowerStars(0);
    co.setSuperpowerStars(3);
};
CO_RATTIGAN.activateSuperpower = function(co, powerMode, map)
{
    CO_RATTIGAN.activatePower(co, powerMode, map);
};
CO_RATTIGAN.getSuperPowerDescription = function()
{
    return CO_RATTIGAN.getPowerDescription();
};
CO_RATTIGAN.getSuperPowerName = function()
{
    return CO_RATTIGAN.getPowerName();
};
CO_RATTIGAN.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                         defender, defPosX, defPosY, isDefender, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        var count = CO_RATTIGAN.getUnitCount(co, defPosX, defPosY, map);
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
        case GameEnums.PowerMode_Power:
            if (count > 0)
            {
                return 20 + count * 5;
            }
            return 0;
        default:
            break;
        }
        if (count > 0)
        {
            return 25;
        }
        else
        {
            return -10;
        }
    }
};

CO_RATTIGAN.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                          defender, defPosX, defPosY, isAttacker, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        var count = CO_RATTIGAN.getUnitCount(co, defPosX, defPosY, map);
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
        case GameEnums.PowerMode_Power:
            return 10;
        default:
            break;
        }
    }
    return 0;
};

CO_RATTIGAN.getMovementpointModifier = function(co, unit, posX, posY, map)
{
    if (co.getIsCO0() === true)
    {
        if (co.getPowerMode() > GameEnums.PowerMode_Off)
        {
            return 1;
        }
    }
    return 0;
};
