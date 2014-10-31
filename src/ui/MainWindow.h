//
//  MainWindow.h
//  StackManagerQt/src/ui
//
//  Created by Mohammed Nafees on 10/17/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>

#include "SvgButton.h"

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    static MainWindow* getInstance();

    void setServerAddress(const QString& address);
    void setRequirementsLastChecked(const QString& lastCheckedDateTime);
    void setDomainServerStarted();
    void setDomainServerStopped();
    QTabWidget* getLogsWidget() { return _logsWidget; }

protected:
    virtual void paintEvent(QPaintEvent*);

private slots:
    void toggleDomainServer();
    void addAssignment();
    void openSettings();

private:
    static MainWindow* _instance;
    bool _domainServerRunning;
    QString _serverAddress;
    QString _requirementsLastCheckedDateTime;
    SvgButton* _serverButton;
    QLabel* _serverAddressLabel;
    QRect _serverButtonBounds;
    QPushButton* _viewLogsButton;
    QPushButton* _settingsButton;
    QPushButton* _runAssignmentButton;
    QTabWidget* _logsWidget;
    QVBoxLayout* _assignmentLayout;
    QScrollArea* _assignmentScrollArea;

    MainWindow();
};

#endif
