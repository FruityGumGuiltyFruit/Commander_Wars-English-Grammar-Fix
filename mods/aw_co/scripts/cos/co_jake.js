CO_JAKE.init = function(co, map)
{
    co.setPowerStars(0);
    co.setSuperpowerStars(3);
};
CO_JAKE.activateSuperpower = function(co, powerMode, map)
{
    CO_JAKE.activatePower(co, powerMode, map);
};
CO_JAKE.getSuperPowerDescription = function()
{
    return CO_JAKE.getPowerDescription();
};
CO_JAKE.getSuperPowerName = function()
{
    return CO_JAKE.getPowerName();
};
CO_JAKE.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                     defender, defPosX, defPosY, isDefender, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        if (map !== null)
        {
            if (map.onMap(atkPosX, atkPosY))
            {
                var terrainID = map.getTerrain(atkPosX, atkPosY).getID();
                switch (co.getPowerMode())
                {
                case GameEnums.PowerMode_Tagpower:
                case GameEnums.PowerMode_Superpower:
                case GameEnums.PowerMode_Power:
                    if (terrainID === "PLAINS")
                    {
                        return 20;
                    }
                    return 0;
                default:
                    if (terrainID === "PLAINS")
                    {
                        return 10;
                    }
                    break;
                }
            }
        }
    }
    return 0;
};

CO_JAKE.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
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

CO_JAKE.getFirerangeModifier = function(co, unit, posX, posY, map)
{
    if (co.getIsCO0() === true)
    {
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
        case GameEnums.PowerMode_Power:
            if (unit.getBaseMaxRange() > 1 &&
                unit.getUnitType() === GameEnums.UnitType_Ground)
            {
                return 1;
            }
            break;
        default:
            return 0;
        }
    }
    return 0;
};
CO_JAKE.getMovementpointModifier = function(co, unit, posX, posY, map)
{
    return 0;
};
