/*This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>*/

#include <windows.h>
#include <bcrypt.h>
#include <stdio.h>

#pragma comment(lib, "bcrypt.lib")

#define KPH_SIGN_ALGORITHM BCRYPT_ECDSA_P256_ALGORITHM
#define KPH_HASH_ALGORITHM BCRYPT_SHA256_ALGORITHM
#define KPH_BLOB_PUBLIC BCRYPT_ECCPUBLIC_BLOB

static wchar_t certificate[256] = {L'\0'};

// 模拟行读取函数（从字符串逐行读取）
char* Conf_Read_Line(char** stream, wchar_t* line, int* line_num) {
    static const wchar_t delimiters[] = L"\r\n";
    static wchar_t* next_token = NULL;
    wchar_t* token = wcstok_s(*stream ? (wchar_t*)*stream : NULL, delimiters, &next_token);
    
    if (!token) return NULL;
    
    // 复制当前行到缓冲区
    wcscpy_s(line, 1024, token);
    (*line_num)++;
    *stream = (char*)next_token; // 更新流指针
    return (char*)1; // 返回非NULL表示成功
}

NTSTATUS SignCertificate(BCRYPT_KEY_HANDLE hKey) {
    BCRYPT_ALG_HANDLE hHashAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;

    BCryptOpenAlgorithmProvider(&hHashAlg, KPH_HASH_ALGORITHM, NULL, 0);
    BCryptCreateHash(hHashAlg, &hHash, NULL, 0, NULL, 0, 0);

    // 初始化流指针
    char* stream = (char*)certificate;
    wchar_t line[1024];
    int line_num = 0;

    // 模拟 Conf_Read_Line 调用
    while (Conf_Read_Line(&stream, line, &line_num)) {
        wchar_t* ptr = wcschr(line, L':');
        if (!ptr || ptr == line) {
            fprintf(stderr, "Invalid line: %d\n", line_num);
            continue;
        }

        // 解析 name 和 value
        wchar_t* name = line;
        wchar_t* value = ptr + 1;

        // 去除 name 末尾空白
        while (ptr > line && *(ptr-1) <= 32) ptr--;
        *ptr = L'\0';

        // 去除 value 首尾空白
        while (*value <= 32 && *value != L'\0') value++;
        ptr = value + wcslen(value);
        while (ptr > value && *(ptr-1) <= 32) ptr--;
        *ptr = L'\0';

        // 处理 SIGNATURE（示例）
        if (_wcsicmp(L"SIGNATURE", name) == 0) continue;

        // 转换为 UTF-8
        char utf8_name[256], utf8_value[256];
        int name_len = WideCharToMultiByte(CP_UTF8, 0, name, -1, utf8_name, 256, NULL, NULL);
        int value_len = WideCharToMultiByte(CP_UTF8, 0, value, -1, utf8_value, 256, NULL, NULL);

        // 传递到哈希函数（去除结尾零）
        if (name_len > 0) {
            BCryptHashData(hHash, (PUCHAR)utf8_name, name_len-1, 0);
        }
        if (value_len > 0) {
            BCryptHashData(hHash, (PUCHAR)utf8_value, value_len-1, 0);
        }

        fprintf(stderr, "%ls: %ls\n", name, value);
    }

    BYTE HashResult[32];
    BCryptFinishHash(hHash, HashResult, 32, 0);

    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hHashAlg, 0);

    DWORD signatureSize = 0x100;
    BYTE signature[signatureSize];

    BCryptSignHash(hKey, NULL, HashResult, 32, signature, signatureSize, &signatureSize, 0);

    DWORD cbBase64 = signatureSize * 4 / 3 + 4;
    char base64[cbBase64];
    CryptBinaryToStringA(signature, signatureSize, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64, &cbBase64);

    fprintf(stderr, "SIGNATURE: %s\n", base64);

    return 0;
}

// 生成输出文件名：原文件名 + .sha
BOOL GetOutputFileName(LPCSTR inputFile, LPSTR outputFile, DWORD maxLen) {
    if (strlen(inputFile) > maxLen - 5) return FALSE; // 保留空间追加扩展名

    strcpy_s(outputFile, maxLen, inputFile);
    strcat_s(outputFile, maxLen, ".sig");
    return TRUE;
}

BOOL HashFile(const char* file, BYTE* hash) {
    BCRYPT_ALG_HANDLE hHashAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;

    FILE* fp = fopen(file, "rb");
    if (!fp) {
        printf("找不到 %s\n", file);
        return FALSE;
    }

    BCryptOpenAlgorithmProvider(&hHashAlg, KPH_HASH_ALGORITHM, NULL, 0);
    BCryptCreateHash(hHashAlg, &hHash, NULL, 0, NULL, 0, 0);

    BYTE buffer[1024];
    DWORD bytesRead;

    // 读取文件并更新哈希
    while ((bytesRead = fread(buffer, 1, 1024, fp))) {
        BCryptHashData(hHash, buffer, bytesRead, 0);
    }

    fclose(fp);

    BCryptFinishHash(hHash, hash, 32, 0);

    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hHashAlg, 0);
    return TRUE;
}

BOOL SignFile(BCRYPT_KEY_HANDLE hKey, const char* file) {
    // 获取最终哈希值
    BYTE hash[32];
    if (!HashFile(file, hash)) return FALSE;

    DWORD signatureSize = 0x100;
    BYTE signature[signatureSize];

    BCryptSignHash(hKey, NULL, hash, 32, signature, signatureSize, &signatureSize, 0);

    CHAR outputFile[MAX_PATH + 4] = {0};

    if (!GetOutputFileName(file, outputFile, MAX_PATH + 4)) {
        printf("Output filename too long\n");
        return FALSE;
    }

    FILE* output = fopen(outputFile, "wb");
    if (!output) return FALSE;

    fwrite(signature, signatureSize, 1, output);

    fclose(output);
    return TRUE;
}

int ReplaceContent(const char *filename, const unsigned char *A, const unsigned char *B, size_t len) {
    HANDLE hFile = CreateFileA(
        filename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "文件打开失败 (Error %lu)\n", GetLastError());
        return -1;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        fprintf(stderr, "获取文件大小失败 (Error %lu)\n", GetLastError());
        CloseHandle(hFile);
        return -1;
    }

    if (len == 0 || fileSize.QuadPart < (LONGLONG)len) {
        CloseHandle(hFile);
        return 0;
    }

    HANDLE hMapping = CreateFileMappingA(
        hFile,
        NULL,
        PAGE_READWRITE,
        0,
        0,
        NULL
    );
    if (hMapping == NULL) {
        fprintf(stderr, "创建内存映射失败 (Error %lu)\n", GetLastError());
        CloseHandle(hFile);
        return -1;
    }

    unsigned char *addr = (unsigned char *)MapViewOfFile(
        hMapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        fileSize.QuadPart
    );
    if (addr == NULL) {
        fprintf(stderr, "内存映射视图失败 (Error %lu)\n", GetLastError());
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return -1;
    }

    int replace_count = 0;
    for (LONGLONG i = 0; i <= fileSize.QuadPart - (LONGLONG)len; i++) {
        if (memcmp(addr + i, A, len) == 0) {
            memcpy(addr + i, B, len);
            replace_count++;
        }
    }

    UnmapViewOfFile(addr);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    return replace_count;
}

static const UCHAR officialKey[] =
{
    0x45, 0x43, 0x53, 0x31, 0x20, 0x00, 0x00, 0x00, 0x05, 0x7A, 0x12, 0x5A, 0xF8, 0x54, 0x01, 0x42,
    0xDB, 0x19, 0x87, 0xFC, 0xC4, 0xE3, 0xD3, 0x8D, 0x46, 0x7B, 0x74, 0x01, 0x12, 0xFC, 0x78, 0xEB,
    0xEF, 0x7F, 0xF6, 0xAF, 0x4D, 0x9A, 0x3A, 0xF6, 0x64, 0x90, 0xDB, 0xE3, 0x48, 0xAB, 0x3E, 0xA7,
    0x2F, 0xC1, 0x18, 0x32, 0xBD, 0x23, 0x02, 0x9D, 0x3F, 0xF3, 0x27, 0x86, 0x71, 0x45, 0x26, 0x14,
    0x14, 0xF5, 0x19, 0xAA, 0x2D, 0xEE, 0x50, 0x10
};

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <UserName> [...FilesToReplace] \"\" [...FilesToSign]", argv[0]);
        return 1;
    }

    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    NTSTATUS status;
    
    BCryptOpenAlgorithmProvider(&hAlg, KPH_SIGN_ALGORITHM, NULL, 0);

    BCryptGenerateKeyPair(hAlg, &hKey, 256, 0);
    BCryptFinalizeKeyPair(hKey, 0);

    ULONG cbPublicKey = 0x100;
    BYTE pbPublicKey[0x100];

    BCryptExportKey(hKey, NULL, KPH_BLOB_PUBLIC, pbPublicKey, cbPublicKey, &cbPublicKey, 0);

    wcscat(certificate, L"NAME: ");
    WCHAR wideStr[128];
    MultiByteToWideChar(
            CP_UTF8, 0,
            argv[1], -1,
            wideStr, sizeof(wideStr)
    );
    wcscat(certificate, wideStr);
    wcscat(certificate, L"\nTYPE: ETERNAL");

    // 以C语言数组格式输出公钥
    printf("static const UCHAR Replace[] = { ");
    for (DWORD i = 0; i < cbPublicKey; i++) {
        printf("0x%02x", pbPublicKey[i]);
        if (i < cbPublicKey - 1) {
            printf(", ");
        }
    }
    printf(" };\n\n");

    SignCertificate(hKey);

    int i = 2;
    for (; i < argc; i++) {
        if (strlen(argv[i]) == 0) {
            i++;
            break;
        }

        int result = ReplaceContent(argv[i], officialKey, pbPublicKey, cbPublicKey);
        if (result != 1) {
            if (result == 0) printf("未在 %s 中找到公钥\n", "SbieSvc.exe");
            goto cleanup;
        }
    }
    for (; i < argc; i++) {
        SignFile(hKey, argv[i]);
    }

    printf("All done");

    cleanup:
    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return 0;
}
