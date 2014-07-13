
#include "LogViewer.h"
#include "ui_LogViewer.h"
#include "GlobalData.h"
#include "FileWatcherListener.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QTextStream>

LogViewer::LogViewer(const QString &type, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LogViewer) {
    ui->setupUi(this);
    QString typeLabelText = type.split("-").join(" ").toUpper();
    ui->typeLabel->setText(typeLabelText);
    setWindowTitle(typeLabelText + " Log");

    _outputLogFile = GlobalData::getInstance()->getOutputLogPathForType(type);
    _errorLogFile = GlobalData::getInstance()->getErrorLogPathForType(type);

    connect(FileWatcherListenerHandler::getInstance(), SIGNAL(fileChanged(QString)),
            SLOT(updateForFileChanges(QString)));
}

LogViewer::~LogViewer() {
    delete ui;
}

void LogViewer::updateForFileChanges(QString path) {
    if (path == _outputLogFile) {
        QFile file(_outputLogFile);
        if (file.open(QIODevice::ReadOnly)) {
            ui->outputView->setPlainText(file.readAll());
            file.close();
        }
    } else if (path == _errorLogFile) {
        QFile file(_errorLogFile);
        if (file.open(QIODevice::ReadOnly)) {
            ui->errorView->setPlainText(file.readAll());
            file.close();
        }
    }
}
