var Constructor = function()
{
    this.getMaxUnitCount = function()
    {
        return 5;
    };
    this.armyData = [["ac", "ac"],
                     ["bd", "bd"],
                     ["bh", "bh"],
                     ["bg", "bh"],
                     ["bm", "bm"],
                     ["dm", "dm"],
                     ["ge", "ge"],
                     ["gs", "gs"],
                     ["ma", "ma"],
                     ["os", "os"],
                     ["pf", "pf"],
                     ["ti", "ti"],
                     ["yc", "yc"],];

    this.animationData = [["ac", [Qt.point(-5, 5),  Qt.point(43, 36)]],
                          ["bd", [Qt.point(-5, 5),  Qt.point(41, 36)]],
                          ["bh", [Qt.point(-5, 5),  Qt.point(39, 38)]],
                          ["bm", [Qt.point(-5, 5),  Qt.point(39, 36)]],
                          ["dm", [Qt.point(-25, 5), Qt.point(28, 40)]],
                          ["ge", [Qt.point(-5, 5),  Qt.point(40, 37)]],
                          ["gs", [Qt.point(-10, 5), Qt.point(26, 28)]],
                          ["ma", [Qt.point(-35, 5), Qt.point(13, 43)]],
                          ["os", [Qt.point(-5, 5),  Qt.point(40, 38)]],
                          ["pf", [Qt.point(-5, 5),  Qt.point(41, 39)]],
                          ["ti", [Qt.point(-15, 5), Qt.point(34, 29)]],
                          ["yc", [Qt.point(-5, 5),  Qt.point(40, 38)]],];

    this.loadStandingAnimation = function(sprite, unit, defender, weapon)
    {
        BATTLEANIMATION_ARTILLERY.loadSprite(sprite, unit, defender, weapon, "", 0, 0);
    };

    this.loadStandingFiredAnimation = function(sprite, unit, defender, weapon)
    {
        BATTLEANIMATION_ARTILLERY.loadSprite(sprite, unit, defender, weapon, "", 1, 1);
    };

    this.loadSprite = function(sprite, unit, defender, weapon, ending, startFrame = 0, endFrame = -1)
    {
        var player = unit.getOwner();
        // get army name
        var armyName = Global.getArmyNameFromPlayerTable(player, BATTLEANIMATION_ARTILLERY.armyData);
        var data = Global.getDataFromTable(armyName, BATTLEANIMATION_ARTILLERY.animationData);
        var offset = data[0];

        sprite.loadSpriteV2("artillery+" + armyName + ending + "+mask", GameEnums.Recoloring_Matrix,
                            BATTLEANIMATION_ARTILLERY.getMaxUnitCount(), offset, 1, 1, 0, 0, false, false, 100, endFrame, startFrame);
        BATTLEANIMATION.loadSpotterOrCoMini(sprite, unit, true);
    };


    this.loadFireAnimation = function(sprite, unit, defender, weapon)
    {
        // get army name
        var player = unit.getOwner();
        var armyName = Global.getArmyNameFromPlayerTable(player, BATTLEANIMATION_ARTILLERY.armyData);
        var count = sprite.getUnitCount(BATTLEANIMATION_ARTILLERY.getMaxUnitCount());
        BATTLEANIMATION_ARTILLERY.loadSprite(sprite, unit, defender, weapon, "+fire");
        var data = Global.getDataFromTable(armyName, BATTLEANIMATION_ARTILLERY.animationData);
        var offset = data[1];
        sprite.loadSprite("artillery_shot", false, sprite.getMaxUnitCount(), offset,
                          1, 1.0, 0, 500);
        for (var i = 0; i < count; i++)
        {
            sprite.loadSound("artillery_weapon_load.wav", 1, i * BATTLEANIMATION.defaultFrameDelay);
            sprite.loadSound("cannon_weapon_fire.wav", 1, 500 + i * BATTLEANIMATION.defaultFrameDelay);
        }
    };

    this.getFireDurationMS = function(sprite, unit, defender, weapon)
    {
        return 1000 + BATTLEANIMATION.defaultFrameDelay * sprite.getUnitCount(BATTLEANIMATION_ARTILLERY.getMaxUnitCount());
    };

    this.loadImpactAnimation = function(sprite, unit, defender, weapon)
    {
        var count = sprite.getUnitCount(BATTLEANIMATION_ARTILLERY.getMaxUnitCount());
        var i = 0;
        sprite.loadSprite("bullet_artillery",  false, sprite.getMaxUnitCount(), Qt.point(0, 20),
                          1, 1.0, 0, 0, true, true, 50);
        sprite.loadSprite("artillery_hit", false, sprite.getMaxUnitCount(), Qt.point(0, 20),
                          1, 1.0, 0, 100, true);
        sprite.addSpriteScreenshake(8, 0.98, 800, 300);
        for (i = 0; i < count; i++)
        {
            sprite.loadSound("artillery_explode.wav", 1, 100 + i * BATTLEANIMATION.defaultFrameDelay);
        }
    };

    this.getImpactDurationMS = function(sprite, unit, defender, weapon)
    {
        return 500 - BATTLEANIMATION.defaultFrameDelay + BATTLEANIMATION.defaultFrameDelay * sprite.getUnitCount(BATTLEANIMATION_ARTILLERY.getMaxUnitCount());
    };
};

Constructor.prototype = BATTLEANIMATION;
var BATTLEANIMATION_ARTILLERY = new Constructor();
