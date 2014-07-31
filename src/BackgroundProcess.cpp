//
//  BackgroundProcess.cpp
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 07/03/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include "BackgroundProcess.h"
#include "GlobalData.h"

#include <QWidget>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

BackgroundProcess::BackgroundProcess(const QString &type, QObject *parent) :
    QProcess(parent),
    _type(type) {
    _logViewer = new LogViewer(_type);

    connect(this, SIGNAL(started()), SLOT(processStarted()));
    connect(this, SIGNAL(readyReadStandardOutput()), SLOT(receivedStandardOutput()));
    connect(this, SIGNAL(readyReadStandardError()), SLOT(receivedStandardError()));
    connect(this, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));

    setWorkingDirectory(GlobalData::getInstance()->getClientsLaunchPath());
}

void BackgroundProcess::displayLog() {
    if (_logViewer->isVisible()) {
        _logViewer->raise();
    } else {
        _logViewer->show();
    }
}

void BackgroundProcess::processStarted() {
    qDebug() << "process " << _type << " started.";
}

void BackgroundProcess::processError() {
    qDebug() << errorString();
}

void BackgroundProcess::receivedStandardOutput() {
    _logViewer->setStandardOutput(readAllStandardOutput());
}

void BackgroundProcess::receivedStandardError() {
    _logViewer->setStandardError(readAllStandardError());
}
