/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_IS_SA_CALL_TEST_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_IS_SA_CALL_TEST_H

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS::AAFwk {
class IsMockSaCall {
public:
    static void IsMockSaCallWithPermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.ACCESS_DLP_FILE",
            perms[1] = "ohos.permission.CLEAN_APPLICATION_DATA",
            perms[2] = "ohos.permission.CLEAN_BACKGROUND_PROCESSES",
            perms[3] = "ohos.permission.GET_RUNNING_INFO",
            perms[4] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS",
            perms[5] = "ohos.permission.MANAGE_MISSIONS",
            perms[6] = "ohos.permission.RUNNING_STATE_OBSERVER",
            perms[7] = "ohos.permission.SET_ABILITY_CONTROLLER",
            perms[8] = "ohos.permission.UPDATE_CONFIGURATION",
            perms[9] = "ohos.permission.INSTALL_BUNDLE",
            perms[10] = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED",
            perms[11] = "ohos.permission.START_INVISIBLE_ABILITY",
            perms[12] = "ohos.permission.START_ABILITIES_FROM_BACKGROUND",
            perms[13] = "ohos.permission.START_ABILIIES_FROM_BACKGROUND",
            perms[14] = "ohos.permission.ABILITY_BACKGROUND_COMMUNICATION",
            perms[15] = "ohos.permission.MANAGER_ABILITY_FROM_GATEWAY",
            perms[16] = "ohos.permission.PROXY_AUTHORIZATION_URI",
            perms[17] = "ohos.permission.EXEMPT_AS_CALLER",
            perms[18] = "ohos.permission.EXEMPT_AS_TARGET",
            perms[19] = "ohos.permission.PREPARE_APP_TERMINATE",
            perms[20] = "ohos.permission.START_ABILITY_WITH_ANIMATION",
            perms[21] = "ohos.permission.MANAGE_APP_BOOT_INTERNAL",
            perms[22] = "ohos.permission.CONNECT_UI_EXTENSION_ABILITY",
            perms[23] = "ohos.permission.START_RECENT_ABILITY"
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "distributedsched";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockCheckObserverCallerPermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.RUNNING_STATE_OBSERVER",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "memmgrservice";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockProcessCachePermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.SET_PROCESS_CACHE_STATE",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "distributedsched";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockSpecificSystemAbilityAccessPermission()
    {
        uint64_t tokenId;
        const char* perms[] = {
            perms[0] = "ohos.permission.SET_PROCESS_CACHE_STATE",
            perms[1] = "ohos.permission.GET_RUNNING_INFO",
            perms[2] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS",
            perms[3] = "ohos.permission.UPDATE_CONFIGURATION",
            perms[4] = "ohos.permission.ACCESS_DEVICE_COLLABORATION_SERVICE",
            perms[5] = "ohos.permission.INPUT_MONITORING",
            perms[6] = "ohos.permission.PERMISSION_USED_STATS",
            perms[7] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER",
            perms[8] = "ohos.permission.DISTRIBUTED_DATASYNC",
            perms[9] = "ohos.permission.MANAGE_AUDIO_CONFIG",
            perms[10] = "ohos.permission.WRITE_CALL_LOG",
            perms[11] = "ohos.permission.READ_CALL_LOG",
            perms[12] = "ohos.permission.READ_CONTACTS",
            perms[13] = "ohos.permission.READ_DFX_SYSEVENT",
            perms[14] = "ohos.permission.GRANT_SENSITIVE_PERMISSIONS",
            perms[15] = "ohos.permission.REVOKE_SENSITIVE_PERMISSIONS",
            perms[16] = "ohos.permission.MANAGE_SETTINGS",
            perms[17] = "ohos.permission.MANAGE_SECURE_SETTINGS",
            perms[18] = "ohos.permission.START_ABILITIES_FROM_BACKGROUND",
            perms[19] = "ohos.permission.ACCESS_SERVICE_DM",
            perms[20] = "ohos.permission.ACCESS_SERVICE_DP",
            perms[21] = "ohos.permission.STORAGE_MANAGER",
            perms[22] = "ohos.permission.PROXY_AUTHORIZATION_URI",
            perms[23] = "ohos.permission.ABILITY_BACKGROUND_COMMUNICATION",
            perms[24] = "ohos.permission.USE_USER_IDM",
            perms[25] = "ohos.permission.MANAGE_LOCAL_ACCOUNTS",
            perms[26] = "ohos.permission.LISTEN_BUNDLE_CHANGE",
            perms[27] = "ohos.permission.GET_TELEPHONY_STATE",
            perms[28] = "ohos.permission.SEND_MESSAGES",
            perms[29] = "ohos.permission.CONNECT_CELLULAR_CALL_SERVICE",
            perms[30] = "ohos.permission.SET_TELEPHONY_STATE",
            perms[31] = "ohos.permission.VIBRATE",
            perms[32] = "ohos.permission.SYSTEM_LIGHT_CONTROL",
            perms[33] = "ohos.permission.MANAGE_HAP_TOKENID",
            perms[34] = "ohos.permission.WRITE_WHOLE_CALENDAR",
            perms[35] = "ohos.permission.REPORT_RESOURCE_SCHEDULE_EVENT",
            perms[36] = "ohos.permission.START_INVISIBLE_ABILITY",
            perms[37] = "ohos.permission.GET_BUNDLE_INFO",
            perms[38] = "ohos.permission.GET_SUSPEND_STATE",
            perms[39] = "ohos.permission.PUBLISH_SYSTEM_COMMON_EVENT",
            perms[40] = "ohos.permission.PUBLISH_DISPLAY_ROTATION_EVENT",
            perms[41] = "ohos.permission.PUBLISH_CAST_PLUGGED_EVENT",
            perms[42] = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED",
            perms[43] = "ohos.permission.CLEAN_APPLICATION_DATA",
            perms[44] = "ohos.permission.REMOVE_CACHE_FILES",
            perms[45] = "ohos.permission.INSTALL_SANDBOX_BUNDLE",
            perms[46] = "ohos.permission.USE_BLUETOOTH",
            perms[47] = "ohos.permission.GET_SENSITIVE_PERMISSIONS",
            perms[48] = "ohos.permission.CONNECTIVITY_INTERNAL",
            perms[49] = "ohos.permission.ACCESS_BLUETOOTH",
            perms[50] = "ohos.permission.MANAGE_BLUETOOTH",
            perms[51] = "ohos.permission.ACCESS_NEARLINK",
            perms[52] = "ohos.permission.MANAGE_NEARLINK",
            perms[53] = "ohos.permission.RUNNING_STATE_OBSERVER",
            perms[54] = "ohos.permission.GET_INSTALLED_BUNDLE_LIST",
            perms[55] = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS_EXTENSION",
            perms[56] = "ohos.permission.MANAGE_USER_ACCOUNT_INFO",
            perms[57] = "ohos.permission.GET_NETWORK_INFO",
            perms[58] = "ohos.permission.VERIFY_ACTIVATION_LOCK",
            perms[59] = "ohos.permission.CAPTURE_SCREEN",
            perms[60] = "ohos.permission.GRANT_URI_PERMISSION_PRIVILEGED",
            perms[61] = "ohos.permission.START_SYSTEM_DIALOG",
            perms[62] = "ohos.permission.GET_BUNDLE_RESOURCES",
            perms[63] = "ohos.permission.REQUIRE_FORM",
            perms[64] = "ohos.permission.MODIFY_AUDIO_SETTINGS",
            perms[65] = "ohos.permission.ACCESS_SPAMSHIELD_SERVICE",
            perms[66] = "ohos.permission.APPROXIMATELY_LOCATION",
            perms[67] = "ohos.permission.LOCATION",
            perms[68] = "ohos.permission.LOCATION_IN_BACKGROUND",
            perms[69] = "ohos.permission.BUNDLE_ACTIVE_INFO",
            perms[70] = "ohos.permission.GET_SUPER_PRIVACY",
            perms[71] = "ohos.permission.FILE_ACCESS_PERSIST",
            perms[72] = "ohos.permission.SET_SANDBOX_POLICY",
            perms[73] = "ohos.permission.CHECK_SANDBOX_POLICY",
            perms[74] = "ohos.permission.SET_SUPER_PRIVACY",
            perms[75] = "ohos.permission.ACCESS_USER_AUTH_INTERNAL",
            perms[76] = "ohos.permission.MANAGE_MISSIONS",
            perms[77] = "ohos.permission.CONNECT_FORM_EXTENSION",
            perms[78] = "ohos.permission.CONNECT_STATIC_SUBSCRIBER_EXTENSION",
            perms[79] = "ohos.permission.KILL_PROCESS_DEPENDED_ON_ARKWEB",
            perms[80] = "ohos.permission.ACCESS_LOWPOWER_MANAGER",
            perms[81] = "ohos.permission.RECEIVER_STARTUP_COMPLETED",
            perms[82] = "ohos.permission.hsdr.HSDR_ACCESS",
            perms[83] = "ohos.permission.START_RESTORE_NOTIFICATION",
            perms[84] = "ohos.permission.ATTEST_KEY",
            perms[85] = "ohos.permission.READ_WHOLE_CALENDAR",
            perms[86] = "ohos.permission.MONITOR_DEVICE_NETWORK_STATE",
            perms[87] = "ohos.permission.NOTIFICATION_CONTROLLER",
            perms[88] = "ohos.permission.ACCESS_SCREEN_LOCK",
            perms[89] = "ohos.permission.hsdr.REQUEST_HSDR",
            perms[90] = "ohos.permission.ACCESS_CUSTOM_RINGTONE",
            perms[91] = "ohos.permission.ACCESS_SEARCH_SERVICE",
            perms[92] = "ohos.permission.RECEIVE_BMS_BROKER_MESSAGES",
            perms[93] = "ohos.permission.RECEIVE_UPDATE_MESSAGE",
            perms[94] = "ohos.permission.WRITE_RINGTONE",
            perms[95] = "ohos.permission.ACCESS_SCREEN_LOCK_INNER",
            perms[96] = "ohos.permission.DATA_IDENTIFY_ANONYMIZE",
            perms[97] = "ohos.permission.SUBSCRIBE_NOTIFICATION_WINDOW_STATE",
            perms[98] = "ohos.permission.NFC_CARD_EMULATION",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = static_cast<int32_t>(sizeof(perms)/sizeof(perms[0])),
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };
        infoInstance.processName = "foundation";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }

    static void IsMockKillAppProcessesPermission()
    {
        int permission = 0;
        uint64_t tokenId;
        const char** perms = new const char* [1];
        perms[permission] = "ohos.permission.KILL_APP_PROCESSES";
        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 1,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .aplStr = "system_core",
        };

        infoInstance.processName = "accesstoken_service";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        delete[] perms;
    }

    static void IsMockResetPermission()
    {
        uint64_t tokenId;
        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 0,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = nullptr,
            .acls = nullptr,
            .aplStr = "system_core",
        };

        infoInstance.processName = "test_reset";
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    }
};
}  // namespace OHOS::AAFwk
#endif // UNITTEST_OHOS_ABILITY_RUNTIME_IS_SA_CALL_TEST_H