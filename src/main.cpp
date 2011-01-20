/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <mk219533 (a) students.mimuw.edu.pl> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 *
 * main.cpp
 *
 *  Created on: 2010-01-04
 *      Author: Marcel Kolodziejczyk
 */

#include <QApplication>
#include <QTextCodec>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(application);

    QApplication app(argc, argv);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    MainWindow mainWin;
    mainWin.show();
    for (int i = 1; i < argc; ++i) {
      mainWin.addChild(argv[i]);
    }
    return app.exec();
}

