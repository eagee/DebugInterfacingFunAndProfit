#include "stdafx.h"
#include "IQProcessDebugger.h"
#include "WinProcessDebugger.h"
#include "QProcessDebuggerFactory.h"

IQProcessDebugger * QProcessDebuggerFactory::GetDebuggerOfType(QString targetExecutable, QString parameters, ProcessDebugger::DebuggerType type, QObject *parent)
{
    if(type == ProcessDebugger::Windows)
    {
        return qobject_cast<IQProcessDebugger*>(new WinProcessDebugger(targetExecutable, parameters, parent));
    }
    else
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, "Platform not implemented!" );
        throw std::exception( "Platform not implemented!" );

    }
}

