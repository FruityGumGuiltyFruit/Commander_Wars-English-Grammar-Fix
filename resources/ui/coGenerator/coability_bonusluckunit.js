var COABILITY_BONUSLUCKUNIT =
{

    getAbilityName : function()
    {
        return qsTr("Unit luck bonus");
    },

    getFunctionNameContext : function(coAbility, coGeneratorMenu)
    {
        return "getBonusLuck";
    },

    writeCoAbilityContent : function(coAbility, coGeneratorMenu)
    {
        var variables = coAbility.getVariables();
        var bonus = variables.createVariable("bonus");
        var unitId = variables.createVariable("unitID").readDataString();
        if (unitId === "")
        {
            unitId = "INFANTRY";
        }
        return COGENERATORMENU.getActiveConditionPrologue(coAbility) +
               "                if (unitId === \"" + unitId + "\")\n" +
               "                {\n" +
               "                    result = result + " + bonus.readDataInt32().toString() + ";\n" +
               "                }\n" +
               "            }\n"
    }
};
