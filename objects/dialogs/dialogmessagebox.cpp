#include "objects/dialogs/dialogmessagebox.h"

#include "coreengine/mainapp.h"

#include "resource_management/objectmanager.h"
#include "resource_management/fontmanager.h"

DialogMessageBox::DialogMessageBox(QString text, bool withCancel, QString confirmText, QString cancelText)
    : QObject(),
      m_Message(text)
{
#ifdef GRAPHICSUPPORT
    setObjectName("DialogMessageBox");
#endif
    Mainapp* pApp = Mainapp::getInstance();
    moveToThread(pApp->getWorkerthread());
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::spBox9Sprite pSpriteBox = oxygine::spBox9Sprite::create();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("codialog");
    pSpriteBox->setResAnim(pAnim);
    pSpriteBox->setSize(Settings::getWidth(), Settings::getHeight());
    addChild(pSpriteBox);
    pSpriteBox->setPosition(0, 0);
    pSpriteBox->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    setPriority(static_cast<qint32>(Mainapp::ZOrder::Dialogs));

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = true;

    m_Text = oxygine::spTextField::create();
    m_Text->setSize(Settings::getWidth() / 2, Settings::getHeight());
    m_Text->setHtmlText(text);
    m_Text->setStyle(style);
    m_Text->setPosition(Settings::getWidth() / 2 - m_Text->getTextRect().getWidth() / 2, Settings::getHeight() / 2 - m_Text->getTextRect().getHeight());
    pSpriteBox->addChild(m_Text);

    m_OkButton = pObjectManager->createButton(confirmText, 150);
    m_OkButton->setPosition(Settings::getWidth() / 2 - m_OkButton->getWidth() / 2,
                            m_Text->getY() + m_Text->getTextRect().getHeight() + 20);
    pSpriteBox->addChild(m_OkButton);
    m_OkButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        emit sigOk();
    });

    if (withCancel)
    {
        m_CancelButton = pObjectManager->createButton(cancelText, 150);
        m_CancelButton->setPosition(Settings::getWidth() / 2 + 10,
                                m_Text->getY() + m_Text->getTextRect().getHeight() + 20);
        pSpriteBox->addChild(m_CancelButton);
        m_CancelButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
        {
            emit sigCancel();
        });
        m_OkButton->setPosition(Settings::getWidth() / 2 - m_OkButton->getWidth() - 10,
                                m_Text->getY() + m_Text->getTextRect().getHeight() + 20);
    }

    connect(this, &DialogMessageBox::sigOk, this, &DialogMessageBox::remove, Qt::QueuedConnection);
    connect(this, &DialogMessageBox::sigCancel, this, &DialogMessageBox::remove, Qt::QueuedConnection);
}

void DialogMessageBox::remove()
{
    detach();
}

