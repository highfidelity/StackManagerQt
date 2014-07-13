
#ifndef ASSIGNMENTCLIENTPROCESS_H
#define ASSIGNMENTCLIENTPROCESS_H

#include "BackgroundProcess.h"

class AssignmentClientProcess : public BackgroundProcess
{
    Q_OBJECT
public:
    explicit AssignmentClientProcess(const QString& type, QObject* parent = 0);

    void startWithType();

private:
    QString _type;
};

#endif
