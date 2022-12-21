var Constructor = function()
{
    this.canBePerformed = function(action, map)
    {
        var unit = action.getTargetUnit();
        var actionTargetField = action.getActionTarget();
        var targetField = action.getTarget();
		var targetUnit = action.getMovementTarget();
        if (unit.getHasMoved() === false &&
            unit.getBaseMovementCosts(actionTargetField.x, actionTargetField.y) > 0 &&
            targetUnit !== null &&
            (actionTargetField.x !== targetField.x || actionTargetField.y !== targetField.y))
        {
            if ((targetUnit.getOwner() === unit.getOwner()) &&
                (targetUnit.getUnitID() === unit.getUnitID()) &&
                // join is only allowed with units that don't have anything loaded
                (unit.getLoadedUnitCount() === 0) && (targetUnit.getLoadedUnitCount() === 0) &&
                (targetUnit.getHpRounded() < 10) &&
                ((targetUnit.getUnitRank() >= GameEnums.UnitRank_None ||
                 (targetUnit.getUnitRank() < GameEnums.UnitRank_None && unit.getUnitRank() >= GameEnums.UnitRank_None))))
			{
				return true;
			}
        }
        return false;
        
    };
    this.getActionText = function(map)
    {
        return qsTr("Join");
    };
    this.getIcon = function(map)
    {
        return "join";
    };
    this.isFinalStep = function(action, map)
    {
        return true;
    };
    this.postAnimationTargetUnit = null;
    this.postAnimationUnit = null;
    this.perform = function(action, map)
    {
        // we need to move the unit to the target position
        ACTION_JOIN.postAnimationUnit = action.getTargetUnit();
        ACTION_JOIN.postAnimationTargetUnit = action.getMovementTarget();
        var animation = Global[ACTION_JOIN.postAnimationUnit.getUnitID()].doWalkingAnimation(action, map);
        animation.setEndOfAnimationCall("ACTION_JOIN", "performPostAnimation");
    };
    this.performPostAnimation = function(postAnimation, map)
    {
        var ammo1 = ACTION_JOIN.postAnimationUnit.getAmmo1() + ACTION_JOIN.postAnimationTargetUnit.getAmmo1();
        var maxValue = ACTION_JOIN.postAnimationUnit.getMaxAmmo1();
        if (ammo1 > maxValue)
        {
            ammo1 = maxValue;
        }
        maxValue = ACTION_JOIN.postAnimationUnit.getMaxAmmo2();
        var ammo2 = ACTION_JOIN.postAnimationUnit.getAmmo2() + ACTION_JOIN.postAnimationTargetUnit.getAmmo2();
        if (ammo2 > maxValue)
        {
            ammo2 = maxValue;
        }
        maxValue = ACTION_JOIN.postAnimationUnit.getMaxFuel();
        var fuel = ACTION_JOIN.postAnimationUnit.getFuel() + ACTION_JOIN.postAnimationTargetUnit.getFuel();
        if (fuel > maxValue)
        {
            fuel = maxValue;
        }
        var hp = ACTION_JOIN.postAnimationUnit.getHpRounded() + ACTION_JOIN.postAnimationTargetUnit.getHpRounded();
        if (hp > 10)
        {
            var overHeal = hp - 10;
            var income = ACTION_JOIN.postAnimationTargetUnit.getCosts() * (overHeal / 10);
            ACTION_JOIN.postAnimationTargetUnit.getOwner().addFunds(income);
            hp = 10;
        }
        if (ACTION_JOIN.postAnimationUnit.getUnitRank() > ACTION_JOIN.postAnimationTargetUnit.getUnitRank() &&
            ACTION_JOIN.postAnimationUnit.getUnitRank() >= GameEnums.UnitRank_None &&
            ACTION_JOIN.postAnimationTargetUnit.getUnitRank() >= GameEnums.UnitRank_None)
        {           
            ACTION_JOIN.postAnimationTargetUnit.setUnitRank(ACTION_JOIN.postAnimationUnit.getUnitRank());
        }
        ACTION_JOIN.postAnimationTargetUnit.setAmmo1(ammo1);
        ACTION_JOIN.postAnimationTargetUnit.setAmmo2(ammo2);
        ACTION_JOIN.postAnimationTargetUnit.setFuel(fuel);
        ACTION_JOIN.postAnimationTargetUnit.setHp(hp);
        var unitRank = ACTION_JOIN.postAnimationUnit.getUnitRank();
        ACTION_JOIN.postAnimationUnit.removeUnit(false);
        // handle co unit creation here
        if (unitRank === GameEnums.UnitRank_CO0)
        {
            ACTION_JOIN.postAnimationTargetUnit.makeCOUnit(0, true);
        }
        else if (unitRank === GameEnums.UnitRank_CO1)
        {
            ACTION_JOIN.postAnimationTargetUnit.makeCOUnit(1, true);
        }
        // disable unit commandments for this turn
        ACTION_JOIN.postAnimationTargetUnit.setHasMoved(true);
        ACTION_JOIN.postAnimationTargetUnit = null;
        ACTION_JOIN.postAnimationUnit = null;
    };
    this.getName = function()
    {
        return qsTr("Join");
    };
    this.getDescription = function()
    {
        return qsTr("Combines a wounded unit of the same type with the current unit. This will lead to a unit with combined ammo, fuel and hp ad the highest rank of both units. Hp exceeding the maximum will be added in form of funds to your war funds.");
    };
}

Constructor.prototype = ACTION;
var ACTION_JOIN = new Constructor();
