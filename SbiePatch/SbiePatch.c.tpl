/*This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>*/

#include "ntddk.h"
#include <intrin.h>
#include <windows.h>

#define IMAGE_NAME L"sbiedrv.sys"

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

static const UCHAR Search[] = {
    0x45, 0x43, 0x53, 0x31, 0x20, 0x00, 0x00, 0x00, 0x05, 0x7A, 0x12, 0x5A, 0xF8, 0x54, 0x01, 0x42,
    0xDB, 0x19, 0x87, 0xFC, 0xC4, 0xE3, 0xD3, 0x8D, 0x46, 0x7B, 0x74, 0x01, 0x12, 0xFC, 0x78, 0xEB,
    0xEF, 0x7F, 0xF6, 0xAF, 0x4D, 0x9A, 0x3A, 0xF6, 0x64, 0x90, 0xDB, 0xE3, 0x48, 0xAB, 0x3E, 0xA7,
    0x2F, 0xC1, 0x18, 0x32, 0xBD, 0x23, 0x02, 0x9D, 0x3F, 0xF3, 0x27, 0x86, 0x71, 0x45, 0x26, 0x14,
    0x14, 0xF5, 0x19, 0xAA, 0x2D, 0xEE, 0x50, 0x10
};

PLACEHOLDER

static ULONG ClampToMappedPages(PVOID BaseAddress, ULONG Size) {
    ULONG offset = 0;
    while (offset < Size) {
        if (!MmIsAddressValid((PUCHAR)BaseAddress + offset)) {
            return offset;
        }
        offset += PAGE_SIZE;
    }
    return Size;
}

// 查找模块内存中的特征码
static PVOID FindSignatureInMemory(PVOID BaseAddress, ULONG ImageSize) {
    ULONG scanSize = ClampToMappedPages(BaseAddress, ImageSize);
    ULONG last = scanSize - (ULONG)sizeof(Search);
    for (ULONG i = 0; i <= last; i++) {
        if (RtlCompareMemory((PUCHAR)BaseAddress + i, Search, sizeof(Search)) == sizeof(Search)) {
            return (PUCHAR)BaseAddress + i;
        }
    }
    return NULL;
}

static PVOID GetModuleBaseByName(PVOID PsLoadedModuleList, UNICODE_STRING ModuleName, ULONG* size) {
	PLDR_DATA_TABLE_ENTRY PsLoadedModuleList1 = (PLDR_DATA_TABLE_ENTRY)PsLoadedModuleList;
	const PLIST_ENTRY head = PsLoadedModuleList1->InLoadOrderLinks.Flink;
	PLIST_ENTRY entry = head->Flink;
	PLDR_DATA_TABLE_ENTRY value = NULL;

	while (entry != head) {
		// 获取LDR_DATA_TABLE_ENTRY结构
		value = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		if (value->BaseDllName.Buffer != 0) {
			// 对比模块名
			if (RtlCompareUnicodeString(&value->BaseDllName, &ModuleName, TRUE) == 0) {
				*size = value->SizeOfImage;
				return (PVOID)value->DllBase;
			}
		}

		entry = entry->Flink;
	}
	return 0;
}

static const uint8_t base64_table[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";
static uint8_t tab[128];

static LONGLONG Base64Decode(const wchar_t* src, size_t count) {
    uint64_t result = 0;
    for (int i = 0; i < 11; i++) {
        result = (result << 6) | (tab[*src++] & 0x3F);
    }
    return result;
}

// sbiedrv.sys 是以映像方式加载的，其 .rdata/.text 等节页是只读的（PAGE_READONLY），
// 直接 RtlCopyMemory 写入会触发 ATTEMPTED_WRITE_TO_READONLY_MEMORY (0xBE) 蓝屏。
//
// 注意不能直接改这些页的保护：MmProtectMdlSystemAddress 只接受普通系统地址范围的 MDL，
// 对映像节（原型 PTE）页会返回 STATUS_NOT_MAPPED_VIEW (0xC0000019)。
static NTSTATUS WriteReadonlyMemory(PVOID Address, const VOID* Data, SIZE_T Size) {
    const ULONG offset = (ULONG)((ULONG_PTR)Address & (PAGE_SIZE - 1));
    PVOID pageBase = (PVOID)((ULONG_PTR)Address - offset);

    // 从页首开始构造 MDL：别名映射的基址正好对应 pageBase，
    // 这样别名 + offset 就指向要改的公钥位置（避免 ByteOffset 偏移歧义）
    PMDL Mdl = IoAllocateMdl(pageBase, offset + (ULONG)Size, FALSE, FALSE, NULL);
    if (!Mdl) {
        DbgPrint("IoAllocateMdl 失败\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // 只读探测并锁定页：页本身是只读的，这里只发起读访问，不会失败
    MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);

    // 建立别名映射：内核态映射使用新的系统 PTE，默认可写；
    // 加 MdlMappingNoExecute 避免弄出一个可写又可执行的映射
    PVOID alias = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmCached, NULL, FALSE,
                                              NormalPagePriority | MdlMappingNoExecute);
    if (!alias) {
        DbgPrint("建立别名映射失败\n");
        MmUnlockPages(Mdl);
        IoFreeMdl(Mdl);
        return STATUS_NOT_MAPPED_VIEW;
    }

    // 通过别名写入同一批物理页
    RtlCopyMemory((PUCHAR)alias + offset, Data, Size);

    MmUnmapLockedPages(alias, Mdl);
    MmUnlockPages(Mdl);
    IoFreeMdl(Mdl);

    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);
    _Static_assert(sizeof(Search) == sizeof(Replace), "Must be the same length");

	for (int i = 0; i < 64; i++) {
		tab[base64_table[i] & 0xFF] = i;
	}

    wchar_t* buf = DriverObject->DriverName.Buffer;
    while (*buf++ != '_');

    PVOID PsLoadedModuleList = (PVOID)Base64Decode(buf, DriverObject->DriverName.Length);
    DbgPrint("模块列表 = %p", PsLoadedModuleList);

    UNICODE_STRING targetName;
    RtlInitUnicodeString(&targetName, IMAGE_NAME);

    // 获取 targetName 对应驱动的基址
    ULONG targetSize = 0;
    PVOID targetEntry = GetModuleBaseByName(PsLoadedModuleList, targetName, &targetSize);
    if (!targetEntry || targetSize == 0) {
        DbgPrint("驱动未加载\n");
        return 0;
    }

	DbgPrint("驱动基址 = %p, %u\n", targetEntry, targetSize);

    PVOID replaceAddress = FindSignatureInMemory(targetEntry, targetSize);

	if (!replaceAddress) {
        DbgPrint("未找到公钥\n");
        return 0;
	}

	DbgPrint("公钥地址 = %p\n", replaceAddress);

    // 目标页是只读的，走 PTE 别名写入
    NTSTATUS writeStatus = WriteReadonlyMemory(replaceAddress, Replace, sizeof(Replace));
    if (!NT_SUCCESS(writeStatus)) {
        DbgPrint("写入公钥失败 = 0x%08X\n", writeStatus);
    }

    UCHAR replacedData[sizeof(Replace)];
    RtlCopyMemory(replacedData, replaceAddress, sizeof(Replace));

    if (RtlCompareMemory(replacedData, Replace, sizeof(Replace)) == sizeof(Replace)) {
        DbgPrint("补丁成功！\n");
    } else {
        DbgPrint("补丁失败！\n");
        for (int i = 0; i < sizeof(Replace); i++) {
            DbgPrint("%02X ", replacedData[i]);
        }
        DbgPrint("\n");
    }

    return 0;
}