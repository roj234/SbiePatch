#include "DBKControl.h"
#include "IoCtlCode.h"
#include "MemLoadDriver.h"

#include <tlhelp32.h>
#include <Psapi.h>
#include <fstream>

static HANDLE g_DBKDevice = INVALID_HANDLE_VALUE;

// �ж��ļ��Ƿ����
bool FileExists(const wchar_t filePath[])
{
	return _waccess(filePath, 0) == 0;
}

// ��ȡ������ַ
PVOID GetDriverAddress(LPCWSTR driverName)
{
	PVOID driverAddrList[1024];
	DWORD cbNeeded = 0;
	if (!EnumDeviceDrivers(driverAddrList, sizeof(driverAddrList), &cbNeeded))
	{
		LOG("EnumDeviceDrivers failed");
		return NULL;
	}

	PVOID pDriverAddr = NULL;
	for (int i = 0; i < (cbNeeded / sizeof(PVOID)); i++)
	{
		if (NULL == driverAddrList[i])
		{
			continue;
		}
		WCHAR curDriverName[MAX_PATH] = { 0 };
		if (GetDeviceDriverBaseNameW(driverAddrList[i], curDriverName, sizeof(curDriverName) / sizeof(WCHAR)))
		{
			if (0 == wcscmp(curDriverName, driverName))
			{
				pDriverAddr = driverAddrList[i];
				break;
			}
		}
	}

	return pDriverAddr;
}

// ����DBK����
BOOL LoadDBKDriver()
{
	// ��ȡ�����ļ�ȫ·��
	wchar_t driverFilePath[MAX_PATH];
	GetModuleFileName(NULL, driverFilePath, sizeof(driverFilePath) / sizeof(wchar_t));
	wchar_t* pos = wcsrchr(driverFilePath, L'\\');
	if (nullptr == pos)
	{
		LOG("wcsrchr failed");
		return false;
	}
	*(pos + 1) = L'\0';
	wcscat(driverFilePath, DBK_DRIVER_NAME);

	// �ж������ļ��Ƿ����
	if (!FileExists(driverFilePath))
	{
		LOG("FileExists failed");
		return false;
	}

	// �򿪷�����ƹ�����
	SC_HANDLE hMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hMgr)
	{
		LOG("OpenSCManager failed");
		return false;
	}

	// ��NT�����������
	SC_HANDLE hService = OpenService(hMgr, DBK_SERVICE_NAME, SERVICE_ALL_ACCESS);
	if (NULL == hService)
	{
		// �򲻿��ʹ�������
		hService = CreateService(
			hMgr,
			DBK_SERVICE_NAME,		// �����������ע����е�����  
			DBK_SERVICE_NAME,		// ע������������ DisplayName ֵ  
			SERVICE_ALL_ACCESS,		// ������������ķ���Ȩ��  
			SERVICE_KERNEL_DRIVER,	// ��ʾ���صķ�������������  
			SERVICE_DEMAND_START,	// ע������������ Start ֵ  
			SERVICE_ERROR_NORMAL,	// ע������������ ErrorControl ֵ  
			driverFilePath,			// ע������������ ImagePath ֵ  
			NULL,					// GroupOrder HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\GroupOrderList
			NULL,
			NULL,
			NULL,
			NULL);
		if (NULL == hService)
		{
			LOG("CreateService failed");
			CloseServiceHandle(hMgr);
			return false;
		}
	}
	else
	{
		bool ret = false;
		ret = ChangeServiceConfig(
			hService,
			SERVICE_KERNEL_DRIVER,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_NORMAL,
			driverFilePath,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			DBK_SERVICE_NAME);
		if (!ret)
		{
			LOG("ChangeServiceConfig failed");
			CloseServiceHandle(hService);
			CloseServiceHandle(hMgr);
			return false;
		}
	}

	// д���ע���
	HKEY hKey;
	wchar_t subKey[] = L"SYSTEM\\CurrentControlSet\\Services\\" DBK_SERVICE_NAME;
	LSTATUS status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_WRITE, &hKey);
	if (ERROR_SUCCESS != status)
	{
		LOG("RegOpenKeyEx failed");
		CloseServiceHandle(hService);
		CloseServiceHandle(hMgr);
		return false;
	}

	wchar_t AValue[] = L"\\Device\\" DBK_SERVICE_NAME;
	RegSetValueEx(hKey, L"A", 0, REG_SZ, reinterpret_cast<const BYTE*>(AValue), sizeof(AValue));
	wchar_t BValue[] = L"\\DosDevices\\" DBK_SERVICE_NAME;
	RegSetValueEx(hKey, L"B", 0, REG_SZ, reinterpret_cast<const BYTE*>(BValue), sizeof(BValue));
	wchar_t CValue[] = L"\\BaseNamedObjects\\" DBK_PROCESS_EVENT_NAME;
	RegSetValueEx(hKey, L"C", 0, REG_SZ, reinterpret_cast<const BYTE*>(CValue), sizeof(CValue));
	wchar_t DValue[] = L"\\BaseNamedObjects\\" DBK_THREAD_EVENT_NAME;
	RegSetValueEx(hKey, L"D", 0, REG_SZ, reinterpret_cast<const BYTE*>(DValue), sizeof(DValue));

	// ��������
	if (!StartService(hService, NULL, NULL))
	{
		DWORD lastError = GetLastError();
		if (ERROR_SERVICE_ALREADY_RUNNING != lastError)
		{
			LOG("StartService failed, last error: %d", lastError);
			RegCloseKey(hKey);
			CloseServiceHandle(hService);
			CloseServiceHandle(hMgr);
			return false;
		}
	}

	RegCloseKey(hKey);
	CloseServiceHandle(hService);
	CloseServiceHandle(hMgr);
	return true;
}

// ж��DBK����
BOOL UninitDBKDriver()
{
	// �ر��豸����
	if (INVALID_HANDLE_VALUE != g_DBKDevice)
	{
		CloseHandle(g_DBKDevice);
	}

	// �򿪷�����ƹ�����
	SC_HANDLE hMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hMgr)
	{
		LOG("OpenSCManager failed");
		return false;
	}

	// ����������Ӧ�ķ���
	SC_HANDLE hService = OpenService(hMgr, DBK_SERVICE_NAME, SERVICE_ALL_ACCESS);
	if (NULL == hService)
	{
		LOG("OpenService failed");
		CloseServiceHandle(hMgr);
		return false;
	}

	// ֹͣ�����������ֹͣʧ�ܣ�ֻ�������������ܣ��ٶ�̬����
	SERVICE_STATUS serviceStatus;
	if (!ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus))
	{
		LOG("ControlService failed");
		CloseServiceHandle(hService);
		CloseServiceHandle(hMgr);
		return false;
	}

	//��̬ж����������
	if (!DeleteService(hService))
	{
		LOG("DeleteService failed");
		CloseServiceHandle(hService);
		CloseServiceHandle(hMgr);
		return false;
	}

	//�뿪ǰ�رմ򿪵ľ��
	CloseServiceHandle(hService);
	CloseServiceHandle(hMgr);
	return true;
}

// ��ʼ��DBK����
BOOL InitDBKDriver()
{
	// ���豸����
	wchar_t deviceName[] = L"\\\\.\\" DBK_SERVICE_NAME;
	HANDLE hDevice = CreateFile(
		deviceName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (INVALID_HANDLE_VALUE == hDevice)
	{
		LOG("CreateFile failed, last error: %d", GetLastError());
		return false;
	}
	g_DBKDevice = hDevice;

	// ������������Ҳ��
	// InitializeDriver(0, 0);

	// ɾ�����ע���
	HKEY hKey;
	wchar_t subKey[] = L"SYSTEM\\CurrentControlSet\\Services\\" DBK_SERVICE_NAME;
	LSTATUS status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_WRITE, &hKey);
	if (ERROR_SUCCESS != status)
	{
		LOG("RegOpenKeyEx failed");
		return false;
	}
	RegDeleteKey(hKey, L"A");
	RegDeleteKey(hKey, L"B");
	RegDeleteKey(hKey, L"C");
	RegDeleteKey(hKey, L"D");

	RegCloseKey(hKey);
	return true;
}

UINT64 DBK_AllocNonPagedMem(ULONG size)
{
#pragma pack(1)
	struct InputBuffer
	{
		ULONG size;
	};
#pragma pack()

	InputBuffer inputBuffer;
	inputBuffer.size = size;
	UINT64 allocAddress = 0LL;
	DWORD retSize;
	if (!DeviceIoControl(g_DBKDevice, IOCTL_CE_ALLOCATEMEM_NONPAGED, (LPVOID)&inputBuffer, sizeof(inputBuffer), &allocAddress, sizeof(allocAddress), &retSize, NULL))
	{
		LOG("DeviceIoControl IOCTL_CE_ALLOCATEMEM_NONPAGED failed");
		return 0;
	}
	return allocAddress;
}
bool DBK_FreeNonPagedMem(UINT64 allocAddress)
{
#pragma pack(1)
	struct InputBuffer
	{
		UINT64 address;
	};
#pragma pack()

	InputBuffer inputBuffer;
	inputBuffer.address = allocAddress;
	DWORD retSize;
	if (!DeviceIoControl(g_DBKDevice, IOCTL_CE_FREE_NONPAGED, (LPVOID)&inputBuffer, sizeof(inputBuffer), NULL, 0, &retSize, NULL))
	{
		LOG("DeviceIoControl IOCTL_CE_FREE_NONPAGED failed");
		return false;
	}
	return true;
}

UINT64 DBK_AllocProcessMem(ULONG pid, UINT64 baseAddress, UINT64 size, UINT64 allocationType, UINT64 protect)
{
#pragma pack(1)
	struct InputBuffer
	{
		UINT64 pid;
		UINT64 baseAddress;
		UINT64 size;
		UINT64 allocationType;
		UINT64 protect;
	};
#pragma pack()

	InputBuffer inputBuffer;
	inputBuffer.pid = pid;
	inputBuffer.baseAddress = baseAddress;
	inputBuffer.size = size;
	inputBuffer.allocationType = allocationType;
	inputBuffer.protect = protect;
	UINT64 allocAddress = 0LL;
	DWORD retSize;
	if (!DeviceIoControl(g_DBKDevice, IOCTL_CE_ALLOCATEMEM, &inputBuffer, sizeof(inputBuffer), &allocAddress, sizeof(allocAddress), &retSize, NULL))
	{
		LOG("DeviceIoControl IOCTL_CE_ALLOCATEMEM_NONPAGED failed");
		return 0;
	}
	return allocAddress;
}

bool DBK_MdlMapMem(MapMemInfo& mapMemInfo, UINT64 fromPid, UINT64 toPid, UINT64 address, DWORD size)
{
#pragma pack(1)
	struct InputBuffer
	{
		UINT64 fromPid;
		UINT64 toPid;
		UINT64 address;
		DWORD size;
	};
	struct OutputBuffer
	{
		UINT64 fromMDL;
		UINT64 address;
	};
#pragma pack()

	InputBuffer inputBuffer;
	inputBuffer.fromPid = fromPid;
	inputBuffer.toPid = toPid;
	inputBuffer.address = address;
	inputBuffer.size = size;
	OutputBuffer outputBuffer;
	DWORD retSize;
	if (!DeviceIoControl(g_DBKDevice, IOCTL_CE_MAP_MEMORY, &inputBuffer, sizeof(inputBuffer), &outputBuffer, sizeof(outputBuffer), &retSize, NULL))
	{
		LOG("DeviceIoControl IOCTL_CE_MAP_MEMORY failed");
		return false;
	}
	mapMemInfo.fromMDL = outputBuffer.fromMDL;
	mapMemInfo.address = outputBuffer.address;
	return true;
}
bool DBK_MdlUnMapMem(MapMemInfo mapMemInfo)
{
#pragma pack(1)
	struct InputBuffer
	{
		UINT64 fromMDL;
		UINT64 address;
	};
#pragma pack()

	InputBuffer inputBuffer;
	inputBuffer.fromMDL = mapMemInfo.fromMDL;
	inputBuffer.address = mapMemInfo.address;
	DWORD retSize;
	if (!DeviceIoControl(g_DBKDevice, IOCTL_CE_UNMAP_MEMORY, &inputBuffer, sizeof(inputBuffer), NULL, 0, &retSize, NULL))
	{
		LOG("DeviceIoControl IOCTL_CE_UNMAP_MEMORY failed");
		return false;
	}
	return true;
}

bool DBK_ReadProcessMem(UINT64 pid, UINT64 toAddr, UINT64 fromAddr, DWORD size, bool failToContinue)
{
#pragma pack(1)
	struct InputBuffer
	{
		UINT64 processid;
		UINT64 startaddress;
		WORD bytestoread;
	};
#pragma pack()

	UINT64 remaining = size;
	UINT64 offset = 0;
	do
	{
		UINT64 toRead = remaining;
		if (remaining > 4096)
		{
			toRead = 4096;
		}

		InputBuffer inputBuffer;
		inputBuffer.processid = pid;
		inputBuffer.startaddress = fromAddr + offset;
		inputBuffer.bytestoread = toRead;
		DWORD retSize;
		if (!DeviceIoControl(g_DBKDevice, IOCTL_CE_READMEMORY, (LPVOID)&inputBuffer, sizeof(inputBuffer), (LPVOID)(toAddr + offset), toRead, &retSize, NULL))
		{
			if (!failToContinue)
			{
				LOG("DeviceIoControl IOCTL_CE_READMEMORY failed");
				return false;
			}
		}

		remaining -= toRead;
		offset += toRead;
	} while (remaining > 0);

	return true;
}
bool DBK_WriteProcessMem(UINT64 pid, UINT64 targetAddr, UINT64 srcAddr, DWORD size)
{
#pragma pack(1)
	struct InputBuffer
	{
		UINT64 processid;
		UINT64 startaddress;
		WORD bytestowrite;
	};
#pragma pack()

	UINT64 remaining = size;
	UINT64 offset = 0;
	do
	{
		UINT64 toWrite = remaining;
		if (remaining > (512 - sizeof(InputBuffer)))
		{
			toWrite = 512 - sizeof(InputBuffer);
		}

		InputBuffer* pInputBuffer = (InputBuffer*)malloc(toWrite + sizeof(InputBuffer));
		if (NULL == pInputBuffer)
		{
			LOG("malloc failed");
			return false;
		}
		pInputBuffer->processid = pid;
		pInputBuffer->startaddress = targetAddr + offset;
		pInputBuffer->bytestowrite = toWrite;
		memcpy((PCHAR)pInputBuffer + sizeof(InputBuffer), (PCHAR)srcAddr + offset, toWrite);
		DWORD retSize;
		if (!DeviceIoControl(g_DBKDevice, IOCTL_CE_WRITEMEMORY, (LPVOID)pInputBuffer, (sizeof(InputBuffer) + toWrite), NULL, 0, &retSize, NULL))
		{
			LOG("DeviceIoControl IOCTL_CE_WRITEMEMORY failed");
			free(pInputBuffer);
			return false;
		}
		free(pInputBuffer);

		remaining -= toWrite;
		offset += toWrite;
	} while (remaining > 0);

	return true;
}

UINT64 DBK_GetKernelFuncAddress(const wchar_t* funcName)
{
#pragma pack(1)
	struct InputBuffer
	{
		UINT64 s;
	};
#pragma pack()

	InputBuffer inputBuffer;
	inputBuffer.s = (UINT64)funcName;
	UINT64 funcAddress = 0;
	DWORD retSize;
	if (!DeviceIoControl(g_DBKDevice, IOCTL_CE_GETPROCADDRESS, (LPVOID)&inputBuffer, sizeof(inputBuffer), &funcAddress, sizeof(funcAddress), &retSize, NULL))
	{
		LOG("DeviceIoControl IOCTL_CE_GETPROCADDRESS failed");
		return 0;
	}
	return funcAddress;
}

bool DBK_ExecuteCode(UINT64 address)
{
#pragma pack(1)
	struct InputBuffer
	{
		UINT64 address;
		UINT64 parameters;
	};
#pragma pack()

	InputBuffer inputBuffer;
	inputBuffer.address = address;
	inputBuffer.parameters = 0;
	DWORD retSize;
	if (!DeviceIoControl(g_DBKDevice, IOCTL_CE_EXECUTE_CODE, (LPVOID)&inputBuffer, sizeof(inputBuffer), NULL, 0, &retSize, NULL))
	{
		LOG("DeviceIoControl IOCTL_CE_EXECUTE_CODE failed");
		return false;
	}
	return true;
}
