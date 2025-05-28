/*This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>*/

#include <windows.h>
#include <stdio.h>

__declspec(dllimport) LONG SbieApi_Call(ULONG api_code, LONG arg_num, ...);

enum {
    API_FIRST                   = 0x12340000L,

    API_GET_VERSION,
    API_GET_WORK_DEPRECATED,    			    // deprecated
    API_LOG_MESSAGE,
    API_GET_LICENSE_PRE_V3_48_DEPRECATED,       // deprecated
    API_SET_LICENSE_PRE_V3_48_DEPRECATED,       // deprecated
    API_START_PROCESS_PRE_V3_44_DEPRECATED,     // deprecated
    API_QUERY_PROCESS,
    API_QUERY_BOX_PATH,
    API_QUERY_PROCESS_PATH,
    API_QUERY_PATH_LIST,
    API_ENUM_PROCESSES,
    API_DISABLE_FORCE_PROCESS,
    API_HOOK_TRAMP_DEPRECATED,					// deprecated
    API_UNMOUNT_HIVES_DEPRECATED,               // deprecated
    API_QUERY_CONF,
    API_RELOAD_CONF,
    API_CREATE_DIR_OR_LINK,
    API_DUPLICATE_OBJECT,
    API_GET_INJECT_SAVE_AREA_DEPRECATED,        // deprecated
    API_RENAME_FILE,
    API_SET_USER_NAME,
    API_INIT_GUI,
    API_UNLOAD_DRIVER,
    API_GET_SET_DEVICE_MAP_DEPRECATED,          // deprecated
    API_SESSION_SET_LEADER_DEPRECATED,          // deprecated
    API_GLOBAL_FORCE_PROCESS_DEPRECATED,        // deprecated
    API_MONITOR_CONTROL,
    API_MONITOR_PUT_DEPRECATED,                 // deprecated
    API_MONITOR_GET_DEPRECATED,                 // deprecated
    API_GET_UNMOUNT_HIVE,
    API_GET_FILE_NAME,
    API_REFRESH_FILE_PATH_LIST,
    API_SET_LSA_AUTH_PKG,
    API_OPEN_FILE,
    API_SESSION_CHECK_LEADER_DEPRECATED,        // deprecated
    API_START_PROCESS,
    API_CHECK_INTERNET_ACCESS,
    API_GET_HOME_PATH,
    API_GET_BLOCKED_DLL,
    API_QUERY_LICENSE,
    API_ACTIVATE_LICENSE_DEPRECATED,            // deprecated
    API_OPEN_DEVICE_MAP,
    API_OPEN_PROCESS,
    API_QUERY_PROCESS_INFO,
    API_IS_BOX_ENABLED,
    API_SESSION_LEADER,
    API_QUERY_SYMBOLIC_LINK,
    API_OPEN_KEY,
    API_SET_LOW_LABEL_KEY,
    API_OVERRIDE_PROCESS_TOKEN_DEPRECATED,      // deprecated
    API_SET_SERVICE_PORT,
    API_INJECT_COMPLETE,
    API_QUERY_SYSCALLS,
    API_INVOKE_SYSCALL,
    API_GUI_CLIPBOARD,
    API_ALLOW_SPOOLER_PRINT_TO_FILE_DEPRECATED, // deprecated
    API_RELOAD_CONF2,                           // unused
    API_MONITOR_PUT2,
    API_GET_SPOOLER_PORT_DEPRECATED,            // deprecated
    API_GET_WPAD_PORT_DEPRECATED,               // deprecated
    API_SET_GAME_CONFIG_STORE_PORT_DEPRECATED,  // deprecated
    API_SET_SMART_CARD_PORT_DEPRECATED,         // deprecated
	API_MONITOR_GET_EX,
	API_GET_MESSAGE,
	API_PROCESS_EXEMPTION_CONTROL,
    API_GET_DYNAMIC_PORT_FROM_PID,
    API_OPEN_DYNAMIC_PORT,
    API_QUERY_DRIVER_INFO,
    API_FILTER_TOKEN,
    API_SET_SECURE_PARAM,
    API_GET_SECURE_PARAM,
    API_MONITOR_GET2,
    API_PROTECT_ROOT,
    API_UNPROTECT_ROOT,
    API_KILL_PROCESS,
    API_FORCE_CHILDREN,
    API_MONITOR_PUT_EX,
    API_UPDATE_CONF,
    API_VERIFY,

    API_LAST
};

int main() {
    const char BlockList0[] = "2687F3F7D9DBF317A05251E2ED3C3D0A";
    const unsigned char BlockListSig0[64] =  { 0 };

    LONG status;
    status = SbieApi_Call(API_SET_SECURE_PARAM, 3, L"CertBlockList", BlockList0, sizeof(BlockList0) - 1);
    printf("CertBlockList=%p\n", status);

    status = SbieApi_Call(API_SET_SECURE_PARAM, 3, L"CertBlockListSig", BlockListSig0, sizeof(BlockListSig0));
    printf("CertBlockListSig=%p", status);

    return 0;
}