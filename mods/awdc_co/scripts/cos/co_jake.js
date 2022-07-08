CO_JAKE.init = function(co, map)
{
    co.setPowerStars(3);
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
                    return 70;
                }
                return 10;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    if (terrainID === "PLAINS")
                    {
                        return 70;
                    }
                    return 10;
                }
                break;
            }
        }
    }
    else if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
    {
        return 10;
    }
    return 0;
};
CO_JAKE.getMovementpointModifier = function(co, unit, posX, posY, map)
{
    return 0;
};
