#include "ingamescriptsupport/conditions/scriptconditionstartofturn.h"

#include "ingamescriptsupport/scripteditor.h"
#include "ingamescriptsupport/genericbox.h"

#include "resource_management/fontmanager.h"

#include "coreengine/gameconsole.h"

#include "objects/base/spinbox.h"
#include "objects/base/label.h"

ScriptConditionStartOfTurn::ScriptConditionStartOfTurn(GameMap* pMap)
    : ScriptCondition(pMap, ConditionType::startOfTurn)
{

}

qint32 ScriptConditionStartOfTurn::getDay() const
{
    return day;
}

void ScriptConditionStartOfTurn::setDay(const qint32 &value)
{
    day = value;
}

qint32 ScriptConditionStartOfTurn::getPlayer() const
{
    return player;
}

void ScriptConditionStartOfTurn::setPlayer(const qint32 &value)
{
    player = value;
}

void ScriptConditionStartOfTurn::readCondition(QTextStream& rStream, QString line)
{
    line = line.simplified();
    QStringList items = line.replace("if (turn === ", "")
                            .replace(" && player === ", ",")
                            .replace(") { // ", ",").split(",");
    CONSOLE_PRINT("Reading ConditionStartOfTurn " + line, GameConsole::eDEBUG);
    if (items.size() >= 2)
    {
        day = items[0].toInt();
        player = items[1].toInt();
        while (!rStream.atEnd())
        {
            if (readSubCondition(m_pMap, rStream, ConditionStartOfTurn, line))
            {
                CONSOLE_PRINT("Read ConditionStartOfTurn", GameConsole::eDEBUG);
                break;
            }
            spScriptEvent event = ScriptEvent::createReadEvent(m_pMap, rStream, line);
            if (event.get() != nullptr)
            {
                events.append(event);
            }
            else
            {
                CONSOLE_PRINT("unable to determine event", GameConsole::eWARNING);
            }
        }
    }
}

void ScriptConditionStartOfTurn::writeCondition(QTextStream& rStream)
{
    CONSOLE_PRINT("Writing ConditionStartOfTurn", GameConsole::eDEBUG);
    rStream << "        if (turn === " + QString::number(day) + " && player === " + QString::number(player) + ") { // "
            << QString::number(getVersion()) << " " << QString(ConditionStartOfTurn) + "\n";
    for (qint32 i = 0; i < events.size(); i++)
    {
        events[i]->writeEvent(rStream);
    }
    if (subCondition.get() != nullptr)
    {
        subCondition->writeCondition(rStream);
    }
    rStream << "        } // " + QString(ConditionStartOfTurn) + " End\n";
}

void ScriptConditionStartOfTurn::showEditCondition(spScriptEditor pScriptEditor)
{
    spGenericBox pBox = MemoryManagement::create<GenericBox>();

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    qint32 width = 300;

    spLabel pText = MemoryManagement::create<Label>(width - 10);
    pText->setStyle(style);
    pText->setHtmlText(tr("At Day: "));
    pText->setPosition(30, 30);
    pBox->addItem(pText);
    spSpinBox spinBox = MemoryManagement::create<SpinBox>(300, 1, 9999);
    spinBox->setTooltipText(tr("Day at which the events get executed at the start of this turn."));
    spinBox->setPosition(width, 30);
    spinBox->setCurrentValue(day);
    connect(spinBox.get(), &SpinBox::sigValueChanged, this, &ScriptConditionStartOfTurn::setDay, Qt::QueuedConnection);
    pBox->addItem(spinBox);

    pText = MemoryManagement::create<Label>(width - 10);
    pText->setStyle(style);
    pText->setHtmlText(tr("Player: "));
    pText->setPosition(30, 70);
    pBox->addItem(pText);
    spinBox = MemoryManagement::create<SpinBox>(300, 1, 9999);
    spinBox->setTooltipText(tr("Player at which the events get executed at the start of this turn."));
    spinBox->setPosition(width, 70);
    spinBox->setCurrentValue(player + 1);
    connect(spinBox.get(), &SpinBox::sigValueChanged, this,
            [this](qreal value)
    {
        setPlayer(static_cast<qint32>(value) - 1);
    });
    pBox->addItem(spinBox);

    pScriptEditor->addChild(pBox);
    connect(pBox.get(), &GenericBox::sigFinished, pScriptEditor.get(), &ScriptEditor::updateConditios, Qt::QueuedConnection);
}
