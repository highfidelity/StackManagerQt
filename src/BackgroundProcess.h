//
//  BackgroundProcess.h
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 07/03/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#ifndef BACKGROUNDPROCESS_H
#define BACKGROUNDPROCESS_H

#include "LogViewer.h"

#include <QProcess>
#include <QTimer>

class BackgroundProcess : public QProcess
{
    Q_OBJECT
public:
    explicit BackgroundProcess(const QString& type, QObject* parent = 0);

    QString& getType() { return _type; }

    void displayLog();

private slots:
    void processStarted();
    void processError();
    void receivedStandardOutput();
    void receivedStandardError();

private:
    QString _type;
    LogViewer* _logViewer;
#ifdef Q_OS_WIN
    QTimer _logTimer;
    QString _stdoutFilename;
    QString _stderrFilename;
    qint64 _stdoutFilePos;
    qint64 _stderrFilePos;
#endif
};

#endif
