//
//  MainWindow.h
//  StackManagerQt/src/ui
//
//  Created by Mohammed Nafees on 10/17/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#ifndef hifi_MainWindow_h
#define hifi_MainWindow_h

#include <QWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>

#include "SvgButton.h"

class MainWindow : public QWidget {
    Q_OBJECT
public:
    static MainWindow* getInstance();

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
    void updateServerAddressLabel(const QString& serverAddress);
    void handleShareButton();

private:
    static MainWindow* _instance;
    bool _domainServerRunning;
    
    QString _requirementsLastCheckedDateTime;
    SvgButton* _startServerButton;
    SvgButton* _stopServerButton;
    QLabel* _serverAddressLabel;
    QPushButton* _viewLogsButton;
    QPushButton* _settingsButton;
    QPushButton* _runAssignmentButton;
    QPushButton* _shareButton;
    QTabWidget* _logsWidget;
    QVBoxLayout* _assignmentLayout;
    QScrollArea* _assignmentScrollArea;

    MainWindow();
};

#endif
