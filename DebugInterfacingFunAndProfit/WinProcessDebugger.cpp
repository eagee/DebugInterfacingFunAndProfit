#include "stdafx.h"
#include "QtExpect.h"
#include "IQProcessDebugger.h"
#include "WinProcessDebugger.h"

const QString ERR = "Error Code: %1";

void WinProcessDebugger::StartAndAttachToProgram()
{
    bool success = EnableDebugging();
    if( !success )
    {
        qDebug() << Q_FUNC_INFO << "Failed to Enable Debugging";
    }
    else 
    {
        SHELLEXECUTEINFOW shellExecuteInfo;
        shellExecuteInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
        shellExecuteInfo.fMask        = SEE_MASK_NOCLOSEPROCESS;
        shellExecuteInfo.hwnd         = NULL;               
        shellExecuteInfo.lpVerb       = L"open";
        shellExecuteInfo.lpFile       = m_TargetProgramName.c_str();
        shellExecuteInfo.lpParameters = m_TargetProgramArgs.c_str();
        shellExecuteInfo.lpDirectory  = NULL;
        shellExecuteInfo.nShow        = SW_SHOW;
        shellExecuteInfo.hInstApp     = NULL;

        success = (int)ShellExecuteExW( &shellExecuteInfo );
        if( !success )
        {
            qDebug() << Q_FUNC_INFO << "Failed to Start Target Process!";
        }
        else
        {
            // Wait until mbam is in a state where it's able to process user input, and then attach to the process! :D
            WaitForInputIdle( shellExecuteInfo.hProcess, 60000 );
            success = AttachToProcess( GetProcessId( shellExecuteInfo.hProcess ) );
            ExecuteDebugLoop();
        }
    }

    //return success;
}

bool WinProcessDebugger::EnableDebugging()
{
    bool success = false;
    HANDLE processToken = NULL;
    DWORD ec = 0;

    // Start by getting the process token
    bool tokenOK = OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &processToken );
    Q_ASSERT_X( tokenOK, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );

    // then look up the current privilege value 
    if( tokenOK )
    {
        TOKEN_PRIVILEGES tokenPrivs; 
        tokenPrivs.PrivilegeCount = 1;
        bool privsFound = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tokenPrivs.Privileges[0].Luid);
        Q_ASSERT_X( privsFound, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );

        // Next we adjust the token privileges so that debugging is enabled...
        if( privsFound )
        {
            tokenPrivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            success = AdjustTokenPrivileges( processToken, FALSE, &tokenPrivs, sizeof(tokenPrivs), NULL, NULL );
            Q_ASSERT_X( success, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        }
    }

    // Voila, and we're done, we can close the handle to our process
    Q_EXPECT( CloseHandle(processToken) );

    return success;
}

bool WinProcessDebugger::AttachToProcess(DWORD processID)
{
    Q_EXPECT( GetCorrectLoadLibraryAddress() );
    bool success = DebugActiveProcess( processID );
    Q_ASSERT_X( success, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
    return success;
}

void WinProcessDebugger::OnCreateProcessEvent(DWORD ProcessId)
{
    qDebug() << Q_FUNC_INFO <<  ": Created Process ID: " << ProcessId;

    // Initialize the symbol engine ...
    m_SymbolEngine.AddOptions( SYMOPT_DEBUG | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME );
    Q_EXPECT( m_SymbolEngine.Init(m_DebugeeProcessHandle) );

}

void WinProcessDebugger::OnExitProcessEvent(DWORD ProcessId)
{
    qDebug() << Q_FUNC_INFO <<  ": Exit Process ID: " << ProcessId;
    m_SymbolEngine.Close();
}

void WinProcessDebugger::OnCreateThreadEvent(DWORD ThreadId)
{
    qDebug() << Q_FUNC_INFO <<  ": Create Thread ID: " << ThreadId;
}

void WinProcessDebugger::OnExitThreadEvent(DWORD ThreadId)
{
    qDebug() << Q_FUNC_INFO <<  ": Exit Thread ID: " << ThreadId;
}

BOOL CALLBACK EnumSymProc( PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    UNREFERENCED_PARAMETER(UserContext);

    qDebug() << Q_FUNC_INFO <<  ": Symbol Name: " << pSymInfo->Name << " Address: " << pSymInfo->Address;

    return TRUE;
}

void WinProcessDebugger::OnLoadModuleEvent(LPVOID ImageBase, HANDLE hFile)
{
    if( m_DebugeeProcessHandle == NULL )
    {
        return;
    }

    if( ( hFile == NULL ) && ( hFile == INVALID_HANDLE_VALUE )  )
    {
        qDebug() << Q_FUNC_INFO <<  ": Unable to load symbols for null file handle...";
        return;
    }

    // Get the module name from the file handle specified and add it to our list of module names
    std::wstring moduleName;
    if( !GetFileNameFromHandle(hFile, moduleName) )
    {
        moduleName = L"";
    }
    m_ModuleNames[ImageBase] = moduleName;

    // We need to figure out how much space the module is using in memory before we load symbols for the module..
    DWORD moduleSize = 0;
    GetModuleSize( m_DebugeeProcessHandle, ImageBase, moduleSize );
    LPVOID ImageEnd = (BYTE*)ImageBase + moduleSize;
    qDebug() << Q_FUNC_INFO <<  ": Module Loaded: " << QString::fromStdWString(moduleName) << " from: " << ImageBase << " to: " << ImageEnd;

    // We only want to load the symbols that are associated with our target executable or Qt...
    if( (QString::fromStdWString(moduleName).contains("QtGui")) || ( moduleName == m_TargetProgramName) )
    {
        // Now we can populate the symbols from that module (which we'll need later when we try to access qmbam application)
        if( m_SymbolEngine.LoadModuleSymbols( hFile, moduleName, (DWORD64)ImageBase, moduleSize ) )
        {
            // Enumerate all of the symbols in our exe file ...
            //char *Mask = "mb::ui*";
            //SymEnumSymbols( m_DebugProcessHandle, (DWORD64)ImageBase, Mask, EnumSymProc, NULL );
        }
        else
        {
            Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(m_SymbolEngine.LastError()).toAscii() );        
        }

        
    }
}

void WinProcessDebugger::OnUnloadModuleEvent(LPVOID ImageBase)
{
    // Create a message for the module being unloaded and then erase it from the module list...
    std::wstring moduleName( L"<unknown>" );
    std::map<LPVOID, std::wstring>::iterator moduleIterator = m_ModuleNames.find( ImageBase );
    if( moduleIterator != m_ModuleNames.end() )
    {
        moduleName = moduleIterator->second;
    }
    
    qDebug() << Q_FUNC_INFO <<  ": Unloading Module: " << QString::fromStdWString(moduleName);

    if( moduleIterator != m_ModuleNames.end() )
    {
        m_ModuleNames.erase( moduleIterator );
    }

    // Unload the symbols for this module ...
    m_SymbolEngine.UnloadModuleSymbols( (DWORD64)ImageBase );
}

void WinProcessDebugger::OnExceptionEvent(DWORD ThreadId, const EXCEPTION_DEBUG_INFO& Info)
{

}

void WinProcessDebugger::OnDebugStringEvent(DWORD ThreadId, const OUTPUT_DEBUG_STRING_INFO& Info)
{
    QString debugString = "";
    // Check parameters and preconditions

    if( m_DebugeeProcessHandle == NULL )
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, "Debugee process handle is null!" );
        return;
    }

    if( ( Info.lpDebugStringData == 0 ) || ( Info.nDebugStringLength == 0 ) )
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, "No debug string information." );
        return;
    }

    // Read the string from the debuggee's address space
    if( Info.fUnicode ) 
    {
        // Read as Unicode string

        const SIZE_T cMaxChars = 0xFFFF; 
        WCHAR Buffer[cMaxChars+1] = {0};

        SIZE_T CharsToRead = Info.nDebugStringLength; 

        if( CharsToRead > cMaxChars ) 
            CharsToRead = cMaxChars;

        SIZE_T BytesRead = 0;

        if( !ReadProcessMemory( m_DebugeeProcessHandle, Info.lpDebugStringData, Buffer, CharsToRead * sizeof(WCHAR), &BytesRead ) || ( BytesRead == 0 ) )
        {
            Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(m_SymbolEngine.LastError()).toAscii() );
            return;
        }

        debugString = QString::fromWCharArray(Buffer);
    }
    else 
    {
        // Read as ANSI string

        const SIZE_T cMaxChars = 0xFFFF; 
        CHAR Buffer[cMaxChars+1] = {0};

        SIZE_T CharsToRead = Info.nDebugStringLength; 

        if( CharsToRead > cMaxChars ) 
            CharsToRead = cMaxChars;

        SIZE_T BytesRead = 0;

        if( !ReadProcessMemory( m_DebugeeProcessHandle, Info.lpDebugStringData, Buffer, CharsToRead * sizeof(CHAR), &BytesRead ) || ( BytesRead == 0 ) )
        {
            Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(m_SymbolEngine.LastError()).toAscii() );
            return;
        }

        debugString = QString::fromAscii(Buffer);
    }

    qDebug() << "Debug String Event: " + debugString;
    if(debugString.contains("Server Active"))
    {
        qDebug() << Q_FUNC_INFO << ": Server process is ready for connections!";
        emit DebugProcessIsReadyForConnections();
    }
    
}

void WinProcessDebugger::OnTimeout()
{
    
    // This code can be abstracted into a method that can be requested
    // whenever a user wants to wait until qApp is available to work from...
    //DWORD64 qAppAddress = 0;
    //DWORD64 qAppDisplacement = 0;
    //std::wstring objectName = L"mb::ui::QMbamApplication::staticMetaObject"; 
    //if( !m_SymbolEngine.FindSymbolByName( objectName, qAppAddress, qAppDisplacement ) )
    //{
    //qDebug() << "DebugLoop - Timeout!";

    //Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(m_SymbolEngine.LastError()).toAscii() );
    //m_SymbolEngine.FindSymbolByName( objectName, qAppAddress, qAppDisplacement );
}

bool WinProcessDebugger::HandleDebugEvent(DEBUG_EVENT &debugEvent, bool &initialBreakpointTriggered)
{
    bool keepDebugging = true;
    DWORD ContinueStatus = DBG_CONTINUE;

    switch( debugEvent.dwDebugEventCode ) 
    {

    case CREATE_PROCESS_DEBUG_EVENT:
        // With this event, the debugger receives the following handles:
        //   CREATE_PROCESS_DEBUG_INFO.hProcess - debuggee process handle
        //   CREATE_PROCESS_DEBUG_INFO.hThread  - handle to the initial thread of the debuggee process
        //   CREATE_PROCESS_DEBUG_INFO.hFile    - handle to the executable file that was 
        //                                        used to create the debuggee process (.EXE file)
        // 
        // hProcess and hThread handles will be closed by the operating system 
        // when the debugger calls ContinueDebugEvent after receiving 
        // EXIT_PROCESS_DEBUG_EVENT for the given process
        // 
        // hFile handle should be closed by the debugger, when the handle 
        // is no longer needed
        //
        // 
        // Save the process handle
        m_DebugeeProcessHandle = debugEvent.u.CreateProcessInfo.hProcess;

        OnCreateProcessEvent( debugEvent.dwProcessId );
        OnCreateThreadEvent( debugEvent.dwThreadId );
        OnLoadModuleEvent( debugEvent.u.CreateProcessInfo.lpBaseOfImage, debugEvent.u.CreateProcessInfo.hFile );

        Q_EXPECT( CloseHandle(debugEvent.u.CreateProcessInfo.hFile) );

        break;

    case EXIT_PROCESS_DEBUG_EVENT:
        // Handle the event
        OnExitProcessEvent( debugEvent.dwProcessId );

        // Reset the process handle (it will be closed at the next call to ContinueDebugEvent)
        m_DebugeeProcessHandle = NULL;
        keepDebugging = false;
        break;

    case CREATE_THREAD_DEBUG_EVENT:
        OnCreateThreadEvent( debugEvent.dwThreadId );

        // With this event, the debugger receives the following handle:
        //   CREATE_THREAD_DEBUG_INFO.hThread  - handle to the thread that has been created
        // 
        // This handle will be closed by the operating system 
        // when the debugger calls ContinueDebugEvent after receiving 
        // EXIT_THREAD_DEBUG_EVENT for the given thread
        // 

        break;

    case EXIT_THREAD_DEBUG_EVENT:
        OnExitThreadEvent( debugEvent.dwThreadId );
        break;

    case LOAD_DLL_DEBUG_EVENT:
        OnLoadModuleEvent( debugEvent.u.LoadDll.lpBaseOfDll, debugEvent.u.LoadDll.hFile );

        // With this event, the debugger receives the following handle:
        //   LOAD_DLL_DEBUG_INFO.hFile    - handle to the DLL file 
        // 
        // This handle should be closed by the debugger, when the handle 
        // is no longer needed
        //

        Q_EXPECT( CloseHandle(debugEvent.u.LoadDll.hFile) );

        // Note: Closing the file handle here can lead to the following side effect:
        //   After the file has been closed, the handle value will be reused 
        //   by the operating system, and if the next "load dll" debug event 
        //   comes (for another DLL), it can contain the file handle with the same 
        //   value (but of course the handle now refers to that another DLL). 
        //   Don't be surprised!
        //

        break;

    case UNLOAD_DLL_DEBUG_EVENT:
        OnUnloadModuleEvent( debugEvent.u.UnloadDll.lpBaseOfDll );
        break;

    case OUTPUT_DEBUG_STRING_EVENT:
        OnDebugStringEvent( debugEvent.dwThreadId, debugEvent.u.DebugString );
        break;

    case RIP_EVENT:
        keepDebugging = false;
        break;

    case EXCEPTION_DEBUG_EVENT:
        OnExceptionEvent( debugEvent.dwThreadId, debugEvent.u.Exception );

        // By default, do not handle the exception 
        // (let the debuggee handle it if it wants to)

        ContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

        // Now the special case - the initial breakpoint 

        DWORD ExceptionCode = debugEvent.u.Exception.ExceptionRecord.ExceptionCode;

        if( !initialBreakpointTriggered && ( ExceptionCode == EXCEPTION_BREAKPOINT ) )
        {
            // This is the initial breakpoint, which is used to notify the debugger 
            // that the debuggee has initialized 
            // 
            // The debugger should handle this exception
            // 
            ContinueStatus = DBG_CONTINUE;

            initialBreakpointTriggered = true;

            // Right here at our initial breakpoint is a great time to inject a dll! :D
            InjectTestOMaticServerDll(L"WinTestOMaticServerd.dll");

        }
        break;
    }

    // Let the debuggee continue 
    if( !ContinueDebugEvent( debugEvent.dwProcessId, debugEvent.dwThreadId, ContinueStatus ) )
    {
        qDebug() << Q_FUNC_INFO <<  "ContinueDebugEvent() failed. Error:" << GetLastError;
        keepDebugging = false;
    }

    return keepDebugging;
}

bool WinProcessDebugger::ExecuteDebugLoop()
{
    const DWORD timeoutEventDelay = 30000;
    
    // Run the debug loop and handle the events 
    DEBUG_EVENT debugEvent;

    bool keepDebugging = true;

    bool initialBreakpointTriggered = false;

    qDebug() << Q_FUNC_INFO << ": Waiting for debug events...";

    while( keepDebugging == true ) 
    {
        if( WaitForDebugEvent( &debugEvent, timeoutEventDelay ) )
        {
            keepDebugging = HandleDebugEvent(debugEvent, initialBreakpointTriggered);
        }
        else
        {
            // Did WaitForDebugEvent because of a timeout?
            DWORD ErrCode = GetLastError();

            if( ErrCode == ERROR_SEM_TIMEOUT ) 
            {
                // Yes, report and continue
                OnTimeout();
            }
            else 
            {
                qDebug() << Q_FUNC_INFO <<  "WaitForDebugEvent() failed. Error: " << GetLastError();
                return false;
            }
        }
    }

    return true;
}

bool WinProcessDebugger::GetFileNameFromHandle( HANDLE hFile, std::wstring& fileName )
{
    DWORD ErrCode = 0;

    // Cleanup the [out] parameter
    fileName = L"";
    if( ( hFile == NULL ) || ( hFile == INVALID_HANDLE_VALUE ) )
    {
        return false;
    }

    if( !FileSizeIsValid(hFile) )
    {
        return false;
    }

    // Get the file name, map the file into memory, and handle any errors we might encounter...
    HANDLE hMapFile     = NULL;
    PVOID pViewOfFile   = NULL;

    // Map the file into memory, return if we fail to do so...
    hMapFile = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 1, NULL );
    if( hMapFile == NULL )
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        return false;
    }
    
    pViewOfFile = MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 1 );
    if( pViewOfFile == NULL ) 
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        return false;
    }

    // Obtain the file name and save it to our output string...
    const DWORD BUFFER_SIZE = (MAX_PATH + 1);
    WCHAR fileNameBuffer[BUFFER_SIZE] = {0};
    if( !GetMappedFileName( GetCurrentProcess(), pViewOfFile, fileNameBuffer, BUFFER_SIZE) )
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        return false;
    }
    fileName = fileNameBuffer;

    // The file name returned by GetMappedFileName won't contain a drive letter, but a device name
    // (logically of course b/c everything has to be a pain in windows), so we have to do some
    // mojojojo to get the proper drive letter for the device.
    ReplaceDeviceNameWithDriveLetter( fileName );

    // Clean up our mapping and handle information
    if( pViewOfFile != NULL )
    {
        Q_EXPECT( UnmapViewOfFile( pViewOfFile ) );
    }

    if( hMapFile != NULL ) 
    {
        Q_EXPECT( CloseHandle( hMapFile ) );
    }

    return true;

}

bool WinProcessDebugger::FileSizeIsValid(HANDLE hFile)
{
    // Does the file have a non-zero size ? (b/c files with zero size can't be mapped)
    DWORD FileSizeHi = 0;
    DWORD FileSizeLo = 0;

    // Get the file size and handle any failures callin that method (we can just return
    // in that case since there won't be any file name info to obtain)
    FileSizeLo = GetFileSize( hFile, &FileSizeHi );
    if( (FileSizeLo == INVALID_FILE_SIZE) && (GetLastError() != NO_ERROR) ) 
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        return false;
    }
    else if( ( FileSizeLo == 0 ) && ( FileSizeHi == 0 ) ) 
    {
        return false;
    }
    
    return true;
}


void WinProcessDebugger::ReplaceDeviceNameWithDriveLetter( std::wstring& fileName )
{
    DWORD ErrCode = 0;

    if( fileName.length() == 0 )
    {
        return;
    }

    // Get the list of drive letters available on the syzytem
    const DWORD BUFFER_SIZE = 512;
    WCHAR driveNames[BUFFER_SIZE + 1] = {0};
    DWORD driveStringLength = GetLogicalDriveStrings( BUFFER_SIZE, driveNames );
    if( ( driveStringLength == 0 ) || ( driveStringLength > BUFFER_SIZE) )
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        return;
    }

    // Walk through the list of characters in the drive name string. If one of the corresponds with 
    // a device name specified in the string replace it with the drive letter specified...
    WCHAR *driveLetter = driveNames;

    do
    {
        WCHAR drive[3] = L" :";
        _tcsncpy( drive, driveLetter, 2 );
        WCHAR device[BUFFER_SIZE+1] = {0};
        driveStringLength = QueryDosDevice( drive, device, BUFFER_SIZE );
        if( ( driveStringLength == 0 ) || ( driveStringLength >= BUFFER_SIZE ) )
        {
            Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        }
        else
        {
            // Is the device name the same as in the file name? If so then we can replace the device name with the drive letter!
            size_t deviceNameLength = _tcslen( device );
            if( _tcsnicmp( fileName.c_str(), device, deviceNameLength ) == 0 )
            {
                // Yes, it is -> Substitute it into the file name
                WCHAR newFileName[BUFFER_SIZE+1] = {0};
                _stprintf( newFileName, L"%s%s", drive, fileName.c_str()+deviceNameLength );
                fileName = newFileName;
                // the new file name has been populated, we can return now..
                return;
            }
        }
        while( *driveLetter++ );

      // Repeat until we've iterated through all of the drive letters.
    } while( *driveLetter );

}

bool WinProcessDebugger::GetModuleSize(HANDLE hProcess, LPVOID imageBase, DWORD& moduleSize)
{
    bool moduleSizeFound = false;

    if( hProcess == NULL )
    {
        return false;
    }

    if( imageBase == 0 )
    {
        return false;
    }

    // Scan the address space of the process and determine where the memory region 
    // allocated for the module ends (that is, we are looking for the first range 
    // of pages whose AllocationBase is not the same as the load address of the module)

    MEMORY_BASIC_INFORMATION mbi;

    BYTE* QueryAddress = (BYTE*)imageBase;

    while( !moduleSizeFound )
    {
        if( VirtualQueryEx( hProcess, QueryAddress, &mbi, sizeof(mbi) ) != sizeof(mbi) )
        {
            break;
        }

        if( mbi.AllocationBase != imageBase )
        {
            // Found, calculate the module size
            moduleSize = QueryAddress - (BYTE*)imageBase;
            moduleSizeFound = true;
            break;
        }

        QueryAddress += mbi.RegionSize;
    }

    return moduleSizeFound;
}

bool WinProcessDebugger::GetCorrectLoadLibraryAddress()
{

#ifndef _UNICODE
    Q_ASSERT_X(false, Q_FUNC_INFO, "Non unicode builds aren't supported!!!");
#endif

    m_hKern32 = GetModuleHandle(TEXT("kernel32.dll"));
    if( !m_hKern32 ) 
    {
        return false;
    }

    m_LoadLibraryAddress = (PVOID) GetProcAddress(m_hKern32, "LoadLibraryW");

    if( !m_LoadLibraryAddress )
    {
        return false;
    }

    return true;
}

bool WinProcessDebugger::InjectTestOMaticServerDll(std::wstring fullPathToDll)
{
    // Ok, in order to inject our dll the first thing we're going to need to do is allocate memory
    // in our target process to the store the name of the dll we're loading...
    size_t lengthInBytes = (fullPathToDll.length() * sizeof(wchar_t));
    LPVOID targetBaseAddress = VirtualAllocEx( m_DebugeeProcessHandle, (LPVOID)0, lengthInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
    if( targetBaseAddress == 0)
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        return false;
    }

    // Then we write the actual string to the memory we just allocated
    if( !WriteProcessMemory( m_DebugeeProcessHandle, targetBaseAddress, (LPCVOID)fullPathToDll.c_str(), lengthInBytes, 0) )
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        return false;
    }

    // Now for the magic! We start a thread on the client that will use LoadModule to load our TestOMatic dll
    // which in turn will (hopefully) start the TestOMatic IPC Server from within the target process! Yea man!
    if(!CreateRemoteThread( m_DebugeeProcessHandle, 0, 0, (LPTHREAD_START_ROUTINE)m_LoadLibraryAddress, targetBaseAddress, 0, 0) )
    {
        Q_ASSERT_X( false, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
        return false;
    }

    qDebug() << Q_FUNC_INFO << ": Successfully injected thread on target process!!!";

    return true;
}


QString WinProcessDebugger::InjectionDllName() const
{
#ifdef DEBUG
    return QString("WinTestOMaticServerd.dll");
#else
    return QString("WinTestOMaticServer.dll");
#endif
}

WinProcessDebugger::WinProcessDebugger(QString program, QString arguments, QObject *parent): QObject(parent), m_TargetProgramName(program.toStdWString()), m_TargetProgramArgs(arguments.toStdWString())
{

}

void WinProcessDebugger::MoveToThread(QThread *thread)
{
    Q_ASSERT( thread != nullptr );
    this->moveToThread(thread);
}

QObject * WinProcessDebugger::GetQObject()
{
    return this;
}
