#include <windows.h>
#include <stdio.h>
#include <winternl.h>
#include "globals.h"


FARPROC GetProcAddressReplacement(IN HMODULE hModule, IN LPCSTR apiName) {

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;

    LONG elfanewOffset = *(LONG*)((BYTE*)hModule + 0x3C);
    PIMAGE_NT_HEADERS pHardcodedNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + elfanewOffset);

    IMAGE_OPTIONAL_HEADER imgOptHeader = pHardcodedNtHeaders->OptionalHeader;

    PIMAGE_EXPORT_DIRECTORY pImgExportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + imgOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    PDWORD functionsNames = (PDWORD)((BYTE*)hModule + pImgExportDir->AddressOfNames);

    PDWORD functionsAddress = (PDWORD)((BYTE*)hModule + pImgExportDir->AddressOfFunctions);

    PWORD functionsOrdinals = (PDWORD)((BYTE*)hModule + pImgExportDir->AddressOfNameOrdinals);

    // Looping through all the function names to compare to the apiName
    for (DWORD i = 0; i < pImgExportDir->NumberOfNames; i++) {

        LPCSTR exportedFunction = (LPCSTR)((BYTE*)hModule + functionsNames[i]);

        if (strcmp(exportedFunction, apiName) == 0) {
            WORD ordinal = functionsOrdinals[i];
            PVOID pFunctionAddress = (PVOID)((BYTE*)hModule + functionsAddress[ordinal]);

            return pFunctionAddress;
        }

    }

    return NULL;
}