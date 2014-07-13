
#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QWidget>

namespace Ui {
    class LogViewer;
}

class LogViewer : public QWidget
{
    Q_OBJECT
public:
    explicit LogViewer(const QString& type, QWidget* parent = 0);
    ~LogViewer();

public slots:
    void updateForFileChanges(QString path);

private:
    Ui::LogViewer *ui;
    QString _outputLogFile;
    QString _errorLogFile;
};

#endif
