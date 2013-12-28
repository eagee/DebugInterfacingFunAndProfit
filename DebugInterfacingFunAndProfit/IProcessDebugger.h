#pragma once;

class IProcessDebugger
{
public:

    // Operations
    virtual std::wstring ProgramArgs() const = 0;
    virtual std::wstring ProgramName() const = 0;
    virtual bool StartAndAttachToProgram() = 0;
    virtual bool ExecuteDebugLoop( DWORD timeout ) = 0;
};