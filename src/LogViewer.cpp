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
}

LogViewer::~LogViewer() {
    delete ui;
}

void LogViewer::setStandardOutput(const QString& output) {
    ui->outputView->insertPlainText(output);
    ui->outputView->ensureCursorVisible();
}

void LogViewer::setStandardError(const QString& error) {
    ui->errorView->insertPlainText(error);
    ui->errorView->ensureCursorVisible();
}
