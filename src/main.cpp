//
//  main.cpp
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 06/27/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

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
