#pragma once

#include "IQProcessDebugger.h"
#include "WinProcessDebugger.h"

class QProcessDebuggerFactory
{
public:
    static IQProcessDebugger *GetDebuggerOfType(QString targetExecutable, QString parameters, ProcessDebugger::DebuggerType type, QObject *parent);
};