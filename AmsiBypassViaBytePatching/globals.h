#pragma once
#include <windows.h>
#include <stdio.h>


FARPROC GetProcAddressReplacement(HMODULE hModule, IN LPCSTR lpApiName);
