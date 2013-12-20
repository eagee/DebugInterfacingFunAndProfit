#include "stdafx.h"
#include "DebugInterfacingFunAndProfit.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DebugInterfacingFunAndProfit w;
    w.show();
    return a.exec();
}
