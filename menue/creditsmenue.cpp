#include <QFile>
#include <QTextStream>

#include "3rd_party/oxygine-framework/oxygine/actor/Button.h"
#include "3rd_party/oxygine-framework/oxygine/actor/Stage.h"

#include "menue/creditsmenue.h"
#include "menue/mainwindow.h"

#include "coreengine/gameconsole.h"
#include "coreengine/mainapp.h"
#include "coreengine/audiomanager.h"
#include "coreengine/virtualpaths.h"

#include "resource_management/backgroundmanager.h"
#include "resource_management/objectmanager.h"
#include "resource_management/fontmanager.h"

#include "ui_reader/uifactory.h"

CreditsMenue::CreditsMenue()
    : Basemenu()
{
#ifdef GRAPHICSUPPORT
    setObjectName("CreditsMenue");
#endif
    Mainapp* pApp = Mainapp::getInstance();
    pApp->pauseRendering();
    CONSOLE_PRINT("Entering Credits Menue", GameConsole::eDEBUG);
    Interpreter::setCppOwnerShip(this);

    BackgroundManager* pBackgroundManager = BackgroundManager::getInstance();
    // load background
    oxygine::spSprite sprite = MemoryManagement::create<oxygine::Sprite>();
    addChild(sprite);
    oxygine::ResAnim* pBackground = pBackgroundManager->getResAnim("creditsmenu");
    if (pBackground != nullptr)
    {
        sprite->setResAnim(pBackground);
        // background should be last to draw
        sprite->setPriority(static_cast<qint32>(Mainapp::ZOrder::Background));
        sprite->setScaleX(static_cast<float>(oxygine::Stage::getStage()->getWidth()) / static_cast<float>(pBackground->getWidth()));
        sprite->setScaleY(static_cast<float>(oxygine::Stage::getStage()->getHeight()) / static_cast<float>(pBackground->getHeight()));
    }
    pApp->getAudioManager()->clearPlayList();
    pApp->getAudioManager()->loadFolder("resources/music/credits_options");
    pApp->getAudioManager()->playRandom();

    oxygine::spButton pButtonExit = ObjectManager::createButton(tr("Exit"));
    addChild(pButtonExit);
    pButtonExit->setPosition(10,
                             oxygine::Stage::getStage()->getHeight() - pButtonExit->getScaledHeight() - 10);
    pButtonExit->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigExitMenue();
    });
    connect(this, &CreditsMenue::sigExitMenue, this, &CreditsMenue::exitMenue, Qt::QueuedConnection);

    QFile file;
    file.setFileName(VirtualPaths::find("resources/credits/credits.cred"));
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    while (!stream.atEnd())
    {
        QString line = stream.readLine().trimmed();
        QString lowerLine = line.toLower();
        if (line.startsWith("//"))
        {
            continue;
        }
        if (lowerLine.startsWith("headline:"))
        {
            QString headline = line.remove(0, line.indexOf(":") + 1);
            m_Headlines.append(headline);
            m_Authors.append(QStringList());
            while (!stream.atEnd())
            {
                line = stream.readLine();
                line = line.trimmed();
                lowerLine = line.toLower();
                if (lowerLine.startsWith("end"))
                {
                    break;
                }
                else if (line.startsWith("//"))
                {
                    continue;
                }
                else
                {
                    m_Authors[m_Headlines.size() - 1].append(line);
                }
            }
        }
    }
    m_creditsActor = MemoryManagement::create<oxygine::Actor>();
    addChild(m_creditsActor);
    m_creditsActor->setY(oxygine::Stage::getStage()->getHeight());
    qint32 y = 0;
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getFont("mainBlack24"));
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    oxygine::TextStyle headstyle = oxygine::TextStyle(FontManager::getFont("mainBlack48"));
    headstyle.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    headstyle.multiline = false;

    qint32 x = oxygine::Stage::getStage()->getWidth() / 2;
    oxygine::spTextField pTextfield;
    for (qint32 i = 0; i < m_Headlines.size(); i++)
    {
        pTextfield = MemoryManagement::create<oxygine::TextField>();
        pTextfield->setStyle(headstyle);
        pTextfield->setHtmlText(m_Headlines[i]);
        pTextfield->setPosition(x - pTextfield->getTextRect().width() / 2, y);
        m_creditsActor->addChild(pTextfield);
        y += pTextfield->getTextRect().height() + 10;
        for (qint32 i2 = 0; i2 < m_Authors[i].size(); i2++)
        {
            pTextfield = MemoryManagement::create<oxygine::TextField>();
            pTextfield->setStyle(style);
            pTextfield->setHtmlText(m_Authors[i][i2]);
            pTextfield->setPosition(x - pTextfield->getTextRect().width() / 2.0f, y);
            pTextfield->setHeight(pTextfield->getTextRect().height());
            m_creditsActor->addChild(pTextfield);
            y += pTextfield->getTextRect().height() + 5;
        }
    }
    m_creditsHeigth = y;
    m_speedTimer.start();

    UiFactory::getInstance().createUi("ui/creditsmenu.xml", this);
    pApp->continueRendering();
}

void CreditsMenue::doUpdate(const oxygine::UpdateState&)
{
    if (m_speedTimer.elapsed() > 40)
    {
        m_creditsActor->setY(m_creditsActor->getY() - 2);
        m_speedTimer.start();
    }
    if (m_creditsActor->getY() - oxygine::Stage::getStage()->getHeight() / 2.0f + m_creditsHeigth < 0)
    {
        emit sigExitMenue();
    }
}

void CreditsMenue::onEnter()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString object = "Init";
    QString func = "creditsMenu";
    if (pInterpreter->exists(object, func))
    {
        CONSOLE_PRINT("Executing:" + object + "." + func, GameConsole::eDEBUG);
        QJSValueList args({pInterpreter->newQObject(this)});
        pInterpreter->doFunction(object, func, args);
    }
}

void CreditsMenue::exitMenue()
{    
    CONSOLE_PRINT("Leaving Credits Menue", GameConsole::eDEBUG);
    m_onEnterTimer.stop();
    auto window = MemoryManagement::create<Mainwindow>("ui/menu/mainoptionmenu.xml");
    oxygine::Stage::getStage()->addChild(window);
    oxygine::Actor::detach();    
}
