//
//  FileWatcherListener.h
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 07/10/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#ifndef FILEWATCHERLISTENER_H
#define FILEWATCHERLISTENER_H

#include <QObject>

#include "efsw/efsw.hpp"

class FileWatcherListenerHandler;

class FileWatcherListener : public efsw::FileWatchListener
{
public:
    explicit FileWatcherListener(FileWatcherListenerHandler* parent);

    void handleFileAction(efsw::WatchID watchid, const std::string& dir,
                          const std::string& filename, efsw::Action action,
                          std::string oldFilename = "");

private:
    FileWatcherListenerHandler* _parent;
};

class FileWatcherListenerHandler : public QObject
{
    Q_OBJECT
public:
    static FileWatcherListenerHandler* getInstance();

    void init();

signals:
    void fileChanged();

private:
    explicit FileWatcherListenerHandler(QObject* parent = 0);
    ~FileWatcherListenerHandler();

    static FileWatcherListenerHandler* _instance;
    FileWatcherListener* _watcherListener;
};

#endif
