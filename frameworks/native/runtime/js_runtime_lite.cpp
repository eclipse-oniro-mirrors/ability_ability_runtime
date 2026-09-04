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

#include "js_runtime_lite.h"

#include <regex>

#include "bundle_mgr_interface.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "js_environment.h"
#include "js_module_reader.h"
#include "js_runtime_common.h"
#include "js_worker.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"
#include "parameters.h"
#include "system_ability_definition.h"
#include "native_engine/native_create_env.h"
#include "worker_info.h"

using Extractor = OHOS::AbilityBase::Extractor;
using ExtractorUtil = OHOS::AbilityBase::ExtractorUtil;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
const std::string SANDBOX_ARK_PROIFILE_PATH = "/data/storage/ark-profile";
const std::string PACKAGE_NAME = "packageName";
const std::string BUNDLE_NAME = "bundleName";
const std::string MODULE_NAME = "moduleName";
const std::string VERSION = "version";
const std::string ENTRY_PATH = "entryPath";
const std::string IS_SO = "isSO";
const std::string DEPENDENCY_ALIAS = "dependencyAlias";
int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
{
    TAG_LOGI(AAFwkTag::JSRUNTIME, "ArkLog: %{public}s", message);
    return 0;
}
}
JsRuntimeLite::JsRuntimeLite()
{}

JsRuntimeLite::~JsRuntimeLite()
{
    std::lock_guard<std::mutex> lock(envMutex_);
    for (auto it : envMap_) {
        it.second.reset();
        it.second = nullptr;
    }
    envMap_.clear();
    threadIds_.clear();
}

JsRuntimeLite& JsRuntimeLite::GetInstance()
{
    static JsRuntimeLite jsRuntimeLite;
    return jsRuntimeLite;
}

napi_status CreateNapiEnv(napi_env *env)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null env");
        return napi_status::napi_invalid_arg;
    }
    auto options = JsRuntimeLite::GetInstance().GetChildOptions();
    if (options == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null options");
        return napi_status::napi_generic_failure;
    }
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto errCode = JsRuntimeLite::GetInstance().CreateJsEnv(*options, jsEnv);
    if (errCode != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "CreateJsEnv failed");
        return errCode;
    }
    *env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null env");
        return napi_status::napi_generic_failure;
    }
    if (JsRuntimeCommon::GetInstance().IsDebugMode()) {
        auto nativeEngine = jsEnv->GetNativeEngine();
        const std::string threadName = "childThread";
        napi_status errCode = JsRuntimeCommon::GetInstance().StartDebugMode(nativeEngine, threadName);
        if (errCode != napi_status::napi_ok) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "start debug mode failed");
        }
    }
    return JsRuntimeLite::GetInstance().Init(*options, *env);
}

napi_status DestroyNapiEnv(napi_env *env)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "Called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null env");
        return napi_status::napi_invalid_arg;
    }
    if (JsRuntimeCommon::GetInstance().IsDebugMode()) {
        auto jsEnv = JsRuntimeLite::GetInstance().GetJsEnv(*env);
        if (jsEnv == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
            return napi_status::napi_generic_failure;
        }
        auto nativeEngine = jsEnv->GetNativeEngine();
        if (nativeEngine == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null nativeEngine");
            return napi_status::napi_generic_failure;
        }
        napi_status errCode = JsRuntimeCommon::GetInstance().StopDebugMode(nativeEngine);
        if (errCode != napi_status::napi_ok) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "stop debug mode failed");
        }
    }
    auto errCode = JsRuntimeLite::GetInstance().RemoveJsEnv(*env);
    if (errCode == napi_status::napi_ok) {
        *env = nullptr;
    }
    return errCode;
}

void JsRuntimeLite::InitJsRuntimeLite(const Options& options)
{
    if (options.isUnique) {
        return;
    }
    GetInstance().SetChildOptions(options);
    NativeCreateEnv::RegCreateNapiEnvCallback(CreateNapiEnv);
    NativeCreateEnv::RegDestroyNapiEnvCallback(DestroyNapiEnv);
}

napi_status JsRuntimeLite::CreateJsEnv(const Options& options, std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
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
    TAG_LOGI(AAFwkTag::JSRUNTIME, "ark properties = %{public}d bundleName = %{public}s",
        arkProperties, bundleName.c_str());
    pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
    pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::FOLLOW);
    pandaOption.SetLogBufPrint(PrintVmLog);

    bool asmInterpreterEnabled = OHOS::system::GetBoolParameter("persist.ark.asminterpreter", true);
    std::string asmOpcodeDisableRange = OHOS::system::GetParameter("persist.ark.asmopcodedisablerange", "");
    pandaOption.SetEnableAsmInterpreter(asmInterpreterEnabled);
    pandaOption.SetAsmOpcodeDisableRange(asmOpcodeDisableRange);
    pandaOption.SetEnableJIT(options.jitEnabled);

    bool useAbilityRuntime = (options.isStageModel) || (options.isTestFramework);
    if (useAbilityRuntime) {
        bool aotEnabled = OHOS::system::GetBoolParameter("persist.ark.aot", true);
        pandaOption.SetEnableAOT(aotEnabled);
        pandaOption.SetProfileDir(SANDBOX_ARK_PROIFILE_PATH);
    }

    OHOSJsEnvLogger::RegisterJsEnvLogger();
    // options eventRunner is nullptr
    jsEnv = std::make_shared<JsEnv::JsEnvironment>(std::make_unique<OHOSJsEnvironmentImpl>(options.eventRunner));
    if (jsEnv == nullptr || !jsEnv->Initialize(pandaOption, static_cast<void*>(this))
        || jsEnv->GetNativeEngine() == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Initialize js environment failed");
        return napi_status::napi_generic_failure;
    }
    jsEnv->GetNativeEngine()->MarkNativeThread();
    return AddEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()), jsEnv);
}

napi_status JsRuntimeLite::Init(const Options& options, napi_env env)
{
    auto jsEnv = GetJsEnv(env);
    if (jsEnv == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
        return napi_status::napi_generic_failure;
    }

    auto vm = GetEcmaVm(jsEnv);
    if (!vm) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null vm");
        return napi_status::napi_generic_failure;
    }

    bool isModular = false;
    if (!options.preload) {
        LoadAotFile(options, jsEnv);
        panda::JSNApi::SetBundle(vm, options.isBundle);
        panda::JSNApi::SetBundleName(vm, options.bundleName);
        panda::JSNApi::SetHostResolveBufferTracker(
            vm, JsModuleReader(options.bundleName, options.hapPath, options.isUnique));
        isModular = !panda::JSNApi::IsBundle(vm);
        panda::JSNApi::SetSearchHapPathTracker(
            vm, [options](const std::string moduleName, std::string &hapPath) -> bool {
                if (options.hapModulePath.find(moduleName) == options.hapModulePath.end()) {
                    return false;
                }
                hapPath = options.hapModulePath.find(moduleName)->second;
                return true;
            });
        std::unordered_map<std::string, std::pair<std::unique_ptr<uint8_t[]>, size_t>> pkgInfoMap;
        GetPkgContextInfoListMap(options.pkgContextInfoJsonStringMap, pkgInfoMap);
        panda::JSNApi::SetPkgContextInfoList(vm, pkgInfoMap);
        panda::JSNApi::SetPkgNameList(vm, options.packageNameList);
    }

    if (!preloaded_) {
        InitConsoleModule(jsEnv);
    }

    if (!options.preload) {
        if (!options.isUnique) {
            InitTimerModule(jsEnv);
        }
        InitWorkerModule(options, jsEnv);
        SetModuleLoadChecker(options.moduleCheckerDelegate, jsEnv);
        SetRequestAotCallback(jsEnv);

        if (!InitLoop(jsEnv)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "Init loop failed");
            return napi_status::napi_generic_failure;
        }
    }

    preloaded_ = options.preload;
    return napi_status::napi_ok;
}

napi_status JsRuntimeLite::AddEnv(napi_env env, std::shared_ptr<JsEnv::JsEnvironment> jsEnv)
{
    std::lock_guard<std::mutex> lock(envMutex_);
    pid_t threadId = gettid();
    if (threadIds_.find(threadId) != threadIds_.end()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "already created");
        return napi_status::napi_create_ark_runtime_only_one_env_per_thread;
    }
    threadIds_.insert(threadId);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "add threadId %{public}d", threadId);
    auto it = envMap_.find(env);
    if (it == envMap_.end()) {
        envMap_[env] = jsEnv;
        return napi_status::napi_ok;
    }
    return napi_status::napi_generic_failure;
}

napi_status JsRuntimeLite::RemoveJsEnv(napi_env env)
{
    std::lock_guard<std::mutex> lock(envMutex_);
    pid_t threadId = gettid();
    TAG_LOGD(AAFwkTag::JSRUNTIME, "remove threadId %{public}d", threadId);
    threadIds_.erase(threadId);
    auto it = envMap_.find(env);
    if (it != envMap_.end()) {
        it->second.reset();
        it->second = nullptr;
        envMap_.erase(env);
        return napi_status::napi_ok;
    }
    return napi_status::napi_destroy_ark_runtime_env_not_exist;
}

panda::ecmascript::EcmaVM* JsRuntimeLite::GetEcmaVm(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv) const
{
    if (jsEnv == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
        return nullptr;
    }
    return jsEnv->GetVM();
}

std::shared_ptr<JsEnv::JsEnvironment> JsRuntimeLite::GetJsEnv(napi_env env)
{
    std::lock_guard<std::mutex> lock(envMutex_);
    auto jsEnv = envMap_.find(env);
    if (jsEnv != envMap_.end()) {
        return jsEnv->second;
    }
    return nullptr;
}

void JsRuntimeLite::LoadAotFile(const Options& options, const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    auto vm = GetEcmaVm(jsEnv);
    if (!vm || options.hapPath.empty()) {
        return;
    }

    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(options.hapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate, true);
    if (extractor != nullptr && newCreate) {
        panda::JSNApi::LoadAotFile(vm, options.moduleName);
    }
}

void JsRuntimeLite::InitConsoleModule(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
        return;
    }
    jsEnv->InitConsoleModule();
}

void JsRuntimeLite::InitTimerModule(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
        return;
    }
    jsEnv->InitTimerModule();
}

void JsRuntimeLite::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate,
    const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
        return;
    }
    jsEnv->SetModuleLoadChecker(moduleCheckerDelegate);
}

void JsRuntimeLite::SetRequestAotCallback(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
        return;
    }
    auto callback = [](const std::string& bundleName, const std::string& moduleName, int32_t triggerMode) -> int32_t {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null SaMgr");
            return ERR_INVALID_VALUE;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObj == nullptr) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "null remoteObj");
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

    jsEnv->SetRequestAotCallback(callback);
}

bool JsRuntimeLite::InitLoop(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
        return false;
    }
    return jsEnv->InitLoop();
}

void JsRuntimeLite::InitWorkerModule(const Options& options, const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv)
{
    if (jsEnv == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null jsEnv");
        return;
    }

    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = panda::panda_file::StringPacProtect(options.codePath);
    workerInfo->isDebugVersion = options.isDebugVersion;
    workerInfo->isBundle = options.isBundle;
    workerInfo->packagePathStr = options.packagePathStr;
    workerInfo->assetBasePathStr = options.assetBasePathStr;
    workerInfo->hapPath = panda::panda_file::StringPacProtect(options.hapPath);
    workerInfo->isStageModel = panda::panda_file::BoolPacProtect(options.isStageModel);
    workerInfo->moduleName = options.moduleName;
    if (options.isJsFramework) {
        SetJsFramework();
    }
    jsEnv->InitWorkerModule(workerInfo);
}

void JsRuntimeLite::SetChildOptions(const Options& options)
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
    childOptions_->pkgContextInfoJsonStringMap = options.pkgContextInfoJsonStringMap;
    childOptions_->packageNameList = options.packageNameList;
    childOptions_->versionCode = options.versionCode;
}

std::shared_ptr<Options> JsRuntimeLite::GetChildOptions()
{
    std::lock_guard<std::mutex> lock(childOptionsMutex_);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    return childOptions_;
}

void JsRuntimeLite::GetPkgContextInfoListMap(const std::map<std::string, std::string> &contextInfoMap,
    std::unordered_map<std::string, std::pair<std::unique_ptr<uint8_t[]>, size_t>> &pkgInfoMap)
{
    for (auto it = contextInfoMap.begin(); it != contextInfoMap.end(); it++) {
        std::string filePath = it->second;
        bool newCreate = false;
        std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(
            ExtractorUtil::GetLoadFilePath(filePath), newCreate, false);
        if (!extractor) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "moduleName: %{public}s load hapPath failed", it->first.c_str());
            continue;
        }
        std::unique_ptr<uint8_t[]> data;
        size_t dataLen = 0;
        if (!extractor->ExtractToBufByName("pkgContextInfo.json", data, dataLen)) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "moduleName: %{public}s get pkgContextInfo failed", it->first.c_str());
            continue;
        }
        pkgInfoMap[it->first] = std::make_pair(std::move(data), dataLen);
        TAG_LOGD(AAFwkTag::JSRUNTIME, "moduleName: %{public}s get pkgContextInfo success, dataLen: %{public}zu",
            it->first.c_str(), dataLen);
    }
}

void JsRuntimeLite::GetPkgContextInfoListMap(const std::map<std::string, std::string> &contextInfoMap,
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
        std::unique_ptr<uint8_t[]> data;
        size_t dataLen = 0;
        if (!extractor->ExtractToBufByName("pkgContextInfo.json", data, dataLen)) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "moduleName: %{public}s get pkgContextInfo failed", it->first.c_str());
            continue;
        }
        auto jsonObject = nlohmann::json::parse(data.get(), data.get() + dataLen, nullptr, false);
        if (jsonObject.is_discarded()) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "moduleName: %{public}s parse json error", it->first.c_str());
            continue;
        }
        ParsePkgContextInfoJson(jsonObject, pkgContextInfoList, pkgAliasMap);
        TAG_LOGD(AAFwkTag::JSRUNTIME, "moduleName: %{public}s parse json success", it->first.c_str());
        pkgContextInfoMap[it->first] = pkgContextInfoList;
    }
}

void JsRuntimeLite::ParsePkgContextInfoJson(nlohmann::json &jsonObject,
    std::vector<std::vector<std::string>> &pkgContextInfoList, std::map<std::string, std::string> &pkgAliasMap)
{
    for (nlohmann::json::iterator jsonIt = jsonObject.begin(); jsonIt != jsonObject.end(); jsonIt++) {
        std::vector<std::string> items;
        items.emplace_back(jsonIt.key());
        nlohmann::json itemObject = jsonIt.value();
        if (!itemObject.is_object()) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "itemObject is not object");
            return;
        }
        std::string pkgName = "";
        items.emplace_back(PACKAGE_NAME);
        if (itemObject[PACKAGE_NAME].is_null() || !itemObject[PACKAGE_NAME].is_string()) {
            items.emplace_back(pkgName);
        } else {
            pkgName = itemObject[PACKAGE_NAME].get<std::string>();
            items.emplace_back(pkgName);
        }

        ParsePkgContextInfoJsonString(itemObject, BUNDLE_NAME, items);
        ParsePkgContextInfoJsonString(itemObject, MODULE_NAME, items);
        ParsePkgContextInfoJsonString(itemObject, VERSION, items);
        ParsePkgContextInfoJsonString(itemObject, ENTRY_PATH, items);
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
}

void JsRuntimeLite::ParsePkgContextInfoJsonString(
    const nlohmann::json &itemObject, const std::string &key, std::vector<std::string> &items)
{
    items.emplace_back(key);
    if (itemObject[key].is_null() || !itemObject[key].is_string()) {
        items.emplace_back("");
    } else {
        items.emplace_back(itemObject[key].get<std::string>());
    }
}
} // namespace AbilityRuntime
} // namespace OHOS