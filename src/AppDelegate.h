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
    
    void stopBaseAssignmentClients();
    void startBaseAssignmentClients();
    
    void requestTemporaryDomain();
    
    const QString getServerAddress(bool withPath = true) const;
public slots:
    void downloadContentSet(const QUrl& contentSetURL);
signals:
    void domainServerIDMissing();
    void domainAddressChanged();
    void temporaryDomainResponse(bool wasSuccessful);
    void contentSetDownloadResponse(bool wasSuccessful);
private slots:
    void cleanupProcesses();
    void onFileSuccessfullyInstalled(QUrl url);
    void requestDomainServerID();
    void handleDomainIDReply();
    void handleDomainGetReply();
    void handleTempDomainReply();
    void handleDomainSettingsResponse();
    void handleContentSetDownloadFinished();

private:
    void createExecutablePath();
    void downloadLatestExecutablesAndRequirements();
    BackgroundProcess* findBackgroundProcess(QString type);
    
    void sendNewIDToDomainServer();

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
    
    QString _sharePath;
};

#endif
