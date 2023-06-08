#ifndef WIKIMENU_H
#define WIKIMENU_H

#include "menue/basemenu.h"
#include "wiki/wikiview.h"

class Wikimenu;
using spWikimenu = oxygine::intrusive_ptr<Wikimenu>;

class Wikimenu final : public Basemenu
{
    Q_OBJECT
public:
    explicit Wikimenu();
   virtual ~Wikimenu() = default;
signals:
    void sigExitMenue();
public slots:
    void exitMenue();
    virtual void onEnter() override;
    WikiView* getWikiView()
    {
        return m_pWikiView.get();
    }
private:
    spWikiView m_pWikiView;
};

#endif // WIKIMENU_H
