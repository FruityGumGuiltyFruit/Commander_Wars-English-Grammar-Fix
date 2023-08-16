#ifndef WIKIPAGE_H
#define WIKIPAGE_H

#include <QObject>
#include "3rd_party/oxygine-framework/oxygine/KeyEvent.h"

#include "objects/base/panel.h"

class Wikipage;
using spWikipage = std::shared_ptr<Wikipage>;

class Wikipage : public QObject, public oxygine::Actor
{
    Q_OBJECT
public:
    explicit Wikipage(QString pageID);
    virtual ~Wikipage();
    /**
     * @brief getPanel
     * @return
     */
    inline Panel* getPanel()
    {
        return m_pPanel.get();
    }
    static QStringList & getPageStack();
signals:
    void sigFinished();
    /**
     * @brief sigShowLink
     * @param pageID
     */
    void sigShowLink(QString pageID);
public slots:
    void keyInput(oxygine::KeyEvent event);
    /**
     * @brief loadText
     * @param text
     */
    void loadText(QString text);
    /**
     * @brief loadImage
     * @param file
     */
    void loadImage(QString file, float scale = 1.0f, QString pageID = "");
    /**
     * @brief loadHeadline
     * @param text
     */
    void loadHeadline(QString text);
    /**
     * @brief showLink
     * @param pageID
     */
    void showLink(QString pageID);
private slots:
    void remove();
protected:
    spPanel m_pPanel;
    oxygine::spButton m_OkButton;
    qint32 m_y{10};
    QString m_pageID;
    static QStringList m_pageStack;
};

Q_DECLARE_INTERFACE(Wikipage, "Wikipage");

#endif // WIKIPAGE_H
