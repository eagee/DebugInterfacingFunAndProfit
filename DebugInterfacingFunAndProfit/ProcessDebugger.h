#pragma once

#include "IProcessDebugger.h"

class ProcessDebugger : IProcessDebugger
{
private:
    // Attributes
    std::wstring m_ProgramName;
    std::wstring m_ProgramArgs;
    std::map<LPVOID, std::wstring> m_ModuleNames;

    // Associations
    PROCESS_INFORMATION m_ProcessInfo;
    HANDLE m_DebugProcessHandle;
public:

    explicit ProcessDebugger(std::wstring program, std::wstring arguments) : m_ProgramName(program), m_ProgramArgs(arguments) { }

    ~ProcessDebugger() { }

    virtual std::wstring ProgramArgs() const { return m_ProgramArgs; }

    virtual std::wstring ProgramName() const { return m_ProgramName; }

    virtual bool StartAndAttachToProgram();

    virtual bool ExecuteDebugLoop(DWORD timeout);

    void ShowSymbolInfo( LPVOID ImageBase );

private: 

    bool EnableDebugging();

    bool AttachToProcess(DWORD processID );

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


};


