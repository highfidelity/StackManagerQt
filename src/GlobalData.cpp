//
//  GlobalData.cpp
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 6/25/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include "GlobalData.h"

#include <QMutex>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

GlobalData* GlobalData::_instance = NULL;

GlobalData* GlobalData::getInstance() {
    static QMutex globalDataInstanceMutex;

    globalDataInstanceMutex.lock();

    if (!_instance) {
        _instance = new GlobalData();
    }

    globalDataInstanceMutex.unlock();

    return _instance;
}

GlobalData::GlobalData()
{
    QString urlBase = "http://s3.amazonaws.com/hifi-public";
#if defined Q_OS_OSX
    _platform = "mac";
#elif defined Q_OS_WIN32
    _platform = "win";
#elif defined Q_OS_LINUX
    _platform = "linux";
#endif

    QString resourcePath = "resources/";
    QString assignmentClientExecutable = "assignment-client";
    QString domainServerExecutable = "domain-server";
    QString applicationSupportDirectory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    _clientsLaunchPath = QDir::toNativeSeparators(applicationSupportDirectory + "/");
    _clientsResourcePath = QDir::toNativeSeparators(applicationSupportDirectory + "/" + resourcePath);
    _assignmentClientExecutablePath = QDir::toNativeSeparators(_clientsLaunchPath + assignmentClientExecutable);
    if (_platform == "win") {
        _assignmentClientExecutablePath.append(".exe");
    }
    _domainServerExecutablePath = QDir::toNativeSeparators(_clientsLaunchPath + domainServerExecutable);
    if (_platform == "win") {
        _domainServerExecutablePath.append(".exe");
    }

    _requirementsURL = urlBase + "/binaries/" + _platform + "/requirements/requirements.zip";
    _requirementsZipPath = _clientsLaunchPath + "requirements.zip";
    _requirementsMD5URL = urlBase + "/binaries/" + _platform + "/requirements/requirements.md5";
    _assignmentClientURL = urlBase + "/binaries/" + _platform + "/assignment-client" + (_platform == "win" ? "/assignment-client.exe" : "/assignment-client");
    _domainServerResourcesURL = urlBase + "/binaries/" + _platform + "/domain-server/resources.zip";
    _domainServerResourcesZipPath = _clientsLaunchPath + "resources.zip";
    _domainServerResourcesMD5URL = urlBase + "/binaries/" + _platform + "/domain-server/resources.md5";
    _domainServerURL = urlBase + "/binaries/" + _platform + "/domain-server" + (_platform == "win" ? "/domain-server.exe" : "/domain-server");

    _assignmentClientMD5URL = urlBase + "/binaries/" + _platform + "/assignment-client/assignment-client.md5";
    _domainServerMD5URL = urlBase + "/binaries/" + _platform + "/domain-server/domain-server.md5";

    _defaultDomain = "localhost";
    _logsPath = QDir::toNativeSeparators(_clientsLaunchPath + "logs/");
    _availableAssignmentTypes.insert("audio-mixer", 0);
    _availableAssignmentTypes.insert("avatar-mixer", 1);
    _availableAssignmentTypes.insert("voxel-server", 3);
    _availableAssignmentTypes.insert("particle-server", 4);
    _availableAssignmentTypes.insert("metavoxel-server", 5);
    _availableAssignmentTypes.insert("model-server", 6);
}

int GlobalData::indexForAssignmentType(const QString &type) {
    for (int i = 0; i < _availableAssignmentTypes.size(); ++i) {
        if (_availableAssignmentTypes.keys().at(i) == type) {
            return _availableAssignmentTypes.value(type);
        }
    }
    
    return -1;
}
