#ifndef IMONPLATFORMS_UI_H
#define IMONPLATFORMS_UI_H

#include <QtWidgets/QMainWindow>
#include "ui_imonplatforms_ui.h"

class iMonPlatformS_UI : public QMainWindow
{
    Q_OBJECT

public:
    iMonPlatformS_UI(QWidget *parent = 0);
    ~iMonPlatformS_UI();

private:
    Ui::iMonPlatformS_UIClass ui;
};

#endif // IMONPLATFORMS_UI_H
