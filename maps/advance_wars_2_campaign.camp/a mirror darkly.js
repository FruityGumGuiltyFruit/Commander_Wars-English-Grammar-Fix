var Constructor = function()
{
    this.immediateStart = function()
    {
        // called to check if the game should start immediately without changing rules or modifying co's
        // return true for an immediate start
        return true;
    };

    this.getVictoryInfo = function()
    {
        return qsTr("You loose if a unit reaches your HQ.");
    };

    this.victory = function(team, map)
    {
        if (team === 0)
        {
            // called when a player wins
            var dialog1 = GameAnimationFactory.createGameAnimationDialog(map, 
                        qsTr(" I think that answers that question. "),
                        "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
            var dialog2 = GameAnimationFactory.createGameAnimationDialog(map, 
                        qsTr("Aaaah... I lost. Oh well, I still have plenty of pieces to play with. "),
                        "co_lash", GameEnums.COMood_Sad, PLAYER.getDefaultColor(4));
            var dialog3 = GameAnimationFactory.createGameAnimationDialog(map, 
                        qsTr(" P-Pieces?! I can't believe you said that! This isn't a game! You're not on some kind of playground! "),
                        "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));            
            var dialog4 = GameAnimationFactory.createGameAnimationDialog(map, 
                        qsTr("Oh, but it IS a game. A wonderfully fun game between you and me! I know you feel the same way. Come on, you can tell me. "),
                        "co_lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
            var dialog5 = GameAnimationFactory.createGameAnimationDialog(map, 
                        qsTr("Don't... Don't be silly... I'm... nothing like you, Lash."),
                        "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
            var dialog6 = GameAnimationFactory.createGameAnimationDialog(map, 
                        qsTr("You're exactly like me. You and I. We're the same. You're always thinking about it, right? Which move will bring victory? You enjoy war like a good game of chess. "),
                        "co_Lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
            var dialog7 = GameAnimationFactory.createGameAnimationDialog(map, 
                        qsTr("......"),
                        "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
            var dialog8 = GameAnimationFactory.createGameAnimationDialog(map, 
                        qsTr("We're not different at all, Sonja. You'll feel it someday, the joy that war brings. Wait... You feel it now, don't you? Oh, Sonja! Tee hee hee! Toodles! "),
                        "co_Lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
            var dialog9 = GameAnimationFactory.createGameAnimationDialog(map, 
                        qsTr("Wait! Hold it! Am I really... like you? I won't become you, Lash. I can't... "),
                        "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
            dialog1.queueAnimation(dialog2);
            dialog2.queueAnimation(dialog3);
            dialog3.queueAnimation(dialog4);
            dialog4.queueAnimation(dialog5);
            dialog5.queueAnimation(dialog6);
            dialog6.queueAnimation(dialog7);
            dialog7.queueAnimation(dialog8);
            dialog8.queueAnimation(dialog9);
        }
    };
    this.gameStart = function(map)
    {
        // called before a game starts
        //we're going to set the game rules here.
        map.getGameRules().setNoPower(false); // co power on
        map.getGameRules().setRandomWeather(false); // no random weather
        map.getGameRules().setFogMode(GameEnums.Fog_OfWar); // no fog of war or GameEnums.Fog_OfWar -> for on
        // here we decide how you can win the game
        map.getGameRules().addVictoryRule("VICTORYRULE_NOUNITS"); // win by destroying all units
        map.getGameRules().addVictoryRule("VICTORYRULE_NOHQ"); // win by capturing all hq's of a player
        var ycList = campaignScript.getYCBuildList();
        map.getPlayer(0).setBuildList(ycList);
        var bhList = campaignScript.getBHBuildList();
        map.getPlayer(1).setBuildList(bhList);
    };
    this.actionDone = function(action, map)
    {
        var unit = map.getTerrain(12, 1).getUnit();
        if (unit !== null && unit.getOwner().getPlayerID() === 1)
        {
            map.getPlayer(0).setIsDefeated(true);
        }
    };

    this.turnStart = function(turn, player, map)
    {
        if (turn === 1 && player === 0)
        {
            gameScript.initDialog(map);
        }
    };

    this.initDialog = function(map)
    {
        // moods are GameEnums.COMood_Normal, GameEnums.COMood_Happy, GameEnums.COMood_Sad
        var dialog1 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Hm? Where is my foe? What is this? There is no enemy in sight. Sonja, what's the meaning of this? "),
                    "co_kanbei", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog2 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("You're right, Father. There's no one here... This is an important piece of the puzzle, though. The enemy must be targeting this area. "),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog3 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("If they're here, Sonja, show me where they're hiding. If there is no enemy, there can be no battle. We must move ahead to the next battlefield. "),
                    "co_sensei", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog4 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("I suppose you're right, and yet, something seems strange... "),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog5 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr(" I've no time for your stubbornness! Come, Sonja! We must go! "),
                    "co_kanbei", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog6 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Hmmm... Yes, Father. I understand. "),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog7 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Just as I thought. Not a soul in sight. Does no one in Yellow Comet have a brain? Oh well, I guess I'll just march right in and capture their HQ. "),
                    "co_lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
        var dialog8 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Took your time getting here, didn't you? "),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog9 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Who? Sonja!? You're not supposed to be here! I was told your army had moved on! "),
                    "co_lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
        var dialog10 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Sorry to disappoint you. That intel came from me, actually. It was false. It took some doing to get my father and Sensei to agree to this plan, but... I was beginning to wonder if you would take the bait. "),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog11 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Wait a minute... Are you saying that YOU trapped ME? "),
                    "co_lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
        var dialog12 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Surrender. There are rockets on both sides of you, and artillery waits ahead. If you try to get to our HQ, you'll be wiped out. "),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog13 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("I wonder... "),
                    "co_lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
        var dialog14 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Huh? "),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog15 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("It's not like you're a combat specialist or anything, right? I really wonder if you can take me. "),
                    "co_lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
        var dialog16 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("......"),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog17 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr(" I have a pretty big army here. If I keep marching, I think I'll take your HQ and win. Don't you agree? "),
                    "co_lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
        var dialog18 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr(" Impossible. My calculations are not wrong. Your army will be destroyed before you even touch our HQ."),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog19 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("I think I can make it. Care to bet on it? "),
                    "co_lash", GameEnums.COMood_Normal, PLAYER.getDefaultColor(4));
        var dialog20 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("If you really want to try, who's going to stop you? Oh, that's right. I am. How's this? If even one of your units gets to our HQ, you win. Let's determine which of us has the better grasp on reality. "),
                    "co_sonja", GameEnums.COMood_Normal, PLAYER.getDefaultColor(3));
        var dialog21 = GameAnimationFactory.createGameAnimationDialog(map, 
                    qsTr("Yay! A game! A game! Tee hee hee! "),
                    "co_lash", GameEnums.COMood_Happy, PLAYER.getDefaultColor(4));
        dialog1.queueAnimation(dialog2);
        dialog2.queueAnimation(dialog3);
        dialog3.queueAnimation(dialog4);
        dialog4.queueAnimation(dialog5);
        dialog5.queueAnimation(dialog6);
        dialog6.queueAnimation(dialog7);
        dialog7.queueAnimation(dialog8);
        dialog8.queueAnimation(dialog9);
        dialog9.queueAnimation(dialog10);
        dialog10.queueAnimation(dialog11);
        dialog11.queueAnimation(dialog12);
        dialog12.queueAnimation(dialog13);
        dialog13.queueAnimation(dialog14);
        dialog14.queueAnimation(dialog15);
        dialog15.queueAnimation(dialog16);
        dialog16.queueAnimation(dialog17);
        dialog17.queueAnimation(dialog18);
        dialog18.queueAnimation(dialog19);
        dialog19.queueAnimation(dialog20);
        dialog20.queueAnimation(dialog21);
    };
};

Constructor.prototype = BASEGAMESCRIPT;
var gameScript = new Constructor();

