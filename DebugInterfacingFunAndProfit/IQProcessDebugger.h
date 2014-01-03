#pragma once;

struct ProcessDebugger
{
    enum DebuggerType
    {
        Windows
    };
};

class IQProcessDebugger
{
public:
    // Potential Operations
    virtual QString InjectionDllName() const = 0;
    virtual QString ProgramArgs() const = 0;
    virtual QString ProgramName() const = 0;
    virtual ProcessDebugger::DebuggerType Type() const = 0;
    virtual void MoveToThread(QThread *thread) = 0;
    virtual QObject *GetQObject() = 0;
    virtual bool ExecuteDebugLoop() = 0;

    // Potential Slots
    virtual void StartAndAttachToProgram() = 0;

    // Potential Signals
    virtual bool DebugProcessIsReadyForConnections() = 0;
};

Q_DECLARE_INTERFACE(IQProcessDebugger, "EaganRackley.IProcessDebugger/1.0")