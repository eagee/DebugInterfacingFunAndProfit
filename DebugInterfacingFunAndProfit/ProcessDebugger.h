#pragma once

#include "IProcessDebugger.h"
#include "SymbolEngine.h"

class ProcessDebugger : IProcessDebugger
{
private:
    // Attributes
    std::wstring m_TargetProgramName;
    std::wstring m_TargetProgramArgs;
    std::map<LPVOID, std::wstring> m_ModuleNames;

    // Associations    
    PROCESS_INFORMATION m_ProcessInfo;
    HANDLE m_DebugProcessHandle;
    CSymbolEngine m_SymbolEngine;
public:

    explicit ProcessDebugger(std::wstring program, std::wstring arguments) : m_TargetProgramName(program), m_TargetProgramArgs(arguments) { }

    ~ProcessDebugger() { }

    virtual std::wstring ProgramArgs() const { return m_TargetProgramArgs; }

    virtual std::wstring ProgramName() const { return m_TargetProgramName; }

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

    bool GetModuleSize( HANDLE hProcess, LPVOID imageBase, DWORD& moduleSize );

};


