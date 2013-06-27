#include "stdafx.h"
#include "imonplatforms_ui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    iMonPlatformS_UI w;
    w.show();
    return a.exec();
}
