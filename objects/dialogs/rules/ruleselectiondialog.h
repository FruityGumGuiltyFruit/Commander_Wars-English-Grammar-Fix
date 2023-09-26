#ifndef RULESELECTIONDIALOG_H
#define RULESELECTIONDIALOG_H

#include <QObject>


#include "3rd_party/oxygine-framework/oxygine/actor/Actor.h"
#include "3rd_party/oxygine-framework/oxygine/actor/Button.h"

#include "objects/ruleselection.h"
#include "objects/base/panel.h"

class GameMap;
class RuleSelectionDialog;
using spRuleSelectionDialog = std::shared_ptr<RuleSelectionDialog>;

class RuleSelectionDialog final : public QObject, public oxygine::Actor
{
    Q_OBJECT
public:
    explicit RuleSelectionDialog(GameMap* pMap, RuleSelection::Mode mode, bool enabled = true);
    virtual ~RuleSelectionDialog() = default;
signals:
    void sigRulesChanged();
    void sigShowLoadRules();
    void sigShowSaveRules();
    void sigOk();
public slots:
    void showLoadRules();
    void showSaveRules();
    void loadRules(QString filename);
    void saveRules(QString filename);
    void ruleSelectionSizeChanged();
protected slots:
    void pressedOk();
private:
    oxygine::spButton m_OkButton;
    oxygine::spButton m_pButtonLoadRules;
    oxygine::spButton m_pButtonSaveRules;
    spRuleSelection m_pRuleSelection;
    spPanel m_pPanel;
    GameMap* m_pMap{nullptr};
};

#endif // RULESELECTIONDIALOG_H
