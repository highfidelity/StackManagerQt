
#include "BackgroundProcess.h"
#include "GlobalData.h"

#include <QWidget>
#include <QDebug>

BackgroundProcess::BackgroundProcess(const QString &type, QObject *parent) :
    QProcess(parent),
    _type(type) {
    QString outputFilePath = GlobalData::getInstance()->getOutputLogPathForType(_type);
    QString errorFilePath = GlobalData::getInstance()->getErrorLogPathForType(_type);

    setStandardOutputFile(outputFilePath);
    setStandardErrorFile(errorFilePath);
    _logViewer = new LogViewer(_type);

    connect(this, SIGNAL(started()), SLOT(processStarted()));
    connect(this, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));
}

void BackgroundProcess::displayLog() {
    _logViewer->updateForFileChanges(GlobalData::getInstance()->getOutputLogPathForType(_type));
    _logViewer->updateForFileChanges(GlobalData::getInstance()->getErrorLogPathForType(_type));
    _logViewer->show();
}

void BackgroundProcess::processStarted() {
    qDebug() << "process: " << _type << " started.";
}

void BackgroundProcess::processError() {
    qDebug() << errorString();
}
