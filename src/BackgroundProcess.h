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

private:
    QString _type;
    LogViewer* _logViewer;
};

#endif
