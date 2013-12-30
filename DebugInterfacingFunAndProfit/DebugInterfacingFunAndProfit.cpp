#include "stdafx.h"
#include <QString>
#include "QtExpect.h"
#include "IProcessDebugger.h"
#include "ProcessDebugger.h"
#include "DebugInterfacingFunAndProfit.h"


DebugInterfacingFunAndProfit::DebugInterfacingFunAndProfit(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);
    TestDIA();
}

bool DebugInterfacingFunAndProfit::TestDIA()
{
    std::wstring targetExe = L"E:\\MBAM\\mbamui\\Vs2010\\Debug\\MbamUI-vc100-x86-gd-0_4_13_388.exe";

    ProcessDebugger processDebugger(targetExe, L"");

    bool success = processDebugger.StartAndAttachToProgram();

    processDebugger.ExecuteDebugLoop( 10 );

    return success;
};

DebugInterfacingFunAndProfit::~DebugInterfacingFunAndProfit()
{
    
}
