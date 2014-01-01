// dllmain.cpp : Defines the entry point for the DLL application.
#include "WinTestOMaticServer/stdafx.h"


HWND g_hMod = 0;

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
            break;
        case DLL_PROCESS_DETACH:
            // TODO: This is where we'd shut that ol' IPC process down and handle cleanup ...
            break;
        break;
    }
    return TRUE;
}

#ifdef __cplusplus
}
#endif
