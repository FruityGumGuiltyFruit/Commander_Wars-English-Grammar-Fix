CO_MINAMOTO.init = function(co, map)
{
    co.setPowerStars(0);
    co.setSuperpowerStars(3);
};
CO_MINAMOTO.activateSuperpower = function(co, powerMode, map)
{
    CO_MINAMOTO.activatePower(co, powerMode, map);
};
CO_MINAMOTO.getSuperPowerDescription = function()
{
    return CO_MINAMOTO.getPowerDescription();
};
CO_MINAMOTO.getSuperPowerName = function()
{
    return CO_MINAMOTO.getPowerName();
};
CO_MINAMOTO.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                         defender, defPosX, defPosY, isDefender, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        var nearMountains = CO_MINAMOTO.getNearMountain(map, atkPosX, atkPosY);
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Tagpower:
        case GameEnums.PowerMode_Superpower:
        case GameEnums.PowerMode_Power:
            if (nearMountains === true)
            {
                return 40;
            }
            else
            {
                return 0;
            }
        default:
            if (nearMountains === true)
            {
                return 20;
            }
            break;
        }
    }
    return 0;
};
CO_MINAMOTO.getMovementpointModifier = function(co, unit, posX, posY, map)
{
    return 0;
};
CO_MINAMOTO.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
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
CO_MINAMOTO.postBattleActions = function(co, attacker, atkDamage, defender, gotAttacked, weapon, action, map)
{
    if (co.getIsCO0() === true)
    {
        if (gotAttacked === false && attacker.getOwner() === co.getOwner())
        {
            // here begins the fun :D
            var blowRange = 0;
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
            case GameEnums.PowerMode_Power:
                if (atkDamage >= 4.5)
                {
                    blowRange = 2;
                }
                break;
            default:
                break;
            }
            var distX = defender.getX() - attacker.getX();
            var distY = defender.getY() - attacker.getY();
            var distance = Math.abs(distX) + Math.abs(distY);
            if (defender.getHp() > 0 && blowRange > 0 && distance === 1)
            {
                var newPosition = Qt.point(defender.getX(), defender.getY());
                // find blow away position
                for (var i = 1; i <= blowRange; i++)
                {
                    var testPosition = Qt.point(defender.getX() + distX * i, defender.getY() + distY * i);
                    if (map.onMap(testPosition.x, testPosition.y))
                    {
                        var terrain = map.getTerrain(testPosition.x, testPosition.y);
                        if (terrain.getUnit() === null &&
                                defender.canMoveOver(testPosition.x, testPosition.y) === true)
                        {
                            newPosition = testPosition;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                // blow unit away
                defender.moveUnitToField(newPosition.x, newPosition.y);
            }
        }
    }
};
