#include <QApplication>

#include "AppDelegate.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Stack Manager");
    QApplication::setOrganizationName("High Fidelity");
    QApplication::setOrganizationDomain("io.highfidelity.StackManager");

    AppDelegate appDelegate;
    appDelegate.show();

    return app.exec();
}
