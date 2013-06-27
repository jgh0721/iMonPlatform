#include "stdafx.h"
#include "imonplatformc_ui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    iMonPlatformC_UI w;
    w.show();
    return a.exec();
}
