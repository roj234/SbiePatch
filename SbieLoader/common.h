#pragma once

#include <windows.h>
#include <string>
#include <thread>
#include <assert.h>
#include <iostream>
#include <filesystem>

#pragma warning(disable: 4996)

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ�ļ����ų�����ʹ�õ�����

#define DBK_SERVICE_NAME L"RichStuff_Service_Name"
#define DBK_PROCESS_EVENT_NAME L"RichStuff_Process_Event_Name"
#define DBK_THREAD_EVENT_NAME L"RichStuff_Thread_Event_Name"
#ifdef _WIN64
#define CHEAT_ENGINE_FILE_NAME L"richstuff-x86_64.exe"
#define DBK_DRIVER_NAME L"richstuffk64.sys"
#else
#define CHEAT_ENGINE_FILE_NAME L"richstuff-i386.exe"
#define DBK_DRIVER_NAME L"richstuffk32.sys"
#endif

enum LoadType
{
	LoadByShellcode,            // ����shellcode���������������ɵ�ǰ����ֱ��������������ڵ����
	LoadByIoCreateDriver,       // ����IoCreateDriver�����������ᴴ���������󣬲���ϵͳ����������������ڵ����
};


void ConvertCharToWString(const char* charStr, wchar_t* wOut);

#define __FILENAME__ \
  (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)

#define STRINGFY(x) _STRINGFY(x)
#define _STRINGFY(x) #x

#define LOG(format, ...) \
  printf( __FILE__ ":" STRINGFY(__LINE__) "(" "%s" ") " format "\n", \
         __func__, ##__VA_ARGS__)

// ��Ȩ
BOOL AdjustProcessTokenPrivilege();

// ģ���ȡ���������ļ���Ŀ¼
bool GetCurrentModuleDirPath(WCHAR* dirPath);
