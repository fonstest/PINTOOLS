// pin_escaping_detect_parent.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <windows.h>
#include <Tlhelp32.h> 
#include <stdio.h>
#include <tchar.h>


#define MAX_PIDS 1024

DWORD pIds[MAX_PIDS] = {-1};

char* GetPluginName(void)
{
	static char PluginName[] = "Detect parent process";
	return PluginName;
}

char* GetPluginDescription(void)
{
	static char MyDescription[] = "This plugin checks the name of the parent process."
								  "If it does not match \"explorer.exe\" nor \"cmd.exe\", "
								  "it assumes that it is being instrumented.";
	return MyDescription;
}

void lowercase(char string[])
{
   int  i = 0;

   while ( string[i] )
   {
      string[i] = tolower(string[i]);
      i++;
   }
}

int GetNameByPid(DWORD pid, char* ProcName, DWORD ProcNameBuffSize)
{
	HINSTANCE hInstLib;
	HANDLE hSnapShot;
	BOOL bContinue;
	PROCESSENTRY32 procentry;

	HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD);
	BOOL (WINAPI *lpfProcess32First)(HANDLE,LPPROCESSENTRY32);
	BOOL (WINAPI *lpfProcess32Next)(HANDLE,LPPROCESSENTRY32);

	hInstLib = LoadLibraryA( "Kernel32.DLL" ) ;
	if( hInstLib == NULL )
	{
		printf("Unable to load Kernel32.dll\n");
		return FALSE ;
	}

	lpfCreateToolhelp32Snapshot= (HANDLE(WINAPI *)(DWORD,DWORD))
	GetProcAddress( hInstLib, "CreateToolhelp32Snapshot" );

	lpfProcess32First= (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
	GetProcAddress( hInstLib, "Process32First" );
	 
	lpfProcess32Next= (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
	GetProcAddress( hInstLib, "Process32Next" );
	 
	if( lpfProcess32Next == NULL || lpfProcess32First == NULL || lpfCreateToolhelp32Snapshot == NULL )
	{
		FreeLibrary( hInstLib );
		return FALSE ;
	}

	hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0 );
	if( hSnapShot == INVALID_HANDLE_VALUE )
	{
		 printf("ERROR: INVALID_HANDLE_VALUE");
		 FreeLibrary( hInstLib );
		 return FALSE;
	}

	memset((LPVOID)&procentry,0,sizeof(PROCESSENTRY32));

	procentry.dwSize = sizeof(PROCESSENTRY32);
	bContinue = lpfProcess32First( hSnapShot, &procentry );

	while(bContinue)
	{
		if(pid == procentry.th32ProcessID)
		{

			strncpy_s(ProcName, ProcNameBuffSize, (char *)procentry.szExeFile, ProcNameBuffSize);
			return 1;
		}

		procentry.dwSize = sizeof(PROCESSENTRY32);
		bContinue = lpfProcess32Next(hSnapShot, &procentry);

	}
	
	return 0;
}


void IsParentExplorerOrCmd(void)
{
	HINSTANCE hInstLib;
	HANDLE hSnapShot;
	BOOL bContinue;
	DWORD crtpid, pid = 0;
	PROCESSENTRY32 procentry;

	char ProcName[MAX_PATH];

	HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD);
	BOOL (WINAPI *lpfProcess32First)(HANDLE,LPPROCESSENTRY32);
	BOOL (WINAPI *lpfProcess32Next)(HANDLE,LPPROCESSENTRY32);

	hInstLib = LoadLibraryA( "Kernel32.DLL" ) ;
	if( hInstLib == NULL )
	{
		printf("Unable to load Kernel32.dll\n");
		exit(0);
	}

	lpfCreateToolhelp32Snapshot= (HANDLE(WINAPI *)(DWORD,DWORD))
	GetProcAddress( hInstLib, "CreateToolhelp32Snapshot" );

	lpfProcess32First= (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
	GetProcAddress( hInstLib, "Process32First" );
	 
	lpfProcess32Next= (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
	GetProcAddress( hInstLib, "Process32Next" );
	 
	if( lpfProcess32Next == NULL || lpfProcess32First == NULL || lpfCreateToolhelp32Snapshot == NULL )
	{
		FreeLibrary( hInstLib );
		exit(0);
	}

	hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0 );
	if( hSnapShot == INVALID_HANDLE_VALUE )
	{
		 printf("ERROR: INVALID_HANDLE_VALUE");
		 FreeLibrary( hInstLib );
		 exit(0);
	}

	memset((LPVOID)&procentry,0,sizeof(PROCESSENTRY32));

	procentry.dwSize = sizeof(PROCESSENTRY32);
	bContinue = lpfProcess32First( hSnapShot, &procentry );

	crtpid = GetCurrentProcessId();
	while(bContinue)
	{
		//printf("-- Process name: %s -- Process ID: %d -- Parent ID: %d\n", procentry.szExeFile, procentry.th32ProcessID, procentry.th32ParentProcessID);

		if(crtpid == procentry.th32ProcessID)
		{
			//__asm{int 3};

			pid =  procentry.th32ParentProcessID;
			
			lowercase((char *)procentry.szExeFile);
			
			FreeLibrary(hInstLib);
			
			GetNameByPid(procentry.th32ParentProcessID, ProcName, sizeof(ProcName));

			if(strcmp("explorer.exe", ProcName) && strcmp("cmd.exe", ProcName))
				printf("Detected!!");
			else
				printf("Not Detected!!");

		}

		procentry.dwSize = sizeof(PROCESSENTRY32);
		bContinue = !pid && lpfProcess32Next( hSnapShot, &procentry );

	}

	FreeLibrary(hInstLib);

}

int _tmain(int argc, _TCHAR* argv[])
{
	
	IsParentExplorerOrCmd();
	return 0;
}

