CO_EAGLE.getFuelCostModifier = function(co, unit, costs, map)
{
    if (co.getIsCO0() === true)
    {
        if (unit.getUnitType() === GameEnums.UnitType_Air)
        {
            return -2;
        }
    }
    return 0;
};

CO_EAGLE.activatePower = function(co, map)
{
    var dialogAnimation = co.createPowerSentence();
    var powerNameAnimation = co.createPowerScreen(GameEnums.PowerMode_Power);
    dialogAnimation.queueAnimation(powerNameAnimation);

    var units = co.getOwner().getUnits();
    var animations = [];
    var counter = 0;
    units.randomize();
    var size = units.size();
    for (var i = 0; i < size; i++)
    {
        var unit = units.at(i);
        if (unit.getUnitType() !== GameEnums.UnitType_Infantry)
        {
            var animation = GameAnimationFactory.createAnimation(map, unit.getX(), unit.getY());
            var delay = globals.randInt(135, 265);
            if (animations.length < 5)
            {
                delay *= i;
            }
            animation.setSound("power1.wav", 1, delay);
            if (animations.length < 5)
            {
                animation.addSprite("power1", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 1.5, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power1", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 1.5, delay);
                animations[counter].queueAnimation(animation);
                animations[counter] = animation;
                counter++;
                if (counter >= animations.length)
                {
                    counter = 0;
                }
            }
        }
    }
};

CO_EAGLE.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                      defender, defPosX, defPosY, isDefender, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Superpower:
            if (attacker.getUnitType() === GameEnums.UnitType_Air)
            {
                return 30;
            }
            else if (attacker.getUnitType() === GameEnums.UnitType_Naval)
            {
                return -30;
            }
            return 10;
        case GameEnums.PowerMode_Power:
            if (attacker.getUnitType() === GameEnums.UnitType_Air)
            {
                return 30;
            }
            else if (attacker.getUnitType() === GameEnums.UnitType_Naval)
            {
                return -30;
            }
            else
            {
                return 0;
            }
        default:
            if (attacker.getUnitType() === GameEnums.UnitType_Air)
            {
                return 20;
            }
            break;
        }
        if (attacker.getUnitType() === GameEnums.UnitType_Naval)
        {
            return -30;
        }
    }
    return 0;
};

CO_EAGLE.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                       defender, defPosX, defPosY, isAttacker, action, luckmode, map)
{
    if (co.getIsCO0() === true)
    {
        switch (co.getPowerMode())
        {
        case GameEnums.PowerMode_Superpower:
            return 10;
        case GameEnums.PowerMode_Power:
            if (defender.getUnitType() === GameEnums.UnitType_Air)
            {
                return 30;
            }
            else
            {
                return 10;
            }
        default:
            if (defender.getUnitType() === GameEnums.UnitType_Air)
            {
                return 10;
            }
            break;
        }
    }
    return 0;
};
