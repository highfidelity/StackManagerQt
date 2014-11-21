//
//  AppDelegate.cpp
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 06/27/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include "AppDelegate.h"
#include "GlobalData.h"
#include "DownloadManager.h"

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFileInfoList>
#include <QDebug>

AppDelegate::AppDelegate(int argc, char* argv[]) :
    QApplication(argc, argv)
{
    setApplicationName("Stack Manager");
    setOrganizationName("High Fidelity");
    setOrganizationDomain("io.highfidelity.StackManager");

    _manager = new QNetworkAccessManager(this);

    createExecutablePath();
    downloadLatestExecutablesAndRequirements();

    connect(this, &QApplication::aboutToQuit, this, &AppDelegate::cleanupProcesses);
}

void AppDelegate::cleanupProcesses() {
    for(int i = 0; i < _backgroundProcesses.size(); ++i) {
        _backgroundProcesses.at(i)->terminate();
        _backgroundProcesses.at(i)->waitForFinished();
    }
}

void AppDelegate::startDomainServer() {
    if (findBackgroundProcess("domain-server") == NULL && findBackgroundProcess("assignmentDS") == NULL) {
        BackgroundProcess* dsProcess = new BackgroundProcess("domain-server");
        _backgroundProcesses.append(dsProcess);
        dsProcess->start(GlobalData::getInstance()->getDomainServerExecutablePath(), QStringList());
        BackgroundProcess* acProcess = new BackgroundProcess("assignmentDS");
        _backgroundProcesses.append(acProcess);
        acProcess->start(GlobalData::getInstance()->getAssignmentClientExecutablePath(),
                                                    QStringList() << "-n" << "5");
    } else {
        findBackgroundProcess("domain-server")->start(GlobalData::getInstance()->getDomainServerExecutablePath(), QStringList());
        findBackgroundProcess("assignmentDS")->start(GlobalData::getInstance()->getAssignmentClientExecutablePath(), QStringList() << "-n" << "5");
    }
    MainWindow::getInstance()->setDomainServerStarted();
    MainWindow::getInstance()->getLogsWidget()->addTab(findBackgroundProcess("domain-server")->getLogViewer(), "Domain Server");
    _logsTabWidgetHash.insert("Domain Server", 0);
}

void AppDelegate::stopDomainServer() {
    for (int i = 0; i < _logsTabWidgetHash.size(); ++i) {
        MainWindow::getInstance()->getLogsWidget()->removeTab(_logsTabWidgetHash.values().at(i));
    }
    _logsTabWidgetHash.clear();
    cleanupProcesses();
    MainWindow::getInstance()->setDomainServerStopped();
}

void AppDelegate::startAssignment(int id, QString poolID) {
    QStringList argList = QStringList() << "-t" << "2";
    if (!poolID.isEmpty()) {
        argList << "--pool" << poolID;
    }
    
    if (findBackgroundProcess("assignment" + QString::number(id)) == NULL) {
        BackgroundProcess* process = new BackgroundProcess("assignment" + QString::number(id));
        _backgroundProcesses.append(process);
        process->start(GlobalData::getInstance()->getAssignmentClientExecutablePath(), argList);
    } else {
        findBackgroundProcess("assignment" + QString::number(id))->start(GlobalData::getInstance()->getAssignmentClientExecutablePath(),
                                                                         argList);
    }
    int index = MainWindow::getInstance()->getLogsWidget()->addTab(findBackgroundProcess("assignment" + QString::number(id))->getLogViewer(), "Assignment " + QString::number(id));
    _logsTabWidgetHash.insert("Assignment " + QString::number(id), index);
}

void AppDelegate::stopAssignment(int id) {
    findBackgroundProcess("assignment" + QString::number(id))->terminate();
    int index = -1;
    for (int i = 1; i < _logsTabWidgetHash.size(); ++i) {
        if (MainWindow::getInstance()->getLogsWidget()->tabText(_logsTabWidgetHash.values().at(i)) == "Assignment " + QString::number(id)) {
            index = i;
            break;
        }
    }
    MainWindow::getInstance()->getLogsWidget()->removeTab(index);
    _logsTabWidgetHash.remove("Assignment " + QString::number(id));
}

void AppDelegate::onFileSuccessfullyInstalled(QUrl url) {
    if (url == GlobalData::getInstance()->getRequirementsURL()) {
        _qtReady = true;
    } else if (url == GlobalData::getInstance()->getAssignmentClientURL()) {
        _acReady = true;
    } else if (url == GlobalData::getInstance()->getDomainServerURL()) {
        _dsReady = true;
    } else if (url == GlobalData::getInstance()->getDomainServerResourcesURL()) {
        _dsResourcesReady = true;
    }

    if (_qtReady && _acReady && _dsReady && _dsResourcesReady) {
        MainWindow::getInstance()->setRequirementsLastChecked(QDateTime::currentDateTime().toString());
        MainWindow::getInstance()->show();
    }
}

void AppDelegate::createExecutablePath() {
    QDir launchDir(GlobalData::getInstance()->getClientsLaunchPath());
    QDir resourcesDir(GlobalData::getInstance()->getClientsResourcesPath());
    QDir logsDir(GlobalData::getInstance()->getLogsPath());
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
    if (GlobalData::getInstance()->getPlatform() == "mac") {
        if (QDir(GlobalData::getInstance()->getClientsLaunchPath() + "QtCore.framework").exists()) {
            _qtReady = true;
        }
    } else if (GlobalData::getInstance()->getPlatform() == "win") {
        if (QFileInfo(GlobalData::getInstance()->getClientsLaunchPath() + "Qt5Core.dll").exists()) {
            _qtReady = true;
        }
    } else { // linux
        if (QFileInfo(GlobalData::getInstance()->getClientsLaunchPath() + "libQt5Core.so.5").exists()) {
            _qtReady = true;
        }
    }

    QFile dsFile(GlobalData::getInstance()->getDomainServerExecutablePath());
    QByteArray dsData;
    if (dsFile.open(QIODevice::ReadOnly)) {
        dsData = dsFile.readAll();
        dsFile.close();
    }
    QFile acFile(GlobalData::getInstance()->getAssignmentClientExecutablePath());
    QByteArray acData;
    if (acFile.open(QIODevice::ReadOnly)) {
        acData = acFile.readAll();
        acFile.close();
    }
    QFile reqZipFile(GlobalData::getInstance()->getRequirementsZipPath());
    QByteArray reqZipData;
    if (reqZipFile.open(QIODevice::ReadOnly)) {
        reqZipData = reqZipFile.readAll();
        reqZipFile.close();
    }
    QFile resZipFile(GlobalData::getInstance()->getDomainServerResourcesZipPath());
    QByteArray resZipData;
    if (resZipFile.open(QIODevice::ReadOnly)) {
        resZipData = resZipFile.readAll();
        resZipFile.close();
    }

    QDir resourcesDir(GlobalData::getInstance()->getClientsResourcesPath());
    if (!(resourcesDir.entryInfoList(QDir::AllEntries).size() < 3)) {
        _dsResourcesReady = true;
    }

    QNetworkRequest acReq(QUrl(GlobalData::getInstance()->getAssignmentClientMD5URL()));
    QNetworkReply* acReply = _manager->get(acReq);
    QEventLoop acLoop;
    connect(acReply, SIGNAL(finished()), &acLoop, SLOT(quit()));
    acLoop.exec();
    QByteArray acMd5Data = acReply->readAll().trimmed();
    if (GlobalData::getInstance()->getPlatform() == "win") {
        // fix for reading the MD5 hash from Windows-generated
        // binary data of the MD5 hash
        QTextStream stream(acMd5Data);
        stream >> acMd5Data;
    }

    // fix for Mac and Linux network accessibility
    if (acMd5Data.size() == 0) {
        // network is not accessible
        qDebug() << "Could not connect to the internet.";
        MainWindow::getInstance()->show();
        return;
    }

    qDebug() << "AC MD5: " << acMd5Data;
    if (acMd5Data.toLower() == QCryptographicHash::hash(acData, QCryptographicHash::Md5).toHex()) {
        _acReady = true;
    }

    QNetworkRequest dsReq(QUrl(GlobalData::getInstance()->getDomainServerMD5URL()));
    QNetworkReply* dsReply = _manager->get(dsReq);
    QEventLoop dsLoop;
    connect(dsReply, SIGNAL(finished()), &dsLoop, SLOT(quit()));
    dsLoop.exec();
    QByteArray dsMd5Data = dsReply->readAll().trimmed();
    if (GlobalData::getInstance()->getPlatform() == "win") {
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
        QNetworkRequest reqZipReq(QUrl(GlobalData::getInstance()->getRequirementsMD5URL()));
        QNetworkReply* reqZipReply = _manager->get(reqZipReq);
        QEventLoop reqZipLoop;
        connect(reqZipReply, SIGNAL(finished()), &reqZipLoop, SLOT(quit()));
        reqZipLoop.exec();
        QByteArray reqZipMd5Data = reqZipReply->readAll().trimmed();
        if (GlobalData::getInstance()->getPlatform() == "win") {
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
        QNetworkRequest resZipReq(QUrl(GlobalData::getInstance()->getDomainServerResourcesMD5URL()));
        QNetworkReply* resZipReply = _manager->get(resZipReq);
        QEventLoop resZipLoop;
        connect(resZipReply, SIGNAL(finished()), &resZipLoop, SLOT(quit()));
        resZipLoop.exec();
        QByteArray resZipMd5Data = resZipReply->readAll().trimmed();
        if (GlobalData::getInstance()->getPlatform() == "win") {
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
        MainWindow::getInstance()->setRequirementsLastChecked(QDateTime::currentDateTime().toString());
        MainWindow::getInstance()->show();
    }

    if (!_qtReady) {
        downloadManager->downloadFile(GlobalData::getInstance()->getRequirementsURL());
    }

    if (!_acReady) {
        downloadManager->downloadFile(GlobalData::getInstance()->getAssignmentClientURL());
    }

    if (!_dsReady) {
        downloadManager->downloadFile(GlobalData::getInstance()->getDomainServerURL());
    }

    if (!_dsResourcesReady) {
        downloadManager->downloadFile(GlobalData::getInstance()->getDomainServerResourcesURL());
    }
}

BackgroundProcess* AppDelegate::findBackgroundProcess(QString type) {
    for (int i = 0; i < _backgroundProcesses.size(); ++i) {
        if (_backgroundProcesses.at(i)->getType() == type) {
            return _backgroundProcesses.at(i);
        }
    }

    return NULL;
}
