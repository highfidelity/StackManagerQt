

#ifndef APPDELEGATE_H
#define APPDELEGATE_H

#include "BackgroundProcess.h"

#include <QWidget>
#include <QList>
#include <QNetworkAccessManager>
#include <QSignalMapper>
#include <QUrl>

namespace Ui {
    class AppDelegate;
}

class AppDelegate : public QWidget
{
    Q_OBJECT
public:
    explicit AppDelegate(QWidget* parent = 0);
    ~AppDelegate();

private slots:
    void retryConnection();
    void buttonClicked(QString buttonId);
    void onFileSuccessfullyInstalled(QUrl url);

private:
    void createExecutablePath();
    void downloadLatestExecutablesAndRequirements();
    BackgroundProcess* findBackgroundProcess(QString type);

    Ui::AppDelegate *ui;
    QString _updatingString;
    QString _upToDateString;
    QNetworkAccessManager* _manager;
    bool _qtReady;
    bool _dsReady;
    bool _dsResourcesReady;
    bool _acReady;
    bool _startStopAll;
    QList<BackgroundProcess*> _backgroundProcesses;
    QSignalMapper* _signalMapper;
};

#endif
