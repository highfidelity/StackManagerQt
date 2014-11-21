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
    _id(id),
    _isRunning(false)
{
    setFont(QFont("sans-serif"));

    QHBoxLayout* layout = new QHBoxLayout;

    _runButton = new SvgButton(this);
    _runButton->setFixedSize(59, 32);
    _runButton->setSvgImage(":/assignment-run.svg");
    _runButton->setCheckable(true);
    _runButton->setChecked(false);

    QLabel* label = new QLabel;
    label->setText("Pool ID");

    _poolIDLineEdit = new QLineEdit;
    _poolIDLineEdit->setPlaceholderText("Optional");

    layout->addWidget(_runButton, 5);
    layout->addWidget(label);
    layout->addWidget(_poolIDLineEdit);

    setLayout(layout);

    connect(_runButton, &QPushButton::clicked, this, &AssignmentWidget::toggleRunningState);
}

void AssignmentWidget::toggleRunningState() {
    if (_isRunning) {
        AppDelegate::getInstance()->stopAssignment(_id);
        _runButton->setSvgImage(":/assignment-run.svg");
        update();
        _poolIDLineEdit->setEnabled(true);
        _isRunning = false;
    } else {
        AppDelegate::getInstance()->startAssignment(_id, _poolIDLineEdit->text());
        _runButton->setSvgImage(":/assignment-stop.svg");
        update();
        _poolIDLineEdit->setEnabled(false);
        _isRunning = true;
    }
}
