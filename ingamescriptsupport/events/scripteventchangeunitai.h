#ifndef SCRIPTEVENTCHANGEUNITAI_H
#define SCRIPTEVENTCHANGEUNITAI_H

#include "ingamescriptsupport/events/scripteventgeneric.h"

class ScriptEventChangeUnitAI;
using spScriptEventChangeUnitAI = oxygine::intrusive_ptr<ScriptEventChangeUnitAI>;

class ScriptEventChangeUnitAI final : public ScriptEventGeneric
{
    Q_OBJECT
public:
    explicit ScriptEventChangeUnitAI(GameMap* pMap);
    /**
     * @brief removeCustomStart
     * @param text
     */
    virtual void removeCustomStart(QString& text) override;
    /**
     * @brief writeCustomStart
     */
    virtual void writeCustomStart(QTextStream& stream) override;
};

#endif // SCRIPTEVENTCHANGEUNITAI_H
