#include "common.h"
#include "export.h"
#include "DBKControl.h"
#include "MemLoadDriver.h"

#pragma comment(lib, "shlwapi.lib")  // 链接 shlwapi.lib
#include <shlwapi.h>  // 需要包含 shlwapi.h

static LoadType g_LoadType;
static WCHAR g_DriverFilePath[MAX_PATH] = { 0 };
#define DEFAULT_DRIVER_NAME L"\\FileSystem\\"
static WCHAR g_DriverName[MAX_PATH] = DEFAULT_DRIVER_NAME;

bool ParseCommandLine()
{
	int nArgs = 0;
	LPWSTR* argList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (nArgs < 3)
	{
		printf("Usage: %ls <loadType> <driverFile> [driverPath]\n", argList[0]);
        return false;
	}

    auto* driverPath = argList[2];

    if (_waccess(driverPath, 0) != 0) {
        LOG("驱动不存在或无法访问: %ls\n", driverPath);
        return 1;
    }

    wchar_t fullPath[MAX_PATH];

    // 转换为绝对路径
    DWORD ret = GetFullPathNameW(driverPath, MAX_PATH, fullPath, NULL);
    if (ret == 0 || ret > MAX_PATH) {
        LOG("文件路径太长, %lu > " STRINGFY(MAX_PATH) "\n", ret);
        return 1;
    }

	wcscpy(g_DriverFilePath, fullPath);
    LOG("Driver file path: %ls", g_DriverFilePath);

    // 获取驱动文件名
    if (nArgs == 3) {
        auto* driverName = PathFindFileNameW(fullPath);
        if (wcslen(driverName) > 90) {
            printf("文件名称太长, %zu > " STRINGFY(90) "\n", wcslen(driverName));
            return false;
        }
        driverName[wcslen(driverName) - 4] = '\0';
        wcscat(g_DriverName, driverName);
	} else {
        wcscpy(g_DriverName, argList[3]);
	}

    // 获取加载类型
    if (0 == _wcsicmp(argList[1], L"-load_by_shellcode"))
    {
        g_LoadType = LoadByShellcode;
        LOG("load by shellcode");
    }
    else if (0 == _wcsicmp(argList[1], L"-load_by_driver"))
    {
		g_LoadType = LoadByIoCreateDriver;
		LOG("load by driver %ls", g_DriverName);
    }
    else
    {
        LOG("Unknown load type: %ls", argList[1]);
        return false;
    }

    return true;
}

static const uint8_t base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";

void Base64Encode(uint64_t bits, wchar_t* dest) {
    *dest++ = L'_';
    // 从最高位到最低位处理11个6位块
    for (int i = 10; i >= 0; i--) {
        int shift = i * 6;
        uint8_t index = (bits >> shift) & 0x3F;
        *dest++ = base64_table[index];
    }
    *dest = L'\0'; // 宽字符空终止符
}

void Worker()
{
    // 提权
    if (!AdjustProcessTokenPrivilege())
    {
        LOG("AdjustProcessTokenPrivilege failed");
        return;
    }

    // 加载DBK驱动
    if (NULL == GetDriverAddress(DBK_DRIVER_NAME))
    {
        if (!LoadDBKDriver())
        {
            LOG("load DBKDriver failed");
            return;
        }
        if (NULL == GetDriverAddress(DBK_DRIVER_NAME))
        {
            LOG("GetDriverAddress failed");
            return;
        }
        LOG("load DBKDriver success");
    }
    else
    {
        LOG("DBKDriver Exists");
    }

    // 初始化DBK驱动
    if (!InitDBKDriver()) {
        LOG("RichStuff init failed");
        return;
    }

    LOG("RichStuff initialized.");

    auto memoryAddress = DBK_GetKernelFuncAddress(L"PsLoadedModuleList");

    Base64Encode(memoryAddress, g_DriverName + wcslen(g_DriverName));

    LOG("address: %p", memoryAddress);
    LOG("driverName: %ls", g_DriverName);

    // 加载自定义驱动
    if (!DBK_LoadMyDriver(g_LoadType, g_DriverFilePath, g_DriverName))
    {
        LOG("load my driver failed");
        return;
    }

    LOG("done %ls", g_DriverName);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);

        __try
        {
			// 输出重定向到父窗口控制台，方便观察打印日志
			AttachConsole(ATTACH_PARENT_PROCESS);
            if (NULL == freopen("CONOUT$", "w+t", stdout))
            {
                LOG("freopen failed");
                __leave;
            }

            // 解析参数
            if (!ParseCommandLine())
            {
                LOG("ParseCommandLine failed");
                __leave;
            }

            // 工作
            Worker();
        }
        __finally
        {
            // 卸载DBK驱动
            UninitDBKDriver();

            // 直接结束进程
            ExitProcess(0);
        }

        break;
    }
    case DLL_PROCESS_DETACH:
    {
        break;
    }
    }
    return TRUE;
}


#include "common.cpp"
#include "DBKControl.cpp"
#include "MemLoadDriver.cpp"