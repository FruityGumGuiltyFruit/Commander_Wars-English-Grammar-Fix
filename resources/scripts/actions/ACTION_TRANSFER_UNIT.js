var Constructor = function()
{
    this.canBePerformed = function(action, map)
    {
        var ret = false;
        var currentPlayer = map.getCurrentPlayer();
        var count = map.getPlayerCount();
        for (var i = 0; i < count; i++)
        {
            var player = map.getPlayer(i);
            if (player !== currentPlayer &&
                    currentPlayer.isAlly(player))
            {
                ret = true;
                break;
            }
        }
        if (ret)
        {
            ret = false;
            var units = map.getCurrentPlayer().getUnits();
            var size = units.size();
            for (var i = 0; i < size; i++)
            {
                var unit = units.at(i);
                if (unit.getHasMoved() === false)
                {
                    ret = true;
                    break;
                }
            }
        }
        return ret;
    };
    
    this.getActionText = function(map)
    {
        return qsTr("Transfer Unit");
    };
    this.getIcon = function(map)
    {
        return "next_player";
    };
    this.isFinalStep = function(action, map)
    {
        if (action.getInputStep() === 2)
        {
            return true;
        }
        else
        {
            return false;
        }
    };
    this.perform = function(action, map)
    {
        action.startReading();
        // read action data
        var x = action.readDataInt32();
        var y = action.readDataInt32();
		var player = action.getCosts();
        var unit = map.getTerrain(x, y).getUnit();
        unit.setOwner(map.getPlayer(player));
    };
    this.getStepCursor = function(action, cursorData, map)
    {
		cursorData.setCursor("cursor+edit");
		cursorData.setXOffset(0);
		cursorData.setYOffset(0);
        cursorData.setScale(1.0);
    };
    this.getStepData = function(action, data, map)
    {
		if (action.getInputStep() === 1)
        {
			var currentPlayer = map.getCurrentPlayer();
            var count = map.getPlayerCount();
            for (var i = 0; i < count; i++)
            {
				var player = map.getPlayer(i);
				if (player !== currentPlayer &&
					currentPlayer.isAlly(player))
				{
					data.addData(qsTr("Player ") + (i + 1).toString(), "PLAYER" + (i + 1).toString(), "capture", i, true);
				}
			}
		}
		else
		{
			var units = map.getCurrentPlayer().getUnits();
			for (var i = 0; i < units.size(); i++)
			{
				var unit = units.at(i);
				if (unit.getHasMoved() === false &&
					unit.getUnitRank() >= GameEnums.UnitRank_None)
				{
					var x = unit.getX();
					var y = unit.getY();				
					data.addPoint(Qt.point(x, y));             
				}
			}
			data.setShowZData(false);
			data.setColor("#FF00FF00");
		}
    };
    this.getStepInputType = function(action, map)
    {
		if (action.getInputStep() === 1)
        {
			return "MENU";
		}
		else
		{
			return "FIELD";			
		}
    };
	this.getDescription = function()
	{
		return qsTr("Transfers a unit to another allied player.");
    };
    this.getBannedByDefault = function()
    {
        return true;
    };
};

Constructor.prototype = ACTION;
var ACTION_TRANSFER_UNIT = new Constructor();
