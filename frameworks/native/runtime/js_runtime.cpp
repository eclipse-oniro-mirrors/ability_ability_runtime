/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "js_runtime.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

#include <atomic>
#include <sys/epoll.h>
#include <unistd.h>

#include "file_ex.h"
#include "accesstoken_kit.h"
#include "config_policy_utils.h"
#include "constants.h"
#include "connect_server_manager.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "extract_resource_manager.h"
#include "file_mapper.h"
#include "file_path_utils.h"
#include "hdc_register.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "js_environment.h"
#include "js_module_reader.h"
#include "js_module_searcher.h"
#include "js_quickfix_callback.h"
#include "js_runtime_common.h"
#include "js_runtime_utils.h"
#include "js_utils.h"
#include "js_worker.h"
#include "module_checker_delegate.h"
#include "napi/native_api.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_create_env.h"
#include "native_engine/native_engine.h"
#include "js_runtime_lite.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"
#include "parameters.h"
#include "extractor.h"
#include "replace_intl_module.h"
#include "system_ability_definition.h"
#include "source_map.h"
#include "source_map_operator.h"
#include "worker_info.h"

#ifdef SUPPORT_SCREEN
#include "hot_reloader.h"
#include "ace_forward_compatibility.h"
#include "declarative_module_preloader.h"
#endif //SUPPORT_SCREEN

#include "syscap_ts.h"

using namespace OHOS::AbilityBase;
using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t PARAM_TWO = 2;
constexpr uint8_t SYSCAP_MAX_SIZE = 100;
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
constexpr int32_t DEFAULT_INTER_VAL = 500;
constexpr int32_t API8 = 8;
const std::string SANDBOX_ARK_CACHE_PATH = "/data/storage/ark-cache/";
const std::string SANDBOX_ARK_PROIFILE_PATH = "/data/storage/ark-profile";

constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
constexpr const char* PERMISSION_RUN_ANY_CODE = "ohos.permission.RUN_ANY_CODE";
constexpr const char* PERMISSION_LOAD_INDEPENDENT_LIBRARY = "ohos.permission.kernel.LOAD_INDEPENDENT_LIBRARY";

const std::string CONFIG_PATH = "/etc/system_kits_config.json";
const std::string SYSTEM_KITS_CONFIG_PATH = "/system/etc/system_kits_config.json";

const std::string SYSTEM_KITS = "systemkits";
const std::string NAMESPACE = "namespace";
const std::string TARGET_OHM = "targetohm";
const std::string SINCE_VERSION = "sinceVersion";
const std::string DEFAULT_PLUGIN = "plugin_default_namespace";

constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
const std::string MERGE_SOURCE_MAP_PATH = "ets/sourceMaps.map";
static auto PermissionCheckFunc = []() {
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();

    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, PERMISSION_RUN_ANY_CODE);
    if (result == Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        return true;
    } else {
        return false;
    }
};

int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
{
    TAG_LOGI(AAFwkTag::JSRUNTIME, "ArkLog: %{public}s", message);
    return 0;
}
} // namespace

std::atomic<bool> JsRuntime::hasInstance(false);
JsRuntime::JsRuntime()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
}

JsRuntime::~JsRuntime()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    Deinitialize();
    StopDebugMode();
}

bool JsRuntime::Init(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    JsRuntimeLite::InitJsRuntimeLite(options);
    return Initialize(options);
}

std::unique_ptr<JsRuntime> JsRuntime::Create(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::unique_ptr<JsRuntime> instance;
    JsRuntimeLite::InitJsRuntimeLite(options);
    if (!options.preload && options.isStageModel) {
        auto preloadedInstance = Runtime::GetPreloaded(Language::JS);
#ifdef SUPPORT_SCREEN
        // reload ace if compatible mode changes
        if (Ace::AceForwardCompatibility::PipelineChanged() && preloadedInstance) {
            preloadedInstance.reset();
        }
#endif
        if (preloadedInstance && preloadedInstance->GetLanguage() == Runtime::Language::JS) {
            instance.reset(static_cast<JsRuntime*>(preloadedInstance.release()));
        } else {
            instance = std::make_unique<JsRuntime>();
        }
    } else {
        instance = std::make_unique<JsRuntime>();
    }

    if (!instance->Initialize(options)) {
        return std::unique_ptr<JsRuntime>();
    }
    return instance;
}

void JsRuntime::StartDebugMode(const DebugOption dOption)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "localDebug %{public}d", dOption.isDebugFromLocal);
    if (!ShouldSkipDebugMode(dOption)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "not start debug");
        return;
    }
    CHECK_POINTER(jsEnv_);
    if (jsEnv_->GetDebugMode()) {
        TAG_LOGI(AAFwkTag::JSRUNTIME, "debugMode");
        return;
    }
    // Set instance id to tid after the first instance.
    if (JsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
        instanceId_ = static_cast<uint32_t>(getproctid());
    }

    bool isStartWithDebug = dOption.isStartWithDebug;
    bool isDebugApp = dOption.isDebugApp;
    std::string appProvisionType = dOption.appProvisionType;
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Ark VM is starting debug mode [%{public}s]", isStartWithDebug ? "break" : "normal");
    JsRuntimeCommon::GetInstance().StartDebuggerModule(isDebugApp, dOption.isStartWithNative);
    const std::string bundleName = bundleName_;
    uint32_t instanceId = instanceId_;
    auto weak = jsEnv_;
    std::string inputProcessName = bundleName_ != dOption.processName ? dOption.processName : "";
    HdcRegister::DebugRegisterMode debugMode = HdcRegister::DebugRegisterMode::HDC_DEBUG_REG;
    if (debugOption_.isDebugFromLocal && debugOption_.isDeveloperMode) {
        debugMode = HdcRegister::DebugRegisterMode::BOTH_REG;
    } else if (debugOption_.isDebugFromLocal) {
        debugMode = HdcRegister::DebugRegisterMode::LOCAL_DEBUG_REG;
    }
    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp, debugMode,
        [bundleName, isStartWithDebug, instanceId, weak, isDebugApp, appProvisionType]
        (int socketFd, std::string option) {
        TAG_LOGI(AAFwkTag::JSRUNTIME,
            "HdcRegister msg, fd %{public}d, option %{public}s, isStartWithDebug %{public}d, isDebugApp %{public}d",
            socketFd, option.c_str(), isStartWithDebug, isDebugApp);
        if (weak == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null weak");
            return;
        }
        // system is unlocked when const.boot.oemmode is rd
        std::string oemmode = OHOS::system::GetParameter("const.boot.oemmode", "");
        bool unlocked = "rd" == oemmode;
        TAG_LOGI(AAFwkTag::JSRUNTIME, "unlocked= %{public}d, oemmode= %{public}s", unlocked, oemmode.c_str());
        // Don't start any server if (system is locked) and app is release version
        // Starting ConnectServer in release app on debuggable system is only for debug mode, not for profiling mode.
        if ((!unlocked) && appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "not support release app");
            return;
        }
        if (option.find(DEBUGGER) == std::string::npos) {
            // if has old connect server, stop it
            ConnectServerManager::Get().StopConnectServer(false);
            ConnectServerManager::Get().SendDebuggerInfo(isStartWithDebug, isDebugApp);
            ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
        } else {
            // if has old debugger server, stop it
            weak->StopDebugger(option);
            weak->StartDebugger(option, socketFd, isDebugApp);
        }
    });
    if (isDebugApp && appProvisionType != AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
        ConnectServerManager::Get().StartConnectServer(bundleName_, -1, true);
    }

    DebuggerConnectionHandler(isDebugApp, isStartWithDebug);
}

bool JsRuntime::ShouldSkipDebugMode(const DebugOption dOption)
{
    if (!dOption.isDebugFromLocal && !dOption.isDeveloperMode) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "developer Mode false");
        return false;
    }
    if (!(dOption.arkTSMode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "developer register in sts");
        return false;
    }
    return true;
}

void JsRuntime::DebuggerConnectionHandler(bool isDebugApp, bool isStartWithDebug)
{
    ConnectServerManager::Get().StoreInstanceMessage(getproctid(), instanceId_);
    EcmaVM* vm = GetEcmaVm();
    CHECK_POINTER(jsEnv_);
    auto dTask = jsEnv_->GetDebuggerPostTask();
    panda::JSNApi::DebugOption option = {ARK_DEBUGGER_LIB_PATH, isDebugApp ? isStartWithDebug : false};
    ConnectServerManager::Get().StoreDebuggerInfo(getproctid(), reinterpret_cast<void*>(vm), option, dTask, isDebugApp);
    jsEnv_->NotifyDebugMode(getproctid(), ARK_DEBUGGER_LIB_PATH, instanceId_, isDebugApp, isStartWithDebug);
}

void JsRuntime::StopDebugMode()
{
    CHECK_POINTER(jsEnv_);
    if (jsEnv_->GetDebugMode()) {
        ConnectServerManager::Get().RemoveInstance(instanceId_);
        StopDebugger();
    }
}

void JsRuntime::InitConsoleModule()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER(jsEnv_);
    jsEnv_->InitConsoleModule();
}

bool JsRuntime::StartDebugger(bool needBreakPoint, uint32_t instanceId)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    return true;
}

void JsRuntime::StopDebugger()
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->StopDebugger();
}

int32_t JsRuntime::JsperfProfilerCommandParse(const std::string &command, int32_t defaultValue)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "profiler command parse %{public}s", command.c_str());
    auto findPos = command.find("jsperf");
    if (findPos == std::string::npos) {
        // jsperf command not found, so not to do, return zero.
        TAG_LOGD(AAFwkTag::JSRUNTIME, "jsperf command not found");
        return 0;
    }

    // match jsperf command
    auto jsPerfStr = command.substr(findPos, command.length() - findPos);
    const std::regex regexJsperf(R"(^jsperf($|\s+($|\d*\s*($|nativeperf.*))))");
    std::match_results<std::string::const_iterator> matchResults;
    if (!std::regex_match(jsPerfStr, matchResults, regexJsperf)) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "the order not match");
        return defaultValue;
    }

    // get match resuflt
    std::string jsperfResuflt;
    constexpr size_t matchResultIndex = 1;
    if (matchResults.size() < PARAM_TWO) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "no results need to be matched");
        return defaultValue;
    }

    jsperfResuflt = matchResults[matchResultIndex].str();
    // match number result
    const std::regex regexJsperfNum(R"(^\s*(\d+).*)");
    std::match_results<std::string::const_iterator> jsperfMatchResults;
    if (!std::regex_match(jsperfResuflt, jsperfMatchResults, regexJsperfNum)) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "the jsperf results not match");
        return defaultValue;
    }

    // get match result
    std::string interval;
    constexpr size_t matchNumResultIndex = 1;
    if (jsperfMatchResults.size() < PARAM_TWO) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "jsperfMatchResults not match");
        return defaultValue;
    }

    interval = jsperfMatchResults[matchNumResultIndex].str();
    if (interval.empty()) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "empty interval");
        return defaultValue;
    }

    return std::stoi(interval);
}

void JsRuntime::StartProfiler(const DebugOption dOption)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "localDebug %{public}d", dOption.isDebugFromLocal);
    if (!dOption.isDebugFromLocal && !dOption.isDeveloperMode) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "developer Mode false");
        return;
    }
    CHECK_POINTER(jsEnv_);
    if (JsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
        instanceId_ = static_cast<uint32_t>(getproctid());
    }

    bool isStartWithDebug = dOption.isStartWithDebug;
    bool isDebugApp = dOption.isDebugApp;
    std::string appProvisionType = dOption.appProvisionType;
    JsRuntimeCommon::GetInstance().StartDebuggerModule(isDebugApp, dOption.isStartWithNative);
    const std::string bundleName = bundleName_;
    auto weak = jsEnv_;
    uint32_t instanceId = instanceId_;
    std::string inputProcessName = bundleName_ != dOption.processName ? dOption.processName : "";
    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp,
        HdcRegister::DebugRegisterMode::HDC_DEBUG_REG,
        [bundleName, isStartWithDebug, instanceId, weak, isDebugApp, appProvisionType](int socketFd, std::string option) {
        TAG_LOGI(AAFwkTag::JSRUNTIME, "HdcRegister msg, fd= %{public}d, option= %{public}s", socketFd, option.c_str());
        if (weak == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null weak");
            return;
        }
        if (appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "not support release app");
            return;
        }
        if (option.find(DEBUGGER) == std::string::npos) {
            // if has old connect server, stop it
            ConnectServerManager::Get().StopConnectServer(false);
            ConnectServerManager::Get().SendDebuggerInfo(isStartWithDebug, isDebugApp);
            ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
        } else {
            // if has old debugger server, stop it
            weak->StopDebugger(option);
            weak->StartDebugger(option, socketFd, isDebugApp);
        }
    });

    DebuggerConnectionManager(isDebugApp, isStartWithDebug, dOption);
}

void JsRuntime::DebuggerConnectionManager(bool isDebugApp, bool isStartWithDebug, const DebugOption dOption)
{
    if (isDebugApp && dOption.appProvisionType != AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
        ConnectServerManager::Get().StartConnectServer(bundleName_, 0, true);
    }
    ConnectServerManager::Get().StoreInstanceMessage(getproctid(), instanceId_);
    JsEnv::JsEnvironment::PROFILERTYPE profiler = JsEnv::JsEnvironment::PROFILERTYPE::PROFILERTYPE_HEAP;
    int32_t interval = 0;
    const std::string profilerCommand("profile");
    if (dOption.perfCmd.find(profilerCommand) != std::string::npos) {
        profiler = JsEnv::JsEnvironment::PROFILERTYPE::PROFILERTYPE_CPU;
        interval = JsperfProfilerCommandParse(dOption.perfCmd, DEFAULT_INTER_VAL);
    }
    EcmaVM* vm = GetEcmaVm();
    CHECK_POINTER(jsEnv_);
    auto dTask = jsEnv_->GetDebuggerPostTask();
    panda::JSNApi::DebugOption option = {ARK_DEBUGGER_LIB_PATH, isDebugApp ? isStartWithDebug : false};
    ConnectServerManager::Get().StoreDebuggerInfo(getproctid(), reinterpret_cast<void*>(vm), option, dTask, isDebugApp);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "profiler:%{public}d interval:%{public}d", profiler, interval);
    jsEnv_->StartProfiler(ARK_DEBUGGER_LIB_PATH, instanceId_, profiler, interval, getproctid(), isDebugApp);
}

bool JsRuntime::GetFileBuffer(const std::string& filePath, std::string& fileFullName, std::vector<uint8_t>& buffer,
                              bool isABC)
{
    Extractor extractor(filePath);
    if (!extractor.Init()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Extractor of %{private}s init failed", filePath.c_str());
        return false;
    }

    std::vector<std::string> fileNames;
    if (isABC) {
        extractor.GetSpecifiedTypeFiles(fileNames, ".abc");
    } else {
        extractor.GetSpecifiedTypeFiles(fileNames, ".map");
    }
    if (fileNames.empty()) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "no .abc in hap/hqf %{private}s", filePath.c_str());
        return true;
    }

    std::string fileName = fileNames.front();
    fileFullName = filePath + "/" + fileName;
    std::unique_ptr<uint8_t[]> data;
    size_t dataLen = 0;
    if (!extractor.ExtractToBufByName(fileName, data, dataLen)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Extract %{public}s failed", fileFullName.c_str());
        return false;
    }

    buffer.assign(data.get(), data.get() + dataLen);
    return true;
}

std::shared_ptr<AbilityBase::FileMapper> JsRuntime::GetSafeData(const std::string& path, std::string& fileFullName)
{
    bool newCreate = false;
    auto extractor = ExtractorUtil::GetExtractor(path, newCreate, true);
    if (extractor == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null extractor path: %{private}s", path.c_str());
        return nullptr;
    }

    std::vector<std::string> fileNames;
    extractor->GetSpecifiedTypeFiles(fileNames, ".abc");
    if (fileNames.empty()) {
        TAG_LOGI(AAFwkTag::JSRUNTIME, "no abc file in hap or hqf: %{private}s", path.c_str());
        return nullptr;
    }
    std::string fileName = fileNames.front();
    fileFullName = path + "/" + fileName;

    auto safeData = extractor->GetSafeData(fileName);
    if (safeData == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null safeData path: %{private}s", path.c_str());
        return nullptr;
    }

    return safeData;
}

bool JsRuntime::LoadRepairPatch(const std::string& hqfFile, const std::string& hapPath)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);

    InitSourceMap(hqfFile);

    std::string patchFile;
    auto hqfSafeData = GetSafeData(hqfFile, patchFile);
    if (hqfSafeData == nullptr) {
        if (patchFile.empty()) {
            TAG_LOGI(AAFwkTag::JSRUNTIME, "No need to load patch cause no ets. path: %{private}s", hqfFile.c_str());
            return true;
        }
        return false;
    }

    std::string baseFile;
    auto hapSafeData = GetSafeData(hapPath, baseFile);
    if (hapSafeData == nullptr) {
        return false;
    }

    std::string resolvedHapPath;
    auto position = hapPath.find(".hap");
    if (position != std::string::npos) {
        resolvedHapPath = hapPath.substr(0, position) + MERGE_ABC_PATH;
    }

    auto hspPosition = hapPath.find(".hsp");
    if (hspPosition != std::string::npos) {
        resolvedHapPath = hapPath.substr(0, hspPosition) + MERGE_ABC_PATH;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "LoadPatch, patchFile: %{private}s, baseFile: %{private}s",
        patchFile.c_str(), resolvedHapPath.c_str());
    auto ret = panda::JSNApi::LoadPatch(vm, patchFile, hqfSafeData->GetDataPtr(), hqfSafeData->GetDataLen(),
        resolvedHapPath, hapSafeData->GetDataPtr(), hapSafeData->GetDataLen());
    if (ret != panda::JSNApi::PatchErrorCode::SUCCESS) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "LoadPatch failed:%{public}d", static_cast<int32_t>(ret));
        return false;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "Load patch %{private}s succeed", patchFile.c_str());
    return true;
}

bool JsRuntime::UnLoadRepairPatch(const std::string& hqfFile)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);

    Extractor extractor(hqfFile);
    if (!extractor.Init()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Extractor of %{private}s init failed", hqfFile.c_str());
        return false;
    }

    std::vector<std::string> fileNames;
    extractor.GetSpecifiedTypeFiles(fileNames, ".abc");
    if (fileNames.empty()) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "no .abc in hqf %{private}s", hqfFile.c_str());
        return true;
    }

    for (const auto &fileName : fileNames) {
        std::string patchFile = hqfFile + "/" + fileName;
        TAG_LOGD(AAFwkTag::JSRUNTIME, "UnloadPatch, patchFile: %{private}s", patchFile.c_str());
        auto ret = panda::JSNApi::UnloadPatch(vm, patchFile);
        if (ret != panda::JSNApi::PatchErrorCode::SUCCESS) {
            TAG_LOGW(AAFwkTag::JSRUNTIME, "UnLoadPatch failed with %{public}d", static_cast<int32_t>(ret));
        }
        TAG_LOGD(AAFwkTag::JSRUNTIME, "UnLoad patch %{private}s succeed", patchFile.c_str());
    }

    return true;
}

bool JsRuntime::NotifyHotReloadPage()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
#ifdef SUPPORT_SCREEN
    Ace::HotReloader::HotReload();
#endif // SUPPORT_SCREEN
    return true;
}

bool JsRuntime::LoadScript(const std::string& path, std::vector<uint8_t>* buffer, bool isBundle)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "path: %{private}s", path.c_str());
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    return jsEnv_->LoadScript(path, buffer, isBundle);
}

bool JsRuntime::LoadScript(const std::string& path, uint8_t* buffer, size_t len, bool isBundle,
    const std::string& srcEntrance)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "path: %{private}s", path.c_str());
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    if (isOhmUrl_ && !moduleName_.empty()) {
        auto vm = GetEcmaVm();
        CHECK_POINTER_AND_RETURN(vm, false);
        std::string srcFilename = "";
        srcFilename = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
        return panda::JSNApi::ExecuteSecureWithOhmUrl(vm, buffer, len, srcFilename, srcEntrance);
    }
    return jsEnv_->LoadScript(path, buffer, len, isBundle);
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModuleByEngine(
    napi_env env, const std::string& moduleName, const napi_value* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "ModuleName %{public}s", moduleName.c_str());
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null env");
        return nullptr;
    }
    HandleScope handleScope(env);
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value propertyValue = nullptr;
    napi_get_named_property(env, globalObj, "requireNapi", &propertyValue);

    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    napi_ref tmpRef = nullptr;
    napi_create_reference(env, propertyValue, 1, &tmpRef);
    methodRequireNapiRef_.reset(reinterpret_cast<NativeReference*>(tmpRef));
    if (!methodRequireNapiRef_) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null methodRequireNapiRef_");
        return nullptr;
    }

    napi_value className = nullptr;
    napi_create_string_utf8(env, moduleName.c_str(), moduleName.length(), &className);
    auto refValue = methodRequireNapiRef_->GetNapiValue();
    napi_value args[1] = { className };
    napi_value classValue = nullptr;
    napi_call_function(env, globalObj, refValue, 1, args, &classValue);
    napi_value instanceValue = nullptr;
    napi_new_instance(env, classValue, argc, argv, &instanceValue);
    if (instanceValue == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null instanceValue");
        return nullptr;
    }

    napi_ref resultRef = nullptr;
    napi_create_reference(env, instanceValue, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

void JsRuntime::FinishPreload()
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    panda::JSNApi::PreFork(vm);
}

void JsRuntime::PostPreload(const Options& options)
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    auto env = GetNapiEnv();
    CHECK_POINTER(env);
    panda::RuntimeOption postOption;
    postOption.SetBundleName(options.bundleName);
    if (!options.arkNativeFilePath.empty()) {
        std::string sandBoxAnFilePath = SANDBOX_ARK_CACHE_PATH + options.arkNativeFilePath;
        postOption.SetAnDir(sandBoxAnFilePath);
    }
    if (options.isMultiThread) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "Multi-Thread Mode: %{public}d", options.isMultiThread);
        panda::JSNApi::SetMultiThreadCheck();
    }
    if (options.isErrorInfoEnhance) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "Start Error-Info-Enhance Mode: %{public}d.", options.isErrorInfoEnhance);
        panda::JSNApi::SetErrorInfoEnhance();
    }
    bool profileEnabled = OHOS::system::GetBoolParameter("ark.profile", false);
    postOption.SetEnableProfile(profileEnabled);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "ASMM JIT Verify PostFork, jitEnabled: %{public}d", options.jitEnabled);
    postOption.SetEnableJIT(options.jitEnabled);
    postOption.SetAOTCompileStatusMap(options.aotCompileStatusMap);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "SmartGC: set options enableWarmStartupSmartGC to RuntimeOption");
    postOption.SetEnableWarmStartupSmartGC(options.enableWarmStartupSmartGC);
    {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "panda::JSNApi::PostFork");
        panda::JSNApi::PostFork(vm, postOption);
    }
    reinterpret_cast<NativeEngine*>(env)->ReinitUVLoop();
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    panda::JSNApi::SetLoop(vm, loop);
    DFXJSNApi::RegisterAsyncDetectCallBack(vm);
}

void JsRuntime::LoadAotFile(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    if (options.hapPath.empty()) {
        return;
    }

    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(options.hapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate, true);
    if (extractor != nullptr && newCreate) {
        panda::JSNApi::LoadAotFile(vm, options.moduleName);
    }
}

bool JsRuntime::Initialize(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
#ifdef SUPPORT_SCREEN
    if (Ace::AceForwardCompatibility::PipelineChanged()) {
        preloaded_ = false;
    }
#endif
    if (!preloaded_) {
        if (!CreateJsEnv(options)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "Create jsEnv failed");
            return false;
        }
    }
    apiTargetVersion_ = options.apiTargetVersion;
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Initialize: %{public}d", apiTargetVersion_);
    bool isModular = false;
    if (IsUseAbilityRuntime(options)) {
        auto env = GetNapiEnv();
        auto nativeEngine = reinterpret_cast<NativeEngine*>(env);
        CHECK_POINTER_AND_RETURN(nativeEngine, false);

        auto vm = GetEcmaVm();
        CHECK_POINTER_AND_RETURN(vm, false);

        panda::JSNApi::SetLargeHeap(options.allowArkTsLargeHeap);
        if (preloaded_) {
            PostPreload(options);
        }
        HandleScope handleScope(*this);
        napi_value globalObj = nullptr;
        napi_get_global(env, &globalObj);
        CHECK_POINTER_AND_RETURN(globalObj, false);
        if (!preloaded_) {
            InitSyscapModule(env);

            // Simple hook function 'isSystemplugin'
            const char* moduleName = "JsRuntime";
            BindNativeFunction(env, globalObj, "isSystemplugin", moduleName,
                [](napi_env env, napi_callback_info cbinfo) -> napi_value {
                    return CreateJsUndefined(env);
                });

            napi_value propertyValue = nullptr;
            napi_get_named_property(env, globalObj, "requireNapi", &propertyValue);
            napi_ref tmpRef = nullptr;
            napi_create_reference(env, propertyValue, 1, &tmpRef);
            methodRequireNapiRef_.reset(reinterpret_cast<NativeReference*>(tmpRef));
            if (!methodRequireNapiRef_) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "null methodRequireNapiRef_");
                return false;
            }
            TAG_LOGD(AAFwkTag::JSRUNTIME, "PreloadAce start");
            PreloadAce(options);
            TAG_LOGD(AAFwkTag::JSRUNTIME, "PreloadAce end");
            nativeEngine->RegisterPermissionCheck(PermissionCheckFunc);
        }

        if (!options.preload) {
            isBundle_ = options.isBundle;
            bundleName_ = options.bundleName;
            codePath_ = options.codePath;
            panda::JSNApi::SetSearchHapPathTracker(
                vm, [options](const std::string moduleName, std::string &hapPath) -> bool {
                    if (options.hapModulePath.find(moduleName) == options.hapModulePath.end()) {
                        return false;
                    }
                    hapPath = options.hapModulePath.find(moduleName)->second;
                    return true;
                });
            ReInitJsEnvImpl(options);
            LoadAotFile(options);
            panda::JSNApi::SetBundle(vm, options.isBundle);
            panda::JSNApi::SetBundleName(vm, options.bundleName);
            panda::JSNApi::SetHostResolveBufferTracker(
                vm, JsModuleReader(options.bundleName, options.hapPath, options.isUnique));
            isModular = !panda::JSNApi::IsBundle(vm);
            std::vector<panda::HmsMap> systemKitsMap = GetSystemKitsMap(apiTargetVersion_);
            panda::JSNApi::SetHmsModuleList(vm, systemKitsMap);
            std::unordered_map<std::string, std::pair<std::unique_ptr<uint8_t[]>, size_t>> pkgInfoMap;
            pkgContextInfoJsonStringMap_ = options.pkgContextInfoJsonStringMap;
            packageNameList_ = options.packageNameList;
            JsRuntimeLite::GetInstance().GetPkgContextInfoListMap(
                options.pkgContextInfoJsonStringMap, pkgInfoMap);
            panda::JSNApi::SetPkgContextInfoList(vm, pkgInfoMap);
            panda::JSNApi::SetPkgNameList(vm, options.packageNameList);
            panda::JSNApi::ModuleDeserialize(vm, options.versionCode);
        }
    }

    if (!preloaded_) {
        InitConsoleModule();
    }

    if (!options.preload) {
        auto operatorObj = std::make_shared<JsEnv::SourceMapOperator>(options.bundleName);
        InitSourceMap(operatorObj);

        if (options.isUnique) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "Not supported TimerModule when form render");
        } else {
            InitTimerModule();
        }

        OHOS::Global::I18n::ReplaceIntlModule(GetNapiEnv());
        InitWorkerModule(options);
        SetModuleLoadChecker(options.moduleCheckerDelegate);
        SetRequestAotCallback();

        if (!InitLoop(options.isStageModel)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "Init loop failed");
            return false;
        }
    }

    preloaded_ = options.preload;
    return true;
}

bool JsRuntime::CreateJsEnv(const Options& options)
{
    panda::RuntimeOption pandaOption;
    int arkProperties = OHOS::system::GetIntParameter<int>("persist.ark.properties", -1);
    std::string bundleName = OHOS::system::GetParameter("persist.ark.arkbundlename", "");
    std::string memConfigProperty = OHOS::system::GetParameter("persist.ark.mem_config_property", "");
    size_t gcThreadNum = OHOS::system::GetUintParameter<size_t>("persist.ark.gcthreads", 7);
    size_t longPauseTime = OHOS::system::GetUintParameter<size_t>("persist.ark.longpausetime", 40);
    pandaOption.SetLargeHeap(options.allowArkTsLargeHeap);
    pandaOption.SetArkProperties(arkProperties);
    pandaOption.SetArkBundleName(bundleName);
    pandaOption.SetMemConfigProperty(memConfigProperty);
    pandaOption.SetGcThreadNum(gcThreadNum);
    pandaOption.SetLongPauseTime(longPauseTime);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "ark properties=%{public}d bundlename=%{public}s",
        arkProperties, bundleName.c_str());
    pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
    pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::FOLLOW);
    pandaOption.SetLogBufPrint(PrintVmLog);

    bool asmInterpreterEnabled = OHOS::system::GetBoolParameter("persist.ark.asminterpreter", true);
    std::string asmOpcodeDisableRange = OHOS::system::GetParameter("persist.ark.asmopcodedisablerange", "");
    pandaOption.SetEnableAsmInterpreter(asmInterpreterEnabled);
    pandaOption.SetAsmOpcodeDisableRange(asmOpcodeDisableRange);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "ASMM JIT Verify CreateJsEnv, jitEnabled: %{public}d", options.jitEnabled);
    pandaOption.SetEnableJIT(options.jitEnabled);

    if (options.isMultiThread) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "Start Multi Thread Mode: %{public}d", options.isMultiThread);
        panda::JSNApi::SetMultiThreadCheck();
    }

    if (options.isErrorInfoEnhance) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "Start Error Info Enhance Mode: %{public}d.", options.isErrorInfoEnhance);
        panda::JSNApi::SetErrorInfoEnhance();
    }

    if (IsUseAbilityRuntime(options)) {
        // aot related
        bool aotEnabled = OHOS::system::GetBoolParameter("persist.ark.aot", true);
        pandaOption.SetEnableAOT(aotEnabled);
        pandaOption.SetProfileDir(SANDBOX_ARK_PROIFILE_PATH);
    }

    OHOSJsEnvLogger::RegisterJsEnvLogger();
    jsEnv_ = std::make_shared<JsEnv::JsEnvironment>(std::make_unique<OHOSJsEnvironmentImpl>(options.eventRunner));
    if (jsEnv_ == nullptr || !jsEnv_->Initialize(pandaOption, static_cast<void*>(this))) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Init jsEnv failed");
        return false;
    }

    return true;
}

void JsRuntime::PreloadAce(const Options& options)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
#ifdef SUPPORT_SCREEN
    if (options.loadAce) {
        // ArkTsCard start
        if (options.isUnique) {
            OHOS::Ace::DeclarativeModulePreloader::PreloadCard(
                *nativeEngine, options.bundleName, options.pkgContextInfoJsonStringMap);
        } else {
            OHOS::Ace::DeclarativeModulePreloader::Preload(*nativeEngine);
        }
        // ArkTsCard end
    }
#endif
}

void JsRuntime::ReloadFormComponent()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    // ArkTsCard update condition, need to reload new component
#ifdef SUPPORT_SCREEN
    OHOS::Ace::DeclarativeModulePreloader::ReloadCard(*nativeEngine, bundleName_, pkgContextInfoJsonStringMap_);
#endif
}

bool JsRuntime::InitLoop(bool isStage)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    return jsEnv_->InitLoop(isStage);
}

void JsRuntime::SetAppLibPath(const AppLibPathMap& appLibPaths, const bool& isSystemApp)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Set library path");

    if (appLibPaths.size() == 0) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "no lib path to set");
        return;
    }

    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null moduleManager");
        return;
    }

    for (const auto &appLibPath : appLibPaths) {
        moduleManager->SetAppLibPath(appLibPath.first, appLibPath.second, isSystemApp);
    }
}

void JsRuntime::InheritPluginNamespace(const std::vector<std::string> &moduleNames)
{
    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null moduleManager");
        return;
    }
    std::string currentNamespace;
    if (!moduleManager->GetLdNamespaceName("default", currentNamespace)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "get current namespace failed");
        return;
    }

    for (const auto& item : moduleNames) {
        if (item.empty()) {
            continue;
        }
        std::string pluginNamespace;
        if (!moduleManager->GetLdNamespaceName(item, pluginNamespace)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "get %{public}s pluginNamespace failed", item.c_str());
            continue;
        }
        moduleManager->InheritNamespaceEachOther(currentNamespace, pluginNamespace);
    }
}

void JsRuntime::CreatePluginDefaultNamespace(const std::string &lddictionaries)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    Security::AccessToken::AccessTokenID selfToken = IPCSkeleton::GetSelfTokenID();
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(selfToken,
        PERMISSION_LOAD_INDEPENDENT_LIBRARY);
    if (result != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "verify access token failed: %{public}d", result);
        return;
    }
    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null moduleManager");
        return;
    }
    std::string currentNamespace;
    if (!moduleManager->GetLdNamespaceName("default", currentNamespace)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "get current namespace failed");
        return;
    }
    std::vector<std::string> appLibPaths;
    moduleManager->SetAppLibPath(DEFAULT_PLUGIN, appLibPaths, false);
    std::string pluginDefaultNamespace;
    moduleManager->GetLdNamespaceName(DEFAULT_PLUGIN, pluginDefaultNamespace);
    moduleManager->SetLdPermittedPathsForNamespace(pluginDefaultNamespace, lddictionaries);
    moduleManager->InheritNamespaceEachOther(currentNamespace, pluginDefaultNamespace);
}

void JsRuntime::InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER(jsEnv_);
    JsEnv::SourceMap::RegisterReadSourceMapCallback(JsRuntime::ReadSourceMapData);
    JsEnv::SourceMap::RegisterGetHapPathCallback(JsModuleReader::GetHapPathList);
    jsEnv_->InitSourceMap(operatorObj);
}

void JsRuntime::InitSourceMap(const std::string hqfFilePath)
{
    std::string patchSoureMapFile;
    std::vector<uint8_t> soureMapBuffer;
    if (!GetFileBuffer(hqfFilePath, patchSoureMapFile, soureMapBuffer, false)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "get patchSoureMap file buffer failed");
        return;
    }
    std::string str(soureMapBuffer.begin(), soureMapBuffer.end());
    CHECK_POINTER(jsEnv_);
    auto sourceMapOperator = jsEnv_->GetSourceMapOperator();
    if (sourceMapOperator != nullptr) {
        auto sourceMapObj = sourceMapOperator->GetSourceMapObj();
        if (sourceMapObj != nullptr) {
            sourceMapObj->SplitSourceMap(str);
        }
    }
}

void JsRuntime::Deinitialize()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    for (auto it = modules_.begin(); it != modules_.end(); it = modules_.erase(it)) {
        delete it->second;
        it->second = nullptr;
    }

    methodRequireNapiRef_.reset();

    CHECK_POINTER(jsEnv_);
    jsEnv_->DeInitLoop();
}

napi_value JsRuntime::LoadJsBundle(const std::string& path, const std::string& hapPath, bool useCommonChunk)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, nullptr);
    HandleEscape handleEscape(env);
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value exports = nullptr;
    napi_create_object(env, &exports);
    napi_set_named_property(env, globalObj, "exports", exports);

    if (!RunScript(path, hapPath, useCommonChunk)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "run script: %{private}s failed", path.c_str());
        return nullptr;
    }

    napi_value exportsObj = nullptr;
    napi_get_named_property(env, globalObj, "exports", &exportsObj);
    if (exportsObj == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null exportObj");
        return nullptr;
    }

    napi_value exportObj = nullptr;
    napi_get_named_property(env, exportsObj, "default", &exportObj);
    if (exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null exportObj");
        return nullptr;
    }

    return handleEscape.Escape(exportObj);
}

napi_value JsRuntime::LoadJsModule(const std::string& path, const std::string& hapPath, const std::string& srcEntrance)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!RunScript(path, hapPath, false, srcEntrance)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "run script: %{private}s failed", path.c_str());
        return nullptr;
    }

    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, nullptr);
    panda::Local<panda::ObjectRef> exportObj;
    if (isOhmUrl_) {
        exportObj = panda::JSNApi::GetExportObjectFromOhmUrl(vm, srcEntrance, "default");
    } else {
        exportObj = panda::JSNApi::GetExportObject(vm, path, "default");
    }

    if (exportObj->IsNull()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get export object failed");
        return nullptr;
    }

    auto env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, nullptr);
    return ArkNativeEngine::ArkValueToNapiValue(env, exportObj);
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(const std::string& moduleName, const std::string& modulePath,
    const std::string& hapPath, bool esmodule, bool useCommonChunk, const std::string& srcEntrance)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Load module(%{public}s, %{private}s, %{private}s, %{public}s)",
        moduleName.c_str(), modulePath.c_str(), hapPath.c_str(), esmodule ? "true" : "false");
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, std::unique_ptr<NativeReference>());
    // use for debugger, js engine need to know load module to handle debug event
    panda::JSNApi::NotifyLoadModule(vm);
    auto env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, std::unique_ptr<NativeReference>());
    isOhmUrl_ = panda::JSNApi::IsOhmUrl(srcEntrance);

    HandleScope handleScope(*this);

    std::string path = moduleName;
    auto pos = path.find("::");
    if (pos != std::string::npos) {
        path.erase(pos, path.size() - pos);
        moduleName_ = path;
    }

    napi_value classValue = nullptr;

    auto it = modules_.find(modulePath);
    if (it != modules_.end()) {
        classValue = it->second->GetNapiValue();
    } else {
        std::string fileName;
        if (!hapPath.empty()) {
            fileName.append(codePath_).append(Constants::FILE_SEPARATOR).append(modulePath);
            std::regex pattern(std::string(Constants::FILE_DOT) + std::string(Constants::FILE_SEPARATOR));
            fileName = std::regex_replace(fileName, pattern, "");
        } else {
            if (!MakeFilePath(codePath_, modulePath, fileName)) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "make module file path: %{private}s failed", modulePath.c_str());
                return std::unique_ptr<NativeReference>();
            }
        }
        classValue = esmodule ? LoadJsModule(fileName, hapPath, srcEntrance)
            : LoadJsBundle(fileName, hapPath, useCommonChunk);
        if (classValue == nullptr) {
            return std::unique_ptr<NativeReference>();
        }

        napi_ref tmpRef = nullptr;
        napi_create_reference(env, classValue, 1, &tmpRef);
        modules_.emplace(modulePath, reinterpret_cast<NativeReference*>(tmpRef));
    }

    napi_value instanceValue = nullptr;
    napi_new_instance(env, classValue, 0, nullptr, &instanceValue);
    if (instanceValue == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null instanceValue");
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    napi_create_reference(env, instanceValue, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModule(
    const std::string& moduleName, const napi_value* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "SystemModule %{public}s", moduleName.c_str());
    napi_env env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, std::unique_ptr<NativeReference>());

    HandleScope handleScope(*this);

    napi_value className = nullptr;
    napi_create_string_utf8(env, moduleName.c_str(), moduleName.length(), &className);
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value refValue = methodRequireNapiRef_->GetNapiValue();
    napi_value args[1] = { className };
    napi_value classValue = nullptr;
    napi_call_function(env, globalObj, refValue, 1, args, &classValue);
    napi_value instanceValue = nullptr;
    napi_new_instance(env, classValue, argc, argv, &instanceValue);
    if (instanceValue == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null instanceValue");
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    napi_create_reference(env, instanceValue, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

napi_value JsRuntime::GetExportObjectFromOhmUrl(const std::string &srcEntrance, const std::string &key)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Get %{private}s export %{private}s", srcEntrance.c_str(), key.c_str());
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, nullptr);
    auto ohmUrl = panda::JSNApi::IsOhmUrl(srcEntrance);
    if (!ohmUrl) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "srcEntrance %{private}s not ohmurl", srcEntrance.c_str());
        return nullptr;
    }

    panda::Local<panda::ObjectRef> exportObj = panda::JSNApi::GetExportObjectFromOhmUrl(vm, srcEntrance, key);
    if (exportObj->IsNull()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get %{private}s export %{private}s failed", srcEntrance.c_str(), key.c_str());
        return nullptr;
    }

    auto env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, nullptr);
    return ArkNativeEngine::ArkValueToNapiValue(env, exportObj);
}

bool JsRuntime::ExecuteSecureWithOhmUrl(const std::string &moduleName, const std::string &hapPath,
    const std::string &srcEntrance)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "moduleName %{public}s, hapPath %{private}s, execute %{private}s",
        moduleName.c_str(), hapPath.c_str(), srcEntrance.c_str());
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);
    auto ohmUrl = panda::JSNApi::IsOhmUrl(srcEntrance);
    if (!ohmUrl) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "srcEntrance %{private}s not ohmurl", srcEntrance.c_str());
        return false;
    }

    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(hapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate, true);
    if (extractor == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "get hapPath %{private}s extractor failed", hapPath.c_str());
        return false;
    }

    std::string srcFileName = BUNDLE_INSTALL_PATH + moduleName + MERGE_ABC_PATH;
    auto safeData = extractor->GetSafeData(srcFileName);
    if (safeData == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null safeData srcFileName %{private}s", srcFileName.c_str());
        return false;
    }

    auto start = std::chrono::high_resolution_clock::now();
    auto ret = panda::JSNApi::ExecuteSecureWithOhmUrl(vm, safeData->GetDataPtr(), safeData->GetDataLen(), srcFileName,
        srcEntrance);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    TAG_LOGI(AAFwkTag::JSRUNTIME, "srcEntrance %{public}s timing %{public}lld", srcEntrance.c_str(), duration_ms);
    return ret;
}

bool JsRuntime::RunScript(const std::string& srcPath, const std::string& hapPath, bool useCommonChunk,
    const std::string& srcEntrance)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);
    HandleScope handleScope(*this);
    std::string commonsPath = std::string(Constants::LOCAL_CODE_PATH) + "/" + moduleName_ + "/ets/commons.abc";
    std::string vendorsPath = std::string(Constants::LOCAL_CODE_PATH) + "/" + moduleName_ + "/ets/vendors.abc";
    if (hapPath.empty()) {
        if (useCommonChunk) {
            (void)LoadScript(commonsPath);
            (void)LoadScript(vendorsPath);
        }
        return LoadScript(srcPath);
    }

    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(hapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate, true);
    if (!extractor) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "hapPath[%{private}s]", hapPath.c_str());
        return false;
    }
    if (newCreate) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "newCreate");
        panda::JSNApi::LoadAotFile(vm, moduleName_);
        auto resourceManager = AbilityBase::ExtractResourceManager::GetExtractResourceManager().GetGlobalObject();
        if (resourceManager) {
            resourceManager->AddResource(loadPath.c_str());
        }
    }

    auto func = [&](std::string modulePath, const std::string abcPath) {
        bool useSafeMempry = apiTargetVersion_ == 0 || apiTargetVersion_ > API8;
        if (!extractor->IsHapCompress(modulePath) && useSafeMempry) {
            auto safeData = extractor->GetSafeData(modulePath);
            if (!safeData) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "null safeData");
                return false;
            }
            return LoadScript(abcPath, safeData->GetDataPtr(), safeData->GetDataLen(), isBundle_, srcEntrance);
        } else {
            std::unique_ptr<uint8_t[]> data;
            size_t dataLen = 0;
            if (!extractor->ExtractToBufByName(modulePath, data, dataLen)) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "get abc file failed");
                return false;
            }
            std::vector<uint8_t> buffer;
            buffer.assign(data.get(), data.get() + dataLen);

            return LoadScript(abcPath, &buffer, isBundle_);
        }
    };

    if (useCommonChunk) {
        (void)func(commonsPath, commonsPath);
        (void)func(vendorsPath, vendorsPath);
    }

    std::string path = srcPath;
    if (!isBundle_) {
        if (moduleName_.empty()) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "moduleName empty");
            return false;
        }
        path = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
        panda::JSNApi::SetAssetPath(vm, path);
        panda::JSNApi::SetModuleName(vm, moduleName_);
    }
    return func(path, srcPath);
}

bool JsRuntime::RunSandboxScript(const std::string& path, const std::string& hapPath)
{
    std::string fileName;
    if (!hapPath.empty()) {
        fileName.append(codePath_).append(Constants::FILE_SEPARATOR).append(path);
        std::regex pattern(std::string(Constants::FILE_DOT) + std::string(Constants::FILE_SEPARATOR));
        fileName = std::regex_replace(fileName, pattern, "");
    } else {
        if (!MakeFilePath(codePath_, path, fileName)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "make module file path: %{private}s failed", path.c_str());
            return false;
        }
    }

    if (!RunScript(fileName, hapPath)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "run script: %{public}s failed", fileName.c_str());
        return false;
    }
    return true;
}

void JsRuntime::PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->PostTask(task, name, delayTime);
}

void JsRuntime::PostSyncTask(const std::function<void()>& task, const std::string& name)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->PostSyncTask(task, name);
}

void JsRuntime::RemoveTask(const std::string& name)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->RemoveTask(name);
}

void JsRuntime::DumpCpuProfile()
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    nativeEngine->DumpCpuProfile();
}

void JsRuntime::DumpHeapSnapshot(bool isPrivate)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    nativeEngine->DumpHeapSnapshot(true, DumpFormat::JSON, isPrivate, false);
}

void JsRuntime::DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary)
{
    DumpHeapSnapshot(tid, isFullGC, isBinary, false);
}

void JsRuntime::DumpHeapSnapshot(uint32_t tid, bool isFullGC, bool isBinary, bool isClearNodeIdCache)
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    panda::ecmascript::DumpSnapShotOption dumpOption;
    if (isBinary) {
        dumpOption.dumpFormat = panda::ecmascript::DumpFormat::BINARY;
    } else {
        dumpOption.dumpFormat = panda::ecmascript::DumpFormat::JSON;
    }
    dumpOption.isVmMode = true;
    dumpOption.isPrivate = false;
    dumpOption.captureNumericValue = true;
    dumpOption.isFullGC = isFullGC;
    dumpOption.isSync = false;
    dumpOption.isClearNodeIdCache = isClearNodeIdCache;
    DFXJSNApi::DumpHeapSnapshot(vm, dumpOption, tid);
}

size_t JsRuntime::GetHeapTotalSize()
{
    auto vm = GetEcmaVm();
    if (!vm) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null vm");
        return 0;
    }
    return DFXJSNApi::GetHeapTotalSize(vm);
}

size_t JsRuntime::GetHeapObjectSize()
{
    auto vm = GetEcmaVm();
    if (!vm) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null vm");
        return 0;
    }
    return DFXJSNApi::GetHeapObjectSize(vm);
}

void JsRuntime::ForceFullGC(uint32_t tid)
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    DFXJSNApi::TriggerGC(vm, tid);
}

void JsRuntime::DestroyHeapProfiler()
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->DestroyHeapProfiler();
}

void JsRuntime::ForceFullGC()
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    panda::JSNApi::TriggerGC(vm, panda::ecmascript::GCReason::TRIGGER_BY_ABILITY,
        panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
}

void JsRuntime::AllowCrossThreadExecution()
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    panda::JSNApi::AllowCrossThreadExecution(vm);
}

void JsRuntime::GetHeapPrepare()
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->GetHeapPrepare();
}

void JsRuntime::NotifyApplicationState(bool isBackground)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    nativeEngine->NotifyApplicationState(isBackground);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "isBackground %{public}d", isBackground);
}

bool JsRuntime::SuspendVM(uint32_t tid)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER_AND_RETURN(nativeEngine, false);
    return nativeEngine->SuspendVMById(tid);
}

void JsRuntime::ResumeVM(uint32_t tid)
{
    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    nativeEngine->ResumeVMById(tid);
}

void JsRuntime::PreloadSystemModule(const std::string& moduleName)
{
    HandleScope handleScope(*this);
    auto env = GetNapiEnv();
    CHECK_POINTER(env);
    napi_value className = nullptr;
    napi_create_string_utf8(env, moduleName.c_str(), moduleName.length(), &className);
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value refValue = methodRequireNapiRef_->GetNapiValue();
    napi_value args[1] = { className };
    napi_call_function(env, globalObj, refValue, 1, args, nullptr);
}

void JsRuntime::PreloadMainAbility(const std::string& moduleName, const std::string& srcPath,
    const std::string& hapPath,  bool isEsMode, const std::string& srcEntrance)
{
    HandleScope handleScope(*this);
    std::string key(moduleName);
    key.append("::");
    key.append(srcPath);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "PreloadMainAbility srcPath: %{public}s", srcPath.c_str());
    preloadList_[key] = LoadModule(moduleName, srcPath, hapPath, isEsMode, false, srcEntrance);
}

void JsRuntime::PreloadModule(const std::string& moduleName, const std::string& srcPath,
    const std::string& hapPath, bool isEsMode, bool useCommonTrunk)
{
    std::string key(moduleName);
    key.append("::");
    key.append(srcPath);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "PreloadModule srcPath: %{public}s", srcPath.c_str());
    preloadList_[key] = LoadModule(moduleName, srcPath, hapPath, isEsMode, useCommonTrunk);
}

bool JsRuntime::PopPreloadObj(const std::string& key, std::unique_ptr<NativeReference>& obj)
{
    if (preloadList_.find(key) == preloadList_.end()) {
        return false;
    }
    if (preloadList_[key] != nullptr) {
        obj = std::move(preloadList_[key]);
        preloadList_.erase(key);
        return true;
    }
    preloadList_.erase(key);
    return false;
}

NativeEngine& JsRuntime::GetNativeEngine() const
{
    return *GetNativeEnginePointer();
}

napi_env JsRuntime::GetNapiEnv() const
{
    return reinterpret_cast<napi_env>(GetNativeEnginePointer());
}

NativeEngine* JsRuntime::GetNativeEnginePointer() const
{
    CHECK_POINTER_AND_RETURN(jsEnv_, nullptr);
    return jsEnv_->GetNativeEngine();
}

panda::ecmascript::EcmaVM* JsRuntime::GetEcmaVm() const
{
    CHECK_POINTER_AND_RETURN(jsEnv_, nullptr);
    return jsEnv_->GetVM();
}

bool JsRuntime::IsUseAbilityRuntime(const Options& options) const
{
    return (options.isStageModel) || (options.isTestFramework);
}

void JsRuntime::UpdateModuleNameAndAssetPath(const std::string& moduleName)
{
    if (isBundle_) {
        return;
    }

    auto vm = GetEcmaVm();
    if (!vm || moduleName.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null vm or moduleName");
        return;
    }

    moduleName_ = moduleName;
    std::string path = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
    panda::JSNApi::SetAssetPath(vm, path);
    panda::JSNApi::SetModuleName(vm, moduleName_);
}

void JsRuntime::RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo& uncaughtExceptionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER(jsEnv_);
    jsEnv_->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
}

void JsRuntime::RegisterUncatchableExceptionHandler(const JsEnv::UncatchableTask& uncatchableTask)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER(jsEnv_);
    jsEnv_->RegisterUncatchableExceptionHandler(uncatchableTask);
}

void JsRuntime::RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    for (auto it = moduleAndPath.begin(); it != moduleAndPath.end(); it++) {
        std::string hqfFile(AbilityBase::GetLoadPath(it->second));
        InitSourceMap(hqfFile);
    }
    panda::JSNApi::RegisterQuickFixQueryFunc(vm, JsQuickfixCallback(moduleAndPath));
}

bool JsRuntime::ReadSourceMapData(const std::string& hapPath, const std::string& sourceMapPath, std::string& content)
{
    // Source map relative path, FA: "/assets/js", Stage: "/ets"
    if (hapPath.empty()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "empty hapPath");
        return false;
    }
    bool newCreate = false;
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(
        ExtractorUtil::GetLoadFilePath(hapPath), newCreate);
    if (extractor == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "hap's path: %{public}s, get extractor failed", hapPath.c_str());
        return false;
    }
    std::unique_ptr<uint8_t[]> dataPtr = nullptr;
    size_t len = 0;
    if (!extractor->ExtractToBufByName(sourceMapPath, dataPtr, len)) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "can't find source map, and switch to stage model");
        std::string tempPath = std::regex_replace(sourceMapPath, std::regex("ets"), "assets/js");
        if (!extractor->ExtractToBufByName(tempPath, dataPtr, len)) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "get mergeSourceMapData fileBuffer failed, map path: %{private}s",
                tempPath.c_str());
            return false;
        }
    }
    content.assign(dataPtr.get(), dataPtr.get() + len);
    return true;
}

void JsRuntime::FreeNativeReference(std::unique_ptr<NativeReference> reference)
{
    FreeNativeReference(std::move(reference), nullptr);
}

void JsRuntime::FreeNativeReference(std::shared_ptr<NativeReference>&& reference)
{
    FreeNativeReference(nullptr, std::move(reference));
}

struct JsNativeReferenceDeleterObject {
    std::unique_ptr<NativeReference> uniqueNativeRef_ = nullptr;
    std::shared_ptr<NativeReference> sharedNativeRef_ = nullptr;
};

void JsRuntime::FreeNativeReference(std::unique_ptr<NativeReference> uniqueNativeRef,
    std::shared_ptr<NativeReference>&& sharedNativeRef)
{
    if (uniqueNativeRef == nullptr && sharedNativeRef == nullptr) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "invalid nativeRef");
        return;
    }

    auto nativeEngine = GetNativeEnginePointer();
    CHECK_POINTER(nativeEngine);
    auto uvLoop = nativeEngine->GetUVLoop();
    CHECK_POINTER(uvLoop);

    auto work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null work");
        return;
    }

    auto cb = new (std::nothrow) JsNativeReferenceDeleterObject();
    if (cb == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null cb");
        delete work;
        work = nullptr;
        return;
    }

    if (uniqueNativeRef != nullptr) {
        cb->uniqueNativeRef_ = std::move(uniqueNativeRef);
    }
    if (sharedNativeRef != nullptr) {
        cb->sharedNativeRef_ = std::move(sharedNativeRef);
    }
    work->data = reinterpret_cast<void*>(cb);
    int ret = uv_queue_work(uvLoop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work != nullptr) {
            if (work->data != nullptr) {
                delete reinterpret_cast<JsNativeReferenceDeleterObject*>(work->data);
                work->data = nullptr;
            }
            delete work;
            work = nullptr;
        }
    });
    if (ret != 0) {
        delete reinterpret_cast<JsNativeReferenceDeleterObject*>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    }
}

void JsRuntime::InitTimerModule()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER(jsEnv_);
    jsEnv_->InitTimerModule();
}

void JsRuntime::InitWorkerModule(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER(jsEnv_);
    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = panda::panda_file::StringPacProtect(options.codePath);
    workerInfo->isDebugVersion = options.isDebugVersion;
    workerInfo->isBundle = options.isBundle;
    workerInfo->packagePathStr = options.packagePathStr;
    workerInfo->assetBasePathStr = options.assetBasePathStr;
    workerInfo->hapPath = panda::panda_file::StringPacProtect(options.hapPath);
    workerInfo->isStageModel = panda::panda_file::BoolPacProtect(options.isStageModel);
    workerInfo->moduleName = options.moduleName;
    workerInfo->apiTargetVersion = panda::panda_file::DataProtect(static_cast<uintptr_t>(options.apiTargetVersion));
    if (options.isJsFramework) {
        SetJsFramework();
    }
    jsEnv_->InitWorkerModule(workerInfo);
}

void JsRuntime::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER(jsEnv_);
    jsEnv_->SetModuleLoadChecker(moduleCheckerDelegate);
}

void JsRuntime::ReInitJsEnvImpl(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER(jsEnv_);
    jsEnv_->ReInitJsEnvImpl(std::make_unique<OHOSJsEnvironmentImpl>(options.eventRunner));
}

void JsRuntime::SetRequestAotCallback()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER(jsEnv_);
    auto callback = [](const std::string& bundleName, const std::string& moduleName, int32_t triggerMode) -> int32_t {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null SaMgr");
            return ERR_INVALID_VALUE;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObj == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null remoteObject");
            return ERR_INVALID_VALUE;
        }

        auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObj);
        if (bundleMgr == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null bms");
            return ERR_INVALID_VALUE;
        }

        TAG_LOGD(AAFwkTag::JSRUNTIME,
            "Reset compile status, bundleName: %{public}s, moduleName: %{public}s, triggerMode: %{public}d",
            bundleName.c_str(), moduleName.c_str(), triggerMode);
        return bundleMgr->ResetAOTCompileStatus(bundleName, moduleName, triggerMode);
    };

    jsEnv_->SetRequestAotCallback(callback);
}

void JsRuntime::SetDeviceDisconnectCallback(const std::function<bool()> &cb)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    CHECK_POINTER(jsEnv_);
    jsEnv_->SetDeviceDisconnectCallback(cb);
}

void JsRuntime::SetStopPreloadSoCallback(const std::function<void()> &callback)
{
    auto vm = GetEcmaVm();
    panda::JSNApi::SetStopPreLoadSoCallback(vm, callback);
}

std::string JsRuntime::GetSystemKitPath()
{
    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(CONFIG_PATH.c_str(), buf, MAX_PATH_LEN);
    if (configPath == nullptr || configPath[0] == '\0' || strlen(configPath) > MAX_PATH_LEN) {
        return SYSTEM_KITS_CONFIG_PATH;
    }
    return configPath;
}

std::vector<panda::HmsMap> JsRuntime::GetSystemKitsMap(uint32_t version)
{
    std::vector<panda::HmsMap> systemKitsMap;
    nlohmann::json jsonBuf;
    std::string configPath = GetSystemKitPath();
    if (configPath == "" || access(configPath.c_str(), F_OK) != 0) {
        return systemKitsMap;
    }

    std::fstream in;
    char errBuf[256];
    errBuf[0] = '\0';
    in.open(configPath, std::ios_base::in);
    if (!in.is_open()) {
        strerror_r(errno, errBuf, sizeof(errBuf));
        return systemKitsMap;
    }

    in.seekg(0, std::ios::end);
    int64_t size = in.tellg();
    if (size <= 0) {
        in.close();
        return systemKitsMap;
    }

    in.seekg(0, std::ios::beg);
    jsonBuf = nlohmann::json::parse(in, nullptr, false);
    in.close();
    if (jsonBuf.is_discarded()) {
        return systemKitsMap;
    }

    if (!jsonBuf.contains(SYSTEM_KITS)) {
        return systemKitsMap;
    }
    for (auto &item : jsonBuf.at(SYSTEM_KITS).items()) {
        nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(NAMESPACE) || !jsonObject.at(NAMESPACE).is_string() ||
            !jsonObject.contains(TARGET_OHM) || !jsonObject.at(TARGET_OHM).is_string() ||
            !jsonObject.contains(SINCE_VERSION) || !jsonObject.at(SINCE_VERSION).is_number()) {
            continue;
        }
        uint32_t sinceVersion = jsonObject.at(SINCE_VERSION).get<uint32_t>();
        panda::HmsMap hmsMap = {
            .originalPath = jsonObject.at(NAMESPACE).get<std::string>(),
            .targetPath = jsonObject.at(TARGET_OHM).get<std::string>(),
            .sinceVersion = sinceVersion
        };
        systemKitsMap.emplace_back(hmsMap);
    }
    TAG_LOGD(AAFwkTag::JSRUNTIME, "The size of the map is %{public}zu", systemKitsMap.size());
    return systemKitsMap;
}

void JsRuntime::SetPkgContextInfoJson(std::string moduleName, std::string hapPath, std::string packageName)
{
    auto iterator = pkgContextInfoJsonStringMap_.find(moduleName);
    if (iterator == pkgContextInfoJsonStringMap_.end()) {
        pkgContextInfoJsonStringMap_[moduleName] = hapPath;
        packageNameList_[moduleName] = packageName;
        auto vm = GetEcmaVm();
        CHECK_POINTER_AND_RETURN(vm,);
        std::unordered_map<std::string, std::pair<std::unique_ptr<uint8_t[]>, size_t>> pkgInfoMap;
        JsRuntimeLite::GetInstance().GetPkgContextInfoListMap(
            pkgContextInfoJsonStringMap_, pkgInfoMap);
        panda::JSNApi::SetPkgContextInfoList(vm, pkgInfoMap);
        panda::JSNApi::SetPkgNameList(vm, packageNameList_);
    }
}

void JsRuntime::UpdatePkgContextInfoJson(const std::string& moduleName, const std::string& hapPath,
    const std::string& packageName)
{
    std::map<std::string, std::string> pkgContextInfoJsonStringMap;
    pkgContextInfoJsonStringMap[moduleName] = hapPath;
    std::map<std::string, std::string> packageNameList;
    packageNameList[moduleName] = packageName;
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm,);
    std::unordered_map<std::string, std::pair<std::unique_ptr<uint8_t[]>, size_t>> pkgInfoMap;
    JsRuntimeLite::GetInstance().GetPkgContextInfoListMap(pkgContextInfoJsonStringMap, pkgInfoMap);
    panda::JSNApi::UpdatePkgContextInfoList(vm, pkgInfoMap);
    panda::JSNApi::UpdatePkgNameList(vm, packageNameList);
}

void JsRuntime::SetDebugOption(const DebugOption debugOption)
{
    debugOption_ = debugOption;
}

void JsRuntime::StartLocalDebugMode(bool isDebugFromLocal)
{
    debugOption_.isDebugFromLocal = isDebugFromLocal;
    StartDebugMode(debugOption_);
}
} // namespace AbilityRuntime
} // namespace OHOS
