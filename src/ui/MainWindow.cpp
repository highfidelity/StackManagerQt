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

const int GLOBAL_X_PADDING = 55;
const int TOP_Y_PADDING = 25;
const int REQUIREMENTS_TEXT_TOP_MARGIN = 19;
const int HORIZONTAL_RULE_TOP_MARGIN = 25;

const int BUTTON_PADDING_FIX = -5;

const int ASSIGNMENT_LAYOUT_RESIZE_FACTOR = 56;
const int ASSIGNMENT_LAYOUT_WIDGET_STRETCH = 0;
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
    QWidget(0),
    _domainServerRunning(false),
    _startServerButton(NULL),
    _stopServerButton(NULL),
    _serverAddressLabel(NULL),
    _viewLogsButton(NULL),
    _settingsButton(NULL),
    _runAssignmentButton(NULL),
    _logsWidget(NULL),
    _assignmentLayout(NULL),
    _assignmentScrollArea(NULL)
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

    const int SERVER_BUTTON_HEIGHT = 47;
    
    _startServerButton = new SvgButton(this);
    
    QPixmap scaledStart(":/server-start.svg");
    scaledStart.scaledToHeight(SERVER_BUTTON_HEIGHT);
    
    _startServerButton->setGeometry((width() / 2.0f) - (scaledStart.width() / 2.0f),
                                    (height() / 2.0f) - (scaledStart.height() / 2.0f),
                                    scaledStart.width(),
                                    scaledStart.height());
    _startServerButton->setSvgImage(":/server-start.svg");
    
    _stopServerButton = new SvgButton(this);
    _stopServerButton->setSvgImage(":/server-stop.svg");
    _stopServerButton->setGeometry(GLOBAL_X_PADDING, TOP_Y_PADDING,
                                   scaledStart.width(), scaledStart.height());
    _stopServerButton->hide();

    const int SERVER_ADDRESS_LABEL_LEFT_MARGIN = 20;
    _serverAddressLabel = new QLabel(this);
    _serverAddressLabel->move(_stopServerButton->geometry().right() + SERVER_ADDRESS_LABEL_LEFT_MARGIN,
                              TOP_Y_PADDING);
    _serverAddressLabel->setOpenExternalLinks(true);
    _serverAddressLabel->hide();

    const int SECONDARY_BUTTON_ROW_TOP_MARGIN = 10;
    
    int secondaryButtonY = _stopServerButton->geometry().bottom() + SECONDARY_BUTTON_ROW_TOP_MARGIN;
    
    _viewLogsButton = new QPushButton("View Logs", this);
    _viewLogsButton->setAutoDefault(false);
    _viewLogsButton->setDefault(false);
    _viewLogsButton->setFocusPolicy(Qt::NoFocus);
    _viewLogsButton->move(GLOBAL_X_PADDING + BUTTON_PADDING_FIX, secondaryButtonY);
    _viewLogsButton->hide();

    _settingsButton = new QPushButton("Settings", this);
    _settingsButton->setAutoDefault(false);
    _settingsButton->setDefault(false);
    _settingsButton->setFocusPolicy(Qt::NoFocus);
    _settingsButton->move(GLOBAL_X_PADDING + _viewLogsButton->width(), secondaryButtonY);
    _settingsButton->hide();
    
    const int ASSIGNMENT_BUTTON_TOP_MARGIN = 10;

    _runAssignmentButton = new QPushButton("Run Assignment", this);
    _runAssignmentButton->setAutoDefault(false);
    _runAssignmentButton->setDefault(false);
    _runAssignmentButton->setFocusPolicy(Qt::NoFocus);
    _runAssignmentButton->move(GLOBAL_X_PADDING + BUTTON_PADDING_FIX,
                               _viewLogsButton->geometry().bottom() + REQUIREMENTS_TEXT_TOP_MARGIN
                               + HORIZONTAL_RULE_TOP_MARGIN + ASSIGNMENT_BUTTON_TOP_MARGIN);
    _runAssignmentButton->hide();

    const QSize logsWidgetSize = QSize(500, 500);
    _logsWidget = new QTabWidget;
    _logsWidget->setUsesScrollButtons(true);
    _logsWidget->setElideMode(Qt::ElideMiddle);
    _logsWidget->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                                Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    _logsWidget->resize(logsWidgetSize);
    
    const int ASSIGNMENT_SCROLL_AREA_TOP_MARGIN = 10;

    _assignmentScrollArea = new QScrollArea(this);
    _assignmentScrollArea->setWidget(new QWidget);
    _assignmentScrollArea->setWidgetResizable(true);
    _assignmentScrollArea->setFrameShape(QFrame::NoFrame);
    _assignmentScrollArea->move(GLOBAL_X_PADDING, _runAssignmentButton->geometry().bottom() + ASSIGNMENT_SCROLL_AREA_TOP_MARGIN);
    _assignmentScrollArea->setMaximumWidth(width() - GLOBAL_X_PADDING * 2);
    _assignmentScrollArea->setMaximumHeight(qApp->desktop()->availableGeometry().height() -
                                            _assignmentScrollArea->geometry().top());

    const int assignmentLayoutSpacingMargin = 0;
    _assignmentLayout = new QVBoxLayout;
    _assignmentLayout->setSpacing(assignmentLayoutSpacingMargin);
    _assignmentLayout->setMargin(assignmentLayoutSpacingMargin);
    _assignmentLayout->setContentsMargins(assignmentLayoutSpacingMargin, assignmentLayoutSpacingMargin,
                                          assignmentLayoutSpacingMargin, assignmentLayoutSpacingMargin);
    _assignmentScrollArea->widget()->setLayout(_assignmentLayout);

    connect(_startServerButton, &QPushButton::clicked, this, &MainWindow::toggleDomainServer);
    connect(_stopServerButton, &QPushButton::clicked, this, &MainWindow::toggleDomainServer);
    connect(_viewLogsButton, &QPushButton::clicked, _logsWidget, &QTabWidget::show);
    connect(_settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
    connect(_runAssignmentButton, &QPushButton::clicked, this, &MainWindow::addAssignment);

    _serverAddress = "hifi://localhost";
    _serverAddressLabel->setText("<html><head/><body style=\"font:14px 'Helvetica', 'Arial', 'sans-serif';"
                                 "font-weight: bold;\"><p><span style=\"color:#545454;\">Accessible at: </span>"
                                 "<a href=\"" + _serverAddress + "\">"
                                 "<span style=\"color:#29957e;\">" + _serverAddress +
                                 "</span></a></p></body></html>");
}

void MainWindow::setServerAddress(const QString &address) {
    _serverAddress = address;
}

void MainWindow::setRequirementsLastChecked(const QString& lastCheckedDateTime) {
    _requirementsLastCheckedDateTime = lastCheckedDateTime;
}

void MainWindow::setDomainServerStarted() {
    _stopServerButton->show();
    _startServerButton->hide();
    _domainServerRunning = true;
    _serverAddressLabel->show();
    _viewLogsButton->show();
    _settingsButton->show();
    _runAssignmentButton->show();
    _assignmentScrollArea->widget()->setEnabled(true);
    update();
}

void MainWindow::setDomainServerStopped() {
    _startServerButton->show();
    _stopServerButton->hide();
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

    QFont font("Helvetica");
    font.insertSubstitutions("Helvetica", QStringList() << "Arial" << "sans-serif");
    
    int currentY = (_domainServerRunning ?  _viewLogsButton->geometry().bottom() : _startServerButton->geometry().bottom())
        + REQUIREMENTS_TEXT_TOP_MARGIN;
    
    if (!_requirementsLastCheckedDateTime.isEmpty()) {
        font.setBold(false);
        font.setUnderline(false);
        painter.setFont(font);
        painter.setPen(darkGrayColor);
        
        QString requirementsString = "Requirements are up to date as of " + _requirementsLastCheckedDateTime;
        float fontWidth = QFontMetrics(font).width(requirementsString);
        
        painter.drawText(QRectF(_domainServerRunning ? GLOBAL_X_PADDING : ((width() - fontWidth)/ 2.0f),
                                currentY,
                                fontWidth,
                                QFontMetrics(font).height()),
                         "Requirements are up to date as of " + _requirementsLastCheckedDateTime);
    }
    
    if (_domainServerRunning) {
        currentY += HORIZONTAL_RULE_TOP_MARGIN;
        painter.setPen(lightGrayColor);
        painter.drawLine(0, currentY, width(), currentY);
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
    _assignmentLayout->addWidget(widget, ASSIGNMENT_LAYOUT_WIDGET_STRETCH, Qt::AlignTop);
    resize(width(), _assignmentScrollArea->geometry().y()
           + ASSIGNMENT_LAYOUT_RESIZE_FACTOR * _assignmentLayout->count()
           + TOP_Y_PADDING);
    _assignmentScrollArea->resize(_assignmentScrollArea->maximumWidth(), height() - _assignmentScrollArea->geometry().top());
}

void MainWindow::openSettings() {
    QDesktopServices::openUrl(QUrl("http://localhost:40100/settings/"));
}
