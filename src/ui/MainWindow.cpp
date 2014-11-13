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
#include <QCursor>

#include "AppDelegate.h"
#include "AssignmentWidget.h"
#include "GlobalData.h"

const int globalX = 55;
const int serverButtonYCoord = 37;
const int assignmentScrollAreaHeightOffset = 215;
const int resizeFactor = 56;
const int assignmentLayoutWidgetStretch = 0;
const int fontPixelSize = 14;
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
    QWidget(0)
{
    setWindowTitle("High Fidelity Stack Manager");
    const int windowFixedWidth = 540;
    const int windowInitialHeight = 200;
    if (GlobalData::getInstance()->getPlatform() == "win") {
        const int windowsYCoord = 30;
        setGeometry(qApp->desktop()->availableGeometry().width() / 2 - windowFixedWidth / 2, windowsYCoord,
                    windowFixedWidth, windowInitialHeight);
    } else {
        const int unixYCoord = 0;
        setGeometry(qApp->desktop()->availableGeometry().width() / 2 - windowFixedWidth / 2, unixYCoord,
                    windowFixedWidth, windowInitialHeight);
    }
    setFixedWidth(windowFixedWidth);
    setMaximumHeight(qApp->desktop()->availableGeometry().height());
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                   Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setMouseTracking(true);
    setStyleSheet("font-family: 'Helvetica', 'Arial', 'sans-serif';");

    _domainServerRunning = false;
    _serverAddress = "";
    _requirementsLastCheckedDateTime = "";

    const int serverButtonHeight = 47;
    _serverButton = new SvgButton(this);
    _serverButtonBounds = QRect(globalX, serverButtonYCoord,
                                QPixmap(":/server-start.svg").scaledToHeight(serverButtonHeight).width(),
                                serverButtonHeight);
    _serverButton->setGeometry(globalX, serverButtonYCoord, width() - globalX * 2,
                               QPixmap(":/server-start.svg").scaledToWidth(width() - globalX * 2).height());
    _serverButton->setSvgImage(":/server-start.svg");

    const int serverAddressLabelXCoord = 360;
    _serverAddressLabel = new QLabel(this);
    _serverAddressLabel->move(serverAddressLabelXCoord, _serverButtonBounds.y());
    _serverAddressLabel->setOpenExternalLinks(true);
    _serverAddressLabel->hide();

    const int viewLogsButtonXCoord = 257;
    const int viewLogsButtonYCoord = 59;
    _viewLogsButton = new QPushButton("View Logs", this);
    _viewLogsButton->setAutoDefault(false);
    _viewLogsButton->setDefault(false);
    _viewLogsButton->setFocusPolicy(Qt::NoFocus);
    _viewLogsButton->move(viewLogsButtonXCoord, viewLogsButtonYCoord);
    _viewLogsButton->hide();

    const int settingsButtonXCoord = 262;
    const int settingsButtonYCoord = 59;
    _settingsButton = new QPushButton("Settings", this);
    _settingsButton->setAutoDefault(false);
    _settingsButton->setDefault(false);
    _settingsButton->setFocusPolicy(Qt::NoFocus);
    _settingsButton->move(settingsButtonXCoord + _viewLogsButton->width(), settingsButtonYCoord);
    _settingsButton->hide();

    const int runAssignmentButtonXCoordOffset = 7;
    const int runAssignmentButtonYCoord = 138;
    _runAssignmentButton = new QPushButton("Run Assignment", this);
    _runAssignmentButton->setAutoDefault(false);
    _runAssignmentButton->setDefault(false);
    _runAssignmentButton->setFocusPolicy(Qt::NoFocus);
    _runAssignmentButton->move(globalX - runAssignmentButtonXCoordOffset, runAssignmentButtonYCoord);
    _runAssignmentButton->hide();

    const QSize logsWidgetSize = QSize(500, 500);
    _logsWidget = new QTabWidget;
    _logsWidget->setUsesScrollButtons(true);
    _logsWidget->setElideMode(Qt::ElideMiddle);
    _logsWidget->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                                Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    _logsWidget->resize(logsWidgetSize);

    const int assignmentScrollAreaXCoordOffset = 14;
    const int assignmentScrollAreaYCoord = 178;
    _assignmentScrollArea = new QScrollArea(this);
    _assignmentScrollArea->setWidget(new QWidget);
    _assignmentScrollArea->setWidgetResizable(true);
    _assignmentScrollArea->setFrameShape(QFrame::NoFrame);
    _assignmentScrollArea->move(globalX - assignmentScrollAreaXCoordOffset, assignmentScrollAreaYCoord);
    _assignmentScrollArea->setMaximumWidth(width() - globalX * 2);
    _assignmentScrollArea->setMaximumHeight(qApp->desktop()->availableGeometry().height() -
                                            assignmentScrollAreaHeightOffset);

    const int assignmentLayoutSpacingMargin = 0;
    _assignmentLayout = new QVBoxLayout;
    _assignmentLayout->setSpacing(assignmentLayoutSpacingMargin);
    _assignmentLayout->setMargin(assignmentLayoutSpacingMargin);
    _assignmentLayout->setContentsMargins(assignmentLayoutSpacingMargin, assignmentLayoutSpacingMargin,
                                          assignmentLayoutSpacingMargin, assignmentLayoutSpacingMargin);
    _assignmentScrollArea->widget()->setLayout(_assignmentLayout);

    connect(_serverButton, &QPushButton::clicked, this, &MainWindow::toggleDomainServer);
    connect(_viewLogsButton, &QPushButton::clicked, _logsWidget, &QTabWidget::show);
    connect(_settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
    connect(_runAssignmentButton, &QPushButton::clicked, this, &MainWindow::addAssignment);

    // temporary
    {
        _serverAddress = "hifi://localhost";
        _serverAddressLabel->setText("<html><head/><body><p><a href=\"" + _serverAddress + "\">"
                                     "<span style=\"font:14px 'Helvetica', 'Arial', 'sans-serif';"
                                     "font-weight: bold; color:#29957e;\">" + _serverAddress +
                                     "</span></a></p></body></html>");
    }
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
    _serverAddressLabel->show();
    _viewLogsButton->show();
    _settingsButton->show();
    _runAssignmentButton->show();
    _assignmentScrollArea->widget()->setEnabled(true);
    update();
}

void MainWindow::setDomainServerStopped() {
    _serverButton->setSvgImage(":/server-start.svg");
    _serverButton->setGeometry(globalX, serverButtonYCoord, width() - globalX * 2,
                               QPixmap(":/server-start.svg").scaledToWidth(width() - globalX * 2).height());
    _serverButton->update();
    _domainServerRunning = false;
    _serverAddressLabel->hide();
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

    int x = globalX, y = _serverButtonBounds.y();

    QFont font("Helvetica");
    font.insertSubstitutions("Helvetica", QStringList() << "Arial" << "sans-serif");
    if (_domainServerRunning) {
        painter.setPen(darkGrayColor);
        painter.setBrush(QBrush(Qt::transparent));
        font.setBold(true);
        font.setPixelSize(fontPixelSize);
        painter.setFont(font);
        x += _serverButtonBounds.width() + 15;
        painter.drawText(QRectF(x, y, QFontMetrics(font).width("Accessible at:") + 2, fontPixelSize),
                         "Accessible at:");
    }

    y += _serverButtonBounds.height() + 19;
    if (!_requirementsLastCheckedDateTime.isEmpty()) {
        x = globalX;
        font.setBold(false);
        font.setUnderline(false);
        painter.setFont(font);
        painter.setPen(darkGrayColor);
        painter.drawText(QRectF(x, y, QFontMetrics(font).width("Requirements are up to date as of " +
                                                               _requirementsLastCheckedDateTime),
                                QFontMetrics(font).height()), "Requirements are up to date as of " +
                         _requirementsLastCheckedDateTime);
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
    AssignmentWidget* widget = new AssignmentWidget(_assignmentLayout->count() + 1);
    _assignmentLayout->addWidget(widget, assignmentLayoutWidgetStretch, Qt::AlignTop);
    resize(width(), _assignmentScrollArea->geometry().y() + resizeFactor * _assignmentLayout->count() +
           serverButtonYCoord);
    _assignmentScrollArea->resize(_assignmentScrollArea->maximumWidth(), height() - assignmentScrollAreaHeightOffset);
}

void MainWindow::openSettings() {
    QDesktopServices::openUrl(QUrl("http://localhost:40100/settings"));
}
