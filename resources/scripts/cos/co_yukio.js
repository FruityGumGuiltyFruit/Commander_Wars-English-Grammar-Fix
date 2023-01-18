var Constructor = function()
{
    this.init = function(co, map)
    {
        co.setPowerStars(8);
        co.setSuperpowerStars(7);
    };

    this.getCOStyles = function()
    {
        // string array containing the endings of the alternate co style
        
        return ["+alt"];
    };

    this.activatePower = function(co, map)
    {
        var invasion = ["ARTILLERY", "FLAK", "LIGHT_TANK", "FLAK", "LIGHT_TANK"];
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(GameEnums.PowerMode_Power);
        dialogAnimation.queueAnimation(powerNameAnimation);
        CO_YUKIO.spawnUnits(co, 0.4, invasion, powerNameAnimation, map);
    };

    this.activateSuperpower = function(co, powerMode, map)
    {
        var invsion = ["HEAVY_TANK", "FLAK", "LIGHT_TANK", "ARTILLERY", "LIGHT_TANK", "K_HELI", "K_HELI"];
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(powerMode);
        powerNameAnimation.queueAnimationBefore(dialogAnimation);

        CO_YUKIO.spawnUnits(co, 0.7, invsion, powerNameAnimation, map);
        CO_YUKIO.yukioDamage(co, 3, powerNameAnimation, map);
    };


    this.yukioDamage = function(co, value, powerNameAnimation, map)
    {
        var player = co.getOwner();
        var animations = [];
        var counter = 0;
        var playerCounter = map.getPlayerCount();
        for (var i2 = 0; i2 < playerCounter; i2++)
        {
            var enemyPlayer = map.getPlayer(i2);
            if ((enemyPlayer !== player) &&
                (player.checkAlliance(enemyPlayer) === GameEnums.Alliance_Enemy))
            {

                var units = enemyPlayer.getUnits();
                units.randomize();
                var size = units.size() / 2;
                for (i = 0; i < size; i++)
                {
                    var unit = units.at(i);
                    var animation = GameAnimationFactory.createAnimation(map, unit.getX(), unit.getY());
                    animation.writeDataInt32(unit.getX());
                    animation.writeDataInt32(unit.getY());
                    animation.writeDataInt32(value);
                    animation.setStartOfAnimationCall("ANIMATION", "postAnimationDamage");
                    var delay = globals.randInt(135, 265);
                    if (animations.length < 5)
                    {
                        delay *= i;
                    }
                    animation.setSound("power4.wav", 1, delay);
                    if (animations.length < 5)
                    {
                        animation.addSprite("power4", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                        powerNameAnimation.queueAnimation(animation);
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
                }
            }
        }
    };

    this.spawnUnits = function(co, count, invasion, powerNameAnimation, map)
    {
        var buildings = co.getOwner().getBuildings();
        var animations = [];
        var counter = 0;
        buildings.randomize();
        var size = buildings.size();
        for (var i = 0; i < size * count; i++)
        {
            var building = buildings.at(i);
            if (building.getBuildingID() === "TOWN")
            {
                if (map.getTerrain(building.getX(), building.getY()).getUnit() === null)
                {
                    var animation = GameAnimationFactory.createAnimation(map, building.getX(), building.getY());
                    animation.writeDataInt32(building.getX());
                    animation.writeDataInt32(building.getY());
                    animation.writeDataString(invasion[i % invasion.length]);
                    animation.writeDataInt32(co.getOwner().getPlayerID());
                    animation.writeDataInt32(10);
                    animation.setStartOfAnimationCall("ANIMATION", "postAnimationSpawnUnit");

                    var delay = globals.randInt(135, 265);
                    if (animations.length < 5)
                    {
                        delay *= i;
                    }
                    if (i % 2 === 0)
                    {
                        animation.setSound("power8_1.wav", 1, delay);
                    }
                    else
                    {
                        animation.setSound("power8_2.wav", 1, delay);
                    }
                    if (animations.length < 5)
                    {
                        animation.addSprite("power8", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                        powerNameAnimation.queueAnimation(animation);
                        animations.push(animation);
                    }
                    else
                    {
                        animation.addSprite("power8", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
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
                audio.addMusic("resources/music/cos/yukio.mp3", 100, 58286);
                break;
        }
    };

    this.getCOUnitRange = function(co, map)
    {
        return 4;
    };
    this.getCOArmy = function()
    {
        return "DM";
    };
    this.mintrueDamage = 10;
    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
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
    this.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isAttacker, action, luckmode, map)
    {
        if (co.inCORange(Qt.point(defPosX, defPosY), defender) ||
            co.getPowerMode() > GameEnums.PowerMode_Off)
        {
            return 10;
        }
        return 0;
    };

    this.getTrueDamage = function(co, damage, attacker, atkPosX, atkPosY, attackerBaseHp,
                                  defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
        // reduce counter damage by a flat amount here
        switch (co.getPowerMode())
        {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
            case GameEnums.PowerMode_Power:
                if (damage >= CO_YUKIO.mintrueDamage)
                {
                    return 15
                }
                break;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    if (damage >= CO_YUKIO.mintrueDamage)
                    {
                        return 15
                    }
                }
                break;
        }
        return 0;
    };

    this.getDamageReduction = function(co, damage, attacker, atkPosX, atkPosY, attackerBaseHp,
                                  defender, defPosX, defPosY, isDefender, luckMode, map)
    {
        // reduce counter damage by a flat amount here
        switch (co.getPowerMode())
        {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
            case GameEnums.PowerMode_Power:
                return 15;
            default:
                if (co.inCORange(Qt.point(defPosX, defPosY), defender))
                {
                    return 10;
                }
                break;
        }
        return 0;
    };
    this.getAiCoUnitBonus = function(co, unit, map)
    {
        return 1;
    };
    this.getCOUnits = function(co, building, map)
    {
        var buildingId = building.getBuildingID();
        if (buildingId === "FACTORY" ||
            buildingId === "TOWN" ||
            buildingId === "HQ")
        {
            return ["ZCOUNIT_LOGIC_TRUCK"];
        }
        return [];
    };
    // CO - Intel
    this.getBio = function(co)
    {
        return qsTr("Dark founder of the Dark Matter Cooperation. He's the primary reason for invasions of Sturm and Von Bolt. He uses war as a way of making money and doesn't care if people die.");
    };
    this.getHits = function(co)
    {
        return qsTr("Astronomic gains");
    };
    this.getMiss = function(co)
    {
        return qsTr("Any losses");
    };
    this.getCODescription = function(co)
    {
        return qsTr("As the leader of dark matter his troops have special armor and armor piercing weapons.");
    };
    this.getLongCODescription = function()
    {
        var text = qsTr("\nSpecial Unit:\nLogistic Truck\n") +
                   qsTr("\nGlobal Effect: \nNone.") +
                   qsTr("\n\nCO Zone Effect: \nDamage against his troops is reduced. Troops deal true damage if the base damage is at least %0%.");
        text = replaceTextArgs(text, [CO_YUKIO.mintrueDamage]);
        return text;
    };
    this.getPowerDescription = function(co)
    {
        return qsTr("A small army spawns and fights for Yukio.");
    };
    this.getPowerName = function(co)
    {
        return qsTr("Invasion");
    };
    this.getSuperPowerDescription = function(co)
    {
        return qsTr("An army spawns and fights for Yukio. In order to support the invasion a bombardment dealing 3 Hp to half of the enemy troops is launched.");
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("Ground-Air-Invasion");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("I show you, what true power means."),
                qsTr("I'm the evil that destroys you."),
                qsTr("Your weapons are the rubbish of my factories."),
                qsTr("The victory is mine!  Alea iacta est!"),
                qsTr("My troops are behind you."),
                qsTr("I'll become the ruler of the world.")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("Investment...Profits...great. Continue."),
                qsTr("War is just another tool to make profit."),
                qsTr("Give me all your money and your lifes.")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("Even with this loss I made profit..."),
                qsTr("I lost but I got your money to build a new army.")];
    };
    this.getName = function()
    {
        return qsTr("Yukio");
    };
}

Constructor.prototype = CO;
var CO_YUKIO = new Constructor();
