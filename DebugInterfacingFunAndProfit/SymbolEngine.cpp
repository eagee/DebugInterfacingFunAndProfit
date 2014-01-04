///////////////////////////////////////////////////////////////////////////////
//
// SymbolEngine.cpp
//
// Author: Oleg Starodumov (www.debuginfo.com)
//
// This file contains the implementation of CSymbolEngine class, 
// which implements a simple DbgHelp-based symbol engine
//
//


///////////////////////////////////////////////////////////////////////////////
// Include files
//

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>

#include <malloc.h>
#include <crtdbg.h>
#include <string>

#include "SymbolEngine.h"


///////////////////////////////////////////////////////////////////////////////
// Directives 
//

#pragma comment( lib, "dbghelp.lib" )
#pragma comment( lib, "psapi.lib" )



///////////////////////////////////////////////////////////////////////////////
// Helper classes
//

// Wrapper for SYMBOL_INFO_PACKAGE structure 
//
struct CSymbolInfoPackage : public SYMBOL_INFO_PACKAGEW 
{
    CSymbolInfoPackage() 
    {
        si.SizeOfStruct = sizeof(SYMBOL_INFOW); 
        si.MaxNameLen   = sizeof(name) / sizeof(WCHAR); 
    }
};

// Wrapper for IMAGEHLP_LINE64 structure 
//
struct CImageHlpLine64 : public IMAGEHLP_LINE64 
{
    CImageHlpLine64() 
    {
        SizeOfStruct = sizeof(IMAGEHLP_LINE64); 
    }
}; 


///////////////////////////////////////////////////////////////////////////////
// Function declarations
//

// Notification callback
BOOL CALLBACK DebugInfoCallback( HANDLE hProcess, ULONG ActionCode, 
    ULONG64 CallbackData, ULONG64 UserContext ); 


///////////////////////////////////////////////////////////////////////////////
// CSymbolEngine - constructors / destructor 
//

CSymbolEngine::CSymbolEngine()
    : m_hProcess( NULL ), m_LastError( 0 ) 
{
}

CSymbolEngine::~CSymbolEngine()
{
    Close();
}


///////////////////////////////////////////////////////////////////////////////
// CSymbolEngine - initialization / cleanup operations
//

bool CSymbolEngine::Init( HANDLE hProcess, std::wstring SearchPath, bool Invade, bool Notify )
{
    // Check parameters and preconditions 

    if( m_hProcess != NULL )
    {
        Q_ASSERT( !_T("Already initialized.") );
        if( hProcess != m_hProcess )
        {
            m_LastError = ERROR_INVALID_FUNCTION;
            return false;
        }
        else
        {
            return true;
        }
    }


    // Call SymInitialize

    if( !SymInitializeW( hProcess, SearchPath.c_str(), Invade ? TRUE : FALSE ) )
    {
        m_LastError = GetLastError();
        Q_ASSERT( !_T("SymInitialize failed.") );
        return false;
    }


    // Register the notification callback, if requested
    if( Notify )
    {
        if( !SymRegisterCallbackW64( hProcess, DebugInfoCallback, (ULONG64)this ) )
        {
            m_LastError = GetLastError();
            Q_ASSERT( !_T("SymInitialize failed.") );
            return false;
        }
    }


    // Complete 

    m_hProcess = hProcess;

    return true;

}

void CSymbolEngine::Close()
{
    if( m_hProcess != NULL )
    {
        if( !SymCleanup( m_hProcess ) )
        {
            m_LastError = GetLastError();
            Q_ASSERT( !_T("SymCleanup failed.") );
        }

        m_hProcess = NULL;
    }
}


///////////////////////////////////////////////////////////////////////////////
// CSymbolEngine - module operations
//

DWORD64 CSymbolEngine::LoadModuleSymbols( HANDLE hFile, const std::wstring& ImageName, DWORD64 ModBase, DWORD ModSize )
{
    // Check preconditions 

    if( m_hProcess == NULL )
    {
        Q_ASSERT( !_T("Symbol engine is not yet initialized.") );
        m_LastError = ERROR_INVALID_FUNCTION;
        return 0;
    }


    // In Unicode build, ImageName parameter should be translated to ANSI

#ifdef _UNICODE
    char* pImageName = 0;
    if( !ImageName.empty() )
    {
        size_t BufSize = 2 * ImageName.length();
        pImageName = (char*)_alloca( BufSize + 2 );
        size_t res = wcstombs( pImageName, ImageName.c_str(), BufSize );
        pImageName[BufSize] = 0;
        if( res == -1 )
        {
            Q_ASSERT( !_T("Module name has bad format.") );
            m_LastError = ERROR_INVALID_PARAMETER;
            return false;
        }
    }
#else
    const char* pImageName = ImageName.empty() ? 0 : ImageName.c_str();
#endif //_UNICODE


    // Load symbols for the module

    DWORD64 rv = SymLoadModule64( m_hProcess, hFile, pImageName, NULL, ModBase, ModSize );

    if( rv == 0 )
    {
        m_LastError = GetLastError();
        Q_ASSERT( !_T("SymLoadModule64() failed.") );
        return 0;
    }


    // Complete 

    return rv;
}

DWORD64 CSymbolEngine::LoadModuleSymbols( const std::wstring& ImageName, DWORD64 ModBase, DWORD ModSize )
{
    // Check parameters

    if( ImageName.empty() )
    {
        Q_ASSERT( !_T("Empty module name.") );
        m_LastError = ERROR_INVALID_PARAMETER;
        return false;
    }

    // Delegate the work to the more generic function

    return LoadModuleSymbols( NULL, ImageName, ModBase, ModSize );
}

DWORD64 CSymbolEngine::LoadModuleSymbols( HANDLE hFile, DWORD64 ModBase, DWORD ModSize )
{
    // Check parameters 

    if( ( hFile == NULL ) || ( hFile == INVALID_HANDLE_VALUE ) )
    {
        Q_ASSERT( !_T("Invalid file handle.") );
        m_LastError = ERROR_INVALID_PARAMETER;
        return false;
    }


    // Delegate the work to the more generic function

    return LoadModuleSymbols( hFile, std::wstring(), ModBase, ModSize );

}

bool CSymbolEngine::UnloadModuleSymbols( DWORD64 ModBase )
{
    // Check preconditions 

    if( m_hProcess == NULL )
    {
        Q_ASSERT( !_T("Symbol engine is not yet initialized.") );
        m_LastError = ERROR_INVALID_FUNCTION;
        return false;
    }

    if( ModBase == 0 )
    {
        Q_ASSERT( !_T("Module base address is null.") );
        m_LastError = ERROR_INVALID_PARAMETER;
        return false;
    }


    // Unload symbols

    if( !SymUnloadModule64( m_hProcess, ModBase ) )
    {
        m_LastError = GetLastError();
        Q_ASSERT( !_T("SymUnloadModule64() failed.") );
        return false;
    }


    // Complete 

    return true;
}

bool CSymbolEngine::GetModuleInfo( DWORD64 Addr, IMAGEHLP_MODULE64& Info )
{
    // Check preconditions 

    if( m_hProcess == NULL )
    {
        Q_ASSERT( !_T("Symbol engine is not yet initialized.") );
        m_LastError = ERROR_INVALID_FUNCTION;
        return false;
    }

    // Obtain module information

    memset( &Info, 0, sizeof(Info) );

    Info.SizeOfStruct = sizeof(Info);

    if( !SymGetModuleInfo64( m_hProcess, Addr, &Info ) )
    {
        m_LastError = GetLastError();
        Q_ASSERT( !_T("SymGetModuleInfo64() failed.") );
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// CSymbolEngine - symbol operations
//

bool CSymbolEngine::FindSymbolByAddress( DWORD64 Address, std::wstring& Name, DWORD64& Displacement )
{
    // Check preconditions 

    if( m_hProcess == NULL )
    {
        Q_ASSERT( !_T("Symbol engine is not yet initialized.") );
        m_LastError = ERROR_INVALID_FUNCTION;
        return false;
    }


    // Look up the symbol

    CSymbolInfoPackage sip; // it contains SYMBOL_INFO structure plus additional 
    // space for the name of the symbol 

    DWORD64 Disp = 0; 

    if( !SymFromAddrW( m_hProcess, Address, &Disp, &sip.si ) )
    {
        // Failed, but do not assert here - it is usually normal when a symbol is not found
        m_LastError = GetLastError();
        return false;
    }
    
    //Name          = sip.si.Name;
    Displacement  = Disp;

    return true;

}


bool CSymbolEngine::FindSymbolByName( std::wstring& Name, DWORD64 &Address, DWORD64& Displacement )
{
    // Check preconditions 

    if( m_hProcess == NULL )
    {
        Q_ASSERT( !_T("Symbol engine is not yet initialized.") );
        m_LastError = ERROR_INVALID_FUNCTION;
        return false;
    }

    // Look up the symbol

    DWORD64 Disp = 0; 
    CSymbolInfoPackage sip; 
    
    if( !SymFromNameW( m_hProcess, Name.c_str(), &sip.si ) )
    {
        // Failed, but do not assert here - it is usually normal when a symbol is not found
        m_LastError = GetLastError();
        return false;
    }

    //Name          = sip.si.Name;
    Displacement  = Disp;
    Address = sip.si.Address;

    return true;

}


///////////////////////////////////////////////////////////////////////////////
// CSymbolEngine - option control operations
//

DWORD CSymbolEngine::GetOptions() const
{
    return SymGetOptions();
}

void CSymbolEngine::SetOptions( DWORD Options )
{
    SymSetOptions( Options );
}

void CSymbolEngine::AddOptions( DWORD Options )
{
    DWORD CurOptions = GetOptions();

    CurOptions |= Options;

    SetOptions( CurOptions );
}


///////////////////////////////////////////////////////////////////////////////
// Notification callback 
//

BOOL CALLBACK DebugInfoCallback
    (
    HANDLE   /* hProcess */ , 
    ULONG    ActionCode, 
    ULONG64  CallbackData, 
    ULONG64  UserContext
    ) 
{
    // Note: This function should return TRUE only if it handles the event, 
    // otherwise it must return FALSE (see documentation). 

    CSymbolEngine* pEngine = (CSymbolEngine*)UserContext;

    if( pEngine == 0 )
    {
        Q_ASSERT( !_T("Engine pointer is null.") );
        return FALSE;
    }

    if( ActionCode == CBA_DEBUG_INFO ) 
    {
        if( CallbackData != 0 ) 
        {
            Q_ASSERT( !::IsBadStringPtr( (const WCHAR*)CallbackData, UINT_MAX ) ); 

            pEngine->OnEngineNotify( std::wstring( (const WCHAR*)CallbackData ) );

            return TRUE; 
        }
    }

    return FALSE; 
}

