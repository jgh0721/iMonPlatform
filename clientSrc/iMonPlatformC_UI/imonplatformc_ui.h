#ifndef IMONPLATFORMC_UI_H
#define IMONPLATFORMC_UI_H

#include <QtWidgets/QDialog>
#include "ui_imonplatformc_ui.h"

class iMonPlatformC_UI : public QDialog
{
    Q_OBJECT

public:
    iMonPlatformC_UI(QWidget *parent = 0);
    ~iMonPlatformC_UI();

private:
    Ui::iMonPlatformC_UIClass ui;
};

#endif // IMONPLATFORMC_UI_H
