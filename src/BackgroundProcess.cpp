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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

BackgroundProcess::BackgroundProcess(const QString &type, QObject *parent) :
    QProcess(parent),
    _type(type) {
    _logViewer = new LogViewer(_type);

    connect(this, SIGNAL(started()), SLOT(processStarted()));
    connect(this, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));


#ifdef Q_OS_WIN
    // On Windows there are issues piping stdout/err - writing to a file first and reading from
    // there seems to solve the problem.
    _stdoutFilePos = 0;
    _stderrFilePos = 0;
    _stdoutFilename = QDir::temp().absolutePath() + "/" + _type + "_stdout.log";
    _stderrFilename = QDir::temp().absolutePath() + "/" + _type + "_stderr.log";

    qDebug() << "stdout for " << type << " being written to: " << _stdoutFilename;
    qDebug() << "stderr for " << type << " being written to: " << _stderrFilename;

    setStandardOutputFile(_stdoutFilename);
    setStandardErrorFile(_stderrFilename);

    _logTimer.setInterval(500);
    _logTimer.setSingleShot(false);
    connect(&_logTimer, SIGNAL(timeout()), this, SLOT(receivedStandardError()));
    connect(&_logTimer, SIGNAL(timeout()), this, SLOT(receivedStandardOutput()));
    connect(this, SIGNAL(started()), &_logTimer, SLOT(start()));
#else
    connect(this, SIGNAL(readyReadStandardOutput()), SLOT(receivedStandardOutput()));
    connect(this, SIGNAL(readyReadStandardError()), SLOT(receivedStandardError()));
#endif

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
    QString output;

#ifdef Q_OS_WIN
    QFile file(_stdoutFilename);

    if (!file.open(QIODevice::ReadOnly)) return;

    if (file.size() > _stdoutFilePos) {
        file.seek(_stdoutFilePos);
        output = file.readAll();
        _stdoutFilePos = file.pos();
    }

    file.close();
#else
    output = readAllStandardOutput();
#endif

    _logViewer->appendStandardOutput(output);
}

void BackgroundProcess::receivedStandardError() {
    QString output;

#ifdef Q_OS_WIN
    QFile file(_stderrFilename);

    if (!file.open(QIODevice::ReadOnly)) return;

    if (file.size() > _stderrFilePos) {
        file.seek(_stderrFilePos);
        output = file.readAll();
        _stderrFilePos = file.pos();
    }

    file.close();
#else
    output = readAllStandardError();
#endif

    _logViewer->appendStandardError(output);
}
