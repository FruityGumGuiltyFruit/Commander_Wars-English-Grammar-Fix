CO_OZZY.init = function(co, map)
{
    co.setPowerStars(3);
    co.setSuperpowerStars(3);
};
CO_OZZY.activateSuperpower = function(co, powerMode, map)
{
	CO_OZZY.activatePower(co, powerMode, map);
};
CO_OZZY.getSuperPowerDescription = function()
{
    return CO_OZZY.getPowerDescription();
};
CO_OZZY.getSuperPowerName = function()
{
    return CO_OZZY.getPowerName();
};
CO_OZZY.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                             defender, defPosX, defPosY, isDefender, action, luckmode, map)
{
    switch (co.getPowerMode())
    {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
        case GameEnums.PowerMode_Power:
            return 10;
        default:
            if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
            {
                return 10;
            }
            break;
    }
    return 0;
};
CO_OZZY.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                       defender, defPosX, defPosY, isAttacker, action, luckmode, map)
{
    switch (co.getPowerMode())
    {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
        case GameEnums.PowerMode_Power:
            if (isAttacker)
            {
                return 60;
            }
            else
            {
                return 100;
            }
        default:
            if (co.inCORange(Qt.point(defPosX, defPosY), defender))
            {
                return 50;
            }
            break;
    }
    return 0;
};
