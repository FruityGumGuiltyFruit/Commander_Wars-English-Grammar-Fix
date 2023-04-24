var Constructor = function()
{
    this.init = function(co, map)
    {
        co.setPowerStars(3);
        co.setSuperpowerStars(5);
    };

    this.getCOStyles = function()
    {
        // string array containing the endings of the alternate co style
        
        return ["+alt"];
    };

    this.activatePower = function(co, map)
    {

        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(GameEnums.PowerMode_Power);
        dialogAnimation.queueAnimation(powerNameAnimation);

        var units = co.getOwner().getUnits();
        var animations = [];
        var counter = 0;
        units.randomize();
        for (var i = 0; i < units.size(); i++)
        {
            var unit = units.at(i);
            var animation = GameAnimationFactory.createAnimation(map, unit.getX(), unit.getY());
            var delay = globals.randInt(135, 265);
            if (animations.length < 5)
            {
                delay *= i;
            }
            if (i % 2 === 0)
            {
                animation.setSound("power2_1.wav", 1, delay);
            }
            else
            {
                animation.setSound("power2_2.wav", 1, delay);
            }
            if (animations.length < 5)
            {
                animation.addSprite("power2", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power2", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                animations[counter].queueAnimation(animation);
                animations[counter] = animation;
                counter++;
                if (counter >= animations.length)
                {
                    counter = 0;
                }
            }
        }
    };
    this.superPowerDamage = 5;
    this.activateSuperpower = function(co, powerMode, map)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(powerMode);
        powerNameAnimation.queueAnimationBefore(dialogAnimation);

        CO_MINA.minaDamage(co, CO_MINA.superPowerDamage, powerNameAnimation, map);
    };

    this.minaDamage = function(co, value, animation2, map)
    {
        var player = co.getOwner();
        var counter = 0;
        var playerCounter = map.getPlayerCount();
        var animation = null;
        var animations = [];
        for (var i2 = 0; i2 < playerCounter; i2++)
        {
            var enemyPlayer = map.getPlayer(i2);
            if ((enemyPlayer !== player) &&
                (player.checkAlliance(enemyPlayer) === GameEnums.Alliance_Enemy))
            {
                var units = enemyPlayer.getUnits();
                units.sortExpensive();
                var count = units.size() * 1 / 5;
                for (i = 0; i < count; i++)
                {
                    var unit = units.at(i);

                    animation = GameAnimationFactory.createAnimation(map, unit.getX(), unit.getY());
                    var delay = globals.randInt(135, 265);
                    if (animations.length < 5)
                    {
                        delay *= i;
                    }
                    animation.setSound("power4.wav", 1, delay);
                    if (animations.length < 5)
                    {
                        animation.addSprite("power4", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                        if (animation2 !== null)
                        {
                            animation2.queueAnimation(animation);
                        }
                        animations.push(animation);
                    }
                    else
                    {
                        animation.addSprite("power4", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                        animations[counter].queueAnimation(animation);
                        animations[counter] = animation;
                        counter++;
                        if (counter >= animations.length)
                        {
                            counter = 0;
                        }
                    }
                    animation.writeDataInt32(unit.getX());
                    animation.writeDataInt32(unit.getY());
                    animation.writeDataInt32(value);
                    animation.setEndOfAnimationCall("ANIMATION", "postAnimationDamage");
                }
            }
        }
    };

    this.loadCOMusic = function(co, map)
    {
        // put the co music in here.
        switch (co.getPowerMode())
        {
            case GameEnums.PowerMode_Power:
                audio.addMusic("resources/music/cos/power.mp3", 992, 45321);
                break;
            case GameEnums.PowerMode_Superpower:
                audio.addMusic("resources/music/cos/superpower.mp3", 1505, 49515);
                break;
            case GameEnums.PowerMode_Tagpower:
                audio.addMusic("resources/music/cos/tagpower.mp3", 14611, 65538);
                break;
            default:
                audio.addMusic("resources/music/cos/mina.mp3", 76, 100073);
                break;
        }
    };

    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
        switch (co.getPowerMode())
        {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
            case GameEnums.PowerMode_Power:
                return 20;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    return 20;
                }
                return 0;
        }
    };
    this.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                       defender, defPosX, defPosY, isAttacker, action, luckmode, map)
    {
        if (co.inCORange(Qt.point(defPosX, defPosY), defender) ||
                co.getPowerMode() > GameEnums.PowerMode_Off)
        {
            return 20;
        }
        return 0;
    };
    this.getDamageReduction = function(co, damage, attacker, atkPosX, atkPosY, attackerBaseHp,
                                  defender, defPosX, defPosY, isDefender, luckMode, map)
    {
        switch (co.getPowerMode())
        {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                return 0;
            case GameEnums.PowerMode_Power:
                if (luckMode === GameEnums.LuckDamageMode_On)
                {
                    return globals.randInt(0, 50);
                }
                else if (luckMode === GameEnums.LuckDamageMode_Average)
                {
                    return 25;
                }
                else if (luckMode === GameEnums.LuckDamageMode_Max)
                {
                    return 0;
                }
                else if (luckMode === GameEnums.LuckDamageMode_Min)
                {
                    return 50;
                }
                return 0;
            default:
                break;
        }
        return 0;
    };
    this.getAiCoUnitBonus = function(co, unit, map)
    {
        return 1;
    };
    this.getCOUnitRange = function(co, map)
    {
        return 3;
    };
    this.getCOArmy = function()
    {
        return "PF";
    };

    // CO - Intel
    this.getBio = function(co)
    {
        return qsTr("Mina is very protective of those around her, and willing to fight to keep them safe. Don't let her cute appearance deceive you.");
    };
    this.getHits = function(co)
    {
        return qsTr("Victory");
    };
    this.getMiss = function(co)
    {
        return qsTr("Uncertainty");
    };
    this.getCODescription = function(co)
    {
        return qsTr("Mina's troops have no real strength or weakness.");
    };
    this.getLongCODescription = function()
    {
        return qsTr("\nGlobal Effect: \nNo Effects.") +
               qsTr("\n\nCO Zone Effect: \nUnits gain additional firepower and defense.");
    };
    this.getPowerDescription = function(co)
    {
        return qsTr("Unit's defenses may unexpectedly rise.");
    };
    this.getPowerName = function(co)
    {
        return qsTr("Potent of Misfortune");
    };
    this.getSuperPowerDescription = function(co)
    {
        var text = qsTr("Most expensive enemy units suffer %0 HP of damage.");
        text = replaceTextArgs(text, [CO_MINA.superPowerDamage])
        return text;
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("Dark Lightning");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("Enemy is in range. Activating counter-measures..."),
                qsTr("You're done hurting my troops!"),
                qsTr("You shouldn't have ticked me off."),
                qsTr("Let me show you something I learned fighting Black Hole."),
                qsTr("You want to storm my defenses? I am the storm!"),
                qsTr("I once let someone I loved get hurt. I'm not making that mistake again!"),
                qsTr("Let me show you why they call it girl power.")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("You'll never hurt my friends again."),
                qsTr("Well done girls. Let's go home."),
                qsTr("They paid the price for their aggression.")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("I hate losing like this."),
                qsTr("No, I need to be stronger than this.")];
    };
    this.getName = function()
    {
        return qsTr("Mina");
    };
}

Constructor.prototype = CO;
var CO_MINA = new Constructor();
