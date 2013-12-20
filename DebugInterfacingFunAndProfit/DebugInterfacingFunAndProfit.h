#ifndef DEBUGINTERFACINGFUNANDPROFIT_H
#define DEBUGINTERFACINGFUNANDPROFIT_H

#include <QtGui/QMainWindow>
#include "ui_DebugInterfacingFunAndProfit.h"

// forward decl

class DebugInterfacingFunAndProfit : public QMainWindow
{
    Q_OBJECT

public:
    DebugInterfacingFunAndProfit(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~DebugInterfacingFunAndProfit();
    bool TestDIA();
private:
    Ui::DebugInterfacingFunAndProfitClass ui;
};

#endif // DEBUGINTERFACINGFUNANDPROFIT_H
