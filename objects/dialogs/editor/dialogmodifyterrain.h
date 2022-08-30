#ifndef DIALOGMODIFYTERRAIN_H
#define DIALOGMODIFYTERRAIN_H

#include <QObject>

#include "3rd_party/oxygine-framework/oxygine/actor/Button.h"

#include "objects/base/panel.h"
#include "objects/base/textbox.h"

class GameMap;
class Terrain;
class DialogModifyTerrain;
using spDialogModifyTerrain = oxygine::intrusive_ptr<DialogModifyTerrain>;

class DialogModifyTerrain : public QObject, public oxygine::Actor
{
    Q_OBJECT
public:
    explicit DialogModifyTerrain(GameMap* pMap, Terrain* pTerrain);
    virtual ~DialogModifyTerrain() = default;
signals:
    void sigFinished();
    void sigTerrainClicked(QString id);
    void sigShowLoadDialog();
public slots:
    void terrainClicked(QString id);
    void showLoadDialog();
    void loadCustomSprite(QString id);
    void remove();
private:
    Terrain* m_pTerrain{nullptr};
    spPanel m_pPanel;
    spTextbox m_pTextbox;
    oxygine::spButton m_OkButton;
    GameMap* m_pMap{nullptr};

};

#endif // DIALOGMODIFYTERRAIN_H
