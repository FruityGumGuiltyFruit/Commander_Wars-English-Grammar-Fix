#include <QFile>

#include "3rd_party/oxygine-framework/oxygine/actor/Stage.h"

#include "coreengine/mainapp.h"
#include "coreengine/console.h"
#include "coreengine/audiothread.h"
#include "coreengine/userdata.h"

#include "menue/mainwindow.h"
#include "menue/costylemenu.h"

#include "resource_management/backgroundmanager.h"
#include "resource_management/fontmanager.h"
#include "resource_management/objectmanager.h"
#include "resource_management/cospritemanager.h"

#include "objects/coselection.h"
#include "objects/dialogs/dialogcostyle.h"
#include "coreengine/userdata.h"

COStyleMenu::COStyleMenu()
    : Basemenu()
{
    setObjectName("COStyleMenu");
    Mainapp* pApp = Mainapp::getInstance();
    pApp->pauseRendering();
    moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);

    CONSOLE_PRINT("Entering Co Style Menue", Console::eDEBUG);
    BackgroundManager* pBackgroundManager = BackgroundManager::getInstance();
    // load background
    oxygine::spSprite sprite = oxygine::spSprite::create();
    addChild(sprite);
    oxygine::ResAnim* pBackground = pBackgroundManager->getResAnim("costylemenu");
    if (pBackground != nullptr)
    {
        sprite->setResAnim(pBackground);
        // background should be last to draw
        sprite->setPriority(static_cast<qint32>(Mainapp::ZOrder::Background));
        sprite->setScaleX(Settings::getWidth() / pBackground->getWidth());
        sprite->setScaleY(Settings::getHeight() / pBackground->getHeight());
    }

    pApp->getAudioThread()->clearPlayList();
    pApp->getAudioThread()->loadFolder("resources/music/hauptmenue");
    pApp->getAudioThread()->playRandom();

    oxygine::spButton pButtonExit = ObjectManager::createButton(tr("Exit"));
    addChild(pButtonExit);
    pButtonExit->setPosition(20, Settings::getHeight() - pButtonExit->getHeight() - 10);
    pButtonExit->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigExitMenue();
    });
    connect(this, &COStyleMenu::sigExitMenue, this, &COStyleMenu::exitMenue, Qt::QueuedConnection);

    oxygine::spButton pButtonEdit = ObjectManager::createButton(tr("Edit CO"));
    addChild(pButtonEdit);
    pButtonEdit->setPosition(Settings::getWidth() - pButtonEdit->getWidth() - 20, Settings::getHeight() - pButtonExit->getHeight() - 10);
    pButtonEdit->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigEditCOStyle();
    });
    connect(this, &COStyleMenu::sigEditCOStyle, this, &COStyleMenu::editCOStyle, Qt::QueuedConnection);

    Userdata* pUserdata = Userdata::getInstance();
    auto items = pUserdata->getShopItemsList(GameEnums::ShopItemType_CO_Skin, false);
    QStringList bannList = COSpriteManager::getInstance()->getLoadedRessources();
    for (const auto & item : items)
    {
        bannList.removeAll(item);
    }
    spCOSelection pCOSelection = spCOSelection::create(QPoint(0, 0), QSize(Settings::getWidth(), Settings::getHeight() - 50), bannList);
    pCOSelection->colorChanged(QColor(248, 88, 0));
    addChild(pCOSelection);
    connect(pCOSelection.get(), &COSelection::coSelected, this, &COStyleMenu::selectedCOIDChanged, Qt::QueuedConnection);
    pApp->continueRendering();
}

void COStyleMenu::onEnter()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString object = "Init";
    QString func = "coStyleMenu";
    if (pInterpreter->exists(object, func))
    {
        CONSOLE_PRINT("Executing:" + object + "." + func, Console::eDEBUG);
        QJSValueList args({pInterpreter->newQObject(this)});
        pInterpreter->doFunction(object, func, args);
    }
}

void COStyleMenu::exitMenue()
{    
    CONSOLE_PRINT("Leaving CO Style Menue", Console::eDEBUG);
    auto window = spMainwindow::create("ui/menu/playermenu.xml");
    oxygine::Stage::getStage()->addChild(window);
    oxygine::Actor::detach();    
}

void COStyleMenu::reloadMenue()
{
    CONSOLE_PRINT("Leaving CO Style Menue", Console::eDEBUG);
    auto window = spCOStyleMenu::create();
    oxygine::Stage::getStage()->addChild(window);
    oxygine::Actor::detach();    
}

void COStyleMenu::selectedCOIDChanged(QString coid)
{    
    m_currentCOID = coid;    
}

void COStyleMenu::editCOStyle()
{    
    if (!m_currentCOID.isEmpty())
    {
        spDialogCOStyle pDialogCOStyle = spDialogCOStyle::create(m_currentCOID);
        addChild(pDialogCOStyle);
        connect(pDialogCOStyle.get(), &DialogCOStyle::sigFinished, this, &COStyleMenu::reloadMenue, Qt::QueuedConnection);
    }    
}
