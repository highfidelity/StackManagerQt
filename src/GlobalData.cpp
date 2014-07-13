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

GlobalData::GlobalData() {
    QString urlBase = "http://highfidelity-public.s3-us-west-1.amazonaws.com";
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
    _domainServerExecutablePath = QDir::toNativeSeparators(_clientsLaunchPath + domainServerExecutable);

    _requirementsURL = urlBase + "/binaries/" + _platform + "/requirements/requirements.zip";
    _assignmentClientURL = urlBase + "/binaries/" + _platform + "/assignment-client" + (_platform == "win" ? "/assignment-client.exe" : "/assignment-client");
    _domainServerResourcesURL = urlBase + "/binaries/" + _platform + "/domain-server/resources.zip";
    _domainServerURL = urlBase + "/binaries/" + _platform + "/domain-server" + (_platform == "win" ? "/domain-server.exe" : "/domain-server");

    _assignmentClientMD5URL = urlBase + "/binaries/" + _platform + "/assignment-client/assignment-client.md5";
    _domainServerMD5URL = urlBase + "/binaries/" + _platform + "/domain-server/domain-server.md5";

    _defaultDomain = "localhost";
    _logsPath = QDir::toNativeSeparators(_clientsLaunchPath + "logs/");
    _availableAssignmentTypes << "audio-mixer"
                              << "avatar-mixer"
                              << "voxel-server"
                              << "particle-server"
                              << "metavoxel-server"
                              << "model-server";
}

QString GlobalData::getOutputLogPathForType(const QString& type) {
    return QDir::toNativeSeparators(_logsPath + type + "_output.log");
}

QString GlobalData::getErrorLogPathForType(const QString& type) {
    return QDir::toNativeSeparators(_logsPath + type + "_error.log");
}
