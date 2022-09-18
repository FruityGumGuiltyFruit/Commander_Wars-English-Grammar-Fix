CO_JUGGER.init = function(co, map)
{
    co.setPowerStars(0);
    co.setSuperpowerStars(3);
};
CO_JUGGER.activateSuperpower = function(co, powerMode, map)
{
    CO_JUGGER.activatePower(co, powerMode, map);
};
CO_JUGGER.getSuperPowerDescription = function()
{
    return CO_JUGGER.getPowerDescription();
};
CO_JUGGER.getSuperPowerName = function()
{
    return CO_JUGGER.getPowerName();
};
CO_JUGGER.getBonusLuck = function(co, unit, posX, posY, map)
{
    if (co.getIsCO0() === true)
    {
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
            return 95;
        case GameEnums.PowerMode_Power:
            return 55;
        default:
            return 30;
        }
    }
    return 0;
};

CO_JUGGER.getBonusMisfortune = function(co, unit, posX, posY, map)
{
    if (co.getIsCO0() === true)
    {
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
            return 45;
        case GameEnums.PowerMode_Power:
            return 25;
        default:
            return 15;
        }
    }
    return 0;
};

CO_JUGGER.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                      defender, defPosX, defPosY, isDefender, action, luckmode, map)
{
    return 0;
};

CO_JUGGER.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
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
