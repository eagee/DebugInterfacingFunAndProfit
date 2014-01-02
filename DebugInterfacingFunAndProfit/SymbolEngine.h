///////////////////////////////////////////////////////////////////////////////
//
// SymbolEngine.h
//
// Author: Oleg Starodumov (www.debuginfo.com)
//
// This file contains the declaration of CSymbolEngine class, 
// which implements a simple DbgHelp-based symbol engine
//
//
#pragma once

#ifndef SymbolEngine_h
#define SymbolEngine_h


///////////////////////////////////////////////////////////////////////////////
// Include files
//

#include <vector>

// Note: This file requires that the following files are included before it:
//   windows.h
//   tchar.h
//   dbghelp.h
//
// The following (commented out) text can be used (e.g. in a precompiled header)
//

/*
// The following is needed to use Unicode functions of DbgHelp in Unicode build
#ifdef UNICODE
#define DBGHELP_TRANSLATE_TCHAR
#endif

#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>
*/

// Note: Such structure of include files is used to simplify creation 
// of sample projects that use CSymbolEngine class (so that it is enough 
// to create a basic project with the wizard, include the source files 
// into it, build and run). In a real application, a precompiled header 
// file is probably a better place for this staff
//


///////////////////////////////////////////////////////////////////////////////
// CSymbolEngine class declaration
//
// This class implements a simple DbgHelp-based symbol engine 
// with the following features:
//   * Loading/unloading symbols
//   * Obtaining information about symbols loaded for a module
//   * Look up of a symbol by address
//   * Look up of a source file/line by address
//   * Call stack walking
//   * DbgHelp options control
//   * Receiving troubleshooting and status messages from DbgHelp.dll
//

class CSymbolEngine
{
public:

    // Constructors / destructor 

    CSymbolEngine();
    virtual ~CSymbolEngine();


public:

    // Operations

    // Initialization / cleanup

    // Init()
    //
    // This function initializes the symbol engine
    //
    // Parameters:
    //   * hProcess: Process handle
    //   * SearchPath: Symbol search path
    //   * Invade: If "true", the loaded modules of the process will be enumerated, 
    //       and their symbols will be loaded
    //   * Notify: If "true", the engine will handle DbgHelp notifications 
    //       and report them to the clients using OnEngineNotify virtual function; 
    //       if "false", the notifications are not handled but passed to debug output 
    //       instead (as by default). Remember that the notifications are enabled 
    //       only if SYMOPT_DEBUG option is set.
    //   (see SymInitialize() and SymRegisterCallback64() documentation in DbgHelp help 
    //   for more information)
    //
    // Return value: "true" if succeeded, "false" if failed. If the function failed, 
    //   LastError() function can be used to obtain the last error code.
    //
    bool Init( HANDLE hProcess    = GetCurrentProcess(), 
        std::wstring SearchPath  = L"", 
        bool   Invade      = false, 
        bool   Notify      = false );

    // Close()
    //
    // This function deinitializes the symbol engine
    //
    void Close();


    // Module operations

    // LoadModuleSymbols()
    // 
    // This function loads symbols for a module
    // 
    // Parameters:
    //   * ImageName:  Module file name (can contain path)
    //   * ModBase:    Load address of the module
    //   * ModSize:    Size of the module
    //   (see SymLoadModule64() documentation in DbgHelp help for more information)
    // 
    // Return value: Non-zero base address of the module if succeeded, zero if failed. 
    // If the function failed, LastError() function can be used to obtain the last error code.
    // 
    DWORD64 LoadModuleSymbols( const std::wstring& ImageName, DWORD64 ModBase = 0, DWORD ModSize = 0 );

    // LoadModuleSymbols()
    // 
    // This function loads symbols for a module
    // 
    // Parameters:
    //   * hFile:    Handle to the module image file
    //   * ModBase:  Load address of the module
    //   * ModSize:  Size of the module
    //   (see SymLoadModule64() documentation in DbgHelp help for more information)
    // 
    // Return value: Non-zero base address of the module if succeeded, zero if failed. 
    // If the function failed, LastError() function can be used to obtain the last error code.
    // 
    DWORD64 LoadModuleSymbols( HANDLE hFile, DWORD64 ModBase = 0, DWORD ModSize = 0 );

    // LoadModuleSymbols()
    // 
    // This function loads symbols for a module
    // 
    // Parameters:
    //   * hFile:      Handle to the module image file
    //   * ImageName:  Module file name (can contain path)
    //   * ModBase:    Load address of the module
    //   * ModSize:    Size of the module
    //   (see SymLoadModule64() documentation in DbgHelp help for more information)
    // 
    // Return value: Non-zero base address of the module if succeeded, zero if failed. 
    // If the function failed, LastError() function can be used to obtain the last error code.
    // 
    DWORD64 LoadModuleSymbols( HANDLE hFile, const std::wstring& ImageName, DWORD64 ModBase = 0, DWORD ModSize = 0 );

    // UnloadModuleSymbols()
    // 
    // This function unloads symbols of a module
    //
    // Parameters:
    //   * ModBase:  Load address of the module
    //   (see SymUnloadModule64() documentation in DbgHelp help for more information)
    // 
    // Return value: "true" if succeeded, "false" if failed. If the function failed, 
    //   LastError() function can be used to obtain the last error code.
    // 
    bool UnloadModuleSymbols( DWORD64 ModBase );

    // GetModuleInfo() 
    //
    // This function obtains information about the module
    //
    // Parameters:
    //   * Addr:  An address inside the module
    //   * Info:  [out] Information about the module
    //   (see SymGetModuleInfo64() documentation in DbgHelp help for more information)
    // 
    // Return value: "true" if succeeded, "false" if failed. If the function failed, 
    //   LastError() function can be used to obtain the last error code.
    // 
    bool GetModuleInfo( DWORD64 Addr, IMAGEHLP_MODULE64& Info );


    // Symbol operations 

    // FindSymbolByAddress()
    //
    // This function looks up a symbol by address
    //
    // Parameters: 
    //   * Address:       Address of the symbol
    //   * Name:          [out] Name of the symbol
    //   * Displacement:  [out] Displacement from the beginning of the symbol
    // (see SymFromAddr() documentation in DbgHelp help for more information)
    //
    // Return value: "true" if succeeded, "false" if failed. If the function failed, 
    //   LastError() function can be used to obtain the last error code.
    //
    bool FindSymbolByAddress( DWORD64 Address, std::wstring& Name, DWORD64& Displacement );

    bool FindSymbolByName( std::wstring& Name, DWORD64 &Address, DWORD64& Displacement );


    // Option control operations (see SymGetOptions and SymSetOptions 
    // in DbgHelp documentation for more information and the list of available options)

    // GetOptions()
    //
    DWORD GetOptions() const;

    // SetOptions()
    // 
    void SetOptions( DWORD Options );

    // AddOptions()
    //
    void AddOptions( DWORD Options );


public:

    // Notifications

    // Engine notification
    // 
    // This function can be used to receive verbose messages from DbgHelp
    // (which can be enabled with the help of SYMOPT_DEBUG option)
    //
    virtual void OnEngineNotify( const std::wstring& /*Message*/ ) {}


public:

    // Accessors

    // Process handle
    HANDLE  ProcessHandle() const { return m_hProcess; }

    // Last error
    DWORD LastError() const { return m_LastError; }


protected:

    // Data members

    // Process handle
    HANDLE m_hProcess;

    // Last error
    DWORD m_LastError;

};


#endif // SymbolEngine_h

