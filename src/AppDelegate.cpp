//
//  AppDelegate.cpp
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 06/27/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include "AppDelegate.h"
#include "ui_AppDelegate.h"
#include "GlobalData.h"
#include "AssignmentClientProcess.h"
#include "DownloadManager.h"
#include "FileWatcherListener.h"

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFileInfoList>
#include <QDebug>

AppDelegate::AppDelegate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AppDelegate),
    _manager(new QNetworkAccessManager(this)),
    _signalMapper(new QSignalMapper(this)) {
    ui->setupUi(this);

    _updatingString = "Updating: ";
    _upToDateString = "Up to date | Updated: ";
    _qtReady = false;
    _dsReady = false;
    _dsResourcesReady = false;
    _acReady = false;

    QList<QPushButton*> buttonList = findChildren<QPushButton*>();
    for (int i = 0; i < buttonList.size(); ++i) {
        // disable all view log buttons at startup
        if (buttonList.at(i)->objectName().startsWith("viewLog")) {
            buttonList.at(i)->setEnabled(false);
        }
    }

    ui->noConnectionLabel->hide();
    ui->retryButton->hide();
    connect(ui->retryButton, SIGNAL(clicked()), SLOT(retryConnection()));

    FileWatcherListenerHandler::getInstance()->init();

    createExecutablePath();
    downloadLatestExecutablesAndRequirements();

    // temporary
    ui->startAllAssignmentsButton->hide();

    connect(ui->startAllAssignmentsButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->startAudioMixerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->startAvatarMixerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->startDomainServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->startMetavoxelServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->startModelServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->startParticleServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->startVoxelServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->viewLogAudioMixerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->viewLogAvatarMixerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->viewLogDomainServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->viewLogMetavoxelServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->viewLogModelServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->viewLogParticleServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    connect(ui->viewLogVoxelServerButton, SIGNAL(clicked()), _signalMapper, SLOT(map()));

    _signalMapper->setMapping(ui->startAllAssignmentsButton, "start-all-assignments-button");
    _signalMapper->setMapping(ui->startAudioMixerButton, "start-audio-mixer-button");
    _signalMapper->setMapping(ui->startAvatarMixerButton, "start-avatar-mixer-button");
    _signalMapper->setMapping(ui->startDomainServerButton, "start-domain-server-button");
    _signalMapper->setMapping(ui->startMetavoxelServerButton, "start-metavoxel-server-button");
    _signalMapper->setMapping(ui->startModelServerButton, "start-model-server-button");
    _signalMapper->setMapping(ui->startParticleServerButton, "start-particle-server-button");
    _signalMapper->setMapping(ui->startVoxelServerButton, "start-voxel-server-button");
    _signalMapper->setMapping(ui->viewLogAudioMixerButton, "view-log-audio-mixer-button");
    _signalMapper->setMapping(ui->viewLogAvatarMixerButton, "view-log-avatar-mixer-button");
    _signalMapper->setMapping(ui->viewLogDomainServerButton, "view-log-domain-server-button");
    _signalMapper->setMapping(ui->viewLogMetavoxelServerButton, "view-log-metavoxel-server-button");
    _signalMapper->setMapping(ui->viewLogModelServerButton, "view-log-model-server-button");
    _signalMapper->setMapping(ui->viewLogParticleServerButton, "view-log-particle-server-button");
    _signalMapper->setMapping(ui->viewLogVoxelServerButton, "view-log-voxel-server-button");

    connect(_signalMapper, SIGNAL(mapped(QString)), SLOT(buttonClicked(QString)));
}

AppDelegate::~AppDelegate() {
    delete ui;
    for (int i = 0; i < _backgroundProcesses.size(); ++i) {
        _backgroundProcesses.at(i)->terminate();
    }
}

void AppDelegate::retryConnection() {
    ui->retryButton->setText("Retrying...");
    ui->retryButton->setEnabled(false);
    repaint();
    downloadLatestExecutablesAndRequirements();
}

void AppDelegate::buttonClicked(QString buttonId) {
    if (buttonId.startsWith("start-")) {
        bool toStart = false;
        QString type = "";
        if (buttonId == "start-all-assignments-button") {
            /*if (ui->startAllAssignmentsButton->text() == "Start All") {
                for (int i = 0; i < GlobalData::getInstance()->getAvailableAssignmentTypes().size(); ++i) {
                    if (findBackgroundProcess(GlobalData::getInstance()->getAvailableAssignmentTypes().at(i)) == NULL) {
                        buttonClicked("start-"+GlobalData::getInstance()->getAvailableAssignmentTypes().at(i)+"-button");
                    }
                }
            } else {
                for (int i = 0; i < GlobalData::getInstance()->getAvailableAssignmentTypes().size(); ++i) {
                    buttonClicked("start-"+GlobalData::getInstance()->getAvailableAssignmentTypes().at(i)+"-button");
                }
            }*/
        } else if (buttonId == "start-audio-mixer-button") {
            type = "audio-mixer";
            if (ui->startAudioMixerButton->text() == "Start") {
                toStart = true;
                if (GlobalData::getInstance()->getPlatform() != "win") {
                    // temporary measure for Windows
                    ui->viewLogAudioMixerButton->setEnabled(true);
                }
                ui->startAudioMixerButton->setText("Stop");
            } else {
                toStart = false;
                ui->viewLogAudioMixerButton->setEnabled(false);
                ui->startAudioMixerButton->setText("Start");
            }
        } else if (buttonId == "start-avatar-mixer-button") {
            type = "avatar-mixer";
            if (ui->startAvatarMixerButton->text() == "Start") {
                toStart = true;
                if (GlobalData::getInstance()->getPlatform() != "win") {
                    // temporary measure for Windows
                    ui->viewLogAvatarMixerButton->setEnabled(true);
                }
                ui->startAvatarMixerButton->setText("Stop");
            } else {
                toStart = false;
                ui->viewLogAvatarMixerButton->setEnabled(false);
                ui->startAvatarMixerButton->setText("Start");
            }
        } else if (buttonId == "start-domain-server-button") {
            type = "domain-server";
            if (ui->startDomainServerButton->text() == "Start") {
                toStart = true;
                if (GlobalData::getInstance()->getPlatform() != "win") {
                    // temporary measure for Windows
                    ui->viewLogDomainServerButton->setEnabled(true);
                }
                ui->startDomainServerButton->setText("Stop");
            } else {
                toStart = false;
                ui->viewLogDomainServerButton->setEnabled(false);
                ui->startDomainServerButton->setText("Start");
            }
        } else if (buttonId == "start-metavoxel-server-button") {
            type = "metavoxel-server";
            if (ui->startMetavoxelServerButton->text() == "Start") {
                toStart = true;
                if (GlobalData::getInstance()->getPlatform() != "win") {
                    // temporary measure for Windows
                    ui->viewLogMetavoxelServerButton->setEnabled(true);
                }
                ui->startMetavoxelServerButton->setText("Stop");
            } else {
                toStart = false;
                ui->viewLogMetavoxelServerButton->setEnabled(false);
                ui->startMetavoxelServerButton->setText("Start");
            }
        } else if (buttonId == "start-model-server-button") {
            type = "model-server";
            if (ui->startModelServerButton->text() == "Start") {
                toStart = true;
                if (GlobalData::getInstance()->getPlatform() != "win") {
                    // temporary measure for Windows
                    ui->viewLogModelServerButton->setEnabled(true);
                }
                ui->startModelServerButton->setText("Stop");
            } else {
                toStart = false;
                ui->viewLogModelServerButton->setEnabled(false);
                ui->startModelServerButton->setText("Start");
            }
        } else if (buttonId == "start-particle-server-button") {
            type = "particle-server";
            if (ui->startParticleServerButton->text() == "Start") {
                toStart = true;
                if (GlobalData::getInstance()->getPlatform() != "win") {
                    // temporary measure for Windows
                    ui->viewLogParticleServerButton->setEnabled(true);
                }
                ui->startParticleServerButton->setText("Stop");
            } else {
                toStart = false;
                ui->viewLogParticleServerButton->setEnabled(false);
                ui->startParticleServerButton->setText("Start");
            }
        } else if (buttonId == "start-voxel-server-button") {
            type = "voxel-server";
            if (ui->startVoxelServerButton->text() == "Start") {
                toStart = true;
                if (GlobalData::getInstance()->getPlatform() != "win") {
                    // temporary measure for Windows
                    ui->viewLogVoxelServerButton->setEnabled(true);
                }
                ui->startVoxelServerButton->setText("Stop");
            } else {
                toStart = false;
                ui->viewLogVoxelServerButton->setEnabled(false);
                ui->startVoxelServerButton->setText("Start");
            }
        }

        if (toStart) {
            if (type == "domain-server") {
                BackgroundProcess* process = new BackgroundProcess(type);
                _backgroundProcesses.append(process);
                process->start(GlobalData::getInstance()->getDomainServerExecutablePath(), QStringList());
            } else {
                AssignmentClientProcess* process = new AssignmentClientProcess(type);
                _backgroundProcesses.append(process);
                process->startWithType();
            }
        } else {
            findBackgroundProcess(type)->terminate();
            _backgroundProcesses.removeOne(findBackgroundProcess(type));
        }

        int c = 0;
        for (int i = 0; i < GlobalData::getInstance()->getAvailableAssignmentTypes().size(); ++i) {
            if (findBackgroundProcess(GlobalData::getInstance()->getAvailableAssignmentTypes().keys().at(i))) {
                c++;
            }
        }
        if (c == GlobalData::getInstance()->getAvailableAssignmentTypes().size()) {
            // all the assignment clients are running
            ui->startAllAssignmentsButton->setText("Stop All");
        } else {
            ui->startAllAssignmentsButton->setText("Start All");
        }
    } else if (buttonId.startsWith("view-log")) {
        QString type;
        if (buttonId == "view-log-audio-mixer-button") {
            type = "audio-mixer";
        } else if (buttonId == "view-log-avatar-mixer-button") {
            type = "avatar-mixer";
        } else if (buttonId == "view-log-domain-server-button") {
            type = "domain-server";
        } else if (buttonId == "view-log-metavoxel-server-button") {
            type = "metavoxel-server";
        } else if (buttonId == "view-log-model-server-button") {
            type = "model-server";
        } else if (buttonId == "view-log-particle-server-button") {
            type = "particle-server";
        } else if (buttonId == "view-log-voxel-server-button") {
            type = "voxel-server";
        }

        findBackgroundProcess(type)->displayLog();
    }
}

void AppDelegate::onFileSuccessfullyInstalled(QUrl url) {
    if (url == GlobalData::getInstance()->getRequirementsURL()) {
        // Qt has been installed successfully
        _qtReady = true;
        ui->requirementsLabel->setText("Requirements: " + _upToDateString + " " + QDateTime::currentDateTime().toString());
    } else if (url == GlobalData::getInstance()->getAssignmentClientURL()) {
        // Assignment client has been installed successfully
        _acReady = true;
        ui->assignmentClientLabel->setText("Assignment Client: " + _upToDateString + " " + QDateTime::currentDateTime().toString());
    } else if (url == GlobalData::getInstance()->getDomainServerURL()) {
        _dsReady = true;
    } else if (url == GlobalData::getInstance()->getDomainServerResourcesURL()) {
        _dsResourcesReady = true;
    }

    if (_dsReady && _dsResourcesReady) {
        ui->domainServerLabel->setText("Domain Server: " + _upToDateString + " " + QDateTime::currentDateTime().toString());
    }

    if (_qtReady && _acReady && _dsReady && _dsResourcesReady) {
        // enable all buttons except the view log buttons
        QList<QPushButton*> buttonList = findChildren<QPushButton*>();
        for (int i = 0; i < buttonList.size(); ++i) {
            if (!buttonList.at(i)->objectName().startsWith("viewLog")) {
                buttonList.at(i)->setEnabled(true);
            }
        }
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
    _manager->setNetworkAccessible(QNetworkAccessManager::Accessible);

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

    if (_qtReady) {
        ui->requirementsLabel->setText("Requirements: " + _upToDateString + " " + QDateTime::currentDateTime().toString());
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
        // fix for reading the MD5 hash from Windows generated
        // binary data of the MD5 hash
        QTextStream stream(acMd5Data);
        stream >> acMd5Data;
    }

    // fix for Mac and Linux network accessibility
    if (acMd5Data.size() == 0) {
        // network is not accessible
        _manager->setNetworkAccessible(QNetworkAccessManager::NotAccessible);
    } else {
        _manager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    }

    if (_manager->networkAccessible() != QNetworkAccessManager::Accessible) {
        ui->requirementsLabel->hide();
        ui->domainServerLabel->hide();
        ui->assignmentClientLabel->hide();
        ui->retryButton->setEnabled(true);
        ui->retryButton->setText("Retry");
        ui->noConnectionLabel->show();
        ui->retryButton->show();
        qDebug() << "Could not connect to the internet.";
        return;
    } else {
        ui->requirementsLabel->show();
        ui->domainServerLabel->show();
        ui->assignmentClientLabel->show();
        ui->noConnectionLabel->hide();
        ui->retryButton->hide();
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

    if (_dsReady && _dsResourcesReady) {
        ui->domainServerLabel->setText("Domain Server: " + _upToDateString + " " + QDateTime::currentDateTime().toString());
    }

    if (_acReady) {
        ui->assignmentClientLabel->setText("Assigment Client: " + _upToDateString + " " + QDateTime::currentDateTime().toString());
    }

    DownloadManager* downloadManager = 0;
    if (!_qtReady || !_acReady || !_dsReady || !_dsResourcesReady) {
        // initialise DownloadManager
        downloadManager = new DownloadManager(_manager);
        downloadManager->setWindowModality(Qt::ApplicationModal);
        connect(downloadManager, SIGNAL(fileSuccessfullyInstalled(QUrl)),
                SLOT(onFileSuccessfullyInstalled(QUrl)));
        downloadManager->show();

        // disable all buttons
        QList<QPushButton*> buttonList = findChildren<QPushButton*>();
        for (int i = 0; i < buttonList.size(); ++i) {
            buttonList.at(i)->setEnabled(false);
        }
    }

    if (!_qtReady) {
        downloadManager->downloadFile(GlobalData::getInstance()->getRequirementsURL());
        ui->requirementsLabel->setText("Requirements: " + _updatingString + " " + QDateTime::currentDateTime().toString());
    }

    if (!_acReady) {
        downloadManager->downloadFile(GlobalData::getInstance()->getAssignmentClientURL());
        ui->assignmentClientLabel->setText("Assignment Client: " + _updatingString + " " + QDateTime::currentDateTime().toString());
    }

    if (!_dsReady) {
        downloadManager->downloadFile(GlobalData::getInstance()->getDomainServerURL());
        ui->domainServerLabel->setText("Domain Server: " + _updatingString + " " + QDateTime::currentDateTime().toString());
    }

    if (!_dsResourcesReady) {
        downloadManager->downloadFile(GlobalData::getInstance()->getDomainServerResourcesURL());
        ui->domainServerLabel->setText("Domain Server: " + _updatingString + " " + QDateTime::currentDateTime().toString());
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
