#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QObject>
#include <QDir>

#include "3rd_party/oxygine-framework/oxygine/actor/Button.h"
#include "3rd_party/oxygine-framework/oxygine/actor/Box9Sprite.h"
#include "3rd_party/oxygine-framework/oxygine/KeyEvent.h"

#include "objects/base/textbox.h"
#include "objects/base/panel.h"
#include "objects/base/dropdownmenu.h"

class FileDialog;
using spFileDialog = oxygine::intrusive_ptr<FileDialog>;

/**
 * @brief The FileDialog class file dialog for accessing folder and files.
 * Upon success or cancel the dialog will remove itself from the scene.
 * You have to hold a smart pointer to this object if you need further info from it.
 */
class FileDialog final : public QObject, public oxygine::Actor
{
    Q_OBJECT
public:
    /**
     * @brief FileDialog
     * @param startFolder initial folder of the file dialog
     * @param wildcards wildcard items will be split at ;
     */
    explicit FileDialog(QString startFolder, const QStringList & wildcards, bool isSaveDialog, QString startFile = "", bool preview = false, QString acceptButtonName = tr("Ok"));
   virtual ~FileDialog() = default;

    bool getPreview() const;
    void setPreview(bool preview);

signals:
    void sigFileSelected(QString file);
    void sigCancel();
    void sigShowFolder(QString folder);
    void sigFinished();
    void sigShowDeleteQuestion();
    void sigShowOverwriteWarning();
public slots:
    void showFolder(QString folder);
    void filterChanged(qint32);
    void KeyInput(oxygine::KeyEvent event);
    void deleteItem();
    void showDeleteQuestion();
    void remove();
    void onFileSelected();
    void showOverwriteWarning();
private:
    spTextbox m_CurrentFolder;
    spTextbox m_CurrentFile;
    spPanel m_MainPanel;
    oxygine::spButton m_OkButton;
    spDropDownmenu m_DropDownmenu;
    oxygine::spButton m_CancelButton;
    QString m_pathPrefix;
    /**
          * @brief m_Items items directories or folders
        */
    QVector<oxygine::spBox9Sprite> m_Items;
    bool m_preview{false};
    bool m_focused{true};
    QVector<oxygine::spResAnim> m_ResAnims;
    bool m_isSaveDialog{false};
};

#endif // FILEDIALOG_H
