#pragma once
#include <QObject>

#include "3rd_party/oxygine-framework/oxygine/core/intrusive_ptr.h"

#include "ai/decisiontree/decisionquestion.h"

#include "coreengine/fileserializable.h"

class DecisionNode;
typedef oxygine::intrusive_ptr<DecisionNode> spDecisionNode;

class DecisionNode : public QObject, public FileSerializable, public oxygine::ref_counter
{
    Q_OBJECT
public:
        DecisionNode();
        DecisionNode(spDecisionQuestion & pQuestion, const std::vector<spDecisionNode> & pNodes);
        virtual ~DecisionNode() = default;

        virtual void serializeObject(QDataStream& pStream) const override;
        virtual void deserializeObject(QDataStream& pStream)  override;
        virtual qint32 getVersion() const override
        {
            return 1;
        }

        inline qint32 getNodeSize()
        {
            return m_pNodes.size();
        }
        inline DecisionNode* getNode(qint32 index)
        {
            return m_pNodes[index].get();
        }
        inline DecisionQuestion* getQuestion()
        {
            return m_pQuestion.get();
        }
public slots:
        virtual float getDecision(std::vector<float>& input);
private:
        spDecisionQuestion m_pQuestion;
        std::vector<spDecisionNode> m_pNodes;
};
