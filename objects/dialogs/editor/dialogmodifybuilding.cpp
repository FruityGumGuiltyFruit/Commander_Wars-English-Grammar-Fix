#include "3rd_party/oxygine-framework/oxygine/actor/Stage.h"

#include "dialogmodifybuilding.h"

#include "coreengine/interpreter.h"

#include "resource_management/objectmanager.h"

#include "resource_management/fontmanager.h"

#include "objects/base/label.h"
#include "objects/base/textbox.h"
#include "objects/base/slider.h"
#include "objects/base/dropdownmenu.h"

#include "game/gamemap.h"

DialogModifyBuilding::DialogModifyBuilding(GameMap* pMap, Building* pBuilding)
    : m_pBuilding(pBuilding)
{
#ifdef GRAPHICSUPPORT
    setObjectName("DialogModifyBuilding");
#endif
    Interpreter::setCppOwnerShip(this);
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::spBox9Sprite pSpriteBox = MemoryManagement::create<oxygine::Box9Sprite>();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("codialog");
    pSpriteBox->setResAnim(pAnim);
    pSpriteBox->setSize(oxygine::Stage::getStage()->getWidth(), oxygine::Stage::getStage()->getHeight());
    addChild(pSpriteBox);
    pSpriteBox->setPosition(0, 0);
    pSpriteBox->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    setPriority(static_cast<qint32>(Mainapp::ZOrder::Dialogs));

    // ok button
    m_OkButton = pObjectManager->createButton(tr("Ok"), 150);
    m_OkButton->setPosition(oxygine::Stage::getStage()->getWidth() / 2 - m_OkButton->getScaledWidth() / 2,
                            oxygine::Stage::getStage()->getHeight() - 30 - m_OkButton->getScaledHeight());
    pSpriteBox->addChild(m_OkButton);
    m_OkButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        emit sigFinished();
    });


    m_pPanel = MemoryManagement::create<Panel>(true, QSize(oxygine::Stage::getStage()->getWidth() - 60, oxygine::Stage::getStage()->getHeight() - 110),
                                     QSize(oxygine::Stage::getStage()->getWidth() - 60, oxygine::Stage::getStage()->getHeight() - 110));
    m_pPanel->setPosition(30, 30);
    pSpriteBox->addChild(m_pPanel);

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    oxygine::TextStyle headerStyle = oxygine::TextStyle(FontManager::getMainFont48());
    headerStyle.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    headerStyle.multiline = false;

    qint32 sliderOffset = 400;
    spLabel pLabel = MemoryManagement::create<Label>(m_pPanel->getScaledWidth() - 80);
    pLabel->setStyle(headerStyle);
    pLabel->setHtmlText(tr("Building: ") + m_pBuilding->getName());
    pLabel->setPosition(m_pPanel->getScaledWidth() / 2 - pLabel->getTextRect().width() / 2, 10);
    m_pPanel->addItem(pLabel);

    qint32 y = pLabel->getY() + pLabel->getTextRect().height() + 10;
    pLabel = MemoryManagement::create<Label>(sliderOffset - 140);
    pLabel->setStyle(style);
    pLabel->setHtmlText(tr("Player: "));
    pLabel->setPosition(10, y);
    m_pPanel->addItem(pLabel);
    QStringList items;
    
    for (qint32 i = 0; i < pMap->getPlayerCount(); i++)
    {
        items.append(tr("Player ") + QString::number(i + 1));
    }
    items.append(tr("Neutral"));
    spDropDownmenu pDropdownmenu = MemoryManagement::create<DropDownmenu>(300, items);
    pDropdownmenu->setTooltipText(tr("Selects the Owner of the current unit. This is immediately applied."));
    pDropdownmenu->setPosition(sliderOffset - 160, y);

    Player* pPlayer = m_pBuilding->getOwner();
    if (pPlayer != nullptr)
    {
        pDropdownmenu->setCurrentItem(pPlayer->getPlayerID());
    }
    else
    {
        pDropdownmenu->setCurrentItem(pMap->getPlayerCount());
    }

    connect(pDropdownmenu.get(), &DropDownmenu::sigItemChanged, this, [this, pMap](qint32 value)
    {
        if (value >= pMap->getPlayerCount())
        {
            m_pBuilding->setOwner(nullptr);
        }
        else
        {
            m_pBuilding->setOwner(pMap->getPlayer(value));
        }
    });
    m_pPanel->addItem(pDropdownmenu);
    y += pLabel->getHeight() + 10;

    pLabel = MemoryManagement::create<Label>(190);
    pLabel->setStyle(style);
    pLabel->setHtmlText(tr("Name:"));
    pLabel->setPosition(10, y);
    spTextbox pTextbox = MemoryManagement::create<Textbox>(m_pPanel->getContentWidth() - 100 - 200 - pLabel->getScaledWidth());
    pTextbox->setTooltipText(tr("Custom Name of the Terrain. Leave the name empty to use its Default Name."));
    pTextbox->setPosition(sliderOffset - 160, y);
    pTextbox->setCurrentText(m_pBuilding->getBuildingName());
    connect(pTextbox.get(), &Textbox::sigTextChanged, m_pBuilding, &Building::setBuildingName, Qt::QueuedConnection);
    m_pPanel->addItem(pTextbox);
    m_pPanel->addItem(pLabel);
    y += pLabel->getHeight() + 10;

    if (m_pBuilding->getHp() > 0)
    {
        pLabel = MemoryManagement::create<Label>(190);
        pLabel->setStyle(style);
        pLabel->setHtmlText(tr("HP:"));
        pLabel->setPosition(10, y);
        spSlider pSlider = MemoryManagement::create<Slider>(oxygine::Stage::getStage()->getWidth() - 100 - 200 - pLabel->getScaledWidth(), 1, 9999, tr("HP"), 200);
        pSlider->setTooltipText(tr("Selects the HP of the current building. This is immediately applied."));
        pSlider->setPosition(200 + 20 + pLabel->getX(), y);
        pSlider->setCurrentValue(m_pBuilding->getHp());
        connect(pSlider.get(), &Slider::sliderValueChanged, this, [this](qint32 value)
                {
                    m_pBuilding->setHp(value);
                });
        m_pPanel->addItem(pSlider);
        m_pPanel->addItem(pLabel);
        y += pLabel->getHeight() + 10;
    }

    connect(this, &DialogModifyBuilding::sigFinished, this, &DialogModifyBuilding::remove, Qt::QueuedConnection);
}

void DialogModifyBuilding::remove()
{
    detach();
}
