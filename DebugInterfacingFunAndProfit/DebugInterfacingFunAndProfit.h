#ifndef DEBUGINTERFACINGFUNANDPROFIT_H
#define DEBUGINTERFACINGFUNANDPROFIT_H

#include <QtGui/QMainWindow>
#include "ui_DebugInterfacingFunAndProfit.h"

#include "ITestOMaticClient.h"
#include "TestOMaticClient.h"

class DebugInterfacingFunAndProfit : public QMainWindow
{
    Q_OBJECT
private:

    // Assocations
    Ui::DebugInterfacingFunAndProfitClass m_Ui;
    ITestOMaticClient *m_TestOMatic;

public:
    DebugInterfacingFunAndProfit(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~DebugInterfacingFunAndProfit();
    bool SelectAndAttachToProcess();

    bool RefreshWidgets();

public slots:
    void OnMenuTriggered(QAction *action);
    void OnItemClicked(QListWidgetItem *item);

};

#endif // DEBUGINTERFACINGFUNANDPROFIT_H
