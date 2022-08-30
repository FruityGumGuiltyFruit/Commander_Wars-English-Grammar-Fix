#include "ai/decisiontree/decisionquestion.h"

#include "coreengine/interpreter.h"

DecisionQuestion::DecisionQuestion()
{
#ifdef GRAPHICSUPPORT
    setObjectName("DecisionQuestion");
#endif
    Interpreter::setCppOwnerShip(this);
}

void DecisionQuestion::appendQuestion(spQuestion pQuestions)
{
    m_Questions.append(pQuestions);
}

void DecisionQuestion::serializeObject(QDataStream& pStream) const
{
    pStream << getVersion();
    pStream << static_cast<qint32>(m_Questions.size());
    for (auto & question : m_Questions)
    {
        question->serializeObject(pStream);
    }
}

void DecisionQuestion::deserializeObject(QDataStream& pStream)
{
    qint32 version;
    pStream >> version;
    qint32 size = 0;
    pStream >> size;
    m_Questions.clear();
    for (qint32 i = 0; i < size; i++)
    {
        m_Questions.append(spQuestion::create());
        m_Questions[i]->deserializeObject(pStream);
    }
}

QString DecisionQuestion::print()
{
    QString ret = "";
    for (auto & question : m_Questions)
    {
        ret += question->print() + " ";
    }
    return ret;
}

qint32 DecisionQuestion::matches(float value)
{
    for (qint32 i = 0; i < m_Questions.size(); i++)
    {
        if (m_Questions[i]->matches(value))
        {
            return i;
        }
    }
    return -1;
}

qint32 DecisionQuestion::matches(std::vector<float> &rData)
{
    for (qint32 i = 0; i < m_Questions.size(); i++)
    {
        if (m_Questions[i]->matches(rData))
        {
            return i;
        }
    }
    return -1;
}
