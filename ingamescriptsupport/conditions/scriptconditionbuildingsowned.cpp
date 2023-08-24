#include "ingamescriptsupport/conditions/scriptconditionbuildingsowned.h"

#include "ingamescriptsupport/scriptdata.h"
#include "ingamescriptsupport/scripteditor.h"
#include "ingamescriptsupport/genericbox.h"

#include "resource_management/fontmanager.h"

#include "coreengine/gameconsole.h"

#include "objects/base/spinbox.h"
#include "objects/base/label.h"

ScriptConditionBuildingsOwned::ScriptConditionBuildingsOwned(GameMap* pMap)
    : ScriptCondition(pMap, ConditionType::buildingsOwned)
{

}

qint32 ScriptConditionBuildingsOwned::getCount() const
{
    return m_count;
}

void ScriptConditionBuildingsOwned::setCount(const qint32 &count)
{
    m_count = count;
}

qint32 ScriptConditionBuildingsOwned::getPlayer() const
{
    return m_player;
}

void ScriptConditionBuildingsOwned::setPlayer(const qint32 &player)
{
    m_player = player;
}

void ScriptConditionBuildingsOwned::readCondition(QTextStream& rStream, QString line)
{
    CONSOLE_PRINT("Reading ConditionBuildingsOwned", GameConsole::eDEBUG);
    line = line.simplified();
    QStringList items = line.replace("if (map.getPlayer(", "")
                            .replace(").getBuildingCount() >= ", ",")
                            .replace(" && ", ",").split(",");
    if (items.size() >= 2)
    {
        m_player = items[0].toInt();
        m_count = items[1].toInt();
    }
    while (!rStream.atEnd())
    {
        if (readSubCondition(m_pMap, rStream, ConditionBuildingsOwned, line))
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

void ScriptConditionBuildingsOwned::writePreCondition(QTextStream& rStream)
{
    CONSOLE_PRINT("Writing ConditionBuildingsOwned", GameConsole::eDEBUG);
    m_executed = ScriptData::getVariableName();
    rStream << "        var " << m_executed << " = " << ScriptData::variables << ".createVariable(\"" << m_executed << "\");\n";
    if (subCondition.get() != nullptr)
    {
        subCondition->writePreCondition(rStream);
    }
}

void ScriptConditionBuildingsOwned::writeCondition(QTextStream& rStream)
{
    rStream << "        if (map.getPlayer(" << QString::number(m_player) << ").getBuildingCount() >= " << QString::number(m_count)
            << " && " << m_executed << ".readDataBool() === false) {"
            << "// " << QString::number(getVersion()) << " " << ConditionBuildingsOwned << "\n";
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
    rStream << "        } // " + QString(ConditionBuildingsOwned) + " End\n";
}

void ScriptConditionBuildingsOwned::writePostCondition(QTextStream& rStream)
{
    ScriptCondition::writePostCondition(rStream);
    for (qint32 i = 0; i < events.size(); i++)
    {
        events[i]->writeEvent(rStream);
    }
    rStream << "            " << m_executed << ".writeDataBool(true);\n";
}

void ScriptConditionBuildingsOwned::showEditCondition(spScriptEditor pScriptEditor)
{
    spGenericBox pBox = MemoryManagement::create<GenericBox>();

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    qint32 width = 300;

    spLabel pText = MemoryManagement::create<Label>(width - 10);
    pText->setStyle(style);
    pText->setHtmlText(tr("Count: "));
    pText->setPosition(30, 30);
    pBox->addItem(pText);
    spSpinBox spinBox = MemoryManagement::create<SpinBox>(300, 0, 99999);
    spinBox->setTooltipText(tr("Amount of Buildings that has to be owned."));
    spinBox->setPosition(width, 30);
    spinBox->setCurrentValue(m_count);
    connect(spinBox.get(), &SpinBox::sigValueChanged, this,
            [this](qreal value)
    {
        setCount(static_cast<qint32>(value));
    });
    pBox->addItem(spinBox);

    pText = MemoryManagement::create<Label>(width - 10);
    pText->setStyle(style);
    pText->setHtmlText(tr("Player: "));
    pText->setPosition(30, 70);
    pBox->addItem(pText);
    spinBox = MemoryManagement::create<SpinBox>(300, 1, 99999);
    spinBox->setTooltipText(tr("Player that has to own the given amount of buildings."));
    spinBox->setPosition(width, 70);
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
