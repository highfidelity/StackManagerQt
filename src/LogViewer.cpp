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

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    ui->outputView->setFont(font);
    ui->errorView->setFont(font);
}

LogViewer::~LogViewer() {
    delete ui;
}

void LogViewer::appendStandardOutput(const QString& output) {
    QTextCursor cursor = ui->outputView->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(output);
    ui->outputView->ensureCursorVisible();
}

void LogViewer::appendStandardError(const QString& error) {
    QTextCursor cursor = ui->errorView->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(error);
    ui->errorView->ensureCursorVisible();
}
