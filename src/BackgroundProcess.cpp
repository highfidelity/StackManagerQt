//
//  BackgroundProcess.cpp
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 07/03/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include "BackgroundProcess.h"
#include "GlobalData.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QWidget>

const int LOG_CHECK_INTERVAL_MS = 500;

const QString DATETIME_FORMAT = "yyyy-MM-dd_hh.mm.ss";
const QString LOGS_DIRECTORY = "/Logs/";

BackgroundProcess::BackgroundProcess(const QString& type, QObject *parent) :
    QProcess(parent),
    _type(type),
    _stdoutFilePos(0),
    _stderrFilePos(0)
{
    _logViewer = new LogViewer;

    connect(this, SIGNAL(started()), SLOT(processStarted()));
    connect(this, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));

    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append(LOGS_DIRECTORY);
    QDir logDir(path);
    if (!logDir.exists(path)) {
        logDir.mkpath(path);
    }

    QDateTime now = QDateTime::currentDateTime();
    QString nowString = now.toString(DATETIME_FORMAT);
    QString baseFilename = path + _type;
    _stdoutFilename = QString("%1_stdout_%2.txt").arg(baseFilename, nowString);
    _stderrFilename = QString("%1_stderr_%2.txt").arg(baseFilename, nowString);

    qDebug() << "stdout for " << type << " being written to: " << _stdoutFilename;
    qDebug() << "stderr for " << type << " being written to: " << _stderrFilename;

    setStandardOutputFile(_stdoutFilename);
    setStandardErrorFile(_stderrFilename);

    _logTimer.setInterval(LOG_CHECK_INTERVAL_MS);
    _logTimer.setSingleShot(false);
    connect(&_logTimer, SIGNAL(timeout()), this, SLOT(receivedStandardError()));
    connect(&_logTimer, SIGNAL(timeout()), this, SLOT(receivedStandardOutput()));
    connect(this, SIGNAL(started()), &_logTimer, SLOT(start()));

    setWorkingDirectory(GlobalData::getInstance()->getClientsLaunchPath());
}

void BackgroundProcess::processStarted() {
    qDebug() << "process " << _type << " started.";
}

void BackgroundProcess::processError() {
    qDebug() << errorString();
}

void BackgroundProcess::receivedStandardOutput() {
    QString output;

    QFile file(_stdoutFilename);

    if (!file.open(QIODevice::ReadOnly)) return;

    if (file.size() > _stdoutFilePos) {
        file.seek(_stdoutFilePos);
        output = file.readAll();
        _stdoutFilePos = file.pos();
    }

    file.close();

    if (!output.isEmpty() && !output.isNull()) {
        _logViewer->appendStandardOutput(output);
    }
}

void BackgroundProcess::receivedStandardError() {
    QString output;

    QFile file(_stderrFilename);

    if (!file.open(QIODevice::ReadOnly)) return;

    if (file.size() > _stderrFilePos) {
        file.seek(_stderrFilePos);
        output = file.readAll();
        _stderrFilePos = file.pos();
    }

    file.close();

    if (!output.isEmpty() && !output.isNull()) {
        _logViewer->appendStandardError(output);
    }
}
