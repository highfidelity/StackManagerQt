

#ifndef APPDELEGATE_H
#define APPDELEGATE_H

#include "BackgroundProcess.h"

#include <QWidget>
#include <QList>
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
    void buttonClicked(QString buttonId);
    void onFileSuccessfullyInstalled(QUrl url);

private:
    void createExecutablePath();
    void downloadLatestExecutablesAndRequirements();
    BackgroundProcess* findBackgroundProcess(QString type);

    Ui::AppDelegate *ui;
    QString _updatingString;
    QString _upToDateString;
    bool _qtReady;
    bool _dsReady;
    bool _dsResourcesReady;
    bool _acReady;
    bool _startStopAll;
    QList<BackgroundProcess*> _backgroundProcesses;
    QSignalMapper* _signalMapper;
};

#endif
