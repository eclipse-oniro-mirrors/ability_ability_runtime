/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_HILOG_TAG_WRAPPER_H
#define OHOS_AAFWK_HILOG_TAG_WRAPPER_H

#include <cinttypes>
#include <map>
#include <string>

#include "hilog/log.h"

#ifndef AAFWK_FUNC_FMT
#define AAFWK_FUNC_FMT "[%{public}s%{public}d]"
#endif

#ifndef AAFWK_FILE_NAME
#define AAFWK_FILE_NAME \
    (__builtin_strrchr(__FILE_NAME__, '/') ? __builtin_strrchr(__FILE_NAME__, '/') + 1 : __FILE_NAME__)
#endif

#ifndef AAFWK_FUNC_INFO
#define AAFWK_FUNC_INFO AAFWK_FILE_NAME, __LINE__
#endif


namespace OHOS::AAFwk {
enum class AAFwkLogTag : uint32_t {
    DEFAULT = 0xD001300,        // 0XD001300
    ABILITY,
    TEST,
    AA_TOOL,
    CC_TOOL,
    ABILITY_SIM,

    APPDFR = DEFAULT + 0x10,    // 0xD001310
    APPMGR,
    DBOBSMGR,
    DIALOG,
    QUICKFIX,
    URIPERMMGR,
    BUNDLEMGRHELPER,
    APPKIT,

    JSENV = DEFAULT + 0x20,     // 0xD001320
    JSRUNTIME,
    FA,
    INTENT,
    JSNAPI,
    CJRUNTIME,
    ETSRUNTIME,
    ANI,

    DELEGATOR = DEFAULT + 0x30, // 0xD001330
    CONTEXT,
    UIABILITY,
    WANT,
    MISSION,
    CONNECTION,
    ABILITYMGR,
    ECOLOGICAL_RULE,
    DATA_ABILITY,

    EXT = DEFAULT + 0x40,       // 0xD001340
    AUTOFILL_EXT,
    SERVICE_EXT,
    FORM_EXT,
    SHARE_EXT,
    UI_EXT,
    ACTION_EXT,
    EMBEDDED_EXT,
    UISERVC_EXT,
    FORM_EDIT_EXT,

    WANTAGENT = DEFAULT + 0x50, // 0xD001350
    AUTOFILLMGR,
    EXTMGR,
    SER_ROUTER,
    CLI_TOOL = SER_ROUTER,
    AUTO_STARTUP,
    STARTUP,
    RECOVERY,
    PROCESSMGR,
    CONTINUATION,
    DISTRIBUTED,
    FREE_INSTALL,
    KEEP_ALIVE,
    QUERY_ERMS,

    LOCAL_CALL = DEFAULT + 0x60, // 0xD001360
    SA_INTERCEPTOR,
    APP_SERVICE_EXT,
    VERTICAL_PANEL,
    USER_CONTROLLER,

    END = 256,               // N.B. never use it
};

inline uint32_t GetOffset(AAFwkLogTag tag, AAFwkLogTag base)
{
    return static_cast<uint32_t>(tag) - static_cast<uint32_t>(base);
}

inline const char* GetDomainName0(AAFwkLogTag tag)
{
    const char* tagNames[] = { "AAFwk", "Ability", "Test", "AATool", "CCTool", "Simulator" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::DEFAULT);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName1(AAFwkLogTag tag)
{
    const char* tagNames[] = { "AppDfr", "AppMS", "DbObsMgr", "Dialog", "Quickfix",
        "UriPermMgr", "BMSHelper", "AppKit" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::APPDFR);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName2(AAFwkLogTag tag)
{
    const char* tagNames[] = { "JsEnv", "JsRuntime", "FA", "Intent", "JsNapi",
        "CjRuntime", "EtsRuntime", "Ani"};
    uint32_t offset = GetOffset(tag, AAFwkLogTag::JSENV);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName3(AAFwkLogTag tag)
{
    const char* tagNames[] = { "Delegator", "Context", "UIAbility", "Want", "Mission",
        "Connection", "AMS", "EcologicalRule", "DataAbility" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::DELEGATOR);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName4(AAFwkLogTag tag)
{
    const char* tagNames[] = { "Ext", "AutoFillExt", "ServiceExt", "FormExt", "ShareExt",
        "UIExt", "ActionExt", "EmbeddedExt", "UIServiceExt" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::EXT);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName5(AAFwkLogTag tag)
{
    const char* tagNames[] = { "WantAgent", "AutoFillMgr", "ExtMgr", "ServiceRouter",
        "AutoStartup", "Startup", "Recovery", "ProcessMgr", "Continuation",
        "Distributed", "FreeInstall", "KeepAlive", "QueryERMS" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::WANTAGENT);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName6(AAFwkLogTag tag)
{
    const char* tagNames[] = { "LocalCall", "SaInterceptor", "AppServiceExt", "VerticalPanel" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::LOCAL_CALL);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "UN";
    }
    return tagNames[offset];
}

static bool ConvertFileName(const std::string& fileBaseName, std::string& abbrFileName)
{
    if (fileBaseName == "ability_connect_callback_stub") {
        abbrFileName = "ACCS";
    } else if (fileBaseName == "ability_connect_manager") {
        abbrFileName = "ABCM";
    } else if (fileBaseName == "ability_manager_client") {
        abbrFileName = "AMC";
    } else if (fileBaseName == "ability_manager_service") {
        abbrFileName = "ABMS";
    } else if (fileBaseName == "ability_record") {
        abbrFileName = "ABR";
    } else if (fileBaseName == "app_mgr_service_inner") {
        abbrFileName = "AMSI";
    } else if (fileBaseName == "app_running_manager") {
        abbrFileName = "ARM";
    } else if (fileBaseName == "app_running_record") {
        abbrFileName = "ARR";
    } else if (fileBaseName == "app_spawn_client") {
        abbrFileName = "ASC";
    } else if (fileBaseName == "app_state_observer_manager") {
        abbrFileName = "ASOM";
    } else if (fileBaseName == "cache_process_manager") {
        abbrFileName = "CPM";
    } else if (fileBaseName == "connect_server_manager") {
        abbrFileName = "CSM";
    } else if (fileBaseName == "connection_record") {
        abbrFileName = "CONN";
    } else if (fileBaseName == "dump_runtime_helper") {
        abbrFileName = "DRH";
    } else if (fileBaseName == "js_service_extension_context") {
        abbrFileName = "JSEC";
    } else if (fileBaseName == "js_ui_ability") {
        abbrFileName = "JUA";
    } else if (fileBaseName == "js_service_extension") {
        abbrFileName = "JSE";
    } else if (fileBaseName == "main_thread") {
        abbrFileName = "MAINTHD";
    } else if (fileBaseName == "prload_uiext_state_observer") {
        abbrFileName = "PUSO";
    } else if (fileBaseName == "service_extension_context") {
        abbrFileName = "SEC";
    } else if (fileBaseName == "ui_ability_lifecycle_manager") {
        abbrFileName = "UALM";
    } else if (fileBaseName == "extension_impl") {
        abbrFileName = "EXTI";
    } else if (fileBaseName == "ams_mgr_scheduler") {
        abbrFileName = "AMSCHE";
    } else if (fileBaseName == "base_extension_record") {
        abbrFileName = "BER";
    }  else if (fileBaseName == "napi_want_agent") {
        abbrFileName = "NWA";
    }  else if (fileBaseName == "pending_want_manager") {
        abbrFileName = "PWM";
    }  else if (fileBaseName == "want_agent_helper") {
        abbrFileName = "WAH";
    }  else if (fileBaseName == "connection_manager") {
        abbrFileName = "CONM";
    }  else if (fileBaseName == "permission_verification") {
        abbrFileName = "PV";
    }  else if (fileBaseName == "task_handler_wrap") {
        abbrFileName = "THW";
    }  else if (fileBaseName == "appfreeze_util") {
        abbrFileName = "AU";
    }  else if (fileBaseName == "appfreeze_binder_collector") {
        abbrFileName = "ABC";
    }  else if (fileBaseName == "appfreeze_manager") {
        abbrFileName = "AM";
    }  else if (fileBaseName == "js_application_context_utils") {
        abbrFileName = "JACU";
    }  else if (fileBaseName == "ams_mgr_stub") {
        abbrFileName = "AMS";
    }  else if (fileBaseName == "js_auto_fill_extension") {
        abbrFileName = "JAFE";
    }  else if (fileBaseName == "auto_fill_extension_callback") {
        abbrFileName = "AFEC";
    }  else if (fileBaseName == "auto_fill_manager") {
        abbrFileName = "AFM";
    }  else if (fileBaseName == "ability_auto_startup_data_manager") {
        abbrFileName = "AASDM";
    }  else if (fileBaseName == "bundle_mgr_helper") {
        abbrFileName = "BMH";
    }  else if (fileBaseName == "js_ability_context") {
        abbrFileName = "JAC";
    }  else if (fileBaseName == "ability_context_impl") {
        abbrFileName = "ACI";
    }  else if (fileBaseName == "dataobs_mgr_inner") {
        abbrFileName = "DOMI";
    }  else if (fileBaseName == "js_ability_delegator_registry") {
        abbrFileName = "JADR";
    }  else if (fileBaseName == "js_dialog_session") {
        abbrFileName = "JDS";
    }  else if (fileBaseName == "js_dialog_request") {
        abbrFileName = "JDR";
    }  else if (fileBaseName == "system_dialog_scheduler") {
        abbrFileName = "SDS";
    }  else if (fileBaseName == "distributed_client") {
        abbrFileName = "DC";
    }  else if (fileBaseName == "extension_manager_client") {
        abbrFileName = "EMC";
    }  else if (fileBaseName == "form_extension") {
        abbrFileName = "FE";
    }  else if (fileBaseName == "form_extension_provider_client") {
        abbrFileName = "FEPC";
    }  else if (fileBaseName == "js_form_extension") {
        abbrFileName = "JFE";
    }  else if (fileBaseName == "free_install_observer_manager") {
        abbrFileName = "FIOM";
    }  else if (fileBaseName == "free_install_manager") {
        abbrFileName = "FIM";
    }  else if (fileBaseName == "insight_intent_execute_manager") {
        abbrFileName = "IIEM";
    }  else if (fileBaseName == "ability_manager_stub") {
        abbrFileName = "AMSTUB";
    }  else if (fileBaseName == "insight_intent_execute_callback_proxy") {
        abbrFileName = "IIECP";
    }  else if (fileBaseName == "js_insight_intent_driver") {
        abbrFileName = "JID";
    }  else if (fileBaseName == "js_error_manager") {
        abbrFileName = "JEM";
    }  else if (fileBaseName == "napi_common_want") {
        abbrFileName = "NCW";
    }  else if (fileBaseName == "keep_alive_process_manager") {
        abbrFileName = "KAPM";
    }  else if (fileBaseName == "local_call_record") {
        abbrFileName = "LCR";
    }  else if (fileBaseName == "local_call_container") {
        abbrFileName = "LCC";
    }  else if (fileBaseName == "distributed_mission_manager") {
        abbrFileName = "DMM";
    }  else if (fileBaseName == "dms_sa_client") {
        abbrFileName = "DMSC";
    }  else if (fileBaseName == "query_erms_manager") {
        abbrFileName = "QEM";
    }  else if (fileBaseName == "query_erms_observer_manager") {
        abbrFileName = "QEOM";
    }  else if (fileBaseName == "app_recovery") {
        abbrFileName = "AR";
    }  else if (fileBaseName == "startup_manager") {
        abbrFileName = "SM";
    }  else if (fileBaseName == "startup_task_manager") {
        abbrFileName = "STM";
    }  else if (fileBaseName == "ui_extension_ability_manager") {
        abbrFileName = "UEAM";
    }  else if (fileBaseName == "preload_ui_extension_host_client") {
        abbrFileName = "PUIEHC";
    }  else if (fileBaseName == "js_ui_extension") {
        abbrFileName = "JUE";
    }  else if (fileBaseName == "js_ui_extension_context") {
        abbrFileName = "JUEC";
    }  else if (fileBaseName == "uri_permission_manager_stub_impl") {
        abbrFileName = "UPMSI";
    }  else if (fileBaseName == "file_uri_distribution_utils") {
        abbrFileName = "FUDU";
    }  else if (fileBaseName == "napi_common_want_agent") {
        abbrFileName = "NCWA";
    }  else if (fileBaseName == "agent_bundle_event_callback") {
        abbrFileName = "ABEC";
    }  else if (fileBaseName == "agent_card_db_mgr") {
        abbrFileName = "ACDM";
    }  else if (fileBaseName == "cli_tool_manager_service") {
        abbrFileName = "CTMS";
    }  else if (fileBaseName == "js_runtime_common") {
        abbrFileName = "JRC";
    } else {
        return false;
    }
    return true;
}

[[maybe_unused]] static std::string CovertFileName(const std::string& fileName)
{
    std::string fileBaseName = fileName;
    auto pos = fileName.find_last_of(".");
    if (pos == std::string::npos) {
        return fileBaseName;
    }
    fileBaseName = fileName.substr(0, pos);
    std::string abbrFileName;
    if (ConvertFileName(fileBaseName, abbrFileName)) {
        return abbrFileName;
    }
    return fileBaseName;
}

constexpr uint32_t BASE_DEFAULT = 0;
constexpr uint32_t BASE_APPDFR = 1;
constexpr uint32_t BASE_JSENV = 2;
constexpr uint32_t BASE_DELEGATOR = 3;
constexpr uint32_t BASE_EXT = 4;
constexpr uint32_t BASE_WANTAGENT = 5;
constexpr uint32_t BASE_LOCAL_CALL = 6;

static inline const char* GetTagInfoFromDomainId(AAFwkLogTag tag)
{
    uint32_t offset = GetOffset(tag, AAFwkLogTag::DEFAULT);
    uint32_t base = offset >> 4;
    switch (base) {
        case BASE_DEFAULT: return GetDomainName0(tag);
        case BASE_APPDFR: return GetDomainName1(tag);
        case BASE_JSENV: return GetDomainName2(tag);
        case BASE_DELEGATOR: return GetDomainName3(tag);
        case BASE_EXT: return GetDomainName4(tag);
        case BASE_WANTAGENT: return GetDomainName5(tag);
        case BASE_LOCAL_CALL: return GetDomainName6(tag);
        default: return "UN";
    }
}
} // OHOS::AAFwk

using AAFwkTag = OHOS::AAFwk::AAFwkLogTag;

#define AAFWK_PRINT_LOG(level, tag, fmt, ...)                                                           \
    do {                                                                                                \
        AAFwkTag logTag = tag;                                                                          \
        std::string abbrFileName = OHOS::AAFwk::CovertFileName(std::string(AAFWK_FILE_NAME));   \
        ((void)HILOG_IMPL(LOG_CORE, level, static_cast<uint32_t>(logTag),                               \
        OHOS::AAFwk::GetTagInfoFromDomainId(logTag), AAFWK_FUNC_FMT fmt, \
        abbrFileName.c_str(), __LINE__, ##__VA_ARGS__));  \
    } while (0)

#define TAG_LOGD(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#define TAG_LOGI(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_INFO,  tag, fmt, ##__VA_ARGS__)
#define TAG_LOGW(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_WARN,  tag, fmt, ##__VA_ARGS__)
#define TAG_LOGE(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define TAG_LOGF(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_FATAL, tag, fmt, ##__VA_ARGS__)

#endif  // OHOS_AAFWK_HILOG_TAG_WRAPPER_H
