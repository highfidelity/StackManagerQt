//
//  AppDelegate.cpp
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 06/27/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include <csignal>

#include "AppDelegate.h"
#include "BackgroundProcess.h"
#include "GlobalData.h"
#include "DownloadManager.h"

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QUuid>

const QString HIGH_FIDELITY_API_URL = "https://data.highfidelity.io/api/v1";

void signalHandler(int param) {
    AppDelegate* app = AppDelegate::getInstance();
    
    app->quit();
}

static QTextStream* outStream = NULL;

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    
    QString dateTime = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    QString txt = QString("[%1] ").arg(dateTime);
    
    //in this function, you can write the message to any stream!
    switch (type) {
        case QtDebugMsg:
            fprintf(stdout, "Debug: %s\n", qPrintable(msg));
            txt += msg;
            break;
        case QtWarningMsg:
            fprintf(stdout, "Warning: %s\n", qPrintable(msg));
            txt += msg;
            break;
        case QtCriticalMsg:
            fprintf(stdout, "Critical: %s\n", qPrintable(msg));
            txt += msg;
            break;
        case QtFatalMsg:
            fprintf(stdout, "Fatal: %s\n", qPrintable(msg));
            txt += msg;
    }
    
    if (outStream) {
        *outStream << txt << endl;
    }
}

AppDelegate::AppDelegate(int argc, char* argv[]) :
    QApplication(argc, argv),
    _qtReady(false),
    _dsReady(false),
    _dsResourcesReady(false),
    _acReady(false),
    _domainServerProcess(NULL),
    _acMonitorProcess(NULL),
    _domainServerName("localhost")
{
    // be a signal handler for SIGTERM so we can stop child processes if we get it
    signal(SIGTERM, signalHandler);
    
    setApplicationName("Stack Manager");
    setOrganizationName("High Fidelity");
    setOrganizationDomain("io.highfidelity.StackManager");
    
    QFile* logFile = new QFile("last_run_log", this);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "Failed to open log file. Will not be able to write STDOUT/STDERR to file.";
    } else {
        outStream = new QTextStream(logFile);
    }
    
    
    qInstallMessageHandler(myMessageHandler);
    
    _domainServerProcess = new BackgroundProcess(GlobalData::getInstance().getDomainServerExecutablePath(), this);
    _acMonitorProcess = new BackgroundProcess(GlobalData::getInstance().getAssignmentClientExecutablePath(), this);

    _manager = new QNetworkAccessManager(this);
    
    _window = new MainWindow();

    createExecutablePath();
    downloadLatestExecutablesAndRequirements();

    connect(this, &QApplication::aboutToQuit, this, &AppDelegate::stopStack);
}

AppDelegate::~AppDelegate() {
    QHash<QUuid, BackgroundProcess*>::iterator it = _scriptProcesses.begin();
    
    qDebug() << "Stopping scripted assignment-client processes prior to quit.";
    while (it != _scriptProcesses.end()) {
        BackgroundProcess* backgroundProcess = it.value();
        
        // remove from the script processes hash
        it = _scriptProcesses.erase(it);
        
        // make sure the process is dead
        backgroundProcess->terminate();
        backgroundProcess->waitForFinished();
        backgroundProcess->deleteLater();
    }
    
    qDebug() << "Stopping domain-server process prior to quit.";
    _domainServerProcess->terminate();
    _domainServerProcess->waitForFinished();
    
    qDebug() << "Stopping assignment-client process prior to quit.";
    _acMonitorProcess->terminate();
    _acMonitorProcess->waitForFinished();
    
    _domainServerProcess->deleteLater();
    _acMonitorProcess->deleteLater();
    
    _window->deleteLater();
    
    delete outStream;
    outStream = NULL;
}

void AppDelegate::toggleStack(bool start) {
    toggleDomainServer(start);
    toggleAssignmentClientMonitor(start);
    toggleScriptedAssignmentClients(start);
    emit stackStateChanged(start);
}

void AppDelegate::toggleDomainServer(bool start) {
    
    if (start) {
        _domainServerProcess->start(QStringList());
        
        _window->getLogsWidget()->addTab(_domainServerProcess->getLogViewer(), "Domain Server");
        
        if (_domainServerID.isEmpty()) {
            // after giving the domain server some time to set up, ask for its ID
            QTimer::singleShot(1000, this, SLOT(requestDomainServerID()));
        }
    } else {
        _domainServerProcess->terminate();
    }
}

void AppDelegate::toggleAssignmentClientMonitor(bool start) {
    if (start) {
        _acMonitorProcess->start(QStringList() << "-n" << "5");
        _window->getLogsWidget()->addTab(_acMonitorProcess->getLogViewer(), "Assignment Clients");
    } else {
        _acMonitorProcess->terminate();
    }
}

void AppDelegate::toggleScriptedAssignmentClients(bool start) {
    foreach(BackgroundProcess* scriptProcess, _scriptProcesses) {
        if (start) {
            scriptProcess->start(scriptProcess->getLastArgList());
        } else {
            scriptProcess->terminate();
        }
    }
}

int AppDelegate::startScriptedAssignment(const QUuid& scriptID, const QString& pool) {
    
    BackgroundProcess* scriptProcess = _scriptProcesses.value(scriptID);
    
    if (!scriptProcess) {
        QStringList argList = QStringList() << "-t" << "2";
        if (!pool.isEmpty()) {
            argList << "--pool" << pool;
        }
        
        scriptProcess = new BackgroundProcess(GlobalData::getInstance().getAssignmentClientExecutablePath(),
                                              this);
        
        scriptProcess->start(argList);
        
        qint64 processID = scriptProcess->processId();
        _scriptProcesses.insert(scriptID, scriptProcess);
        
        _window->getLogsWidget()->addTab(scriptProcess->getLogViewer(), "Scripted Assignment "
                                        + QString::number(processID));
    } else {
        scriptProcess->QProcess::start();
    }
    
    return scriptProcess->processId();
}

void AppDelegate::stopScriptedAssignment(BackgroundProcess* backgroundProcess) {
    _window->getLogsWidget()->removeTab(_window->getLogsWidget()->indexOf(backgroundProcess->getLogViewer()));
    backgroundProcess->terminate();
}

void AppDelegate::stopScriptedAssignment(const QUuid& scriptID) {
    BackgroundProcess* processValue = _scriptProcesses.take(scriptID);
    if (processValue) {
        stopScriptedAssignment(processValue);
    }
}
        

void AppDelegate::requestDomainServerID() {
    // ask the domain-server for its ID so we can update the accessible name
    QUrl domainIDURL = DOMAIN_SERVER_BASE_URL + "/id";
    
    qDebug() << "Requesting domain server ID from" << domainIDURL.toString();
    
    QNetworkReply* idReply = _manager->get(QNetworkRequest(domainIDURL));
    
    connect(idReply, &QNetworkReply::finished, this, &AppDelegate::handleDomainIDReply);
}

void AppDelegate::requestTemporaryDomain() {
    QUrl tempDomainURL = HIGH_FIDELITY_API_URL + "/domains";
    QString tempDomainJSON = "{\"domain\": {\"temporary\": true }}";
    
    QNetworkRequest tempDomainRequest(tempDomainURL);
    tempDomainRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* tempReply = _manager->post(tempDomainRequest, tempDomainJSON.toLocal8Bit());
    connect(tempReply, &QNetworkReply::finished, this, &AppDelegate::handleTempDomainReply);
}

const QString AppDelegate::getServerAddress(bool withPath) const {
    return "hifi://" + _domainServerName + (withPath ? _sharePath : "");
}

void AppDelegate::handleDomainIDReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    
    if (reply->error() == QNetworkReply::NoError
        && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
        _domainServerID = QString(reply->readAll());
        
        if (!_domainServerID.isEmpty()) {
            
            if (!QUuid(_domainServerID).isNull()) {
                qDebug() << "The domain server ID is" << _domainServerID;
                qDebug() << "Asking High Fidelity API for associated domain name.";
                
                // fire off a request to high fidelity API to see if this domain exists with them
                QUrl domainGetURL = HIGH_FIDELITY_API_URL + "/domains/" + _domainServerID;
                QNetworkReply* domainGetReply = _manager->get(QNetworkRequest(domainGetURL));
                connect(domainGetReply, &QNetworkReply::finished, this, &AppDelegate::handleDomainGetReply);
            } else {
                emit domainServerIDMissing();
            }
        }
    } else {
        qDebug() << "Error getting domain ID from domain-server - "
            << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
            << reply->errorString();
    }
}

void AppDelegate::handleDomainGetReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    
    if (reply->error() == QNetworkReply::NoError
        && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
        QJsonDocument responseDocument = QJsonDocument::fromJson(reply->readAll());
        _domainServerName = responseDocument.object()["domain"].toObject()["name"].toString();
        
        qDebug() << "This domain server's name is" << _domainServerName << "- updating address link.";
        
        emit domainAddressChanged();
    }
}

void AppDelegate::handleTempDomainReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    
    if (reply->error() == QNetworkReply::NoError
        && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
        QJsonDocument responseDocument = QJsonDocument::fromJson(reply->readAll());
        QJsonObject domainObject = responseDocument.object()["domain"].toObject();
        
        _domainServerName = domainObject["name"].toString();
        _domainServerID = domainObject["id"].toString();
        
        qDebug() << "Received new name" << _domainServerName << "and new ID" << _domainServerID << "for temp domain.";
        
        sendNewIDToDomainServer();
    } else {
        qDebug() << "Error creating temporary domain -" << reply->errorString();
        emit temporaryDomainResponse(false);
    }
}

void AppDelegate::sendNewIDToDomainServer() {
    // setup a JSON object for the settings we are posting
    // it is possible this will require authentication - if so there's nothing we can do about it for now
    QString settingsJSON = "{\"metaverse\": { \"id\": \"%1\", \"automatic_networking\": \"full\", \"local_port\": \"0\" } }";

    QNetworkRequest settingsRequest(DOMAIN_SERVER_BASE_URL + "/settings.json");
    settingsRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* settingsReply = _manager->post(settingsRequest, settingsJSON.arg(_domainServerID).toLocal8Bit());
    connect(settingsReply, &QNetworkReply::finished, this, &AppDelegate::handleDomainSettingsResponse);
    
}

void AppDelegate::handleDomainSettingsResponse() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    
    if (reply->error() == QNetworkReply::NoError
        && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
        
        qDebug() << "Successfully stored new ID in domain-server.";
        
        emit temporaryDomainResponse(true);
        emit domainAddressChanged();
    } else {
        qDebug() << "Error saving ID with domain-server -" << reply->errorString();
        emit temporaryDomainResponse(false);
    }
}

void AppDelegate::downloadContentSet(const QUrl& contentSetURL) {
    // make sure this link was an svo
    if (contentSetURL.path().endsWith(".svo")) {
        // setup a request for this content set
        QNetworkRequest contentRequest(contentSetURL);
        QNetworkReply* contentReply = _manager->get(contentRequest);
        connect(contentReply, &QNetworkReply::finished, this, &AppDelegate::handleContentSetDownloadFinished);
    }
}

void AppDelegate::handleContentSetDownloadFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    
    if (reply->error() == QNetworkReply::NoError
        && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
        
        QString modelFilename = GlobalData::getInstance().getClientsResourcesPath() + "models.svo";
        
        // write the model file
        QFile modelFile(modelFilename);
        modelFile.open(QIODevice::WriteOnly);
        
        // stop the base assignment clients before we try to write the new content
        toggleAssignmentClientMonitor(false);
        _acMonitorProcess->waitForFinished();
        
        if (modelFile.write(reply->readAll()) == -1) {
            qDebug() << "Error writing content set to" << modelFilename;
            modelFile.close();
            toggleAssignmentClientMonitor(true);
        } else {
            qDebug() << "Wrote new content set to" << modelFilename;
            modelFile.close();
            
            // restart the assignment-client
            toggleAssignmentClientMonitor(true);
            
            emit contentSetDownloadResponse(true);
            
            // did we have a path in the query?
            // if so when we copy our share link we should append it
            QUrlQuery svoQuery(reply->url().query());
            _sharePath = svoQuery.queryItemValue("path");
            
            emit domainAddressChanged();
            
            return;
        }
    }
    
    // if we failed we should clean up the share path
    _sharePath = QString();
    emit contentSetDownloadResponse(false);
    emit domainAddressChanged();
}

void AppDelegate::onFileSuccessfullyInstalled(const QUrl& url) {
    if (url == GlobalData::getInstance().getRequirementsURL()) {
        _qtReady = true;
    } else if (url == GlobalData::getInstance().getAssignmentClientURL()) {
        _acReady = true;
    } else if (url == GlobalData::getInstance().getDomainServerURL()) {
        _dsReady = true;
    } else if (url == GlobalData::getInstance().getDomainServerResourcesURL()) {
        _dsResourcesReady = true;
    }

    if (_qtReady && _acReady && _dsReady && _dsResourcesReady) {
        _window->setRequirementsLastChecked(QDateTime::currentDateTime().toString());
        _window->show();
    }
}

void AppDelegate::createExecutablePath() {
    QDir launchDir(GlobalData::getInstance().getClientsLaunchPath());
    QDir resourcesDir(GlobalData::getInstance().getClientsResourcesPath());
    QDir logsDir(GlobalData::getInstance().getLogsPath());
    if (!launchDir.exists()) {
        if (QDir().mkpath(launchDir.absolutePath())) {
            qDebug() << "Successfully created directory: "
                     << launchDir.absolutePath();
        } else {
            qCritical() << "Failed to create directory: "
                        << launchDir.absolutePath();
        }
    }
    if (!resourcesDir.exists()) {
        if (QDir().mkpath(resourcesDir.absolutePath())) {
            qDebug() << "Successfully created directory: "
                     << resourcesDir.absolutePath();
        } else {
            qCritical() << "Failed to create directory: "
                        << resourcesDir.absolutePath();
        }
    }
    if (!logsDir.exists()) {
        if (QDir().mkpath(logsDir.absolutePath())) {
            qDebug() << "Successfully created directory: "
                     << logsDir.absolutePath();
        } else {
            qCritical() << "Failed to create directory: "
                        << logsDir.absolutePath();
        }
    }
}

void AppDelegate::downloadLatestExecutablesAndRequirements() {
    // Check if Qt is already installed
    if (GlobalData::getInstance().getPlatform() == "mac") {
        if (QDir(GlobalData::getInstance().getClientsLaunchPath() + "QtCore.framework").exists()) {
            _qtReady = true;
        }
    } else if (GlobalData::getInstance().getPlatform() == "win") {
        if (QFileInfo(GlobalData::getInstance().getClientsLaunchPath() + "Qt5Core.dll").exists()) {
            _qtReady = true;
        }
    } else { // linux
        if (QFileInfo(GlobalData::getInstance().getClientsLaunchPath() + "libQt5Core.so.5").exists()) {
            _qtReady = true;
        }
    }

    QFile dsFile(GlobalData::getInstance().getDomainServerExecutablePath());
    QByteArray dsData;
    if (dsFile.open(QIODevice::ReadOnly)) {
        dsData = dsFile.readAll();
        dsFile.close();
    }
    QFile acFile(GlobalData::getInstance().getAssignmentClientExecutablePath());
    QByteArray acData;
    if (acFile.open(QIODevice::ReadOnly)) {
        acData = acFile.readAll();
        acFile.close();
    }
    QFile reqZipFile(GlobalData::getInstance().getRequirementsZipPath());
    QByteArray reqZipData;
    if (reqZipFile.open(QIODevice::ReadOnly)) {
        reqZipData = reqZipFile.readAll();
        reqZipFile.close();
    }
    QFile resZipFile(GlobalData::getInstance().getDomainServerResourcesZipPath());
    QByteArray resZipData;
    if (resZipFile.open(QIODevice::ReadOnly)) {
        resZipData = resZipFile.readAll();
        resZipFile.close();
    }

    QDir resourcesDir(GlobalData::getInstance().getClientsResourcesPath());
    if (!(resourcesDir.entryInfoList(QDir::AllEntries).size() < 3)) {
        _dsResourcesReady = true;
    }

    QNetworkRequest acReq(QUrl(GlobalData::getInstance().getAssignmentClientMD5URL()));
    QNetworkReply* acReply = _manager->get(acReq);
    QEventLoop acLoop;
    connect(acReply, SIGNAL(finished()), &acLoop, SLOT(quit()));
    acLoop.exec();
    QByteArray acMd5Data = acReply->readAll().trimmed();
    if (GlobalData::getInstance().getPlatform() == "win") {
        // fix for reading the MD5 hash from Windows-generated
        // binary data of the MD5 hash
        QTextStream stream(acMd5Data);
        stream >> acMd5Data;
    }

    // fix for Mac and Linux network accessibility
    if (acMd5Data.size() == 0) {
        // network is not accessible
        qDebug() << "Could not connect to the internet.";
        _window->show();
        return;
    }

    qDebug() << "AC MD5: " << acMd5Data;
    if (acMd5Data.toLower() == QCryptographicHash::hash(acData, QCryptographicHash::Md5).toHex()) {
        _acReady = true;
    }

    QNetworkRequest dsReq(QUrl(GlobalData::getInstance().getDomainServerMD5URL()));
    QNetworkReply* dsReply = _manager->get(dsReq);
    QEventLoop dsLoop;
    connect(dsReply, SIGNAL(finished()), &dsLoop, SLOT(quit()));
    dsLoop.exec();
    QByteArray dsMd5Data = dsReply->readAll().trimmed();
    if (GlobalData::getInstance().getPlatform() == "win") {
        // fix for reading the MD5 hash from Windows generated
        // binary data of the MD5 hash
        QTextStream stream(dsMd5Data);
        stream >> dsMd5Data;
    }
    qDebug() << "DS MD5: " << dsMd5Data;
    if (dsMd5Data.toLower() == QCryptographicHash::hash(dsData, QCryptographicHash::Md5).toHex()) {
        _dsReady = true;
    }

    if (_qtReady) {
        // check MD5 of requirements.zip only if Qt is found
        QNetworkRequest reqZipReq(QUrl(GlobalData::getInstance().getRequirementsMD5URL()));
        QNetworkReply* reqZipReply = _manager->get(reqZipReq);
        QEventLoop reqZipLoop;
        connect(reqZipReply, SIGNAL(finished()), &reqZipLoop, SLOT(quit()));
        reqZipLoop.exec();
        QByteArray reqZipMd5Data = reqZipReply->readAll().trimmed();
        if (GlobalData::getInstance().getPlatform() == "win") {
            // fix for reading the MD5 hash from Windows generated
            // binary data of the MD5 hash
            QTextStream stream(reqZipMd5Data);
            stream >> reqZipMd5Data;
        }
        qDebug() << "Requirements ZIP MD5: " << reqZipMd5Data;
        if (reqZipMd5Data.toLower() != QCryptographicHash::hash(reqZipData, QCryptographicHash::Md5).toHex()) {
            _qtReady = false;
        }
    }

    if (_dsResourcesReady) {
        // check MD5 of resources.zip only if Domain Server
        // resources are installed
        QNetworkRequest resZipReq(QUrl(GlobalData::getInstance().getDomainServerResourcesMD5URL()));
        QNetworkReply* resZipReply = _manager->get(resZipReq);
        QEventLoop resZipLoop;
        connect(resZipReply, SIGNAL(finished()), &resZipLoop, SLOT(quit()));
        resZipLoop.exec();
        QByteArray resZipMd5Data = resZipReply->readAll().trimmed();
        if (GlobalData::getInstance().getPlatform() == "win") {
            // fix for reading the MD5 hash from Windows generated
            // binary data of the MD5 hash
            QTextStream stream(resZipMd5Data);
            stream >> resZipMd5Data;
        }
        qDebug() << "Domain Server Resources ZIP MD5: " << resZipMd5Data;
        if (resZipMd5Data.toLower() != QCryptographicHash::hash(resZipData, QCryptographicHash::Md5).toHex()) {
            _dsResourcesReady = false;
        }
    }

    DownloadManager* downloadManager = 0;
    if (!_qtReady || !_acReady || !_dsReady || !_dsResourcesReady) {
        // initialise DownloadManager
        downloadManager = new DownloadManager(_manager);
        downloadManager->setWindowModality(Qt::ApplicationModal);
        connect(downloadManager, SIGNAL(fileSuccessfullyInstalled(QUrl)),
                SLOT(onFileSuccessfullyInstalled(QUrl)));
        downloadManager->show();
    } else {
        _window->setRequirementsLastChecked(QDateTime::currentDateTime().toString());
        _window->show();
    }

    if (!_qtReady) {
        downloadManager->downloadFile(GlobalData::getInstance().getRequirementsURL());
    }

    if (!_acReady) {
        downloadManager->downloadFile(GlobalData::getInstance().getAssignmentClientURL());
    }

    if (!_dsReady) {
        downloadManager->downloadFile(GlobalData::getInstance().getDomainServerURL());
    }

    if (!_dsResourcesReady) {
        downloadManager->downloadFile(GlobalData::getInstance().getDomainServerResourcesURL());
    }
}
