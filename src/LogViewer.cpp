//
//  LogViewer.cpp
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 07/10/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

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

    _outputLogFilePath = GlobalData::getInstance()->getOutputLogPathForType(type);
    _errorLogFilePath = GlobalData::getInstance()->getErrorLogPathForType(type);

    connect(FileWatcherListenerHandler::getInstance(), SIGNAL(fileChanged()),
            SLOT(updateForFileChanges()));
}

LogViewer::~LogViewer() {
    delete ui;
}

void LogViewer::updateForFileChanges() {
    QFile outputFile(_outputLogFilePath);
    if (outputFile.open(QIODevice::ReadOnly)) {
        ui->outputView->setPlainText("");
        ui->outputView->insertPlainText(outputFile.readAll());
        ui->outputView->ensureCursorVisible();
        outputFile.close();
    }

    QFile errorFile(_errorLogFilePath);
    if (errorFile.open(QIODevice::ReadOnly)) {
        ui->errorView->setPlainText("");
        ui->errorView->insertPlainText(errorFile.readAll());
        ui->errorView->ensureCursorVisible();
        errorFile.close();
    }
}
