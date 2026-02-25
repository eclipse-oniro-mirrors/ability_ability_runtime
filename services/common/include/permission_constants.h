/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_PERMISSION_CONSTANTS_H
#define OHOS_ABILITY_RUNTIME_PERMISSION_CONSTANTS_H

namespace OHOS {
namespace AAFwk {
namespace PermissionConstants {
constexpr const char* PERMISSION_ACCESS_DLP = "ohos.permission.ACCESS_DLP_FILE";
constexpr const char* PERMISSION_CLEAN_APPLICATION_DATA = "ohos.permission.CLEAN_APPLICATION_DATA";
constexpr const char* PERMISSION_CLEAN_BACKGROUND_PROCESSES = "ohos.permission.CLEAN_BACKGROUND_PROCESSES";
constexpr const char* PERMISSION_CONNECT_AGENT = "ohos.permission.CONNECT_AGENT";
constexpr const char* PERMISSION_GET_RUNNING_INFO = "ohos.permission.GET_RUNNING_INFO";
constexpr const char* PERMISSION_INTERACT_ACROSS_LOCAL_ACCOUNTS = "ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS";
constexpr const char* PERMISSION_MANAGE_MISSION = "ohos.permission.MANAGE_MISSIONS";
constexpr const char* PERMISSION_RUNNING_STATE_OBSERVER = "ohos.permission.RUNNING_STATE_OBSERVER";
constexpr const char* PERMISSION_SET_ABILITY_CONTROLLER = "ohos.permission.SET_ABILITY_CONTROLLER";
constexpr const char* PERMISSION_UPDATE_CONFIGURATION = "ohos.permission.UPDATE_CONFIGURATION";
constexpr const char* PERMISSION_UPDATE_APP_CONFIGURATION = "ohos.permission.UPDATE_APP_CONFIGURATION";
constexpr const char* PERMISSION_INSTALL_BUNDLE = "ohos.permission.INSTALL_BUNDLE";
constexpr const char* PERMISSION_LISTEN_BUNDLE_CHANGE = "ohos.permission.LISTEN_BUNDLE_CHANGE";
constexpr const char* PERMISSION_GET_BUNDLE_INFO_PRIVILEGED = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED";
constexpr const char* PERMISSION_START_INVISIBLE_ABILITY = "ohos.permission.START_INVISIBLE_ABILITY";
constexpr const char* PERMISSION_START_ABILITIES_FROM_BACKGROUND = "ohos.permission.START_ABILITIES_FROM_BACKGROUND";
constexpr const char* PERMISSION_START_ABILIIES_FROM_BACKGROUND = "ohos.permission.START_ABILIIES_FROM_BACKGROUND";
constexpr const char* PERMISSION_ABILITY_BACKGROUND_COMMUNICATION = "ohos.permission.ABILITY_BACKGROUND_COMMUNICATION";
constexpr const char* PERMISSION_MANAGER_ABILITY_FROM_GATEWAY = "ohos.permission.MANAGER_ABILITY_FROM_GATEWAY";
constexpr const char* PERMISSION_PROXY_AUTHORIZATION_URI = "ohos.permission.PROXY_AUTHORIZATION_URI";
constexpr const char* PERMISSION_FILE_ACCESS_MANAGER = "ohos.permission.FILE_ACCESS_MANAGER";
constexpr const char* PERMISSION_SANDBOX_ACCESS_MANAGER = "ohos.permission.SANDBOX_ACCESS_MANAGER";
constexpr const char* PERMISSION_WRITE_IMAGEVIDEO = "ohos.permission.WRITE_IMAGEVIDEO";
constexpr const char* PERMISSION_READ_IMAGEVIDEO = "ohos.permission.READ_IMAGEVIDEO";
constexpr const char* PERMISSION_WRITE_AUDIO = "ohos.permission.WRITE_AUDIO";
constexpr const char* PERMISSION_READ_AUDIO = "ohos.permission.READ_AUDIO";
constexpr const char* PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED = "ohos.permission.GRANT_URI_PERMISSION_PRIVILEGED";
constexpr const char* PERMISSION_GRANT_URI_PERMISSION_AS_CALLER = "ohos.permission.GRANT_URI_PERMISSION_AS_CALLER";
constexpr const char* PERMISSION_READ_WRITE_DOWNLOAD = "ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY";
constexpr const char* PERMISSION_READ_WRITE_DESKTOP = "ohos.permission.READ_WRITE_DESKTOP_DIRECTORY";
constexpr const char* PERMISSION_READ_WRITE_DOCUMENTS = "ohos.permission.READ_WRITE_DOCUMENTS_DIRECTORY";
constexpr const char* PERMISSION_EXEMPT_AS_CALLER = "ohos.permission.EXEMPT_AS_CALLER";
constexpr const char* PERMISSION_EXEMPT_AS_TARGET = "ohos.permission.EXEMPT_AS_TARGET";
constexpr const char* PERMISSION_PREPARE_TERMINATE = "ohos.permission.PREPARE_APP_TERMINATE";
constexpr const char* PERMISSION_START_RECENT_ABILITY = "ohos.permission.START_RECENT_ABILITY";
constexpr const char* PERMISSION_MANAGE_APP_BOOT = "ohos.permission.MANAGE_APP_BOOT";
constexpr const char* PERMISSION_START_ABILITY_WITH_ANIMATION = "ohos.permission.START_ABILITY_WITH_ANIMATION";
constexpr const char* PERMISSION_MANAGE_APP_BOOT_INTERNAL = "ohos.permission.MANAGE_APP_BOOT_INTERNAL";
constexpr const char* PERMISSION_CONNECT_UI_EXTENSION_ABILITY = "ohos.permission.CONNECT_UI_EXTENSION_ABILITY";
constexpr const char* PERMISSION_NOTIFY_DEBUG_ASSERT_RESULT = "ohos.permission.NOTIFY_DEBUG_ASSERT_RESULT";
constexpr const char* PERMISSION_START_SHORTCUT = "ohos.permission.START_SHORTCUT";
constexpr const char* PERMISSION_PRELOAD_APPLICATION = "ohos.permission.PRELOAD_APPLICATION";
constexpr const char* PERMISSION_SET_PROCESS_CACHE_STATE = "ohos.permission.SET_PROCESS_CACHE_STATE";
constexpr const char* PERMISSION_PRELOAD_UI_EXTENSION_ABILITY = "ohos.permission.PRELOAD_UI_EXTENSION_ABILITY";
constexpr const char* PERMISSION_KILL_APP_PROCESSES = "ohos.permission.KILL_APP_PROCESSES";
constexpr const char* PERMISSION_KILL_PROCESS_DEPENDED_ON_WEB = "ohos.permission.KILL_PROCESS_DEPENDED_ON_ARKWEB";
constexpr const char* PERMISSION_PRE_START_ATOMIC_SERVICE = "ohos.permission.PRE_START_ATOMIC_SERVICE";
constexpr const char* PERMISSION_START_NATIVE_CHILD_PROCESS = "ohos.permission.START_NATIVE_CHILD_PROCESS";
constexpr const char* PERMISSION_BLOCK_ALL_APP_START = "ohos.permission.BLOCK_ALL_APP_START";
constexpr const char* PERMISSION_START_UIABILITY_TO_HIDDEN = "ohos.permission.START_UIABILITY_TO_HIDDEN";
constexpr const char* PERMISSION_SUPERVISE_KIA_SERVICE = "ohos.permission.SUPERVISE_KIA_SERVICE";
constexpr const char* PERMISSION_GET_TELEPHONY_STATE = "ohos.permission.GET_TELEPHONY_STATE";
constexpr const char* PERMISSION_MANAGE_APP_KEEP_ALIVE = "ohos.permission.MANAGE_APP_KEEP_ALIVE";
constexpr const char* PERMISSION_MANAGE_APP_KEEP_ALIVE_INTERNAL = "ohos.permission.MANAGE_APP_KEEP_ALIVE_INTERNAL";
constexpr const char* PERMISSION_SET_LAUNCH_REASON_MESSAGE = "ohos.permission.SET_LAUNCH_REASON_MESSAGE";
constexpr const char* PERMISSION_PERFORM_LOCAL_DEBUG = "ohos.permission.PERFORM_LOCAL_DEBUG";
constexpr const char* PERMISSION_NDK_START_SELF_UI_ABILITY = "ohos.permission.NDK_START_SELF_UI_ABILITY";
constexpr const char* PERMISSION_FUSION_ACCESS = "ohos.permission.ACCESS_AMS_FROM_FUSION";
constexpr const char* PERMISSION_CUSTOM_SANDBOX = "ohos.permission.CUSTOM_SANDBOX";
constexpr const char* PERMISSION_FILE_ACCESS_PERSIST = "ohos.permission.FILE_ACCESS_PERSIST";
constexpr const char* PERMISSION_SUPPORT_APP_SERVICE_EXTENSION = "ohos.permission.SUPPORT_APP_SERVICE_EXTENSION";
constexpr const char* PERMISSION_MANAGE_EDM_POLICY = "ohos.permission.MANAGE_EDM_POLICY";
constexpr const char* PERMISSION_TRIGGER_LOCAL_WANTAGENT = "ohos.permission.TRIGGER_LOCAL_WANTAGENT";
constexpr const char* PERMISSION_GET_EDM_CONFIG = "ohos.permission.GET_EDM_CONFIG";
constexpr const char* PERMISSION_ALL_DEBUG = "ohos.permission.kernel.ALLOW_DEBUG";
constexpr const char* PERMISSION_GET_AGENT_CARD = "ohos.permission.GET_AGENT_CARD";
} // namespace PermissionConstants
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PERMISSION_CONSTANTS_H
