#include "ai/decisiontree/leaf.h"
#include "ai/decisiontree/decisiontree.h"

#include "coreengine/mainapp.h"
#include "coreengine/globalutils.h"
#include "coreengine/interpreter.h"

Leaf::Leaf()
{
#ifdef GRAPHICSUPPORT
    setObjectName("Leaf");
#endif
    Interpreter::setCppOwnerShip(this);
}

Leaf::Leaf(std::vector<std::vector<float>>& trainingData)
    : m_AnswersChances(DecisionTree::countClassItems(trainingData))
{
#ifdef GRAPHICSUPPORT
    setObjectName("Leaf");
#endif
    Interpreter::setCppOwnerShip(this);
    for (qint32 i = 0; i < trainingData.size(); i++)
	{
        float answer = trainingData[i][trainingData[i].size() - 1];
        if (GlobalUtils::contains(m_Answers, answer) == false)
		{
            m_Answers.push_back(answer);
		}
	}
    for (qint32 i = 0; i < m_AnswersChances.size(); i++)
    {
        m_totalChance += m_AnswersChances[i];
    }
}

float Leaf::getDecision(std::vector<float>&)
{
    if (m_Answers.size() > 1)
    {
        qint32 chance = GlobalUtils::randIntBase(1, m_totalChance);
        qint32 lowThreshold = 1;
        for (qint32 i = 0; i < m_AnswersChances.size(); i++)
        {
            if ((chance >= lowThreshold) &&
                (chance < lowThreshold + m_AnswersChances[i]))
            {
                return m_Answers[i];
            }
            else
            {
                lowThreshold += m_AnswersChances[i];
            }
        }
    }
    if (m_Answers.size() > 0)
    {
        return m_Answers[0];
    }
    else
    {
        return -1;
    }
}

QString Leaf::print()
{
    QString ret = "Answer ";
    for (auto & answer : m_Answers)
    {
        ret += QString::number(answer, 'f', 1) + " ";
    }
    return ret;
}

void Leaf::serializeObject(QDataStream& pStream) const
{
    pStream << false; // --> 0 for node and 1 for leaf
    pStream << getVersion();
    pStream << static_cast<qint32>(m_AnswersChances.size());
    for (auto & chance : m_AnswersChances)
    {
        pStream << chance;
    }
    pStream << static_cast<qint32>(m_Answers.size());
    for (auto & answer : m_Answers)
    {
        pStream << answer;
    }
    pStream << m_totalChance;
}

void Leaf::deserializeObject(QDataStream& pStream)
{
    qint32 version = 0;
    pStream >> version;
    qint32 size = 0;
    pStream >> size;
    m_AnswersChances.clear();
    for (qint32 i = 0; i < size; i++)
    {
        qint32 item = 0;
        pStream >> item;
        m_AnswersChances.push_back(item);
    }
    size = 0;
    pStream >> size;
    m_Answers.clear();
    for (qint32 i = 0; i < size; i++)
    {
        float item = 0;
        pStream >> item;
        m_Answers.push_back(item);
    }
    pStream >> m_totalChance;
}
