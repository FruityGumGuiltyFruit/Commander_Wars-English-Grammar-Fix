var Constructor = function()
{
    this.getMaxUnitCount = function()
    {
        return 1;
    };

    this.loadMoveInAnimation = function(sprite, unit, defender, weapon)
    {
        sprite.setBackgroundSpeed(sprite.getBackgroundSpeed() + 1);
        BATTLEANIMATION_BATTLECRUISER.loadSprite(sprite, unit, defender, weapon, Qt.point(-116, 0), 1200, 0, 0, 0, 0,
                                             "+cannon", "+flak", Qt.point(20, 20));
    };

    this.getStopDurationMS = function(sprite, unit, defender, weapon)
    {
        return 0;
    };

    this.loadStandingAnimation = function(sprite, unit, defender, weapon)
    {
        BATTLEANIMATION_BATTLECRUISER.loadSprite(sprite, unit, defender, weapon);
    };

    this.loadSprite = function(sprite, unit, defender, weapon, movement = Qt.point(0, 0), moveTime = 0, mgStartFrame = 0, mgEndFrame = 0,
                               rocketStartFrame = 0, rocketEndFrame = 0,
                               rocketSprite = "+cannon", mgSprite = "+flak", startPos = Qt.point(-96, 20))
    {
        var player = unit.getOwner();
        // get army name
        sprite.loadMovingSprite("battlecruiser",  false,
                                BATTLEANIMATION_BATTLECRUISER.getMaxUnitCount(), startPos, movement, moveTime, false, -1);
        sprite.loadMovingSpriteV2("battlecruiser+mask", GameEnums.Recoloring_Matrix,
                                  BATTLEANIMATION_BATTLECRUISER.getMaxUnitCount(), startPos, movement, moveTime, false, -1);
        sprite.loadMovingSpriteV2("battlecruiser" + rocketSprite + "+mask", GameEnums.Recoloring_Matrix,
                                  BATTLEANIMATION_BATTLECRUISER.getMaxUnitCount(), Qt.point(startPos.x + 160, startPos.y + 56),
                                  movement, moveTime, false, 1, 1, 0, 0,
                                  false, 200, rocketEndFrame, rocketStartFrame);
        sprite.loadMovingSpriteV2("battlecruiser" + mgSprite + "+mask", GameEnums.Recoloring_Matrix,
                                  BATTLEANIMATION_BATTLECRUISER.getMaxUnitCount(), Qt.point(startPos.x + 100, startPos.y + 64),
                                  movement, moveTime, false, 2, 1, 0, 0,
                                  false, 100, mgEndFrame, mgStartFrame);
    };

    this.loadFireAnimation = function(sprite, unit, defender, weapon)
    {
        sprite.restoreBackgroundSpeed();
        var offset = Qt.point(0, 0);
        if (weapon === 0)
        {
            var count = sprite.getUnitCount(5);
            BATTLEANIMATION_BATTLECRUISER.loadSprite(sprite, unit, defender, weapon, Qt.point(0, 0), 0, 0, 0,
                                                 0, count - 1, "+cannon+fire");
            offset = Qt.point(100, 100);
            for (var i = 0; i < count; i++)
            {
                var offset2 = Qt.point(0, 0);
                switch (i)
                {
                case 1:
                    offset2 = Qt.point(-6, -5);
                    break;
                case 2:
                    offset2 = Qt.point(-11, -5);
                    break;
                    //
                case 3:
                    offset2 = Qt.point(6, -32);
                    break;
                case 4:
                    offset2 = Qt.point(-12, -38);
                    break;
                }
                sprite.loadSingleMovingSprite("medium_shot", false,
                                              Qt.point(offset.x + offset2.x,
                                                       offset.y + offset2.y),
                                              Qt.point(0, 0), 0, true,
                                              1, 1.0, 5, 200 * i, false);
                sprite.loadSound("megacannon_weapon_fire.wav", 1, 200 * i);
            }
            sprite.addSpriteShakeY(4, 0.7, 0.4, 200, 0, count);
        }
        else
        {
            BATTLEANIMATION_BATTLECRUISER.loadSprite(sprite, unit, defender, weapon, Qt.point(0, 0), 0, 0, 2,
                                                 0, 0, "+cannon", "+flak+fire");
            offset = Qt.point(29, 109);
            for (var i2 = 0; i2 < 2; ++i2)
            {
                sprite.loadSprite("mg_shot_air", false, sprite.getMaxUnitCount(),
                                  Qt.point(offset.x - i2 * 17, offset.y - i2 * 11),
                                  2, 1, 0, 0, false, true);
            }
            sprite.loadSound("anti_air_gun_fire.wav", 1, 0);
            sprite.loadSound("anti_air_gun_fire.wav", 1, 100, 1, true);
            sprite.loadSound("anti_air_gun_fire.wav", 1, 200, 1, true);
            sprite.loadSound("anti_air_gun_fire.wav", 1, 300, 1, true);
            sprite.loadSound("anti_air_gun_fire.wav", 1, 400, 1, true);
            sprite.loadSound("anti_air_gun_fire.wav", 1, 500, 1, true);
        }
    };

    this.getFireDurationMS = function(sprite, unit, defender, weapon)
    {
        if (weapon === 0)
        {
            var count = sprite.getUnitCount(5);
            return 600 + 200 * count;
        }
        else
        {
            return 1100;
        }
    };

    this.getFiredDurationMS = function(sprite, unit, defender, weapon)
    {
        if (weapon === 0)
        {
            var count = sprite.getUnitCount(5);
            return 200 * count;
        }
        else
        {
            return -1;
        }
    };

    this.loadImpactAnimation = function(sprite, unit, defender, weapon)
    {
        var count = sprite.getUnitCount(5);
        var i = 0;
        if (weapon === 0)
        {
            if (defender.getUnitType()  === GameEnums.UnitType_Naval)
            {
                for (var i = 0; i < count; i++)
                {
                    sprite.loadSingleMovingSprite("water_hit",  false, Qt.point(68 - i * 25, 20),
                                                  Qt.point(0, 0), 0, false,
                                                  1, 1.0, 2, i * BATTLEANIMATION.defaultFrameDelay * 2, true,
                                                  100, -1, 0, 0, 180);
                }
            }
            else
            {
                sprite.loadSprite("artillery_heavy_hit",  false, 5, Qt.point(-16, 20),
                                  1, 1.0, 0, 0, true);
            }
            for (var i = 0; i < count; i++)
            {
                sprite.loadSound("impact_explosion.wav", 1, i * BATTLEANIMATION.defaultFrameDelay);
            }
            sprite.addSpriteScreenshake(8, 0.95, 800, 200);
        }
        else
        {
            var yOffset = 22;
            if (unit.getUnitType()  === GameEnums.UnitType_Air)
            {
                yOffset = 40
            }
            sprite.loadSprite("mg_hit",  false, 5, Qt.point(0, yOffset),
                              1, 1.0, 0, 0);
            BATTLEANIMATION.playMgImpactSound(sprite, unit, defender, weapon, count);
        }
    };

    this.getImpactDurationMS = function(sprite, unit, defender, weapon)
    {
        var count = sprite.getUnitCount(5);
        if (weapon === 0)
        {
            if (defender.getUnitType()  === GameEnums.UnitType_Naval)
            {
                return 500 - BATTLEANIMATION.defaultFrameDelay + BATTLEANIMATION.defaultFrameDelay * 2 * sprite.getUnitCount(5);
            }
            else
            {
                return 50 - BATTLEANIMATION.defaultFrameDelay + BATTLEANIMATION.defaultFrameDelay * sprite.getUnitCount(5);
            }
        }
        else
        {
            return 400 - BATTLEANIMATION.defaultFrameDelay + BATTLEANIMATION.defaultFrameDelay * count;
        }
    };

    this.hasMoveInAnimation = function(sprite, unit, defender, weapon)
    {
        return true;
    };
    this.getMoveInDurationMS = function(sprite, unit, defender, weapon)
    {
        return 1210;
    };

    this.getDyingDurationMS = function(sprite, unit, defender, weapon)
    {
        return 2200;
    };

    this.hasDyingAnimation = function()
    {
        return true;
    };

    this.loadDyingAnimation = function(sprite, unit, defender, weapon)
    {
        BATTLEANIMATION_BATTLECRUISER.loadSprite(sprite, unit, defender, weapon, Qt.point(-140, 0), 2000);
        sprite.loadSound("ship_dying_move.wav", -2);
    };
};

Constructor.prototype = BATTLEANIMATION;
var BATTLEANIMATION_BATTLECRUISER = new Constructor();
