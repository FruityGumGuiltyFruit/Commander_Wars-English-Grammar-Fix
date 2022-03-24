#include <QDir>
#include <QFileInfoList>
#include <QTextStream>
#include <QProcess>
#include <QGuiApplication>
#include <QApplication>
#include <QScreen>

#include "menue/optionmenue.h"
#include "menue/mainwindow.h"

#include "coreengine/mainapp.h"
#include "coreengine/console.h"
#include "coreengine/settings.h"
#include "coreengine/audiothread.h"
#include "coreengine/globalutils.h"
#include "coreengine/Gamepad.h"

#include "game/gamemap.h"

#include "resource_management/backgroundmanager.h"
#include "resource_management/objectmanager.h"
#include "resource_management/fontmanager.h"

#include "objects/base/slider.h"
#include "objects/base/selectkey.h"
#include "objects/base/spinbox.h"
#include "objects/base/textbox.h"
#include "objects/base/label.h"
#include "objects/base/timespinbox.h"
#include "objects/base/moveinbutton.h"
#include "objects/dialogs/dialogmessagebox.h"
#include "objects/dialogs/gamepadinfo.h"

QVector<OptionMenue::GamemodeMods> OptionMenue::m_gamemodeMods =
{
    // enabled                                // disabled
    OptionMenue::GamemodeMods(QStringList(), {"mods/aw_unloading",
                                              "mods/aw2_damage_formula",
                                              "mods/awds_unit",
                                              "mods/awds_co",
                                              "mods/awds_weather",
                                              "mods/awdc_co",
                                              "mods/awdc_unit",
                                              "mods/awdc_powergain",
                                              "mods/awdc_weather",
                                              "mods/awdc_terrain",
                                              "mods/awdc_flare",
                                              "map_creator",
                                              "coop_mod"}),
    OptionMenue::GamemodeMods(
    {"mods/aw_unloading",
     "mods/aw2_damage_formula",
     "mods/awds_unit",
     "mods/awds_co",
     "mods/awds_weather"},
    {"mods/awdc_co",
     "mods/awdc_unit",
     "mods/awdc_powergain",
     "mods/awdc_weather",
     "mods/awdc_terrain",
     "mods/awdc_flare",
     "map_creator",
     "coop_mod"}),
    OptionMenue::GamemodeMods(
    {"mods/aw_unloading",
     "mods/awdc_co",
     "mods/awdc_unit",
     "mods/awdc_powergain",
     "mods/awdc_weather",
     "mods/awdc_terrain",
     "mods/awdc_flare"},
    {"mods/aw2_damage_formula",
     "mods/awds_unit",
     "mods/awds_co",
     "mods/awds_weather",
     "map_creator",
     "coop_mod"})
};

OptionMenue::OptionMenue()
{
    setObjectName("OptionMenue");
    Mainapp* pApp = Mainapp::getInstance();
    pApp->pauseRendering();
    Interpreter::setCppOwnerShip(this);
    moveToThread(pApp->getWorkerthread());
    CONSOLE_PRINT("Entering Option Menue", Console::eDEBUG);

    BackgroundManager* pBackgroundManager = BackgroundManager::getInstance();
    // load background
    oxygine::spSprite sprite = oxygine::spSprite::create();
    addChild(sprite);
    oxygine::ResAnim* pBackground = pBackgroundManager->getResAnim("optionmenu");
    if (pBackground != nullptr)
    {
        sprite->setResAnim(pBackground);
        // background should be last to draw
        sprite->setPriority(static_cast<qint32>(Mainapp::ZOrder::Background));
        sprite->setScaleX(Settings::getWidth() / pBackground->getWidth());
        sprite->setScaleY(Settings::getHeight() / pBackground->getHeight());
    }

    pApp->getAudioThread()->clearPlayList();
    pApp->getAudioThread()->loadFolder("resources/music/credits_options");
    pApp->getAudioThread()->playRandom();


    oxygine::spButton pButtonExit = ObjectManager::createButton(tr("Exit"));
    addChild(pButtonExit);
    pButtonExit->setPosition(Settings::getWidth()  / 2.0f - 10,
                             Settings::getHeight() - pButtonExit->getHeight() - 10);
    pButtonExit->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigExitMenue();
    });
    connect(this, &OptionMenue::sigExitMenue, this, &OptionMenue::exitMenue, Qt::QueuedConnection);

    oxygine::spButton pButtonReset = ObjectManager::createButton(tr("Reset settings"));
    addChild(pButtonReset);
    pButtonReset->setPosition(Settings::getWidth()  / 2.0f - pButtonReset->getWidth() - 10,
                             Settings::getHeight() - pButtonExit->getHeight() - 10);
    pButtonReset->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigShowResetBox();
    });
    connect(this, &OptionMenue::sigShowResetBox, this, &OptionMenue::showResetBox, Qt::QueuedConnection);


    oxygine::spButton pButtonMods = ObjectManager::createButton(tr("Mods"));
    addChild(pButtonMods);
    pButtonMods->setPosition(Settings::getWidth() - pButtonMods->getWidth() - 10, 10);
    pButtonMods->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigShowMods();
    });
    connect(this, &OptionMenue::sigShowMods, this, &OptionMenue::showMods, Qt::QueuedConnection);

    oxygine::spButton pButtonSettings = ObjectManager::createButton(tr("Settings"));
    addChild(pButtonSettings);
    pButtonSettings->setPosition(10, 10);
    pButtonSettings->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigShowSettings();
    });
    connect(this, &OptionMenue::sigShowSettings, this, &OptionMenue::showSettings, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigChangeScreenSize, pApp, &Mainapp::changeScreenSize, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigReloadSettings, this, &OptionMenue::reloadSettings, Qt::QueuedConnection);
    connect(pApp, &Mainapp::sigWindowLayoutChanged, this, &OptionMenue::reloadSettings, Qt::QueuedConnection);

    oxygine::spButton pButtonGameplayAndKeys = ObjectManager::createButton(tr("Gameplay &amp; Keys"), 220);
    addChild(pButtonGameplayAndKeys);
    pButtonGameplayAndKeys->setPosition(Settings::getWidth()  / 2.0f - pButtonExit->getWidth() / 2.0f, 10);
    pButtonGameplayAndKeys->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigShowGameplayAndKeys();
    });
    connect(this, &OptionMenue::sigShowGameplayAndKeys, this, &OptionMenue::showGameplayAndKeys, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigUpdateModCheckboxes, this, &OptionMenue::updateModCheckboxes, Qt::QueuedConnection);

    QSize size(Settings::getWidth() - 20,
               Settings::getHeight() - static_cast<qint32>(20 + pButtonMods->getHeight()) * 2);
    m_pOptions = spPanel::create(true,  size, size);
    m_pOptions->setPosition(10, 20 + pButtonMods->getHeight());
    if (size.width() < 750)
    {
        m_pOptions->setContentWidth(750);
    }
    addChild(m_pOptions);

    m_pGameplayAndKeys = spGameplayAndKeys::create(size.height());
    m_pGameplayAndKeys->setPosition(10, 20 + pButtonMods->getHeight());
    addChild(m_pGameplayAndKeys);

    size.setWidth(Settings::getWidth() - 60);
    size.setHeight(size.height() - 50);
    m_pMods = spPanel::create(true,  size - QSize(0, 50), size);
    m_pMods->setPosition(10, 20 + pButtonMods->getHeight() + 100);
    addChild(m_pMods);
    size.setHeight(size.height() + 70);
    m_pModDescription = spPanel::create(true,  size, size, "panel");
    m_pModDescription->setPosition(Settings::getWidth() - 1, 25);
    auto moveInButton = spMoveInButton::create(m_pModDescription.get(), m_pModDescription->getScaledWidth());
    m_pModDescription->addChild(moveInButton);
    addChild(m_pModDescription);
    m_ModSelector = oxygine::spActor::create();
    m_ModSelector->setPosition(10, 20 + pButtonMods->getHeight());
    connect(this, &OptionMenue::sigUpdateModFilter, this, &OptionMenue::updateModFilter, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigLoadModInfo, this, &OptionMenue::loadModInfo, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigShowResetBox, this, &OptionMenue::showResetBox, Qt::QueuedConnection);

    addChild(m_ModSelector);
    showSettings();
    pApp->continueRendering();
}

void OptionMenue::onEnter()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString object = "Init";
    QString func = "optionMenu";
    if (pInterpreter->exists(object, func))
    {
        CONSOLE_PRINT("Executing:" + object + "." + func, Console::eDEBUG);
        QJSValueList args({pInterpreter->newQObject(this)});
        pInterpreter->doFunction(object, func, args);
    }
}

void OptionMenue::exitMenue()
{    
    // save changed settings :)
    Settings::saveSettings();
    if (m_restartNeeded)
    {
        spDialogMessageBox pMessage = spDialogMessageBox::create("Some changes need a restart of the game. The game will restart. Press Ok to restart.", true);
        connect(pMessage.get(), &DialogMessageBox::sigOk, this, &OptionMenue::restart, Qt::QueuedConnection);
        addChild(pMessage);
    }
    else
    {
        CONSOLE_PRINT("Leaving Option Menue", Console::eDEBUG);
        auto window = spMainwindow::create();
        oxygine::Stage::getStage()->addChild(window);
        oxygine::Actor::detach();
    }
}

void OptionMenue::showGameplayAndKeys()
{    
    m_pOptions->setVisible(false);
    m_pMods->setVisible(false);
    m_pModDescription->setVisible(false);
    m_ModSelector->setVisible(false);
    m_pGameplayAndKeys->setVisible(true);
}

void OptionMenue::reloadSettings()
{    
    CONSOLE_PRINT("Leaving Option Menue", Console::eDEBUG);
    spOptionMenue newMenu = spOptionMenue::create();
    // carry over restart flag
    newMenu->m_restartNeeded = m_restartNeeded;
    oxygine::Stage::getStage()->addChild(newMenu);
    oxygine::Actor::detach();
}

void OptionMenue::showSettings()
{    
    m_pOptions->setVisible(true);
    m_pMods->setVisible(false);
    m_pModDescription->setVisible(false);
    m_ModSelector->setVisible(false);
    m_pGameplayAndKeys->setVisible(false);
    m_pOptions->clearContent();
    Mainapp* pApp = Mainapp::getInstance();
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    qint32 y = 10;
    // cache all supported display modes
    // we're cheating here little bit since qt offers no way to get the screen resolutions from the hardware driver
    QVector<QSize> supportedSizes = {QSize(15360, 8640),
                                     QSize(8192 , 8192),
                                     QSize(10240, 4320),
                                     QSize(8192 , 4608),
                                     QSize(8192 , 4320),
                                     QSize(7680 , 4800),
                                     QSize(7680 , 4320),
                                     QSize(6400 , 4800),
                                     QSize(6400 , 4096),
                                     QSize(5120 , 4096),
                                     QSize(5120 , 3200),
                                     QSize(5120 , 2880),
                                     QSize(5120 , 2160),
                                     QSize(4500 , 3000),
                                     QSize(4096 , 3072),
                                     QSize(4096 , 2160),
                                     QSize(3840 , 2400),
                                     QSize(3840 , 2160),
                                     QSize(3840 , 1600),
                                     QSize(3440 , 1440),
                                     QSize(3240 , 2160),
                                     QSize(3200 , 2400),
                                     QSize(3200 , 2048),
                                     QSize(3200 , 1800),
                                     QSize(3000 , 2000),
                                     QSize(2960 , 1440),
                                     QSize(2880 , 1800),
                                     QSize(2880 , 1620),
                                     QSize(2880 , 1440),
                                     QSize(2880 , 900 ),
                                     QSize(2800 , 2100),
                                     QSize(2736 , 1824),
                                     QSize(2732 , 2048),
                                     QSize(2560 , 2048),
                                     QSize(2560 , 1920),
                                     QSize(2560 , 1800),
                                     QSize(2560 , 1700),
                                     QSize(2560 , 1600),
                                     QSize(2560 , 1440),
                                     QSize(2560 , 1080),
                                     QSize(2538 , 1080),
                                     QSize(2436 , 1125),
                                     QSize(2304 , 1728),
                                     QSize(2256 , 1504),
                                     QSize(2304 , 1440),
                                     QSize(2280 , 1080),
                                     QSize(2160 , 1440),
                                     QSize(2160 , 1200),
                                     QSize(2160 , 1080),
                                     QSize(2048 , 1536),
                                     QSize(2048 , 1280),
                                     QSize(2048 , 1152),
                                     QSize(2048 , 1080),
                                     QSize(1920 , 1440),
                                     QSize(1920 , 1400),
                                     QSize(1920 , 1280),
                                     QSize(1920 , 1200),
                                     QSize(1920 , 1080),
                                     QSize(1856 , 1392),
                                     QSize(1800 , 1440),
                                     QSize(1792 , 1344),
                                     QSize(1776 , 1000),
                                     QSize(1680 , 1050),
                                     QSize(1600 , 1280),
                                     QSize(1600 , 1200),
                                     QSize(1600 , 1024),
                                     QSize(1600 , 900 ),
                                     QSize(1600 , 768 ),
                                     QSize(1440 , 1440),
                                     QSize(1440 , 1080),
                                     QSize(1440 , 1024),
                                     QSize(1440 , 960 ),
                                     QSize(1440 , 900 ),
                                     QSize(1400 , 1050),
                                     QSize(1366 , 768 ),
                                     QSize(1334 , 750 ),
                                     QSize(1280 , 1024),
                                     QSize(1280 , 960 ),
                                     QSize(1280 , 854 ),
                                     QSize(1280 , 800 ),
                                     QSize(1280 , 768 ),
                                     QSize(1280 , 720 ),
                                     QSize(1152 , 900 ),
                                     QSize(1152 , 864 ),
                                     QSize(1152 , 768 ),
                                     QSize(1152 , 720 ),
                                     QSize(1136 , 640 ),
                                     QSize(1120 , 832 ),
                                     QSize(1080 , 1200),
                                     QSize(1024 , 1024),
                                     QSize(1024 , 800 ),
                                     QSize(1024 , 768 ),
                                     QSize(1024 , 640 ),
                                     QSize(1024 , 600 ),
                                     QSize(960 ,  720 ),
                                     QSize(960 ,  640 )};
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenSize;
    if (Settings::getFullscreen())
    {
        screenSize  = screen->geometry();
    }
    else
    {
        screenSize  = screen->availableGeometry();
    }
    supportedSizes.push_front(screenSize.size());
    qint32 count = 0;
    while  (count < supportedSizes.size())
    {
        if (supportedSizes[count].width() <= screenSize.width() &&
            supportedSizes[count].height() <= screenSize.height())
        {
            count++;
        }
        else
        {
            supportedSizes.removeAt(count);
        }
    }
    QStringList displaySizes;
    qint32 currentDisplayMode = 0;
    for  (qint32 i = 0; i < supportedSizes.size(); i++)
    {
        if (supportedSizes[i].width() == Settings::getWidth() &&
            supportedSizes[i].height() == Settings::getHeight())
        {
            currentDisplayMode = i;
        }
        displaySizes.append(QString::number(supportedSizes[i].width()) + " x " + QString::number(supportedSizes[i].height()));
    }
    qint32 sliderOffset = 400;

    spLabel pTextfield = spLabel::create(800);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Screen Settings"));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    y += 40;


    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Screen Resolution: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    spDropDownmenu pScreenResolution = spDropDownmenu::create(400, displaySizes);
    pScreenResolution->setPosition(sliderOffset - 130, y);
    pScreenResolution->setCurrentItem(currentDisplayMode);
    pScreenResolution->setTooltipText(tr("Selects the screen resolution for the game"));
    m_pOptions->addItem(pScreenResolution);
    auto* pPtrScreenResolution = pScreenResolution.get();
    connect(pScreenResolution.get(), &DropDownmenu::sigItemChanged, this, [this, pPtrScreenResolution](qint32)
    {
        QStringList itemData = pPtrScreenResolution->getCurrentItemText().split(" x ");
        qint32 width = itemData[0].toInt();
        qint32 heigth = itemData[1].toInt();
        Settings::setWidth(width);
        Settings::setHeight(heigth);
        emit sigChangeScreenSize(width, heigth);
    });
    pScreenResolution->setEnabled(!Settings::getSmallScreenDevice());
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Screen Mode: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    QStringList items = {tr("Window"), tr("Bordered"), tr("Fullscreen")};
    spDropDownmenu pScreenModes = spDropDownmenu::create(400, items);
    pScreenModes->setTooltipText(tr("Selects the screen mode for the game"));
    pScreenModes->setPosition(sliderOffset - 130, y);
    pScreenModes->setCurrentItem(pApp->getScreenMode());
    pScreenModes->setEnabled(!Settings::getSmallScreenDevice());
    m_pOptions->addItem(pScreenModes);
    connect(pScreenModes.get(), &DropDownmenu::sigItemChanged, pApp, &Mainapp::changeScreenMode, Qt::QueuedConnection);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Brightness: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    spSlider pSlider = spSlider::create(Settings::getWidth() - 20 - sliderOffset, -50, 50);
    pSlider->setTooltipText(tr("Selects the brightness for the game"));
    pSlider->setPosition(sliderOffset - 130, y);
    pSlider->setCurrentValue(Settings::getBrightness());
    connect(pSlider.get(), &Slider::sliderValueChanged, this, [=](qint32 value)
    {
        Settings::setBrightness(-value);
        pApp->setBrightness(-value);
    });
    m_pOptions->addItem(pSlider);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Gamma: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    pSlider = spSlider::create(Settings::getWidth() - 20 - sliderOffset, 1, 160, "");
    pSlider->setTooltipText(tr("Selects the gamma factor for the game"));
    pSlider->setPosition(sliderOffset - 130, y);
    pSlider->setCurrentValue(Settings::getGamma() * 30.0f);
    connect(pSlider.get(), &Slider::sliderValueChanged, this, [=](qint32 value)
    {
        Settings::setGamma(value / 30.0f);
        pApp->setGamma(value / 30.0f);
    });
    m_pOptions->addItem(pSlider);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Small screen: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    spCheckbox pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("If checked several ui elements are hidden and get shown with an additional button.\nWarning disabling this on a smaller screen may lead to unplayable game experience."));
    pCheckbox->setChecked(Settings::getSmallScreenDevice());
    pCheckbox->setPosition(sliderOffset - 130, y);
    connect(pCheckbox.get(), &Checkbox::checkChanged, Settings::getInstance(), &Settings::setSmallScreenDevice, Qt::QueuedConnection);
    if (Settings::hasSmallScreen())
    {
        pCheckbox->setEnabled(false);
    }
    else
    {
        pCheckbox->setEnabled(true);
    }
    m_pOptions->addItem(pCheckbox);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Max FPS: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    pSlider = spSlider::create(Settings::getWidth() - 20 - sliderOffset, 30, 60, "");
    pSlider->setTooltipText(tr("Selects the maximum FPS use it to reduce power consumption on smartphones."));
    pSlider->setPosition(sliderOffset - 130, y);
    pSlider->setCurrentValue(Settings::getFramesPerSecond());
    connect(pSlider.get(), &Slider::sliderValueChanged, Settings::getInstance(), &Settings::setFramesPerSecond, Qt::QueuedConnection);
    m_pOptions->addItem(pSlider);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Touch screen: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("If checked some ingame inputs require double clicks or other gestures. This is automatically detected and changing it isn't recommended."));
    pCheckbox->setChecked(Settings::getTouchScreen());
    pCheckbox->setPosition(sliderOffset - 130, y);
    connect(pCheckbox.get(), &Checkbox::checkChanged, Settings::getInstance(), &Settings::setTouchScreen, Qt::QueuedConnection);
    m_pOptions->addItem(pCheckbox);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Touch Sensitivity: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    spSpinBox touchPointSensitivity = spSpinBox::create(200, 0, std::numeric_limits<quint16>::max());
    touchPointSensitivity->setTooltipText(tr("Selects how long a touch is treated as the same point. Used for detecting long press events."));
    touchPointSensitivity->setCurrentValue(Settings::getTouchPointSensitivity());
    touchPointSensitivity->setPosition(sliderOffset - 130, y);
    connect(touchPointSensitivity.get(), &SpinBox::sigValueChanged, Settings::getInstance(), &Settings::setTouchPointSensitivity);
    m_pOptions->addItem(touchPointSensitivity);
    y += 40;

    if (Gamepad::isSupported())
    {
        pTextfield = spLabel::create(sliderOffset - 140);
        pTextfield->setStyle(style);
        pTextfield->setHtmlText(tr("Gamepad: "));
        pTextfield->setPosition(10, y);
        m_pOptions->addItem(pTextfield);
        pCheckbox = spCheckbox::create();
        pCheckbox->setTooltipText(tr("Enables Gamepad support for controllers. Note: This is experimental and won't 100% with all controllers and isn't supported for android, iOS, macOS and linux."));
        pCheckbox->setChecked(Settings::getGamepadEnabled());
        pCheckbox->setPosition(sliderOffset - 130, y);
        connect(pCheckbox.get(), &Checkbox::checkChanged, Settings::getInstance(), &Settings::setGamepadEnabled, Qt::QueuedConnection);
        m_pOptions->addItem(pCheckbox);
        // gamepad button
        oxygine::spButton pButtonGamepad = ObjectManager::createButton(tr("Info"), 100);
        pButtonGamepad->setPosition(pCheckbox->getX() + 80, y);
        m_pOptions->addItem(pButtonGamepad);
        pButtonGamepad->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
        {
            emit sigShowGamepadInfo();
        });
        connect(this, &OptionMenue::sigShowGamepadInfo, this, &OptionMenue::showGamepadInfo, Qt::QueuedConnection);
        y += 40;

        pTextfield = spLabel::create(sliderOffset - 140);
        pTextfield->setStyle(style);
        pTextfield->setHtmlText(tr("Gamepad Sensitivity: "));
        pTextfield->setPosition(10, y);
        m_pOptions->addItem(pTextfield);
        spSpinBox gamepadSensitivity = spSpinBox::create(200, 0.1, 100);
        gamepadSensitivity->setTooltipText(tr("Selects how often events are send by the gamepad. Smaller values create a faster cursor."));
        gamepadSensitivity->setCurrentValue(Settings::getGamepadSensitivity());
        gamepadSensitivity->setPosition(sliderOffset - 130, y);
        connect(gamepadSensitivity.get(), &SpinBox::sigValueChanged, Settings::getInstance(), &Settings::setGamepadSensitivity);
        m_pOptions->addItem(gamepadSensitivity);
        y += 40;
    }
    showSoundOptions(m_pOptions, sliderOffset, y, this);

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Language: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    items.clear();
    QLocale english("en");
    items.append(english.nativeLanguageName());
    m_languages.append("en");
    qint32 current = 0;
    QStringList paths = {QString(oxygine::Resource::RCC_PREFIX_PATH) + "resources/translation/", "resources/translation/"};
    QStringList filter;
    filter << "*.qm";
    for (const QString & path : qAsConst(paths))
    {
        QDirIterator dirIter(path, filter, QDir::Files, QDirIterator::Subdirectories);
        while (dirIter.hasNext())
        {
            dirIter.next();
            QString lang = dirIter.fileName().replace(".qm", "").replace("lang_", "");
            if (lang != "en")
            {
                m_languages.append(lang);
                QLocale langLoc(lang);
                items.append(langLoc.nativeLanguageName());
                if (lang == Settings::getLanguage())
                {
                    current = items.size() - 1;
                }
            }
        }
    }
    spDropDownmenu pLanguageMenu = spDropDownmenu::create(400, items);
    pLanguageMenu->setTooltipText(tr("Selects the language for the game. Note: Not everything may be translated."));
    pLanguageMenu->setPosition(sliderOffset - 130, y);
    pLanguageMenu->setCurrentItem(current);
    m_pOptions->addItem(pLanguageMenu);
    connect(pLanguageMenu.get(), &DropDownmenu::sigItemChanged, pApp,
            [this](qint32 item)
    {
        CONSOLE_PRINT("Marking restart cause language changed.", Console::eDEBUG);
        Settings::setLanguage(m_languages[item]);
        m_restartNeeded = true;
        emit sigReloadSettings();
    });
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Auto Saving Time: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    spTimeSpinBox autoSavingCycleTime = spTimeSpinBox::create(200);
    autoSavingCycleTime->setTooltipText(tr("Selects the auto saving cycle in hours:minutes:seconds"));
    autoSavingCycleTime->setCurrentValue(std::chrono::duration_cast<std::chrono::milliseconds>(Settings::getAutoSavingCylceTime()).count());
    autoSavingCycleTime->setPosition(sliderOffset - 130, y);
    connect(autoSavingCycleTime.get(), &TimeSpinBox::sigValueChanged, [=](qint32 value)
    {
        Settings::setAutoSavingCylceTime(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(value)));
    });
    m_pOptions->addItem(autoSavingCycleTime);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Auto Saving Cycle: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    spSpinBox autoSavingCycle = spSpinBox::create(200, 0, std::numeric_limits<quint16>::max());
    autoSavingCycle->setTooltipText(tr("Selects the amount of auto save games that get cycled through while auto saving. A value 0 disables this feature."));
    autoSavingCycle->setCurrentValue(Settings::getAutoSavingCycle());
    autoSavingCycle->setPosition(sliderOffset - 130, y);
    connect(autoSavingCycle.get(), &SpinBox::sigValueChanged, Settings::getInstance(), &Settings::setAutoSavingCycle);
    m_pOptions->addItem(autoSavingCycle);
    y += 40;


    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Record Games: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("If checked games will be recorded and you can rewatch them in the replay section."));
    pCheckbox->setChecked(Settings::getRecord());
    pCheckbox->setPosition(sliderOffset - 130, y);
    connect(pCheckbox.get(), &Checkbox::checkChanged, Settings::getInstance(), &Settings::setRecord, Qt::QueuedConnection);
    m_pOptions->addItem(pCheckbox);
    y += 40;

    pTextfield = spLabel::create(800);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Network Settings"));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Username: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    spTextbox pTextbox = spTextbox::create(Settings::getWidth() - 20 - sliderOffset);
    pTextbox->setTooltipText(tr("Selects your username shown at various places of the game"));
    pTextbox->setCurrentText(Settings::getUsername());
    Textbox* pPtrTextbox = pTextbox.get();
    connect(pTextbox.get(), &Textbox::sigTextChanged, this, [this, pPtrTextbox](QString value)
    {
        if (value.isEmpty())
        {
            pPtrTextbox->setCurrentText(Settings::getUsername());
        }
        else
        {
            if (value != Settings::getUsername())
            {
                CONSOLE_PRINT("Marking restart cause user changed.", Console::eDEBUG);
                m_restartNeeded = true;
            }
            Settings::setUsername(value);
        }
    });
    pTextbox->setPosition(sliderOffset - 130, y);
    m_pOptions->addItem(pTextbox);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Dedicated address: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    pTextbox = spTextbox::create(Settings::getWidth() - 20 - sliderOffset);
    pTextbox->setTooltipText(tr("Selects the dedicated game server you wan't to connect to when playing a multiplayer game."));
    pTextbox->setCurrentText(Settings::getServerAdress());
    connect(pTextbox.get(), &Textbox::sigTextChanged, [=](QString value)
    {
        Settings::setServerAdress(value);
    });
    pTextbox->setPosition(sliderOffset - 130, y);
    m_pOptions->addItem(pTextbox);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Dedicated port: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    spSpinBox portBox = spSpinBox::create(200, 0, std::numeric_limits<quint16>::max());
    portBox->setTooltipText(tr("Selects the dedicated servers chat port for used to chat in the lobby of the dedicated server"));
    portBox->setCurrentValue(Settings::getServerPort());
    portBox->setPosition(sliderOffset - 130, y);
    connect(portBox.get(), &SpinBox::sigValueChanged, [=](float value)
    {
        Settings::setServerPort(static_cast<quint16>(value));
    });
    m_pOptions->addItem(portBox);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Dedicated server: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("Enables this game as global server."));
    pCheckbox->setChecked(Settings::getServer());
    connect(pCheckbox.get(), &Checkbox::checkChanged, this, [this](bool value)
    {
        CONSOLE_PRINT("Marking restart cause server settings changed.", Console::eDEBUG);
        Settings::setServer(value);
        m_restartNeeded = true;
    });
    pCheckbox->setPosition(sliderOffset - 130, y);
    m_pOptions->addItem(pCheckbox);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Game port: "));
    pTextfield->setPosition(10, y);
    m_pOptions->addItem(pTextfield);
    portBox = spSpinBox::create(200, 0, std::numeric_limits<quint16>::max());
    portBox->setTooltipText(tr("Selects the game port for used to play the game with the server either with a dedicated server or direct connection."));
    portBox->setCurrentValue(Settings::getGamePort());
    portBox->setPosition(sliderOffset - 130, y);
    connect(portBox.get(), &SpinBox::sigValueChanged, [=](float value)
    {
        Settings::setGamePort(static_cast<quint16>(value));
    });
    m_pOptions->addItem(portBox);
    y += 40;

    m_pOptions->setContentHeigth(20 + y);
}

void OptionMenue::showSoundOptions(spPanel pOwner, qint32 sliderOffset, qint32 & y, QObject* pSignalOwner)
{    
#ifdef AUDIOSUPPORT
    AudioThread* pAudio = Mainapp::getInstance()->getAudioThread();
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    spLabel pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Audio Settings"));
    pTextfield->setPosition(10, y);
    pOwner->addItem(pTextfield);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Muted: "));
    pTextfield->setPosition(10, y);
    pOwner->addItem(pTextfield);
    spCheckbox pCheckbox = spCheckbox::create();
    pCheckbox->setTooltipText(tr("If checked mutes all sounds and music are muted."));
    pCheckbox->setChecked(Settings::getMuted());
    pCheckbox->setPosition(sliderOffset - 130, y);
    connect(pCheckbox.get(), &Checkbox::checkChanged, Settings::getInstance(), [=](bool checked)
    {
        Settings::setMuted(checked);
        pAudio->setVolume(Settings::getMusicVolume());
    }, Qt::QueuedConnection);
    pOwner->addItem(pCheckbox);
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Audio Device: "));
    pTextfield->setPosition(10, y);
    pOwner->addItem(pTextfield);
    auto currentDevice = Settings::getAudioOutput().value<QAudioDevice>();
    const auto deviceInfos = QMediaDevices::audioOutputs();
    QStringList items;
    qint32 currentItem = 0;

    for (qint32 i = 0; i < deviceInfos.size(); ++i)
    {
        items.append(deviceInfos[i].description());
        if (deviceInfos[i] == currentDevice)
        {
            currentItem = i;
        }
    }
    spDropDownmenu pAudioDevice = spDropDownmenu::create(Settings::getWidth() - 20 - sliderOffset, items);
    pAudioDevice->setTooltipText(tr("Selects the primary audio output device"));
    pAudioDevice->setPosition(sliderOffset - 130, y);
    pAudioDevice->setCurrentItem(currentItem);
    pAudioDevice->setEnabled(!Settings::getSmallScreenDevice());
    pOwner->addItem(pAudioDevice);
    connect(pAudioDevice.get(), &DropDownmenu::sigItemChanged, pSignalOwner, [=](qint32 value)
    {
        auto item = QVariant::fromValue(deviceInfos[value]);
        Settings::setAudioOutput(item);
        pAudio->changeAudioDevice(item);
    });
    y += 40;

    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Global Volume: "));
    pTextfield->setPosition(10, y);
    pOwner->addItem(pTextfield);
    spSlider pSlider = spSlider::create(Settings::getWidth() - 20 - sliderOffset, 0, 100);
    pSlider->setTooltipText(tr("Selects the global volume for the game"));
    pSlider->setPosition(sliderOffset - 130, y);
    pSlider->setCurrentValue(Settings::getTotalVolume());
    connect(pSlider.get(), &Slider::sliderValueChanged, pSignalOwner, [=](qint32 value)
    {
        Settings::setTotalVolume(value);
        pAudio->setVolume(Settings::getMusicVolume());
    });
    pOwner->addItem(pSlider);

    y += 40;
    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Music Volume: "));
    pTextfield->setPosition(10, y);
    pOwner->addItem(pTextfield);
    pSlider = spSlider::create(Settings::getWidth() - 20 - sliderOffset, 0, 100);
    pSlider->setTooltipText(tr("Selects the music volume for the game"));
    pSlider->setPosition(sliderOffset - 130, y);
    pSlider->setCurrentValue(Settings::getMusicVolume());
    connect(pSlider.get(), &Slider::sliderValueChanged, pSignalOwner, [=](qint32 value)
    {
        Settings::setMusicVolume(value);
        pAudio->setVolume(value);
    });
    pOwner->addItem(pSlider);

    y += 40;
    pTextfield = spLabel::create(sliderOffset - 140);
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(tr("Sound Volume: "));
    pTextfield->setPosition(10, y);
    pOwner->addItem(pTextfield);
    pSlider = spSlider::create(Settings::getWidth() - 20 - sliderOffset, 0, 100);
    pSlider->setTooltipText(tr("Selects the sound volume for the game"));
    pSlider->setPosition(sliderOffset - 130, y);
    pSlider->setCurrentValue(Settings::getSoundVolume());
    connect(pSlider.get(), &Slider::sliderValueChanged, [=](qint32 value)
    {
        Settings::setSoundVolume(value);
    });
    pOwner->addItem(pSlider);
    y += 40;
#endif
}

void OptionMenue::showMods()
{    
    Mainapp* pApp = Mainapp::getInstance();
    pApp->pauseRendering();
    m_pMods->clearContent();
    m_pModDescription->clearContent();
    m_ModBoxes.clear();
    m_ModCheckboxes.clear();
    m_ModSelector->removeChildren();

    m_pOptions->setVisible(false);
    m_pMods->setVisible(true);
    m_ModSelector->setVisible(true);
    m_pModDescription->setVisible(true);
    m_pGameplayAndKeys->setVisible(false);
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = true;
    m_ModDescriptionText = oxygine::spTextField::create();
    m_ModDescriptionText->setStyle(style);
    m_ModDescriptionText->setSize(m_pModDescription->getContentWidth() - 60, 500);
    m_modThumbnail = oxygine::spSprite::create();
    m_modThumbnail->setPosition(5, 5);
    m_pModDescription->addItem(m_modThumbnail);
    m_pModDescription->addItem(m_ModDescriptionText);

    spLabel pLabel = spLabel::create(250);
    style.multiline = false;
    pLabel->setStyle(style);
    pLabel->setHtmlText(tr("Advance Wars Game:"));
    m_ModSelector->addChild(pLabel);
    qint32 y = 0;
    QStringList versions = {tr("Unkown"),
                                 tr("Commander Wars"),
                                 tr("Advance Wars DS"),
                                 tr("Advance Wars DC")};
    m_pModSelection = spDropDownmenu::create(300, versions);
    m_pModSelection->setTooltipText(tr("Select an Advance Wars Game to preselect all mods which are required to play like this Advance Wars Game"));
    m_pModSelection->setX(260);
    connect(m_pModSelection.get(), &DropDownmenu::sigItemChanged, this, &OptionMenue::selectMods, Qt::QueuedConnection);
    m_ModSelector->addChild(m_pModSelection);
    updateModSelection();
    y += 50;
    pLabel = spLabel::create(250);
    style.multiline = false;
    pLabel->setStyle(style);
    pLabel->setHtmlText(tr("Tag Filter:"));
    pLabel->setY(y);
    m_ModSelector->addChild(pLabel);
    QStringList tags;
    QStringList currentMods = Settings::getMods();
    qint32 width = 0;
    qint32 mods = 0;
    style.multiline = false;
    QStringList availableMods = Settings::getAvailableMods();
    for (const auto & mod : qAsConst(availableMods))
    {
        QString name;
        QString description;
        QString version;
        QStringList compatibleMods;
        QStringList incompatibleMods;
        QStringList requiredMods;
        bool isComsetic = false;
        QStringList modTags;
        QString thumbnail;
        Settings::getModInfos(mod, name, description, version,
                              compatibleMods, incompatibleMods, requiredMods, isComsetic,
                              modTags, thumbnail);
        for (const auto & tag : qAsConst(modTags))
        {
            if (!tags.contains(tag))
            {
                tags.append(tag);
            }
        }
        oxygine::ResAnim* pAnim = pObjectManager->getResAnim("topbar+dropdown");
        oxygine::spBox9Sprite pBox = oxygine::spBox9Sprite::create();
        pBox->setResAnim(pAnim);

        spCheckbox modCheck = spCheckbox::create();
        m_ModCheckboxes.append(modCheck);
        modCheck->setPosition(10, 5);
        pBox->addChild(modCheck);

        spLabel pTextfield = spLabel::create(Settings::getWidth() - 190);
        pTextfield->setStyle(style);
        pTextfield->setHtmlText(name);
        pTextfield->setPosition(50, 5);
        pBox->addChild(pTextfield);
        if (currentMods.contains(mod))
        {
            modCheck->setChecked(true);
        }
        connect(modCheck.get(), &Checkbox::checkChanged, this, [this, mod](bool checked)
        {
            if (checked)
            {
                Settings::addMod(mod);
            }
            else
            {
                Settings::removeMod(mod);
            }
            m_restartNeeded = true;
            emit sigUpdateModCheckboxes();
        });
        pBox->setPosition(10, 10 + mods * 50);
        pBox->setSize(Settings::getWidth() - 130, 50);

        auto* pPtrBox = pBox.get();
        pBox->addClickListener([this, pPtrBox, name, description, version,
                               compatibleMods, incompatibleMods, requiredMods, isComsetic,
                               modTags, thumbnail](oxygine::Event* pEvent)
        {
            pEvent->stopPropagation();
            sigLoadModInfo(pPtrBox, name, description, version,
                           compatibleMods, incompatibleMods, requiredMods, isComsetic,
                           modTags, thumbnail);
        });
        m_ModBoxes.append(pBox);
        m_pMods->addItem(pBox);
        mods++;
    }
    m_pMods->setContentWidth(width);
    m_pMods->setContentHeigth(50 + mods * 50);

    tags.sort();
    tags.push_front(tr("All"));
    spDropDownmenu pTagSelection = spDropDownmenu::create(300, tags);
    pTagSelection->setTooltipText(tr("Filters the mods by the given tags"));
    pTagSelection->setPosition(260, y);
    connect(pTagSelection.get(), &DropDownmenu::sigItemChanged, this, [this, tags](qint32 value)
    {
        QString tag;
        if (value > 0)
        {
            tag = tags[value];
        }
        emit sigUpdateModFilter(tag);
    });
    m_ModSelector->addChild(pTagSelection);
    updateModFilter("");
    updateModCheckboxes();
    pApp->continueRendering();
}

void OptionMenue::updateModSelection()
{
    QStringList currentMods = Settings::getMods();
    qint32 index = 0;
    bool set = false;
    for (const auto & gameMode : qAsConst(m_gamemodeMods))
    {
        bool valid = (currentMods.size() == gameMode.m_enableMods.size());
        for (const auto & activeMod : gameMode.m_enableMods)
        {
            if (!currentMods.contains(activeMod))
            {
                valid = false;
            }
        }
        if (valid)
        {
            set = true;
            m_pModSelection->setCurrentItem(index + 1);
            break;
        }
        ++index;
    }
    if (!set)
    {
        m_pModSelection->setCurrentItem(0);
    }
}

void OptionMenue::loadModInfo(oxygine::Box9Sprite* pPtrBox,
                              QString name, QString description, QString version,
                              QStringList compatibleMods, QStringList incompatibleMods, QStringList requiredMods,
                              bool isComsetic, QStringList modTags, QString thumbnail)
{
    for (qint32 i2 = 0; i2 < m_ModBoxes.size(); i2++)
    {
        m_ModBoxes[i2]->addTween(oxygine::Sprite::TweenAddColor(QColor(0, 0, 0, 0)), oxygine::timeMS(300));
    }
    pPtrBox->addTween(oxygine::Sprite::TweenAddColor(QColor(32, 200, 32, 0)), oxygine::timeMS(300));
    qint32 y = 0;
    if (!thumbnail.isEmpty())
    {
        QImage img;
        if (QFile::exists(Settings::getUserPath() + thumbnail))
        {
            img = QImage(Settings::getUserPath() + thumbnail);
        }
        else if (QFile::exists(oxygine::Resource::RCC_PREFIX_PATH + thumbnail))
        {
            img = QImage(oxygine::Resource::RCC_PREFIX_PATH + thumbnail);
        }
        else
        {
            oxygine::handleErrorPolicy(oxygine::ep_show_error, "unable to locate thumbnail for mod " + name);
        }
        m_modThumbnail->setVisible(true);
        m_modThumbnailAnim = oxygine::spSingleResAnim::create();
        Mainapp::getInstance()->loadResAnim(m_modThumbnailAnim, img, 1, 1, 1.0f, false);
        m_modThumbnail->setResAnim(m_modThumbnailAnim.get());
        if (m_modThumbnailAnim->getWidth() > m_pModDescription->getContentWidth() - 60)
        {
            m_modThumbnail->setScale(static_cast<float>(m_pModDescription->getContentWidth() - 60) / static_cast<float>(m_modThumbnailAnim->getWidth()));
        }
        y += m_modThumbnail->getY() + m_modThumbnail->getScaledHeight() + 10;
    }
    else
    {
        m_modThumbnail->setVisible(false);
    }
    QString cosmeticInfo;
    if (isComsetic)
    {
        cosmeticInfo = QString("\n\n") + tr("The mod is claimed to be pure cosmetic by the creator and may be used during multiplayer games based on the game rules.");
    }
    QString modInfo = "\n\n" + tr("Compatible Mods:\n");
    for (const auto & mod : compatibleMods)
    {
        modInfo += Settings::getModName(mod) + "\n";
    }
    modInfo += "\n" + tr("Incompatible Mods:\n");
    for (const auto & mod : incompatibleMods)
    {
        modInfo += Settings::getModName(mod) + "\n";
    }
    modInfo += "\n" + tr("Required Mods:\n");
    for (const auto & mod : requiredMods)
    {
        modInfo += Settings::getModName(mod) + "\n";
    }
    modInfo += "\n" + tr("Tags:\n");
    for (const auto & tag : modTags)
    {
        modInfo += tag + "\n";
    }
    m_ModDescriptionText->setHtmlText(description + cosmeticInfo + modInfo + "\n\n" + tr("Version: ") + version);
    m_ModDescriptionText->setHeight(m_ModDescriptionText->getTextRect().getHeight());
    m_ModDescriptionText->setY(y);
    m_pModDescription->setContentHeigth(y + m_ModDescriptionText->getTextRect().getHeight() + 40);
}

void OptionMenue::selectMods(qint32 item)
{    
    if (item > 0)
    {
        QStringList removeList;
        QStringList addList;
        removeList = m_gamemodeMods[item - 1].m_disableMods;
        addList = m_gamemodeMods[item - 1].m_enableMods;
        for (auto & removeMod : removeList)
        {
            Settings::removeMod(removeMod);
        }
        for (auto & addMod : addList)
        {
            Settings::addMod(addMod);
        }
        CONSOLE_PRINT("Marking restart cause mods changed.", Console::eDEBUG);
        m_restartNeeded = true;
        showMods();
    }
}

void OptionMenue::restart()
{
    CONSOLE_PRINT("Forcing restart to reload required data changed in the options.", Console::eDEBUG);
    removeChildren();
    detach();
    emit Mainapp::getInstance()->sigQuit(1);
}

void OptionMenue::updateModCheckboxes()
{
    const auto availableMods = Settings::getAvailableMods();
    const auto mods = Settings::getActiveMods();
    for (auto & checkbox : m_ModCheckboxes)
    {
        checkbox->setEnabled(true);
    }
    for (const auto & mod : mods)
    {
        QString name;
        QString description;
        QString version;
        QStringList compatibleMods;
        QStringList incompatibleMods;
        QStringList requiredMods;
        QStringList tags;
        QString thumbnail;
        bool isComsetic = false;
        Settings::getModInfos(mod, name, description, version,
                              compatibleMods, incompatibleMods, requiredMods, isComsetic, tags, thumbnail);
        qint32 i2 = 0;
        for (const auto & checkBoxMod : qAsConst(availableMods))
        {
            if (incompatibleMods.contains(checkBoxMod))
            {
                m_ModCheckboxes[i2]->setEnabled(false);
            }
            ++i2;
        }
    }
    qint32 i = 0;
    for (const auto & mod : qAsConst(availableMods))
    {
        QString name;
        QString description;
        QString version;
        QStringList compatibleMods;
        QStringList incompatibleMods;
        QStringList requiredMods;
        QStringList tags;
        QString thumbnail;
        bool isComsetic = false;
        Settings::getModInfos(mod, name, description, version,
                              compatibleMods, incompatibleMods, requiredMods, isComsetic, tags, thumbnail);
        for (const auto & incompatibleMod : qAsConst(incompatibleMods))
        {
            if (mods.contains(incompatibleMod))
            {
                m_ModCheckboxes[i]->setEnabled(false);
                break;
            }
        }
        if (m_ModCheckboxes[i]->getEnabled())
        {
            for (const auto & requiredMod : qAsConst(requiredMods))
            {
                if (!mods.contains(requiredMod))
                {
                    if (m_ModCheckboxes[i]->getChecked())
                    {
                        m_ModCheckboxes[i]->setChecked(false);
                        Settings::removeMod(mod);
                    }
                    m_ModCheckboxes[i]->setEnabled(false);
                    break;
                }
            }
        }
        ++i;
    }
    updateModSelection();
}

void OptionMenue::updateModFilter(QString tag)
{
    const auto mods = Settings::getAvailableMods();
    qint32 visibleCounter = 0;
    for (qint32 i = 0; i < m_ModBoxes.size(); ++i)
    {
        QString name;
        QString description;
        QString version;
        QStringList compatibleMods;
        QStringList incompatibleMods;
        QStringList requiredMods;
        QStringList tags;
        QString thumbnail;
        bool isComsetic = false;
        Settings::getModInfos(mods[i], name, description, version,
                              compatibleMods, incompatibleMods, requiredMods, isComsetic, tags, thumbnail);


        if (tag.isEmpty() ||
            tags.contains(tag))
        {
            m_ModBoxes[i]->setY(visibleCounter * 50);
            m_ModBoxes[i]->setVisible(true);
            ++visibleCounter;
        }
        else
        {
            m_ModBoxes[i]->setVisible(false);
        }
    }
    m_pMods->setContentHeigth(50 + visibleCounter * 50);
}

void OptionMenue::showGamepadInfo()
{
    spGamepadInfo pGamepadInfo = spGamepadInfo::create();
    addChild(pGamepadInfo);
}

void OptionMenue::showResetBox()
{
    spDialogMessageBox pMessage = spDialogMessageBox::create("This will reset most settings including mods and key bindings. Press Ok to reset the settings. This will force a restart upon leaving this menu. ", true);
    connect(pMessage.get(), &DialogMessageBox::sigOk, this, &OptionMenue::onReset, Qt::QueuedConnection);
    addChild(pMessage);
}

void OptionMenue::onReset()
{
    Settings::resetSettings();
    m_restartNeeded = true;
    reloadSettings();
}
