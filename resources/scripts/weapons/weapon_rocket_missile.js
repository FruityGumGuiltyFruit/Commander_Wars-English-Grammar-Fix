var Constructor = function()
{
    this.getName = function()
    {
        return qsTr("Missile");
    };
    this.getEnviromentDamage = function(enviromentId)
    {
        return 55;
    };
    this.damageTable = [["INFANTRY", 160],
                        ["MECH", 120],
                        ["MOTORBIKE", 120],
                        ["SNIPER", 120],

                        // supporter
                        ["APC", 80],
                        ["FLARE", 80],
                        ["RECON", 80],

                        // tanks
                        ["FLAK", 75],
                        ["HOVERFLAK", 75],
                        ["LIGHT_TANK", 75],
                        ["HOVERCRAFT", 75],

                        // heavy tanks
                        ["HEAVY_HOVERCRAFT", 55],
                        ["HEAVY_TANK", 50],
                        ["NEOTANK", 55],

                        // very heavy tanks
                        ["MEGATANK", 45],

                        ["HOELLIUM", 15],

                        // ranged land units
                        ["ARTILLERY", 80],
                        ["ARTILLERYCRAFT", 80],
                        ["ANTITANKCANNON", 65],
                        ["MISSILE", 85],
                        ["ROCKETTHROWER", 85],
                        ["PIPERUNNER", 85],
                        // ships
                        ["GUNBOAT", 105],
                        ["CANNONBOAT", 105],
                        ["BLACK_BOAT", 105],
                        ["TORPEDOBOAT", 105],
                        ["LANDER", 75],
                        ["FRIGATE", 55],
                        ["DESTROYER", 55],
                        ["CRUISER", 55],
                        ["SUBMARINE", 65],
                        ["BATTLECRUISER", 55],
                        ["BATTLESHIP", 55],
                        ["AIRCRAFTCARRIER", 55],];

    this.getBaseDamage = function(unit)
    {
        return WEAPON.getDamageFromTable(unit, WEAPON_ROCKET_MISSILE.damageTable, "WEAPON_ROCKET_MISSILE");
    };
};

Constructor.prototype = WEAPON;
var WEAPON_ROCKET_MISSILE = new Constructor();
