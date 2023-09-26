#pragma once
#include <QObject>
#include <QVector>

#include "coreengine/fileserializable.h"
#include "ai/decisiontree/question.h"

class DecisionQuestion;
using spDecisionQuestion = std::shared_ptr<DecisionQuestion>;

class DecisionQuestion final : public QObject, public FileSerializable
{
    Q_OBJECT
public:
    explicit DecisionQuestion();
    virtual ~DecisionQuestion() = default;
    void appendQuestion(spQuestion pQuestions);
    virtual void serializeObject(QDataStream& pStream) const override;
    virtual void deserializeObject(QDataStream& pStream)  override;
    virtual qint32 getVersion() const override
    {
        return 1;
    }
    qint32 getIndex() const;
    QString print();
    inline qint32 getSize()
    {
        return m_Questions.size();
    }
public slots:
        qint32 matches(float value);
        qint32 matches(std::vector<float> &rData);

private:
        QVector<spQuestion> m_Questions;
};
