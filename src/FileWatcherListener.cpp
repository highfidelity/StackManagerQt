
#include "FileWatcherListener.h"
#include "GlobalData.h"

#include <QMutex>
#include <QDir>
#include <QDebug>

FileWatcherListener::FileWatcherListener(FileWatcherListenerHandler* parent) {
    _parent = parent;
}

void FileWatcherListener::handleFileAction(efsw::WatchID watchid, const std::string& dir,
                                           const std::string& filename, efsw::Action action,
                                           std::string oldFilename) {
    Q_UNUSED(watchid);
    Q_UNUSED(oldFilename);

    if (action == efsw::Actions::Modified || action == efsw::Actions::Add) {
        QString path = QDir::toNativeSeparators(QString::fromStdString(dir) + QString::fromStdString(filename));
        emit _parent->fileChanged(path);
    }
}

FileWatcherListenerHandler* FileWatcherListenerHandler::_instance = NULL;

FileWatcherListenerHandler* FileWatcherListenerHandler::getInstance() {
    static QMutex fileWatcherListenerHandlerInstanceMutex;

    fileWatcherListenerHandlerInstanceMutex.lock();

    if (!_instance) {
        _instance = new FileWatcherListenerHandler;
    }

    fileWatcherListenerHandlerInstanceMutex.unlock();

    return _instance;
}

void FileWatcherListenerHandler::init() {
    _watcherListener = new FileWatcherListener(this);
    efsw::FileWatcher* watcher = new efsw::FileWatcher;
    watcher->addWatch(GlobalData::getInstance()->getLogsPath().toStdString(), _watcherListener);
}

FileWatcherListenerHandler::FileWatcherListenerHandler(QObject *parent)
    : QObject(parent) {
}

FileWatcherListenerHandler::~FileWatcherListenerHandler() {
    delete _watcherListener;
}
