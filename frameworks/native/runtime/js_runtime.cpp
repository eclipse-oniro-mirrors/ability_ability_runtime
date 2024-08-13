/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "js_runtime_utils.h"
#include "js_utils.h"
#include "js_worker.h"
#include "module_checker_delegate.h"
#include "napi/native_api.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_create_env.h"
#include "native_engine/native_engine.h"
#include "native_runtime_impl.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"
#include "parameters.h"
#include "extractor.h"
#include "system_ability_definition.h"
#include "systemcapability.h"
#include "source_map.h"
#include "source_map_operator.h"

#ifdef SUPPORT_SCREEN
#include "hot_reloader.h"
#include "ace_forward_compatibility.h"
#include "declarative_module_preloader.h"
#endif //SUPPORT_SCREEN


using namespace OHOS::AbilityBase;
using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t PARAM_TWO = 2;
constexpr uint8_t SYSCAP_MAX_SIZE = 100;
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
constexpr int32_t DEFAULT_INTER_VAL = 500;
constexpr int32_t TRIGGER_GC_AFTER_CLEAR_STAGE_MS = 3000;
constexpr int32_t API8 = 8;
const std::string SANDBOX_ARK_CACHE_PATH = "/data/storage/ark-cache/";
const std::string SANDBOX_ARK_PROIFILE_PATH = "/data/storage/ark-profile";
const std::string DEBUGGER = "@Debugger";

constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
constexpr const char* PERMISSION_RUN_ANY_CODE = "ohos.permission.RUN_ANY_CODE";

const std::string CONFIG_PATH = "/etc/system_kits_config.json";
const std::string SYSTEM_KITS_CONFIG_PATH = "/system/etc/system_kits_config.json";

const std::string SYSTEM_KITS = "systemkits";
const std::string NAMESPACE = "namespace";
const std::string TARGET_OHM = "targetohm";
const std::string SINCE_VERSION = "sinceVersion";

const std::string PACKAGE_NAME = "packageName";
const std::string BUNDLE_NAME = "bundleName";
const std::string MODULE_NAME = "moduleName";
const std::string VERSION = "version";
const std::string ENTRY_PATH = "entryPath";
const std::string IS_SO = "isSO";
constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
const std::string DEPENDENCY_ALIAS = "dependencyAlias";

static auto PermissionCheckFunc = []() {
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();

    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, PERMISSION_RUN_ANY_CODE);
    if (result == Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        return true;
    } else {
        return false;
    }
};

napi_value CanIUse(napi_env env, napi_callback_info info)
{
    if (env == nullptr || info == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null env or info");
        return nullptr;
    }
    napi_value undefined = CreateJsUndefined(env);

    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "invalid argc");
        return undefined;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_string) {
        TAG_LOGI(AAFwkTag::JSRUNTIME, "invalid type");
        return undefined;
    }

    char syscap[SYSCAP_MAX_SIZE] = { 0 };

    size_t strLen = 0;
    napi_get_value_string_utf8(env, argv[0], syscap, sizeof(syscap), &strLen);

    bool ret = HasSystemCapability(syscap);
    return CreateJsValue(env, ret);
}

void InitSyscapModule(napi_env env, napi_value globalObject)
{
    const char *moduleName = "JsRuntime";
    BindNativeFunction(env, globalObject, "canIUse", moduleName, CanIUse);
}

int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
{
    TAG_LOGI(AAFwkTag::JSRUNTIME, "ArkLog: %{public}s", message);
    return 0;
}

napi_status CreateNapiEnv(napi_env *env)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Invalid arg");
        return napi_status::napi_invalid_arg;
    }
    auto options = JsRuntime::GetChildOptions();
    if (options == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null options");
        return napi_status::napi_generic_failure;
    }
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto errCode = NativeRuntimeImpl::GetNativeRuntimeImpl().CreateJsEnv(*options, jsEnv);
    if (errCode != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "CreateJsEnv failed");
        return errCode;
    }
    *env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null env");
        return napi_status::napi_generic_failure;
    }
    return NativeRuntimeImpl::GetNativeRuntimeImpl().Init(*options, *env);
}

napi_status DestroyNapiEnv(napi_env *env)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Invalid arg");
        return napi_status::napi_invalid_arg;
    }
    auto errCode = NativeRuntimeImpl::GetNativeRuntimeImpl().RemoveJsEnv(*env);
    if (errCode == napi_status::napi_ok) {
        *env = nullptr;
    }
    return errCode;
}

} // namespace

std::atomic<bool> JsRuntime::hasInstance(false);
std::shared_ptr<Runtime::Options> JsRuntime::childOptions_ = nullptr;
std::mutex childOptionsMutex_;
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

std::unique_ptr<JsRuntime> JsRuntime::Create(const Options& options)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::unique_ptr<JsRuntime> instance;
    SetChildOptions(options);
    if (!options.preload && options.isStageModel) {
        auto preloadedInstance = Runtime::GetPreloaded();
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
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Developer Mode is false.");
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
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Ark VM is starting debug mode [%{public}s]", isStartWithDebug ? "break" : "normal");
    StartDebuggerInWorkerModule(isDebugApp, dOption.isStartWithNative);
    const std::string bundleName = bundleName_;
    uint32_t instanceId = instanceId_;
    auto weak = jsEnv_;
    std::string inputProcessName = bundleName_ != dOption.processName ? dOption.processName : "";
    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp, [bundleName,
            isStartWithDebug, instanceId, weak, isDebugApp] (int socketFd, std::string option) {
            TAG_LOGI(AAFwkTag::JSRUNTIME, "HdcRegister msg, fd= %{public}d, option= %{public}s",
                socketFd, option.c_str());
        if (weak == nullptr) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "null weak");
            return;
        }
        if (option.find(DEBUGGER) == std::string::npos) {
            if (isDebugApp) {
                ConnectServerManager::Get().StopConnectServer(false);
            }
            ConnectServerManager::Get().SendDebuggerInfo(isStartWithDebug, isDebugApp);
            ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
        } else {
            if (isDebugApp) {
                weak->StopDebugger(option);
            }
            weak->StartDebugger(option, socketFd, isDebugApp);
        }
    });
    if (isDebugApp) {
        ConnectServerManager::Get().StartConnectServer(bundleName_, -1, true);
    }

    DebuggerConnectionHandler(isDebugApp, isStartWithDebug);
}

void JsRuntime::DebuggerConnectionHandler(bool isDebugApp, bool isStartWithDebug)
{
    ConnectServerManager::Get().StoreInstanceMessage(getproctid(), instanceId_);
    EcmaVM* vm = GetEcmaVm();
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
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Developer Mode is false.");
        return;
    }
    CHECK_POINTER(jsEnv_);
    if (JsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
        instanceId_ = static_cast<uint32_t>(getproctid());
    }

    bool isStartWithDebug = dOption.isStartWithDebug;
    bool isDebugApp = dOption.isDebugApp;
    StartDebuggerInWorkerModule(isDebugApp, dOption.isStartWithNative);
    const std::string bundleName = bundleName_;
    auto weak = jsEnv_;
    uint32_t instanceId = instanceId_;
    std::string inputProcessName = bundleName_ != dOption.processName ? dOption.processName : "";
    HdcRegister::Get().StartHdcRegister(bundleName_, inputProcessName, isDebugApp,
        [bundleName, isStartWithDebug, instanceId, weak, isDebugApp](int socketFd, std::string option) {
        TAG_LOGI(AAFwkTag::JSRUNTIME, "HdcRegister msg, fd= %{public}d, option= %{public}s", socketFd, option.c_str());
        if (weak == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
            return;
        }
        if (option.find(DEBUGGER) == std::string::npos) {
            if (isDebugApp) {
                ConnectServerManager::Get().StopConnectServer(false);
            }
            ConnectServerManager::Get().SendDebuggerInfo(isStartWithDebug, isDebugApp);
            ConnectServerManager::Get().StartConnectServer(bundleName, socketFd, false);
        } else {
            if (isDebugApp) {
                weak->StopDebugger(option);
            }
            weak->StartDebugger(option, socketFd, isDebugApp);
        }
    });

    DebuggerConnectionManager(isDebugApp, isStartWithDebug, dOption);
}

void JsRuntime::DebuggerConnectionManager(bool isDebugApp, bool isStartWithDebug, const DebugOption dOption)
{
    if (isDebugApp) {
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
        TAG_LOGW(
            AAFwkTag::JSRUNTIME, "no .abc in hap/hqf %{private}s", filePath.c_str());
        return true;
    }

    std::string fileName = fileNames.front();
    fileFullName = filePath + "/" + fileName;
    std::ostringstream outStream;
    if (!extractor.ExtractByName(fileName, outStream)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Extract %{public}s failed", fileFullName.c_str());
        return false;
    }

    const auto &outStr = outStream.str();
    buffer.assign(outStr.begin(), outStr.end());
    return true;
}

std::shared_ptr<AbilityBase::FileMapper> JsRuntime::GetSafeData(const std::string& path, std::string& fileFullName)
{
    bool newCreate = false;
    auto extractor = ExtractorUtil::GetExtractor(path, newCreate, true);
    if (extractor == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get extractor failed. path: %{private}s", path.c_str());
        return nullptr;
    }

    std::vector<std::string> fileNames;
    extractor->GetSpecifiedTypeFiles(fileNames, ".abc");
    if (fileNames.empty()) {
        TAG_LOGI(AAFwkTag::JSRUNTIME, "There's no abc file in hap or hqf: %{private}s", path.c_str());
        return nullptr;
    }
    std::string fileName = fileNames.front();
    fileFullName = path + "/" + fileName;

    auto safeData = extractor->GetSafeData(fileName);
    if (safeData == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get safe data failed. path: %{private}s", path.c_str());
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

bool JsRuntime::LoadScript(const std::string& path, uint8_t* buffer, size_t len, bool isBundle)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "path: %{private}s", path.c_str());
    CHECK_POINTER_AND_RETURN(jsEnv_, false);
    return jsEnv_->LoadScript(path, buffer, len, isBundle);
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModuleByEngine(
    napi_env env, const std::string& moduleName, const napi_value* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "ModuleName %{public}s", moduleName.c_str());
    if (env == nullptr) {
        TAG_LOGI(AAFwkTag::JSRUNTIME, "invalid engine");
        return nullptr;
    }

    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value propertyValue = nullptr;
    napi_get_named_property(env, globalObj, "requireNapi", &propertyValue);

    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    napi_ref tmpRef = nullptr;
    napi_create_reference(env, propertyValue, 1, &tmpRef);
    methodRequireNapiRef_.reset(reinterpret_cast<NativeReference*>(tmpRef));
    if (!methodRequireNapiRef_) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to create reference for global.requireNapi");
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
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to create object instance");
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
    panda::JSNApi::PostFork(vm, postOption);
    reinterpret_cast<NativeEngine*>(env)->ReinitUVLoop();
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    panda::JSNApi::SetLoop(vm, loop);
}

void JsRuntime::LoadAotFile(const Options& options)
{
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
        NativeCreateEnv::RegCreateNapiEnvCallback(CreateNapiEnv);
        NativeCreateEnv::RegDestroyNapiEnvCallback(DestroyNapiEnv);
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

        if (preloaded_) {
            PostPreload(options);
        }
        HandleScope handleScope(*this);
        napi_value globalObj = nullptr;
        napi_get_global(env, &globalObj);
        CHECK_POINTER_AND_RETURN(globalObj, false);

        if (!preloaded_) {
            InitSyscapModule(env, globalObj);

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
                TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to create reference for global.requireNapi");
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
            std::map<std::string, std::vector<std::vector<std::string>>> pkgContextInfoMap;
            std::map<std::string, std::string> pkgAliasMap;
            pkgContextInfoJsonStringMap_ = options.pkgContextInfoJsonStringMap;
            packageNameList_ = options.packageNameList;
            GetPkgContextInfoListMap(options.pkgContextInfoJsonStringMap, pkgContextInfoMap, pkgAliasMap);
            panda::JSNApi::SetpkgContextInfoList(vm, pkgContextInfoMap);
            panda::JSNApi::SetPkgAliasList(vm, pkgAliasMap);
            panda::JSNApi::SetPkgNameList(vm, options.packageNameList);
        }
    }

    if (!preloaded_) {
        InitConsoleModule();
    }

    if (!options.preload) {
        auto operatorObj = std::make_shared<JsEnv::SourceMapOperator>(options.bundleName, isModular,
                                                                      options.isDebugVersion);
        InitSourceMap(operatorObj);

        if (options.isUnique) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "Not supported TimerModule when form render");
        } else {
            InitTimerModule();
        }

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

void JsRuntime::DoCleanWorkAfterStageCleaned()
{
    // Force gc. If the jsRuntime is destroyed, this task should not be executed.
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    RemoveTask("ability_destruct_gc");
    auto gcTask = [this]() {
        panda::JSNApi::TriggerGC(GetEcmaVm(), panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    };
    PostTask(gcTask, "ability_destruct_gc", TRIGGER_GC_AFTER_CLEAR_STAGE_MS);
}

bool JsRuntime::InitLoop(bool isStage)
{
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

void JsRuntime::InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->InitSourceMap(operatorObj);
    JsEnv::SourceMap::RegisterReadSourceMapCallback(JsRuntime::ReadSourceMapData);
    JsEnv::SourceMap::RegisterGetHapPathCallback(JsModuleReader::GetHapPathList);
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
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value exports = nullptr;
    napi_create_object(env, &exports);
    napi_set_named_property(env, globalObj, "exports", exports);

    if (!RunScript(path, hapPath, useCommonChunk)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to run script: %{private}s", path.c_str());
        return nullptr;
    }

    napi_value exportsObj = nullptr;
    napi_get_named_property(env, globalObj, "exports", &exportsObj);
    if (exportsObj == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to get exports objcect: %{private}s", path.c_str());
        return nullptr;
    }

    napi_value exportObj = nullptr;
    napi_get_named_property(env, exportsObj, "default", &exportObj);
    if (exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to get default objcect: %{private}s", path.c_str());
        return nullptr;
    }

    return exportObj;
}

napi_value JsRuntime::LoadJsModule(const std::string& path, const std::string& hapPath)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!RunScript(path, hapPath, false)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to run script: %{private}s", path.c_str());
        return nullptr;
    }

    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, nullptr);
    panda::Local<panda::ObjectRef> exportObj = panda::JSNApi::GetExportObject(vm, path, "default");
    if (exportObj->IsNull()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get export object failed");
        return nullptr;
    }

    auto env = GetNapiEnv();
    CHECK_POINTER_AND_RETURN(env, nullptr);
    return ArkNativeEngine::ArkValueToNapiValue(env, exportObj);
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(const std::string& moduleName, const std::string& modulePath,
    const std::string& hapPath, bool esmodule, bool useCommonChunk)
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
                TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to make module file path: %{private}s", modulePath.c_str());
                return std::unique_ptr<NativeReference>();
            }
        }
        classValue = esmodule ? LoadJsModule(fileName, hapPath) : LoadJsBundle(fileName, hapPath, useCommonChunk);
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
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to create object instance");
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
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    napi_create_reference(env, instanceValue, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

bool JsRuntime::RunScript(const std::string& srcPath, const std::string& hapPath, bool useCommonChunk)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto vm = GetEcmaVm();
    CHECK_POINTER_AND_RETURN(vm, false);
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
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get extractor failed. hapPath[%{private}s]", hapPath.c_str());
        return false;
    }
    if (newCreate) {
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
                TAG_LOGE(AAFwkTag::JSRUNTIME, "Get safeData abc file failed");
                return false;
            }
            return LoadScript(abcPath, safeData->GetDataPtr(), safeData->GetDataLen(), isBundle_);
        } else {
            std::ostringstream outStream;
            if (!extractor->GetFileBuffer(modulePath, outStream)) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "Get File  Buffer abc file failed");
                return false;
            }
            const auto& outStr = outStream.str();
            std::vector<uint8_t> buffer;
            buffer.assign(outStr.begin(), outStr.end());

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
            TAG_LOGE(AAFwkTag::JSRUNTIME, "moduleName is hole");
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
            TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to make module file path: %{private}s", path.c_str());
            return false;
        }
    }

    if (!RunScript(fileName, hapPath)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Failed to run script: %{public}s", fileName.c_str());
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

void JsRuntime::DumpHeapSnapshot(uint32_t tid, bool isFullGC)
{
    auto vm = GetEcmaVm();
    CHECK_POINTER(vm);
    panda::ecmascript::DumpSnapShotOption dumpOption;
    dumpOption.dumpFormat = panda::ecmascript::DumpFormat::JSON;
    dumpOption.isVmMode = true;
    dumpOption.isPrivate = false;
    dumpOption.captureNumericValue = false;
    dumpOption.isFullGC = isFullGC;
    dumpOption.isSync = false;
    DFXJSNApi::DumpHeapSnapshot(vm, dumpOption, tid);
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
    panda::JSNApi::TriggerGC(vm, panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
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
        TAG_LOGE(AAFwkTag::JSRUNTIME, "vm is nullptr or moduleName is empty");
        return;
    }

    moduleName_ = moduleName;
    std::string path = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
    panda::JSNApi::SetAssetPath(vm, path);
    panda::JSNApi::SetModuleName(vm, moduleName_);
}

void JsRuntime::RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo& uncaughtExceptionInfo)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
}

void JsRuntime::RegisterQuickFixQueryFunc(const std::map<std::string, std::string>& moduleAndPath)
{
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
    CHECK_POINTER(jsEnv_);
    jsEnv_->InitTimerModule();
}

void JsRuntime::InitWorkerModule(const Options& options)
{
    CHECK_POINTER(jsEnv_);
    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = options.codePath;
    workerInfo->isDebugVersion = options.isDebugVersion;
    workerInfo->isBundle = options.isBundle;
    workerInfo->packagePathStr = options.packagePathStr;
    workerInfo->assetBasePathStr = options.assetBasePathStr;
    workerInfo->hapPath = options.hapPath;
    workerInfo->isStageModel = options.isStageModel;
    workerInfo->moduleName = options.moduleName;
    workerInfo->apiTargetVersion = options.apiTargetVersion;
    if (options.isJsFramework) {
        SetJsFramework();
    }
    jsEnv_->InitWorkerModule(workerInfo);
}

void JsRuntime::ReInitJsEnvImpl(const Options& options)
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->ReInitJsEnvImpl(std::make_unique<OHOSJsEnvironmentImpl>(options.eventRunner));
}

void JsRuntime::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate) const
{
    CHECK_POINTER(jsEnv_);
    jsEnv_->SetModuleLoadChecker(moduleCheckerDelegate);
}

void JsRuntime::SetRequestAotCallback()
{
    CHECK_POINTER(jsEnv_);
    auto callback = [](const std::string& bundleName, const std::string& moduleName, int32_t triggerMode) -> int32_t {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "get SaMgr failed");
            return ERR_INVALID_VALUE;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObj == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null remoteObject");
            return ERR_INVALID_VALUE;
        }

        auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObj);
        if (bundleMgr == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "get bms failed");
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
        if (version >= sinceVersion) {
            panda::HmsMap hmsMap = {
                .originalPath = jsonObject.at(NAMESPACE).get<std::string>(),
                .targetPath = jsonObject.at(TARGET_OHM).get<std::string>(),
                .sinceVersion = sinceVersion
            };
            systemKitsMap.emplace_back(hmsMap);
        }
    }
    TAG_LOGD(AAFwkTag::JSRUNTIME, "The size of the map is %{public}zu", systemKitsMap.size());
    return systemKitsMap;
}

void JsRuntime::GetPkgContextInfoListMap(const std::map<std::string, std::string> &contextInfoMap,
    std::map<std::string, std::vector<std::vector<std::string>>> &pkgContextInfoMap,
    std::map<std::string, std::string> &pkgAliasMap)
{
    for (auto it = contextInfoMap.begin(); it != contextInfoMap.end(); it++) {
        std::vector<std::vector<std::string>> pkgContextInfoList;
        std::string filePath = it->second;
        bool newCreate = false;
        std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(
            ExtractorUtil::GetLoadFilePath(filePath), newCreate, false);
        if (!extractor) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "moduleName: %{public}s load hapPath failed", it->first.c_str());
            continue;
        }
        std::ostringstream outStream;
        if (!extractor->ExtractByName("pkgContextInfo.json", outStream)) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "moduleName: %{public}s get pkgContextInfo failed", it->first.c_str());
            continue;
        }
        auto jsonObject = nlohmann::json::parse(outStream.str(), nullptr, false);
        if (jsonObject.is_discarded()) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "moduleName: %{public}s parse json error", it->first.c_str());
            continue;
        }
        for (nlohmann::json::iterator jsonIt = jsonObject.begin(); jsonIt != jsonObject.end(); jsonIt++) {
            std::vector<std::string> items;
            items.emplace_back(jsonIt.key());
            nlohmann::json itemObject = jsonIt.value();
            std::string pkgName = "";
            items.emplace_back(PACKAGE_NAME);
            if (itemObject[PACKAGE_NAME].is_null() || !itemObject[PACKAGE_NAME].is_string()) {
                items.emplace_back(pkgName);
            } else {
                pkgName = itemObject[PACKAGE_NAME].get<std::string>();
                items.emplace_back(pkgName);
            }

            items.emplace_back(BUNDLE_NAME);
            if (itemObject[BUNDLE_NAME].is_null() || !itemObject[BUNDLE_NAME].is_string()) {
                items.emplace_back("");
            } else {
                items.emplace_back(itemObject[BUNDLE_NAME].get<std::string>());
            }

            items.emplace_back(MODULE_NAME);
            if (itemObject[MODULE_NAME].is_null() || !itemObject[MODULE_NAME].is_string()) {
                items.emplace_back("");
            } else {
                items.emplace_back(itemObject[MODULE_NAME].get<std::string>());
            }

            items.emplace_back(VERSION);
            if (itemObject[VERSION].is_null() || !itemObject[VERSION].is_string()) {
                items.emplace_back("");
            } else {
                items.emplace_back(itemObject[VERSION].get<std::string>());
            }

            items.emplace_back(ENTRY_PATH);
            if (itemObject[ENTRY_PATH].is_null() || !itemObject[ENTRY_PATH].is_string()) {
                items.emplace_back("");
            } else {
                items.emplace_back(itemObject[ENTRY_PATH].get<std::string>());
            }

            items.emplace_back(IS_SO);
            if (itemObject[IS_SO].is_null() || !itemObject[IS_SO].is_boolean()) {
                items.emplace_back("false");
            } else {
                bool isSo = itemObject[IS_SO].get<bool>();
                if (isSo) {
                    items.emplace_back("true");
                } else {
                    items.emplace_back("false");
                }
            }
            if (!itemObject[DEPENDENCY_ALIAS].is_null() && itemObject[DEPENDENCY_ALIAS].is_string()) {
                std::string pkgAlias = itemObject[DEPENDENCY_ALIAS].get<std::string>();
                if (!pkgAlias.empty()) {
                    pkgAliasMap[pkgAlias] = pkgName;
                }
            }
            pkgContextInfoList.emplace_back(items);
        }
        TAG_LOGI(AAFwkTag::JSRUNTIME, "moduleName: %{public}s parse json success", it->first.c_str());
        pkgContextInfoMap[it->first] = pkgContextInfoList;
    }
}

void JsRuntime::SetChildOptions(const Options& options)
{
    std::lock_guard<std::mutex> lock(childOptionsMutex_);
    if (childOptions_ == nullptr) {
        childOptions_ = std::make_shared<Options>();
    }
    childOptions_->lang = options.lang;
    childOptions_->bundleName = options.bundleName;
    childOptions_->moduleName = options.moduleName;
    childOptions_->codePath = options.codePath;
    childOptions_->bundleCodeDir = options.bundleCodeDir;
    childOptions_->hapPath = options.hapPath;
    childOptions_->arkNativeFilePath = options.arkNativeFilePath;
    childOptions_->hapModulePath = options.hapModulePath;
    childOptions_->loadAce = options.loadAce;
    childOptions_->preload = options.preload;
    childOptions_->isBundle = options.isBundle;
    childOptions_->isDebugVersion = options.isDebugVersion;
    childOptions_->isJsFramework = options.isJsFramework;
    childOptions_->isStageModel = options.isStageModel;
    childOptions_->isTestFramework = options.isTestFramework;
    childOptions_->uid = options.uid;
    childOptions_->isUnique = options.isUnique;
    childOptions_->moduleCheckerDelegate = options.moduleCheckerDelegate;
    childOptions_->apiTargetVersion = options.apiTargetVersion;
    childOptions_->packagePathStr = options.packagePathStr;
    childOptions_->assetBasePathStr = options.assetBasePathStr;
    childOptions_->jitEnabled = options.jitEnabled;
}

std::shared_ptr<Runtime::Options> JsRuntime::GetChildOptions()
{
    std::lock_guard<std::mutex> lock(childOptionsMutex_);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    return childOptions_;
}

void JsRuntime::UpdatePkgContextInfoJson(std::string moduleName, std::string hapPath, std::string packageName)
{
    auto iterator = pkgContextInfoJsonStringMap_.find(moduleName);
    if (iterator == pkgContextInfoJsonStringMap_.end()) {
        pkgContextInfoJsonStringMap_[moduleName] = hapPath;
        packageNameList_[moduleName] = packageName;
        auto vm = GetEcmaVm();
        std::map<std::string, std::vector<std::vector<std::string>>> pkgContextInfoMap;
        std::map<std::string, std::string> pkgAliasMap;
        GetPkgContextInfoListMap(pkgContextInfoJsonStringMap_, pkgContextInfoMap, pkgAliasMap);
        panda::JSNApi::SetpkgContextInfoList(vm, pkgContextInfoMap);
        panda::JSNApi::SetPkgAliasList(vm, pkgAliasMap);
        panda::JSNApi::SetPkgNameList(vm, packageNameList_);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
