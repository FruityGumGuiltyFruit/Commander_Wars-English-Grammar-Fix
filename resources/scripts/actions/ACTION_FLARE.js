var Constructor = function()
{
    this.canBePerformed = function(action, map)
    {
        var unit = action.getTargetUnit();
        var actionTargetField = action.getActionTarget();
        var targetField = action.getTarget();
        if (unit.getHasMoved() === false &&
           (actionTargetField.x === targetField.x) &&
           (actionTargetField.y === targetField.y))
        {
            if (unit.hasAmmo1())
            {
                return true;
            }
        }
        return false;
    };
    this.getActionText = function(map)
    {
        return qsTr("Flare");
    };
    this.getIcon = function(map)
    {
        return "flareshot";
    };
    this.isFinalStep = function(action, map)
    {
        if (action.getInputStep() === 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    };
    this.getStepInputType = function(action, map)
    {
        return "FIELD";
    };

    this.getMinRange = function()
    {
        return 1;
    }

    this.getMaxRange = function()
    {
        return 5;
    }
    this.getUnfogRange = function()
    {
        return 2;
    }

    this.getStepData = function(action, data, map)
    {
        var unit = action.getTargetUnit();
        var targetField = action.getTarget();
        var actionTargetField = action.getActionTarget();
        var fields = globals.getCircle(ACTION_FLARE.getMinRange(),
                                       ACTION_FLARE.getMaxRange());
        data.setColor("#C8FF0000");
        data.setZLabelColor("#ff4500");
        data.setShowZData(false);
        // check all fields we can attack
        var size = fields.size();
        for (var i = 0; i < size; i++)
        {
            var x = fields.at(i).x + actionTargetField.x;
            var y = fields.at(i).y + actionTargetField.y;
            if (map.onMap(x, y))
            {
                data.addPoint(Qt.point(x, y));
            }
        }
    };
    this.postAnimationTargetX = -1;
    this.postAnimationTargetY = -1;
    this.postAnimationPlayer = null;
    this.perform = function(action, map)
    {
        // we need to move the unit to the target position
        var unit = action.getTargetUnit();
        var animation = Global[unit.getUnitID()].doWalkingAnimation(action, map);
        animation.setEndOfAnimationCall("ACTION_FLARE", "performPostAnimation");
        // move unit to target position
        unit.moveUnitAction(action);
        // disable unit commandments for this turn
        unit.setHasMoved(true);
        unit.reduceAmmo1(1);
        action.startReading();
        // read action data
        ACTION_FLARE.postAnimationTargetX = action.readDataInt32();
        ACTION_FLARE.postAnimationTargetY = action.readDataInt32();
        ACTION_FLARE.postAnimationPlayer = unit.getOwner();
    };
    this.performPostAnimation = function(postAnimation, map)
    {
        var animation = GameAnimationFactory.createAnimation(map, ACTION_FLARE.postAnimationTargetX, ACTION_FLARE.postAnimationTargetY);
        animation.addSprite("flare_explosion", -map.getImageSize() * 2.5, -map.getImageSize() * 2.5 - 1, 0, 2);
        animation.addSound("flare_launch.wav");
        animation.addSound("flare_explosion.wav", 1, 300);
        var fields = globals.getCircle(0, ACTION_FLARE.getUnfogRange());
        var size = fields.size();
        for (var i = 0; i < size; i++)
        {
            var x = fields.at(i).x + ACTION_FLARE.postAnimationTargetX;
            var y = fields.at(i).y + ACTION_FLARE.postAnimationTargetY;
            if (map.onMap(x, y))
            {
                ACTION_FLARE.postAnimationPlayer.addVisionField(x, y, 1, true);
            }
        }
        ACTION_FLARE.postAnimationTargetX = -1;
        ACTION_FLARE.postAnimationTargetY = -1;
        ACTION_FLARE.postAnimationPlayer = null;
    };
    this.getStepCursor = function(action, cursorData, map)
    {
        cursorData.setCursor("cursor+missile");
        cursorData.setXOffset(- map.getImageSize() * 2);
        cursorData.setYOffset(- map.getImageSize() * 2);
        cursorData.setScale(2);
    };
    this.getName = function()
    {
        return qsTr("Flare");
    };
    this.getDescription = function()
    {
        return qsTr("Fires a flare rocket at the given location. The flare rocket will reveal all terrains in an area around the target during fog of war.");
    };
}

Constructor.prototype = ACTION;
var ACTION_FLARE = new Constructor();
