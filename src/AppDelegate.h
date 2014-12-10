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

    void toggleStack(bool start);
    void toggleDomainServer(bool start);
    void toggleAssignmentClientMonitor(bool start);
    
    void stopStack() { toggleStack(false); }
    
    void requestTemporaryDomain();
    
    const QString getServerAddress(bool withPath = true) const;
public slots:
    void downloadContentSet(const QUrl& contentSetURL);
signals:
    void domainServerIDMissing();
    void domainAddressChanged();
    void temporaryDomainResponse(bool wasSuccessful);
    void contentSetDownloadResponse(bool wasSuccessful);
    void stackStateChanged(bool isOn);
private slots:
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
    
    void sendNewIDToDomainServer();

    QNetworkAccessManager* _manager;
    bool _qtReady;
    bool _dsReady;
    bool _dsResourcesReady;
    bool _acReady;
    BackgroundProcess _domainServerProcess;
    BackgroundProcess _acMonitorProcess;
    
    QString _domainServerID;
    QString _domainServerName;
    
    QString _sharePath;
    
    MainWindow _window;
};

#endif
