#pragma once

#include <windows.h>
#include <string>
#include <thread>
#include <assert.h>
#include <iostream>
#include <filesystem>

#pragma warning(disable: 4996)

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容

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
	LoadByShellcode,            // 当作shellcode来加载驱动，会由当前进程直接运行驱动的入口点代码
	LoadByIoCreateDriver,       // 调用IoCreateDriver加载驱动，会创建驱动对象，并由系统进程运行驱动的入口点代码
};


void ConvertCharToWString(const char* charStr, wchar_t* wOut);

#define __FILENAME__ \
  (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)

#define STRINGFY(x) _STRINGFY(x)
#define _STRINGFY(x) #x

#define LOG(format, ...) \
  printf( __FILE__ ":" STRINGFY(__LINE__) "(" "%s" ") " format "\n", \
         __func__, ##__VA_ARGS__)

// 提权
BOOL AdjustProcessTokenPrivilege();

// 模块获取自身所处文件夹目录
bool GetCurrentModuleDirPath(WCHAR* dirPath);
