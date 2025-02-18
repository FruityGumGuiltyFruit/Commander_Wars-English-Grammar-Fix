#include "updater/gameupdater.h"

#include "objects/loadingscreen.h"
#include "objects/dialogs/dialogmessagebox.h"

#include "coreengine/mainapp.h"
#include "coreengine/gameconsole.h"
#include "coreengine/settings.h"

#include <QDir>
#include <QProcess>
#include <QDirIterator>

constexpr const char* const LATEST = "latest";
constexpr const char* const UPDATE_URL = "https://github.com/Robosturm/Commander_Wars/releases/";
constexpr const char* const DOWNLOADTARGET = "update.zip";
constexpr const char* const UNPACK_PATH = "update";
const char* const GameUpdater::MODE_FORCE = "force";
const char* const GameUpdater::MODE_INSTALL = "install";

GameUpdater::GameUpdater()
    : m_filedownloader(QString(UPDATE_FILE), // COW_UPDATETARGET),
                       UPDATE_URL,
                       LATEST,
                       QString(COW_BUILD_TAG),
                       DOWNLOADTARGET),
    m_downloadFile(DOWNLOADTARGET),
    m_zipReader(&m_downloadFile),
    m_updateTimer(this)
{
    QString mode = Settings::getInstance()->getUpdateStep();
    if (mode == MODE_INSTALL)
    {
        install();
    }
    else
    {
        connect(&m_updateTimer, &QTimer::timeout, this, &GameUpdater::updateUi, Qt::QueuedConnection);
        connect(&m_filedownloader, &FileDownloader::sigNewProgress, this, &GameUpdater::onNewProgress);
        connect(&m_filedownloader, &FileDownloader::sigNewState, this, &GameUpdater::onNewState);
        connect(&m_zipReader, &QZipReader::sigExtractProgress, this, &GameUpdater::onExtractProgress);
        m_updateTimer.setSingleShot(false);
        spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
        pLoadingScreen->setProgress(tr("Cleaning up old update artifacts ..."), 0);
        Mainapp::getInstance()->redrawUi();
        QCoreApplication::processEvents();
        cleanUpOldArtifacts();
        pLoadingScreen->setProgress(tr("Checking for new version..."), 1);
        Mainapp::getInstance()->redrawUi();
        QCoreApplication::processEvents();
    }
}

void GameUpdater::cleanUpOldArtifacts()
{
    CONSOLE_PRINT("Clean up old artifacts", GameConsole::eDEBUG);
    QFile downloadFile(DOWNLOADTARGET);
    if (downloadFile.exists())
    {
        downloadFile.remove();
    }
    QDir dir(QCoreApplication::applicationDirPath() + "/" + UNPACK_PATH);
    dir.removeRecursively();
}

void GameUpdater::install()
{
    CONSOLE_PRINT("Installing update", GameConsole::eDEBUG);
    QString path = QCoreApplication::applicationDirPath();
    QDirIterator dirIter(path, QDir::Files, QDirIterator::Subdirectories);
    qint32 count = 0;
    while (dirIter.hasNext())
    {
        QCoreApplication::processEvents();
        dirIter.next();
        QString filePath = dirIter.fileInfo().filePath();
        if (!filePath.endsWith("*.ini"))
        {
            QString targetName = filePath;
            QString targetPath = QCoreApplication::applicationDirPath() + "/../../" + targetName.replace(path, "");
            QFile file(targetPath);
            if (file.exists())
            {
                file.remove();
            }
            else
            {
                QFileInfo info(targetPath);
                QDir dir(info.absoluteDir().canonicalPath());
                dir.mkpath(".");
            }
            QFile::copy(filePath, targetPath);
            CONSOLE_PRINT("Copying " + filePath + " to " + targetPath, GameConsole::eDEBUG);
            spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
            pLoadingScreen->setProgress(tr("Copying files to target directory"), count);
            Mainapp::getInstance()->redrawUi();
            ++count;
        }
    }
    spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
    pLoadingScreen->setProgress(tr("Cleaning up"), 99);
    Mainapp::getInstance()->redrawUi();
    QCoreApplication::processEvents();
    QDir dir(QCoreApplication::applicationDirPath() + "/" + UNPACK_PATH);
    dir.removeRecursively();
    emit Mainapp::getInstance()->sigQuit(3);
}

void GameUpdater::onNewState(FileDownloader::State state)
{
    m_updateTimer.stop();
    switch (state)
    {
    case FileDownloader::State::DownloadingFailed:
    {
        GameConsole::print("Downloading new version failed.", GameConsole::eINFO);
        spDialogMessageBox pMessageBox = MemoryManagement::create<DialogMessageBox>(QString(tr("Failed to download update. Continue with current version.")).arg(m_filedownloader.getLatestTag()));
        spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
        pLoadingScreen->addChild(pMessageBox);
        connect(pMessageBox.get(), &DialogMessageBox::sigOk, this, &GameUpdater::continueBooting, Qt::QueuedConnection);
        m_updateTimer.start(33);
        break;
    }
    case FileDownloader::State::SameVersion:
    {
        continueBooting();
        break;
    }
    case FileDownloader::State::DownloadingNewVersion:
    {
        GameConsole::print("Start downloading new version.", GameConsole::eINFO);
        break;
    }
    case FileDownloader::State::DownloadingFinished:
    {
        finishDownload();
        break;
    }
    case FileDownloader::State::NewVersion:
    {
        spDialogMessageBox pMessageBox = MemoryManagement::create<DialogMessageBox>(QString(tr("Do you wist to update the current version to %1 ?")).arg(m_filedownloader.getLatestTag()), true, tr("Yes"), tr("No"));
        spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
        pLoadingScreen->addChild(pMessageBox);
        connect(pMessageBox.get(), &DialogMessageBox::sigOk, &m_filedownloader, &FileDownloader::startDownloading, Qt::QueuedConnection);
        connect(pMessageBox.get(), &DialogMessageBox::sigCancel, this, &GameUpdater::continueBooting, Qt::QueuedConnection);
        m_updateTimer.start(33);
        break;
    }
    }
}

void GameUpdater::updateUi()
{
    Mainapp::getInstance()->redrawUi();
    QCoreApplication::processEvents();
}

void GameUpdater::finishDownload()
{
    spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
    pLoadingScreen->setProgress(tr("Preparing extraction..."), 0);
    Mainapp::getInstance()->redrawUi();
    QCoreApplication::processEvents();
    m_downloadFile.open(QIODevice::ReadOnly);
    QDir dir(COW_INSTALLDIR);
    dir.removeRecursively();
    bool success = m_zipReader.extractAll(UNPACK_PATH);
    pLoadingScreen->setProgress(tr("Removing zip..."), 50);
    Mainapp::getInstance()->redrawUi();
    QCoreApplication::processEvents();
    m_downloadFile.close();
    m_downloadFile.remove();
    if (success)
    {
        pLoadingScreen->setProgress(tr("Launching patcher..."), 75);
        Mainapp::getInstance()->redrawUi();
        QCoreApplication::processEvents();
        emit Mainapp::getInstance()->sigQuit(2);
    }
    else
    {
        GameConsole::print("Failed to extract new version.", GameConsole::eINFO);
        continueBooting();
    }

}

void GameUpdater::launchPatcher()
{
    spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
    pLoadingScreen->setProgress(tr("Preparing update installation..."), 50);
    Mainapp::getInstance()->redrawUi();
    QCoreApplication::processEvents();
    QThread::currentThread()->msleep(100);
    QFileInfo info(QCoreApplication::applicationFilePath());
    QString program = info.fileName();
    QString patcherProgram =  QCoreApplication::applicationDirPath() + "/" + UNPACK_PATH + "/" + COW_INSTALLDIR + "/" + program;
    QThread::currentThread()->msleep(350);
    const char* const prefix = "--";
    QStringList args({QString(prefix) + CommandLineParser::ARG_UPDATE,
                      MODE_INSTALL});
    CONSOLE_PRINT("Starting patcher application " + patcherProgram, GameConsole::eDEBUG);
    if (!QProcess::startDetached(patcherProgram, args))
    {
        CONSOLE_PRINT("Failed to start patcher application " + patcherProgram, GameConsole::eERROR);
    }
}

void GameUpdater::launchApplication()
{
    QThread::currentThread()->msleep(100);
    QFileInfo info(QCoreApplication::applicationFilePath());
    QString program = info.fileName();
    QString workingDir = QCoreApplication::applicationDirPath() + "/../../";
    QString appPath = workingDir + program;
    QThread::currentThread()->msleep(350);
    CONSOLE_PRINT("Starting application " + appPath, GameConsole::eDEBUG);
    QProcess process;
    process.setProgram(appPath);
    process.setWorkingDirectory(workingDir);
    process.startDetached();
}

void GameUpdater::continueBooting()
{
    if (!m_continued)
    {
        m_continued = true;
        Mainapp* pApp = Mainapp::getInstance();
        spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
        pLoadingScreen->setProgress(tr("Loading Object Textures ..."), 8);
        pApp->redrawUi();
        QCoreApplication::processEvents();
        emit pApp->sigNextStartUpStep(static_cast<Mainapp::StartupPhase>(static_cast<qint8>(pApp->getStartUpStep()) + 1));
    }
}

void GameUpdater::onNewProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    constexpr qint64 MB_DIVIDER = (1024 * 1024);
    qint32 kBytesReceived = bytesReceived / MB_DIVIDER;
    qint32 kBytesTotal = bytesTotal / MB_DIVIDER;
    qint32 progress = 0;
    if (kBytesTotal > 0)
    {
        progress = kBytesReceived * 100 / kBytesTotal;
    }
    spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
    pLoadingScreen->setProgress(tr("Downloading new version ") + QString::number(kBytesReceived) + " MB / " + QString::number(kBytesTotal) + " MB", progress);
    Mainapp::getInstance()->redrawUi();
    QCoreApplication::processEvents();
}

void GameUpdater::onExtractProgress(qint32 progress)
{
    spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
    pLoadingScreen->setProgress(tr("Extracting new version..."), progress);
    Mainapp::getInstance()->redrawUi();
    QCoreApplication::processEvents();
}
