var Constructor = function()
{
    this.canBePerformed = function(action, map)
    {
        var unit = action.getTargetUnit();
        var actionTargetField = action.getActionTarget();
        var targetField = action.getTarget();
        if (ACTION.isEmptyFieldAndHasNotMoved(action, unit, actionTargetField, targetField, map) &&
            unit.getAmmo2() > 0)
        {
            if (ACTION_PLACE_WATERMINE.getMineFields(action, map).length > 0)
            {
                return true;
            }
        }
        return false;
    };
    this.getMineFields = function(action, map)
    {
        var targetField = action.getActionTarget();
        var targetFields = [Qt.point(targetField.x + 1, targetField.y),
                            Qt.point(targetField.x - 1, targetField.y),
                            Qt.point(targetField.x,     targetField.y - 1),
                            Qt.point(targetField.x,     targetField.y + 1)];
        var unit = action.getTargetUnit();
        var targetTerrain = map.getTerrain(targetField.x, targetField.y);
        var ret = [];
        // check all neighbour terrains
        for (var i = 0; i < targetFields.length; i++)
        {
            if (map.onMap(targetFields[i].x, targetFields[i].y))
            {
                var terrain = map.getTerrain(targetFields[i].x, targetFields[i].y);
                var defUnit = terrain.getUnit();
                // can the transported unit move over the terrain?
                var terrainId = terrain.getID();
                if (terrainId !== "BRIDGE" &&
                    terrainId !== "BRIDGE1" &&
                    terrainId !== "BRIDGE2" &&
                    terrainId !== "BEACH")
                {
                    if ((Global[unit.getMovementType()].getMovementpoints(terrain, unit, terrain, false, map) > 0) &&
                        (defUnit === null))
                    {
                        ret.push(targetFields[i]);
                    }
                }
            }
        }
        return ret;
    };

    this.getActionText = function(map)
    {
        return qsTr("Place Mine");
    };
    this.getIcon = function(map)
    {
        return "WATERMINE";
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

    this.getStepData = function(action, data, map)
    {
        var unit = action.getTargetUnit();
        var actionTargetField = action.getActionTarget();
        data.setColor("#C800FF00");
        var fields = ACTION_PLACE_WATERMINE.getMineFields(action, map);
        for (var i3 = 0; i3 < fields.length; i3++)
        {
            data.addPoint(Qt.point(fields[i3].x, fields[i3].y));
        }
    };

    this.postAnimationMinePosX = -1;
    this.postAnimationMinePosY = -1;
    this.perform = function(action, map)
    {
        // we need to move the unit to the target position
        var unit = action.getTargetUnit();
        var animation = Global[unit.getUnitID()].doWalkingAnimation(action, map);
        animation.setEndOfAnimationCall("ACTION_PLACE_WATERMINE", "performPostAnimation");
        // move unit to target position
        unit.moveUnitAction(action);
        // disable unit commandments for this turn
        unit.setHasMoved(true);
        unit.reduceAmmo2(1);
        action.startReading();
        var x = action.readDataInt32();
        var y = action.readDataInt32();
        ACTION_PLACE_WATERMINE.postAnimationMinePosX = x;
        ACTION_PLACE_WATERMINE.postAnimationMinePosY = y;
        ACTION_PLACE_WATERMINE.postAnimationMineOwner = unit.getOwner();
    };
    this.performPostAnimation = function(postAnimation, map)
    {
        // unloading the units here :)
        var player = ACTION_PLACE_WATERMINE.postAnimationMineOwner;
        var unit = map.spawnUnit(ACTION_PLACE_WATERMINE.postAnimationMinePosX,
                                 ACTION_PLACE_WATERMINE.postAnimationMinePosY,
                                 "WATERMINE", player);
        if (unit !== null)
        {
            map.getGameRecorder().buildUnit(player.getPlayerID(), unit.getUnitID(), player.getPlayerID());
            unit.setHasMoved(true);
        }
        player.buildedUnit(unit);
        audio.playSound("unload.wav");
        ACTION_PLACE_WATERMINE.postAnimationMinePosX = -1;
        ACTION_PLACE_WATERMINE.postAnimationMinePosY = -1;
        ACTION_PLACE_WATERMINE.postAnimationMineOwner = null;
    };
    this.getName = function()
    {
        return qsTr("Place Watermine");
    };
    this.getDescription = function()
    {
        return qsTr("Places a stealthed Watermine at a given Position. The Mine automatically explodes at the start of a turn if an enemy is in explosion range.");
    };
}

Constructor.prototype = ACTION;
var ACTION_PLACE_WATERMINE = new Constructor();
