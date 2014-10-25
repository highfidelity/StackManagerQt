//
//  AppDelegate.h
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 06/27/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#ifndef APPDELEGATE_H
#define APPDELEGATE_H

#include "BackgroundProcess.h"

#include <QApplication>
#include <QCoreApplication>
#include <QList>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QHash>

#include "MainWindow.h"

class AppDelegate : public QApplication
{
    Q_OBJECT
public:
    static AppDelegate* getInstance() { return static_cast<AppDelegate*>(QCoreApplication::instance()); }

    AppDelegate(int argc, char* argv[]);

    void startDomainServer();
    void stopDomainServer();

    void startAssignment(int id, QString poolId = "");
    void stopAssignment(int id);

private slots:
    void cleanupProcesses();
    void onFileSuccessfullyInstalled(QUrl url);

private:
    void createExecutablePath();
    void downloadLatestExecutablesAndRequirements();
    BackgroundProcess* findBackgroundProcess(QString type);

    MainWindow* _window;
    QNetworkAccessManager* _manager;
    bool _qtReady;
    bool _dsReady;
    bool _dsResourcesReady;
    bool _acReady;
    QList<BackgroundProcess*> _backgroundProcesses;
    QHash<QString, int> _logsTabWidgetHash;
};

#endif
