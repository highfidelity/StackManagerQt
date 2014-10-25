//
//  AssignmentWidget.cpp
//  StackManagerQt/src/ui
//
//  Created by Mohammed Nafees on 10/18/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include "AssignmentWidget.h"

#include <QLabel>
#include <QHBoxLayout>

#include "AppDelegate.h"

AssignmentWidget::AssignmentWidget(int id, QWidget* parent) :
    QWidget(parent),
    _isRunning(false),
    _id(id) {
    setFont(QFont("sans-serif"));

    QHBoxLayout* layout = new QHBoxLayout;

    _runButton = new SvgButton(this);
    _runButton->setFixedSize(59, 32);
    _runButton->setSvgImage(":/assignment-run.svg");
    _runButton->setCheckable(true);
    _runButton->setChecked(false);

    QLabel* label = new QLabel;
    label->setText("Pool ID");

    _poolIdLineEdit = new QLineEdit;
    _poolIdLineEdit->setPlaceholderText("Optional");

    layout->addWidget(_runButton, 5);
    layout->addWidget(label);
    layout->addWidget(_poolIdLineEdit);

    setLayout(layout);

    connect(_runButton, &QPushButton::clicked, this, &AssignmentWidget::toggleRunningState);
}

void AssignmentWidget::toggleRunningState(bool toggleProcess) {
    if (_isRunning) {
        if (toggleProcess) {
            AppDelegate::getInstance()->stopAssignment(_id);
        }
        _runButton->setSvgImage(":/assignment-run.svg");
        update();
        _poolIdLineEdit->setEnabled(true);
        _isRunning = false;
    } else {
        if (toggleProcess) {
            AppDelegate::getInstance()->startAssignment(_id, _poolIdLineEdit->text());
        }
        _runButton->setSvgImage(":/assignment-stop.svg");
        update();
        _poolIdLineEdit->setEnabled(false);
        _isRunning = true;
    }
}
