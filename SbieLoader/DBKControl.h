#pragma once
#include "common.h"

// �ж��ļ��Ƿ����
bool FileExists(const wchar_t filePath[]);

// ��ȡ������ַ
PVOID GetDriverAddress(LPCWSTR driverName);

// ����DBK����
BOOL LoadDBKDriver();

// ж��DBK����
BOOL UninitDBKDriver();

// ��ʼ��DBK����
BOOL InitDBKDriver();

// ����/�ͷ� �Ƿ�ҳ�ڴ�
UINT64 DBK_AllocNonPagedMem(ULONG size);
bool DBK_FreeNonPagedMem(UINT64 allocAddress);

// �����ڴ�
UINT64 DBK_AllocProcessMem(ULONG pid, UINT64 baseAddress, UINT64 size, UINT64 allocationType, UINT64 protect);

// ӳ���ں�̬��ַ���û�̬
struct MapMemInfo
{
	UINT64 fromMDL;
	UINT64 address;
};
bool DBK_MdlMapMem(MapMemInfo& mapMemInfo, UINT64 fromPid, UINT64 toPid, UINT64 address, DWORD size);
bool DBK_MdlUnMapMem(MapMemInfo mapMemInfo);

// ��/д�����ַ
bool DBK_ReadProcessMem(UINT64 pid, UINT64 toAddr, UINT64 fromAddr, DWORD size, bool failToContinue = false);
bool DBK_WriteProcessMem(UINT64 pid, UINT64 targetAddr, UINT64 srcAddr, DWORD size);

UINT64 DBK_GetKernelFuncAddress(const wchar_t* funcName);

bool DBK_ExecuteCode(UINT64 address);
