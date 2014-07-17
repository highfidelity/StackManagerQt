
#include "AssignmentClientProcess.h"
#include "GlobalData.h"

AssignmentClientProcess::AssignmentClientProcess(const QString &type, QObject* parent) :
    BackgroundProcess(type, parent),
    _type(type) {
}

void AssignmentClientProcess::startWithType() {
    QString program = GlobalData::getInstance()->getAssignmentClientExecutablePath();
    QStringList args;
    args << "-t" << QString::number(GlobalData::getInstance()->indexForAssignmentType(_type)) << "-a" << GlobalData::getInstance()->getDefaultDomain();
    start(program, args);
}
