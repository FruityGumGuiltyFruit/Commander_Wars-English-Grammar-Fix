var Constructor = function()
{
    this.getName = function()
    {
        return qsTr("Torpedo");
    };
    this.damageTable = [["BATTLESHIP", 80],
                        ["DESTROYER", 120],

                        ["CRUISER", 25],
                        ["BATTLECRUISER", 95],
                        ["SUBMARINE", 55],

                        ["LANDER", 120],
                        ["BLACK_BOAT", 120],
                        ["AIRCRAFTCARRIER", 120],
                        ["WATERMINE", 400]];

    this.getBaseDamage = function(unit)
    {
        return WEAPON.getDamageFromTable(unit, WEAPON_TORPEDO.damageTable, "WEAPON_TORPEDO");
    };
};

Constructor.prototype = WEAPON;
var WEAPON_TORPEDO = new Constructor();
