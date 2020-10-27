#ifndef DIALOGPASSWORDANDADRESS_H
#define DIALOGPASSWORDANDADRESS_H

#include <QObject>

#include "oxygine-framework.h"

#include "objects/textbox.h"
#include "objects/passwordbox.h"

class DialogPasswordAndAdress;
typedef oxygine::intrusive_ptr<DialogPasswordAndAdress> spDialogPasswordAndAdress;

class DialogPasswordAndAdress : public QObject, public oxygine::Actor
{
    Q_OBJECT
public:
    explicit DialogPasswordAndAdress(QString text);
    virtual ~DialogPasswordAndAdress() = default;

signals:
    void sigTextChanged(QString text, QString password);
    void sigCancel();
public slots:

private:
    oxygine::spButton m_OkButton;
    oxygine::spButton m_CancelButton;
    spTextbox m_pTextbox;
    spPasswordbox m_pPasswordbox;
};

#endif // DIALOGPASSWORDANDADRESS_H