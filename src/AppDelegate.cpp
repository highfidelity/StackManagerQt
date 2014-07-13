

#include "AppDelegate.h"
#include "ui_AppDelegate.h"
#include "GlobalData.h"
#include "AssignmentClientProcess.h"
#include "DownloadManager.h"
#include "FileWatcherListener.h"

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFileInfoList>
#include <QDebug>

AppDelegate::AppDelegate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AppDelegate),
    _signalMapper(new QSignalMapper(this)) {
    ui->setupUi(this);

    _updatingString = "Updating ";
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
        findBackgroundProcess(_backgroundProcesses.at(i)->getType())->terminate();
    }
}

void AppDelegate::buttonClicked(QString buttonId) {
    if (buttonId.startsWith("start-")) {
        bool toStart = false;
        QString type = "";
        if (buttonId == "start-all-assignments-button") {
            if (ui->startAllAssignmentsButton->text() == "Start All") {
                for (int i = 0; i < GlobalData::getInstance()->getAvailableAssignmentTypes().size(); ++i) {
                    if (findBackgroundProcess(GlobalData::getInstance()->getAvailableAssignmentTypes().at(i)) == NULL) {
                        buttonClicked("start-"+GlobalData::getInstance()->getAvailableAssignmentTypes().at(i)+"-button");
                    }
                }
            } else {
                for (int i = 0; i < GlobalData::getInstance()->getAvailableAssignmentTypes().size(); ++i) {
                    buttonClicked("start-"+GlobalData::getInstance()->getAvailableAssignmentTypes().at(i)+"-button");
                }
            }
        } else if (buttonId == "start-audio-mixer-button") {
            type = "audio-mixer";
            if (ui->startAudioMixerButton->text() == "Start") {
                toStart = true;
                ui->viewLogAudioMixerButton->setEnabled(true);
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
                ui->viewLogAvatarMixerButton->setEnabled(true);
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
                ui->viewLogDomainServerButton->setEnabled(true);
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
                ui->viewLogMetavoxelServerButton->setEnabled(true);
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
                ui->viewLogModelServerButton->setEnabled(true);
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
                ui->viewLogParticleServerButton->setEnabled(true);
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
                ui->viewLogVoxelServerButton->setEnabled(true);
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
            if (findBackgroundProcess(GlobalData::getInstance()->getAvailableAssignmentTypes().at(i))) {
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
    bool downloadQt = true;
    bool downloadAC = true;
    bool downloadDS = true;
    bool downloadDSResources = true;

    // Check if Qt is already installed
    if (GlobalData::getInstance()->getPlatform() == "mac") {
        if (QDir(QDir::toNativeSeparators(GlobalData::getInstance()->getClientsLaunchPath() + "/QtCore.framework")).exists()) {
            downloadQt = false;
            _qtReady = true;
        }
    } else if (GlobalData::getInstance()->getPlatform() == "win") {
        if (QFileInfo(QDir::toNativeSeparators(GlobalData::getInstance()->getClientsLaunchPath() + "/QtCore.dll")).exists()) {
            downloadQt = false;
            _qtReady = true;
        }
    } else { // linux
        if (QFileInfo(QDir::toNativeSeparators(GlobalData::getInstance()->getClientsLaunchPath() + "/hifi/libQt5Core.so.5")).exists()) {
            downloadQt = false;
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

    QDir resourcesDir(GlobalData::getInstance()->getClientsResourcesPath());
    if (!(resourcesDir.entryInfoList(QDir::AllEntries).size() < 3)) {
        downloadDSResources = false;
        _dsResourcesReady = true;
    }

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest acReq(QUrl(GlobalData::getInstance()->getAssignmentClientMD5URL()));
    QNetworkReply* acReply = manager->get(acReq);
    QEventLoop acLoop;
    connect(acReply, SIGNAL(finished()), &acLoop, SLOT(quit()));
    acLoop.exec();
    QByteArray acMd5Data = acReply->readAll().trimmed();
    qDebug() << "AC MD5: " << acMd5Data;
    if (acMd5Data == QCryptographicHash::hash(acData, QCryptographicHash::Md5).toHex()) {
        downloadAC = false;
        _acReady = true;
    }

    QNetworkRequest dsReq(QUrl(GlobalData::getInstance()->getDomainServerMD5URL()));
    QNetworkReply* dsReply = manager->get(dsReq);
    QEventLoop dsLoop;
    connect(dsReply, SIGNAL(finished()), &dsLoop, SLOT(quit()));
    dsLoop.exec();
    QByteArray dsMd5Data = dsReply->readAll().trimmed();
    qDebug() << "DS MD5: " << dsMd5Data;
    if (dsMd5Data == QCryptographicHash::hash(dsData, QCryptographicHash::Md5).toHex()) {
        downloadDS = false;
        _dsReady = true;
    }

    if (_dsReady && _dsResourcesReady) {
        ui->domainServerLabel->setText("Domain Server: " + _upToDateString + " " + QDateTime::currentDateTime().toString());
    }

    if (_acReady) {
        ui->assignmentClientLabel->setText("Assigment Client: " + _upToDateString + " " + QDateTime::currentDateTime().toString());
    }

    DownloadManager* downloadManager = 0;
    if (downloadQt || downloadAC || downloadDS || downloadDSResources) {
        // initialise DownloadManager
        downloadManager = new DownloadManager(manager);
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

    if (downloadQt) {
        downloadManager->downloadFile(GlobalData::getInstance()->getRequirementsURL());
        ui->requirementsLabel->setText("Requirements: " + _updatingString + " " + QDateTime::currentDateTime().toString());
    }

    if (downloadAC) {
        downloadManager->downloadFile(GlobalData::getInstance()->getAssignmentClientURL());
        ui->assignmentClientLabel->setText("Assignment Client: " + _updatingString + " " + QDateTime::currentDateTime().toString());
    }

    if (downloadDS) {
        downloadManager->downloadFile(GlobalData::getInstance()->getDomainServerURL());
        ui->domainServerLabel->setText("Domain Server: " + _updatingString + " " + QDateTime::currentDateTime().toString());
    }

    if (downloadDSResources) {
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
