#include "common.h"

#include <tlhelp32.h>
#include <process.h>


void ConvertCharToWString(const char* charStr, wchar_t* wOut) {
	int len = strlen(charStr);
	int size = MultiByteToWideChar(CP_UTF8, 0, charStr, len, NULL, NULL);
	MultiByteToWideChar(CP_UTF8, 0, charStr, len, wOut, size);
	wOut[size] = '\0';
}

BOOL AdjustProcessTokenPrivilege()
{
	LUID luidTmp;
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		LOG("OpenProcessToken failed");
		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidTmp))
	{
		LOG("LookupPrivilegeValue failed");
		CloseHandle(hToken);
		return FALSE;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luidTmp;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		LOG("AdjustTokenPrivileges failed");
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hToken);
	return TRUE;
}

bool GetCurrentModuleDirPath(WCHAR* dirPath)
{
	HMODULE hModule = NULL;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModuleDirPath, &hModule);
	GetModuleFileName(hModule, dirPath, MAX_PATH);
	wchar_t* pos = wcsrchr(dirPath, L'\\');
	if (nullptr == pos)
	{
		LOG("wcsrchr failed");
		return false;
	}
	*(pos + 1) = L'\0';
	return true;
}
