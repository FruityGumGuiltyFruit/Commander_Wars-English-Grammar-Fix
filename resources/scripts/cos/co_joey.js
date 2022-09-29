var Constructor = function()
{
    this.init = function(co, map)
    {
        co.setPowerStars(4);
        co.setSuperpowerStars(3);
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
            animation.setSound("power0.wav", 1, delay);
            if (animations.length < 5)
            {
                animation.addSprite("power0", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power0", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
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

    this.activateSuperpower = function(co, powerMode, map)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(powerMode);
        powerNameAnimation.queueAnimationBefore(dialogAnimation);

        var units = co.getOwner().getUnits();
        var animations = [];
        var counter = 0;
        units.randomize();
        for (var i = 0; i < units.size(); i++)
        {
            var unit = units.at(i);
            var animation = GameAnimationFactory.createAnimation(map, unit.getX(), unit.getY());
            var delay = globals.randInt(135, 265);
            if (animations.length < 7)
            {
                delay *= i;
            }
            if (i % 2 === 0)
            {
                animation.setSound("power12_1.wav", 1, delay);
            }
            else
            {
                animation.setSound("power12_2.wav", 1, delay);
            }
            if (animations.length < 7)
            {
                animation.addSprite("power12", -map.getImageSize() * 2, -map.getImageSize() * 2, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power12", -map.getImageSize() * 2, -map.getImageSize() * 2, 0, 2, delay);
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

    this.loadCOMusic = function(co, map)
    {
        // put the co music in here.
        switch (co.getPowerMode())
        {
            case GameEnums.PowerMode_Power:
                audio.addMusic("resources/music/cos/bh_power.mp3", 1091 , 49930);
                break;
            case GameEnums.PowerMode_Superpower:
                audio.addMusic("resources/music/cos/bh_superpower.mp3", 3161 , 37731);
                break;
            case GameEnums.PowerMode_Tagpower:
                audio.addMusic("resources/music/cos/bh_tagpower.mp3", 779 , 51141);
                break;
            default:
                audio.addMusic("resources/music/cos/joey.mp3")
                break;
        }
    };

    this.getCOUnitRange = function(co, map)
    {
        return 3;
    };
    this.getCOArmy = function()
    {
        return "TI";
    };
    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
        if(defender !== null)
        {
            var attackerValue = attacker.getUnitValue();
            var defenderValue = defender.getUnitValue();
            switch (co.getPowerMode())
            {
                case GameEnums.PowerMode_Tagpower:
                case GameEnums.PowerMode_Superpower:
                    if (attackerValue > defenderValue)
                    {
                        return 0;
                    }
                    else if (attackerValue < defenderValue)
                    {
                        return 60;
                    }
                    else
                    {
                        return 10;
                    }
                case GameEnums.PowerMode_Power:
                    if (attackerValue > defenderValue)
                    {
                        return 0;
                    }
                    else if (attackerValue < defenderValue)
                    {
                        return 60;
                    }
                    else
                    {
                        return 10;
                    }
                default:
                    if (attackerValue > defenderValue)
                    {
                        if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                        {
                            return 0;
                        }
                        return -10;
                    }
                    else if (attackerValue < defenderValue)
                    {
                        if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                        {
                            return 60;
                        }
                        return 10;
                    }
                    else
                    {
                        if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                        {
                            return 10;
                        }
                        return 0;
                    }
                }
        }
        return 0;
    };

    this.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isAttacker, action, luckmode, map)
    {
        if(attacker !== null)
        {
            var attackerValue = attacker.getUnitValue();
            var defenderValue = defender.getUnitValue();
            switch (co.getPowerMode())
            {
                case GameEnums.PowerMode_Tagpower:
                case GameEnums.PowerMode_Superpower:
                    return 10;
                case GameEnums.PowerMode_Power:
                    if (attackerValue > defenderValue)
                    {
                        return 30;
                    }
                    return 10;
                default:
            }
        }
        return 0;
    };

    this.getFirstStrike = function(co, unit, posX, posY, attacker, isDefender, map, atkPosX, atkPosY)
    {
        if(unit !== null &&
           isDefender)
        {
            var defenderValue = unit.getUnitValue();
            var attackerValue = attacker.getUnitValue();
            switch (co.getPowerMode())
            {
                case GameEnums.PowerMode_Tagpower:
                case GameEnums.PowerMode_Superpower:
                    if (attackerValue > defenderValue)
                    {
                        return true;
                    }
                    return false;
                case GameEnums.PowerMode_Power:
                    return false;
                default:
                    return false;
            }
        }
        return false;
    };

    this.getCostModifier = function(co, id, baseCost, posX, posY, map)
    {
        switch (co.getPowerMode())
        {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                return -baseCost * 0.2;
            case GameEnums.PowerMode_Power:
                return 0;
            default:
                return 0;
        }
    };

    this.getAiCoUnitBonus = function(co, unit, map)
    {
        return 1;
    };
    // CO - Intel
    this.getBio = function(co)
    {
        return qsTr("Loves to live and fight on the edge. He prefers risks and gambles over safety.");
    };
    this.getHits = function(co)
    {
        return qsTr("Disadvantages");
    };
    this.getMiss = function(co)
    {
        return qsTr("Cheaters");
    };
    this.getCODescription = function(co)
    {
        return qsTr("Joey likes to live on the edge. Units are stronger when engaging stronger units, but firepower is reduced when engaging a weaker unit.");
    };
    this.getLongCODescription = function()
    {
        return qsTr("\nGlobal Effect: \nUnits are stronger when engaging stronger units, but firepower is reduced by when engaging a weaker unit.") +
               qsTr("\n\nCO Zone Effect: \nUnits are way stronger when engaging stronger units");
    };
    this.getPowerDescription = function(co)
    {
        return qsTr("Units receive a large firepower boost when engaging a stronger unit.");
    };
    this.getPowerName = function(co)
    {
        return qsTr("Eccentricity");
    };
    this.getSuperPowerDescription = function(co)
    {
        return qsTr("Deployment costs drop and units get first strike when engaging a stronger unit.");
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("Tempestous Technique");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("I feel a bit dirty for doing this..."),
                qsTr("I gave you a chance, but you didn't want it."),
                qsTr("You gotta live life on the edge!"),
                qsTr("You were never too good."),
                qsTr("You're going to be feeling edgy about this..."),
                qsTr("C'mon, give the little guys a chance!"),
                qsTr("What's a little risk taking going to hurt?")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("Heh, good game."),
                qsTr("Power isn't everything y'know..."),
                qsTr("Big risk, big reward!")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("This was to risky..."),
                qsTr("I thought my risk was calculated but man i'm bad at math!")];
    };
    this.getName = function()
    {
        return qsTr("Joey");
    };
}

Constructor.prototype = CO;
var CO_JOEY = new Constructor();
