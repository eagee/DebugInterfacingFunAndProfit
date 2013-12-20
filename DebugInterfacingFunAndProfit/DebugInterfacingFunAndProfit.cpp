#include "stdafx.h"
#include <QString>
#include "QtExpect.h"
#include "DebugInterfacingFunAndProfit.h"


const QString ERR = "Error Code: %1";

class MbamDIA
{

private:
    CComPtr<IDiaDataSource> m_DataSource;
    CComPtr<IDiaSession> m_DiaSession;
    CComPtr<IDiaSymbol> m_Symbols;
    CComPtr<IDiaEnumTables> m_Tables;

public:

    MbamDIA()
    {

    }

    ~MbamDIA()
    {
        CoUninitialize();
    }

    /**
    * @fn   bool MbamDIA::HandleDIAInit()
    * @brief    Handle DIA Initialization via COM
    * @author   Eagan
    * @date 11/25/2013
    * @return   true if it succeeds, false if it fails.
    */
    bool HandleDIAInit()
    {
        HRESULT hr = CoCreateInstance( CLSID_DiaSource,
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof( IDiaDataSource ),
            (void **) &m_DataSource);

        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }


    bool OpenDIASession()
    {
        if( FAILED( m_DataSource->openSession( &m_DiaSession ) ) )
        {
            return false;
        }
        return true;
    }

    bool LoadSymbols()
    {
        if( FAILED( m_DiaSession->get_globalScope( &m_Symbols ) ) )
        {
            return false;
        }
        return true;
    }

    bool LoadTables()
    {
        if( FAILED( m_DiaSession->getEnumTables( &m_Tables ) ) )
        {
            return false;
        }
        return true;
    }

    /**
    * @fn   bool MbamDIA::LoadDebugInformationForExe( QString exeName )
    * @brief    Loads a debug information for the executable. specified
    * @author   Eagan
    * @date 11/25/2013
    * @param    exeName Name of the executable.
    * @return   true if it succeeds, false if it fails.
    */
    bool LoadDebugInformationForExe( QString exeName )
    {
        if ( FAILED( m_DataSource->loadDataForExe( exeName.toStdWString().c_str(), NULL, NULL ) ) )
        {
            return false;
        }
        
        if( OpenDIASession() == false ) 
        {
            return false;
        }

        if( LoadSymbols() == false )
        {
            return false;
        }
        
        if( LoadTables() == false )
        {
            return false;
        }
        
        return true;
    }
};

class MbamProcessDebugger
{
    private:
        // Attributes
        std::wstring m_ProgramName;
        std::wstring m_ProgramArgs;

        // Associations
        PROCESS_INFORMATION m_ProcessInfo;

        // Operations
        bool EnableDebugging()
        {
            bool success = false;
            HANDLE processToken = NULL;
            DWORD ec = 0;

            // Start by getting the process token
            bool tokenOK = OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &processToken );
            Q_ASSERT_X( tokenOK, Q_FUNC_INFO, QString().arg(GetLastError()).toAscii() );

            // then look up the current privilege value 
            if( tokenOK )
            {
                TOKEN_PRIVILEGES tokenPrivs; 
                tokenPrivs.PrivilegeCount = 1;
                bool privsFound = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tokenPrivs.Privileges[0].Luid);
                Q_ASSERT_X( privsFound, Q_FUNC_INFO, QString().arg(GetLastError()).toAscii() );

                // Next we adjust the token privileges so that debugging is enabled...
                if( privsFound )
                {
                    tokenPrivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                    success = AdjustTokenPrivileges( processToken, FALSE, &tokenPrivs, sizeof(tokenPrivs), NULL, NULL );
                    Q_ASSERT_X( success, Q_FUNC_INFO, QString().arg(GetLastError()).toAscii() );
                }
            }

            // Voila, and we're done, we can close the handle to our process
            Q_EXPECT( CloseHandle(processToken) );

            return success;
        }

        bool AttachToProcess(DWORD processID )
        {
            bool success = DebugActiveProcess( processID );
            Q_ASSERT_X( success, Q_FUNC_INFO, QString(ERR).arg(GetLastError()).toAscii() );
            return success;
        }


public:
        MbamProcessDebugger(std::wstring program, std::wstring arguments) : m_ProgramName(program), m_ProgramArgs(arguments)
        {

        }

        ~MbamProcessDebugger()
        {

        }


public:

       bool StartAndAttachToProcess()
       {
           bool success = EnableDebugging();
           if( success )
           {
               SHELLEXECUTEINFOW shellExecuteInfo;
               shellExecuteInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
               shellExecuteInfo.fMask        = SEE_MASK_NOCLOSEPROCESS;
               shellExecuteInfo.hwnd         = NULL;               
               shellExecuteInfo.lpVerb       = L"open";
               shellExecuteInfo.lpFile       = m_ProgramName.c_str();
               shellExecuteInfo.lpParameters = m_ProgramArgs.c_str();
               shellExecuteInfo.lpDirectory  = NULL;
               shellExecuteInfo.nShow        = SW_SHOW;
               shellExecuteInfo.hInstApp     = NULL;

               bool success = (int)ShellExecuteExW( &shellExecuteInfo );

               if( success )
               {
                   // Wait until mbam is in a state where it's able to process user input, and then attach to the process! :D
                   WaitForInputIdle( shellExecuteInfo.hProcess, 60000 );
                   success = AttachToProcess( GetProcessId( shellExecuteInfo.hProcess ) );
               }
           }

           return success;
       }
};

DebugInterfacingFunAndProfit::DebugInterfacingFunAndProfit(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);
    TestDIA();
}

bool DebugInterfacingFunAndProfit::TestDIA()
{
    std::wstring targetExe = L"E:\\MBAM\\mbamui\\Vs2010\\Debug\\MbamUI-vc100-x86-gd-0_4_13_388.exe";
    MbamDIA diaClass;
    bool success = diaClass.HandleDIAInit();
    if( success )
    {
        success = diaClass.LoadDebugInformationForExe( QString::fromStdWString(targetExe) );
    }
    Q_ASSERT( success == true );

    MbamProcessDebugger debugProcess(targetExe, L"");
    debugProcess.StartAndAttachToProcess();
    
    return success;
};

DebugInterfacingFunAndProfit::~DebugInterfacingFunAndProfit()
{
    
}
