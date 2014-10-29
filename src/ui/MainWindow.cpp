//
//  MainWindow.cpp
//  StackManagerQt/src/ui
//
//  Created by Mohammed Nafees on 10/17/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include "MainWindow.h"

#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QDesktopServices>
#include <QMutex>
#include <QLayoutItem>

#include "AppDelegate.h"
#include "AssignmentWidget.h"
#include "GlobalData.h"

const int globalx = 55;
const QColor lightGrayColor = QColor(205, 205, 205);
const QColor darkGrayColor = QColor(84, 84, 84);
const QColor redColor = QColor(189, 54, 78);
const QColor greenColor = QColor(3, 150, 126);

MainWindow* MainWindow::_instance = NULL;

MainWindow* MainWindow::getInstance() {
    static QMutex instanceMutex;

    instanceMutex.lock();

    if (!_instance) {
        _instance = new MainWindow();
    }

    instanceMutex.unlock();

    return _instance;
}

MainWindow::MainWindow() :
    QWidget(0) {
    setWindowTitle("High Fidelity Stack Manager");
    if (GlobalData::getInstance()->getPlatform() == "win") {
        setGeometry(qApp->desktop()->availableGeometry().width()/2 - 270, 30, 540, 200);
    } else {
        setGeometry(qApp->desktop()->availableGeometry().width()/2 - 270, 0, 540, 200);
    }
    setFixedWidth(width());
    setMaximumHeight(qApp->desktop()->availableGeometry().height() - 40);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                   Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setMouseTracking(true);
    setStyleSheet("font-family: 'Helvetica', 'Arial', 'sans-serif';");

    _domainServerRunning = false;
    _serverAddress = "";
    _requirementsLastCheckedDateTime = "";

    _serverButton = new SvgButton(this);
    _serverButtonBounds = QRect(globalx, 37, QPixmap(":/server-start.svg").scaledToHeight(47).width(), 47);
    _serverButton->setGeometry(globalx, 37, width() - globalx*2, QPixmap(":/server-start.svg").scaledToWidth(width() - globalx*2).height());
    _serverButton->setSvgImage(":/server-start.svg");

    _viewLogsButton = new QPushButton("View Logs", this);
    _viewLogsButton->setAutoDefault(false);
    _viewLogsButton->setDefault(false);
    _viewLogsButton->setFocusPolicy(Qt::NoFocus);
    _viewLogsButton->move(257, 59);
    _viewLogsButton->hide();

    _settingsButton = new QPushButton("Settings", this);
    _settingsButton->setAutoDefault(false);
    _settingsButton->setDefault(false);
    _settingsButton->setFocusPolicy(Qt::NoFocus);
    _settingsButton->move(262 + _viewLogsButton->width(), 59);
    _settingsButton->hide();

    _runAssignmentButton = new QPushButton("Run Assignment", this);
    _runAssignmentButton->setAutoDefault(false);
    _runAssignmentButton->setDefault(false);
    _runAssignmentButton->setFocusPolicy(Qt::NoFocus);
    _runAssignmentButton->move(globalx - 7, 138);
    _runAssignmentButton->hide();

    _logsWidget = new QTabWidget;
    _logsWidget->setUsesScrollButtons(true);
    _logsWidget->setElideMode(Qt::ElideMiddle);
    _logsWidget->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    _logsWidget->resize(500, 500);

    _assignmentScrollArea = new QScrollArea(this);
    _assignmentScrollArea->setWidget(new QWidget);
    _assignmentScrollArea->setWidgetResizable(true);
    _assignmentScrollArea->setFrameShape(QFrame::NoFrame);
    _assignmentScrollArea->move(globalx - 14, 178);
    _assignmentScrollArea->setMaximumWidth(width() - globalx*2);
    _assignmentScrollArea->setMaximumHeight(qApp->desktop()->availableGeometry().height() - 215);

    _assignmentLayout = new QVBoxLayout;
    _assignmentLayout->setSpacing(0);
    _assignmentLayout->setMargin(0);
    _assignmentLayout->setContentsMargins(0, 0, 0, 0);
    _assignmentScrollArea->widget()->setLayout(_assignmentLayout);

    connect(_serverButton, &QPushButton::clicked, this, &MainWindow::toggleDomainServer);
    connect(_viewLogsButton, &QPushButton::clicked, _logsWidget, &QTabWidget::show);
    connect(_settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
    connect(_runAssignmentButton, &QPushButton::clicked, this, &MainWindow::addAssignment);

    // temporary
    _serverAddress = "hifi://localhost";
}

void MainWindow::setServerAddress(const QString &address) {
    _serverAddress = address;
}

void MainWindow::setRequirementsLastChecked(const QString& lastCheckedDateTime) {
    _requirementsLastCheckedDateTime = lastCheckedDateTime;
}

void MainWindow::setDomainServerStarted() {
    _serverButton->setSvgImage(":/server-stop.svg");
    _serverButton->setGeometry(_serverButtonBounds);
    _serverButton->update();
    _domainServerRunning = true;
    _viewLogsButton->show();
    _settingsButton->show();
    _runAssignmentButton->show();
    _assignmentScrollArea->widget()->setEnabled(true);
    update();
}

void MainWindow::setDomainServerStopped() {
    _serverButton->setSvgImage(":/server-start.svg");
    _serverButton->setGeometry(globalx, 37, width() - globalx*2, QPixmap(":/server-start.svg").scaledToWidth(width() - globalx*2).height());
    _serverButton->update();
    _domainServerRunning = false;
    _viewLogsButton->hide();
    _settingsButton->hide();
    _runAssignmentButton->hide();
    _assignmentScrollArea->widget()->setEnabled(false);
    update();
    for (int i = 0; i < _assignmentLayout->count(); ++i) {
        if (qobject_cast<AssignmentWidget*>(_assignmentLayout->itemAt(i)->widget())->isRunning()) {
            qobject_cast<AssignmentWidget*>(_assignmentLayout->itemAt(i)->widget())->toggleRunningState(false);
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int x = globalx, y = _serverButtonBounds.y();

    QFont font("sans-serif");
    if (_domainServerRunning) {
        painter.setPen(darkGrayColor);
        painter.setBrush(QBrush(Qt::transparent));
        font.setBold(true);
        font.setPixelSize(14);
        painter.setFont(font);
        x += _serverButtonBounds.width() + 15;
        painter.drawText(QRectF(x, y, QFontMetrics(font).width("Accessible at:"), 14), "Accessible at:");

        x += QFontMetrics(font).width("Accessible at:") + 5;
        font.setUnderline(true);
        painter.setFont(font);
        painter.setPen(greenColor);
        painter.drawText(QRectF(x, y, QFontMetrics(font).width(_serverAddress),
                            QFontMetrics(font).height()), _serverAddress);
    }

    y += _serverButtonBounds.height() + 19;
    if (!_requirementsLastCheckedDateTime.isEmpty()) {
        x = globalx;
        font.setBold(false);
        font.setUnderline(false);
        painter.setFont(font);
        painter.setPen(darkGrayColor);
        painter.drawText(QRectF(x, y, QFontMetrics(font).width("Requirements are up to date as of " + _requirementsLastCheckedDateTime), QFontMetrics(font).height()), "Requirements are up to date as of " + _requirementsLastCheckedDateTime);
    }

    y += QFontMetrics(font).height() + 9;
    if (_domainServerRunning) {
        painter.setPen(lightGrayColor);
        painter.drawLine(0, y, width(), y);
    }
}

void MainWindow::toggleDomainServer() {
    if (_domainServerRunning) {
        AppDelegate::getInstance()->stopDomainServer();
    } else {
        AppDelegate::getInstance()->startDomainServer();
    }
}

void MainWindow::addAssignment() {
    AssignmentWidget* widget = new AssignmentWidget(_assignmentLayout->count()+1);
    _assignmentLayout->addWidget(widget, 0, Qt::AlignTop);
    resize(width(), _assignmentScrollArea->geometry().y() + 56*_assignmentLayout->count() + 37);
    _assignmentScrollArea->resize(_assignmentScrollArea->maximumWidth(), height() - 215);
}

void MainWindow::openSettings() {
    QDesktopServices::openUrl(QUrl("http://localhost:40100/settings"));
}
