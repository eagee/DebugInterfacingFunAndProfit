#pragma once

#include "IQProcessDebugger.h"
#include "SymbolEngine.h"

class WinProcessDebugger : public QObject, public IQProcessDebugger
{
    Q_OBJECT
    Q_INTERFACES( IQProcessDebugger )

private:
    // Attributes
    std::wstring m_TargetProgramName;
    std::wstring m_TargetProgramArgs;
    std::map<LPVOID, std::wstring> m_ModuleNames;

    // Associations    
    PROCESS_INFORMATION m_ProcessInfo;
    DWORD m_ProcessID;
    HANDLE m_DebugeeProcessHandle;
    HMODULE m_hKern32;
    CSymbolEngine m_SymbolEngine;
    PVOID m_LoadLibraryAddress;

public:

    explicit WinProcessDebugger(QString program, QString arguments, QObject *parent);

    ~WinProcessDebugger() { }

    virtual ProcessDebugger::DebuggerType Type() const { return ProcessDebugger::Windows; };

    virtual QString InjectionDllName() const;

    virtual QString ProgramArgs() const { return QString::fromStdWString(m_TargetProgramArgs); }

    virtual QString ProgramName() const { return QString::fromStdWString(m_TargetProgramName); }

    virtual void MoveToThread(QThread *thread);

    virtual bool ExecuteDebugLoop();

    virtual QObject *GetQObject();

public slots:

    virtual void StartAndAttachToProgram();

signals:

    bool DebugProcessIsReadyForConnections();

private: 

    bool EnableDebugging();

    bool GetCorrectLoadLibraryAddress();

    bool AttachToProcess(DWORD processID );

    bool InjectTestOMaticServerDll(std::wstring fullPathToDll);

    void OnCreateProcessEvent( DWORD ProcessId );
    
    void OnExitProcessEvent( DWORD ProcessId );
    
    void OnCreateThreadEvent( DWORD ThreadId );
    
    void OnExitThreadEvent( DWORD ThreadId );
    
    void OnLoadModuleEvent( LPVOID ImageBase, HANDLE hFile );
    
    void OnUnloadModuleEvent( LPVOID ImageBase );
    
    void OnExceptionEvent( DWORD ThreadId, const EXCEPTION_DEBUG_INFO& Info );
    
    void OnDebugStringEvent( DWORD ThreadId, const OUTPUT_DEBUG_STRING_INFO& Info );

    void OnTimeout();

    bool HandleDebugEvent(DEBUG_EVENT &debugEvent, bool &initialBreakpointTriggered);

    bool GetFileNameFromHandle( HANDLE hFile, std::wstring& fileName );

    bool FileSizeIsValid(HANDLE hFile);

    void ReplaceDeviceNameWithDriveLetter( std::wstring& fileName );

    bool GetModuleSize( HANDLE hProcess, LPVOID imageBase, DWORD& moduleSize );

};


