//
//  AppDelegate.h
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 06/27/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#ifndef hifi_AppDelegate_h
#define hifi_AppDelegate_h

#include "BackgroundProcess.h"

#include <QApplication>
#include <QCoreApplication>
#include <QList>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QHash>

#include "MainWindow.h"

const QString DOMAIN_SERVER_BASE_URL = "http://localhost:40100";

class AppDelegate : public QApplication
{
    Q_OBJECT
public:
    static AppDelegate* getInstance() { return static_cast<AppDelegate*>(QCoreApplication::instance()); }

    AppDelegate(int argc, char* argv[]);

    void startDomainServer();
    void stopDomainServer();

    void startAssignment(int id, QString poolID = "");
    void stopAssignment(int id);
    
    const QString getServerAddress() const { return "hifi://" + _domainServerName; }
    
signals:
    void domainServerIDMissing();
    void domainAddressChanged(const QString& newAddress);
private slots:
    void cleanupProcesses();
    void onFileSuccessfullyInstalled(QUrl url);
    void handleDomainIDReply();
    void handleDomainGetReply();

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
    
    QString _domainServerID;
    QString _domainServerName;
};

#endif
