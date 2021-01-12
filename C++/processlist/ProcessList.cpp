
// ProcessList.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


void PrintProcessNameAndID( DWORD processID )
{
    char szProcessName[MAX_PATH] = "unknown";

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.

    if ( hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
             &cbNeeded) )
        {
            GetModuleBaseName( hProcess, hMod, szProcessName, 
                               sizeof(szProcessName) );
        }
    }

    // Print the process name and identifier.

    printf( "%s (Process ID: %u)\n", szProcessName, processID );

    CloseHandle( hProcess );
}

int _tmain(int argc, _TCHAR* argv[])
{
    // Get the list of process identifiers.

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return 0;

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.

    for ( i = 0; i < cProcesses; i++ )
        PrintProcessNameAndID( aProcesses[i] );

		return 0;
}
