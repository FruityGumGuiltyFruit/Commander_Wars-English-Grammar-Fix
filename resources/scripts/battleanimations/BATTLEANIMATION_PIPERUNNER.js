var Constructor = function()
{
    this.getMaxUnitCount = function()
    {
        return 1;
    };

    this.loadStandingAnimation = function(sprite, unit, defender, weapon)
    {
        sprite.loadSpriteV2("piperunner+mask", GameEnums.Recoloring_Matrix,
                            BATTLEANIMATION_PIPERUNNER.getMaxUnitCount(), Qt.point(0, 30));
        BATTLEANIMATION.loadSpotterOrCoMini(sprite, unit, false);
    };

    this.loadFireAnimation = function(sprite, unit, defender, weapon)
    {
        var count = sprite.getUnitCount(5);
        sprite.loadSpriteV2("piperunner+fire+mask", GameEnums.Recoloring_Matrix,
                            BATTLEANIMATION_PIPERUNNER.getMaxUnitCount(), Qt.point(0, 30),
                            count);
        for (var i = 0; i < count; i++)
        {
            sprite.loadSingleMovingSprite("pipe_rocket_up", false, Qt.point(50, 82),
                                          Qt.point(128, 64), 400, false,
                                          -1, 1, -1, i * 300);
            sprite.loadSingleMovingSprite("pipe_shot", false, Qt.point(69, 78),
                                          Qt.point(0, 0), 0, false,
                                          1, 1, 5, i * 300);
            sprite.loadSound("pipecannon_weapon_fire.wav", 1, i * 300);
        }
        BATTLEANIMATION.loadSpotterOrCoMini(sprite, unit, false);
    };

    this.getFireDurationMS = function(sprite, unit, defender, weapon)
    {
        return 500 + 300 * sprite.getUnitCount(5);
    };

    this.loadImpactUnitOverlayAnimation = function(sprite, unit, defender, weapon)
    {
        sprite.loadColorOverlayForLastLoadedFrame("#969696", 1000, 1, 300);
    };

    this.loadImpactAnimation = function(sprite, unit, defender, weapon)
    {
        var count = sprite.getUnitCount(5);
        if (defender.getUnitType() === GameEnums.UnitType_Air)
        {
            sprite.loadMovingSprite("rocket_hit_air", false, sprite.getMaxUnitCount(), Qt.point(0, 20),
                                    Qt.point(-30, 15), 400, false,
                                    1, 1.0, 0, 300, true);
            sprite.loadMovingSprite("pipe_rocket_up", false, 5, Qt.point(127, 0),
                                    Qt.point(-128, 64), 400, true,
                                    -1, 1, 0, 0, true);
        }
        else
        {
            sprite.loadSprite("rocket_hit",  false, 5, Qt.point(0, 20),
                              1, 1.0, 0, 400, true);
            sprite.loadMovingSprite("pipe_rocket_down", false, 5, Qt.point(127, 80),
                                    Qt.point(-128, -64), 400, true,
                                    -1, 1, 0, 0, true);
        }
        sprite.addSpriteScreenshake(8, 0.95, 800, 500);
        for (var i = 0; i < count; i++)
        {
            sprite.loadSound("rocket_flying.wav", 1, 0);
            sprite.loadSound("impact_explosion.wav", 1, 200 + i * BATTLEANIMATION.defaultFrameDelay);
        }
    };

    this.getImpactDurationMS = function(sprite, unit, defender, weapon)
    {
        if (defender.getUnitType() === GameEnums.UnitType_Air)
        {
            return 400 - BATTLEANIMATION.defaultFrameDelay + BATTLEANIMATION.defaultFrameDelay * sprite.getUnitCount(5);
        }
        else
        {
            return 400 - BATTLEANIMATION.defaultFrameDelay + BATTLEANIMATION.defaultFrameDelay * sprite.getUnitCount(5);
        }
    };
};

Constructor.prototype = BATTLEANIMATION;
var BATTLEANIMATION_PIPERUNNER = new Constructor();
