#include "ingamescriptsupport/conditions/scriptconditionplayerdefeated.h"

#include "ingamescriptsupport/scriptdata.h"
#include "ingamescriptsupport/scripteditor.h"
#include "ingamescriptsupport/genericbox.h"

#include "resource_management/fontmanager.h"

#include "coreengine/gameconsole.h"

#include "objects/base/spinbox.h"
#include "objects/base/label.h"

ScriptConditionPlayerDefeated::ScriptConditionPlayerDefeated(GameMap* pMap)
    : ScriptCondition(pMap, ConditionType::playerDefeated)
{

}

qint32 ScriptConditionPlayerDefeated::getPlayer() const
{
    return m_player;
}

void ScriptConditionPlayerDefeated::setPlayer(const qint32 &player)
{
    m_player = player;
}

void ScriptConditionPlayerDefeated::readCondition(QTextStream& rStream, QString line)
{
    CONSOLE_PRINT("Reading ConditionPlayerDefeated", GameConsole::eDEBUG);
    line = line.simplified();
    QStringList items = line.replace("if (map.getPlayer(", "")
                            .replace(").getIsDefeated() === true", ",").split(",");
    if (items.size() >= 1)
    {
        m_player = items[0].toInt();
    }
    while (!rStream.atEnd())
    {
        if (readSubCondition(m_pMap, rStream, ConditionPlayerDefeated, line))
        {
            break;
        }
        spScriptEvent event = ScriptEvent::createReadEvent(m_pMap, rStream, line);
        if (event.get() != nullptr)
        {
            events.append(event);
        }
    }
}

void ScriptConditionPlayerDefeated::writePreCondition(QTextStream& rStream)
{
    CONSOLE_PRINT("Writing ConditionPlayerDefeated", GameConsole::eDEBUG);
    m_executed = ScriptData::getVariableName();
    rStream << "        var " << m_executed << " = " << ScriptData::variables << ".createVariable(\"" << m_executed << "\");\n";
    if (subCondition.get() != nullptr)
    {
        subCondition->writePreCondition(rStream);
    }
}

void ScriptConditionPlayerDefeated::writeCondition(QTextStream& rStream)
{
    rStream << "        if (map.getPlayer(" << QString::number(m_player) << ").getIsDefeated() === true"
            << " && " << m_executed << ".readDataBool() === false) {"
            << "// " << QString::number(getVersion()) << " " << ConditionPlayerDefeated << "\n";
    if (subCondition.get() != nullptr)
    {
        subCondition->writeCondition(rStream);
    }
    else if (pParent != nullptr)
    {
        pParent->writePostCondition(rStream);
        for (qint32 i = 0; i < events.size(); i++)
        {
            events[i]->writeEvent(rStream);
        }
        rStream << "            " << m_executed << ".writeDataBool(true);\n";
    }
    else
    {
        for (qint32 i = 0; i < events.size(); i++)
        {
            events[i]->writeEvent(rStream);
        }
        rStream << "            " << m_executed << ".writeDataBool(true);\n";
    }
    rStream << "        } // " + QString(ConditionPlayerDefeated) + " End\n";
}

void ScriptConditionPlayerDefeated::writePostCondition(QTextStream& rStream)
{
    ScriptCondition::writePostCondition(rStream);
    for (qint32 i = 0; i < events.size(); i++)
    {
        events[i]->writeEvent(rStream);
    }
    rStream << "            " << m_executed << ".writeDataBool(true);\n";
}

void ScriptConditionPlayerDefeated::showEditCondition(spScriptEditor pScriptEditor)
{
    spGenericBox pBox = MemoryManagement::create<GenericBox>();

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    qint32 width = 300;

    spLabel pText = MemoryManagement::create<Label>(width - 10);
    pText->setStyle(style);
    pText->setHtmlText(tr("Player: "));
    pText->setPosition(30, 30);
    pBox->addItem(pText);
    spSpinBox spinBox = MemoryManagement::create<SpinBox>(300, 1, 99999);
    spinBox->setTooltipText(tr("The player that has to be defeated in order to executed the events."));
    spinBox->setPosition(width, 30);
    spinBox->setCurrentValue(m_player + 1);
    connect(spinBox.get(), &SpinBox::sigValueChanged, this,
            [this](qreal value)
    {
        setPlayer(static_cast<qint32>(value) - 1);
    });
    pBox->addItem(spinBox);

    pScriptEditor->addChild(pBox);
    connect(pBox.get(), &GenericBox::sigFinished, pScriptEditor.get(), &ScriptEditor::updateConditios, Qt::QueuedConnection);
}
