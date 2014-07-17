
#include "BackgroundProcess.h"
#include "GlobalData.h"

#include <QWidget>
#include <QFile>
#include <QDebug>

BackgroundProcess::BackgroundProcess(const QString &type, QObject *parent) :
    QProcess(parent),
    _type(type) {
    QString outputFilePath = GlobalData::getInstance()->getOutputLogPathForType(_type);
    QString errorFilePath = GlobalData::getInstance()->getErrorLogPathForType(_type);

    setStandardOutputFile(outputFilePath);
    setStandardErrorFile(errorFilePath);

    _logViewer = new LogViewer(_type);

    //connect(this, SIGNAL(readyRead()), SLOT(onReadyRead()));
    connect(this, SIGNAL(started()), SLOT(processStarted()));
    connect(this, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));
}

void BackgroundProcess::onReadyRead() {
    qDebug() << readAll();
}

void BackgroundProcess::displayLog() {
    _logViewer->updateForFileChanges(GlobalData::getInstance()->getOutputLogPathForType(_type));
    _logViewer->updateForFileChanges(GlobalData::getInstance()->getErrorLogPathForType(_type));
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
