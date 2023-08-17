#include "ingamescriptsupport/events/scripteventdialog.h"

#include "ingamescriptsupport/scriptdialogdialog.h"
#include "ingamescriptsupport/scripteditor.h"

#include "coreengine/gameconsole.h"
#include "coreengine/interpreter.h"

const char* const ScriptEventDialog::m_CurrentPlayerCO0 = "map.getCurrentPlayer().getCO(0).getCoID()";
const char* const ScriptEventDialog::m_CurrentPlayerCO1 = "map.getCurrentPlayer().getCO(1).getCoID()";

const char* const ScriptEventDialog::ScriptEventDialogItem = "ScriptEventDialogItem";

ScriptEventDialog::ScriptEventDialog(GameMap* pMap)
    : ScriptEvent(pMap, EventType::dialog)
{
    Interpreter::setCppOwnerShip(this);
}

void ScriptEventDialog::addDialog(QString text, QString coid, GameEnums::COMood mood, QColor color)
{
    spDialogEntry dialog = MemoryManagement::create<DialogEntry>();
    dialog->text = text;
    dialog->coid = coid;
    dialog->mood = mood;
    dialog->color = color;
    m_Dialog.append(dialog);
}

void ScriptEventDialog::showEditEvent(spScriptEditor pScriptEditor)
{
    spScriptDialogDialog pScriptDialogDialog = MemoryManagement::create<ScriptDialogDialog>(getSharedPtr<ScriptEventDialog>());
    pScriptEditor->addChild(pScriptDialogDialog);
    connect(pScriptDialogDialog.get(), &ScriptDialogDialog::sigFinished, pScriptEditor.get(), &ScriptEditor::updateEvents, Qt::QueuedConnection);
}

void ScriptEventDialog::removeDialog(qint32 index)
{
    if (index >= 0 && index < m_Dialog.size())
    {
        m_Dialog.removeAt(index);
    }
}

spDialogEntry ScriptEventDialog::getDialog(qint32 index)
{
    if (index >= 0 && index < m_Dialog.size())
    {
        return m_Dialog[index];
    }
    return spDialogEntry();
}

void ScriptEventDialog::readEvent(QTextStream& rStream, QString line)
{
    CONSOLE_PRINT("Reading EventDialog", GameConsole::eDEBUG);
    line = line.simplified();
    while (!rStream.atEnd())
    {
        line = rStream.readLine();
        line = line.simplified();
        if (line.endsWith(EventDialog))
        {
            CONSOLE_PRINT("Read EventDialog", GameConsole::eDEBUG);
            break;
        }
        else if (line.endsWith(ScriptEventDialogItem))
        {
            line = line.simplified();

            QStringList items = line.replace("var dialog" + QString::number(m_Dialog.size()) + " = GameAnimationFactory.createGameAnimationDialog(map, qsTr(\"", "")
                                    .replace("var dialog" + QString::number(m_Dialog.size()) + " = GameAnimationFactory.createGameAnimationDialog(qsTr(\"", "")
                                    .replace("\"), \"", "@")
                                    .replace("\"), ", "@")
                                    .replace("\", GameEnums.COMood_", "@")
                                    .replace(", GameEnums.COMood_", "@")
                                    .replace(", \"", "@")
                                    .replace("\"); // ", "@").
                                    replace(" " + QString(ScriptEventDialogItem), "@" + QString(ScriptEventDialogItem)).split("@");
            qint32 version = 0;

            if (items.size() >= 2)
            {
                version = items[items.size() - 2].toInt();
                items = line.replace("\"); dialog" + QString::number(m_Dialog.size()) + ".loadBackground(\"", "@").split("@");
            }

            if (items.size() >= 4)
            {
                spDialogEntry dialog = MemoryManagement::create<DialogEntry>();
                dialog->text = items[0].replace("\\\"", "\"");
                dialog->coid = items[1];
                if (items[2] == "Sad")
                {
                    dialog->mood = GameEnums::COMood_Sad;
                }
                else if (items[2] == "Normal")
                {
                    dialog->mood = GameEnums::COMood_Normal;
                }
                else if (items[2] == "Happy")
                {
                    dialog->mood = GameEnums::COMood_Happy;
                }
                dialog->color = items[3];
                if (version > 0 && items.size() >= 5)
                {
                    dialog->background = items[4];
                }
                m_Dialog.append(dialog);
            }
        }
    }
}

void ScriptEventDialog::writeEvent(QTextStream& rStream)
{
    rStream << "            // " + QString(EventDialog) + "\n";
    for (qint32 i = 0; i < m_Dialog.size(); i++)
    {
        rStream <<  "            var dialog" << QString::number(i) << " = GameAnimationFactory.createGameAnimationDialog(map, qsTr(\"";
        QString text = m_Dialog[i]->text;
        text = text.replace("\"", "\\\"");
        rStream <<  text;
        if (m_Dialog[i]->coid.contains("."))
        {
            rStream << "\"), " << m_Dialog[i]->coid << ", GameEnums.COMood_";
        }
        else
        {
            rStream << "\"), \"" << m_Dialog[i]->coid << "\", GameEnums.COMood_";
        }
        switch (m_Dialog[i]->mood)
        {
            case GameEnums::COMood_Sad:
            {
                rStream << "Sad";
                break;
            }
            case GameEnums::COMood_Happy:
            {
                rStream << "Happy";
                break;
            }
            case GameEnums::COMood_Normal:
            {
                rStream << "Normal";
                break;
            }
        }
        rStream << ", \"" << m_Dialog[i]->color.name() << "\"); ";
        rStream << "dialog" << QString::number(i) << ".loadBackground(\"" << m_Dialog[i]->background << "\"); ";
        rStream << "// " << QString::number(getVersion()) << " " << ScriptEventDialogItem << "\n";
    }
    for (qint32 i = 0; i < m_Dialog.size() - 1; i++)
    {
        rStream << "            dialog" << QString::number(i) << ".queueAnimation(dialog" << QString::number(i + 1) << ");\n";
    }
    rStream << "            // " + QString(EventDialog) + "\n";
}
