// dllmain.cpp : Defines the entry point for the DLL application.
#include "WinTestOMaticServer/stdafx.h"
#include "WinTestOMaticServer/LocalSocketIpcServer.h"
#include "WinTestOMaticServer/TestOMaticServer.h"

static QThread *g_Thread;
static TestOMaticServer *g_Server;
static HWND g_hMod = 0;

// We need to export the C interface if this is used by C++ code ...
#ifdef __cplusplus
extern "C" {
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            // Eliminate some un-necessary overhead...
            DisableThreadLibraryCalls(hModule);

            // Save the handle to our dll in case we need it later ...
            g_hMod = (HWND)hModule;
            
            
            // TODO: Launch yourself a QThread class that can manage IPC stuff ...
            g_Thread = new QThread( );
            g_Server = new TestOMaticServer( );
            g_Thread->setObjectName( "WinTestOMaticServerThread" );
            g_Server->moveToThread( g_Thread );
            if(g_Thread->isRunning() == false)
            {
                g_Thread->start();
            }
            QMetaObject::invokeMethod( g_Server, "OnStartServer", Qt::QueuedConnection );
            break;
        case DLL_PROCESS_DETACH:
            //g_Server->deleteLater();
            g_Thread->deleteLater();
            break;
        break;
    }
    return TRUE;
}

#ifdef __cplusplus
}
#endif
