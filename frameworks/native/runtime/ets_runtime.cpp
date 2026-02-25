/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ets_runtime.h"

#include <algorithm>
#include <cstddef>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <unistd.h>

#include "bundle_constants.h"
#include "constants.h"
#include "ets_interface.h"
#include "file_path_utils.h"
#include "hilog_tag_wrapper.h"
#include "hdc_register.h"
#include "hitrace_meter.h"
#include "hybrid_js_module_reader.h"
#include "nocopyable.h"
#include "parameters.h"
#include "ets/runtime/ets_namespace_manager.h"

#ifdef SUPPORT_SCREEN
#include "ace_forward_compatibility.h"
#include "arkts_module_preloader.h"
#include "declarative_module_preloader.h"
#include "hot_reloader.h"
#endif //SUPPORT_SCREEN

using namespace OHOS::AbilityBase;
using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifdef APP_USE_ARM64
const std::string SANDBOX_LIB_PATH = "/system/lib64";
const std::string ETS_RT_PATH = SANDBOX_LIB_PATH;
const std::string ETS_SYSLIB_PATH =
    "/system/lib64:/system/lib64/platformsdk:/system/lib64/module:/system/lib64/ndk";
#else
const std::string SANDBOX_LIB_PATH = "/system/lib";
const std::string ETS_RT_PATH = SANDBOX_LIB_PATH;
const std::string ETS_SYSLIB_PATH =
    "/system/lib:/system/lib/platformsdk:/system/lib/module:/system/lib/ndk";
#endif
constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
constexpr char SANDBOX_ARK_CACHE_PATH[] = "/data/storage/ark-cache/";
constexpr char SANDBOX_SHARED_BUNDLE_ARK_CACHE_PATH[] =
    "/data/service/el1/public/for-all-app/shared_bundles_ark_cache/";
constexpr char MERGE_ABC_PATH[] = "/ets/modules_static.abc";
const std::string SYS_HSP_FILE_PATH_PREFIX = "/system/app/";

const char *ETS_ENV_LIBNAME = "libets_environment.z.so";
const char *ETS_ENV_REGISTER_FUNCS = "OHOS_ETS_ENV_RegisterFuncs";

ETSEnvFuncs *g_etsEnvFuncs = nullptr;

bool RegisterETSEnvFuncs()
{
    if (g_etsEnvFuncs != nullptr) {
        return true;
    }
    auto handle = dlopen(ETS_ENV_LIBNAME, RTLD_LAZY);
    if (!handle) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "dlopen failed %{public}s, %{public}s", ETS_ENV_LIBNAME, dlerror());
        return false;
    }
    auto symbol = dlsym(handle, ETS_ENV_REGISTER_FUNCS);
    if (!symbol) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "dlsym failed %{public}s, %{public}s", ETS_ENV_REGISTER_FUNCS, dlerror());
        dlclose(handle);
        return false;
    }
    auto func = reinterpret_cast<ETSEnvFuncs* (*)()>(symbol);
    g_etsEnvFuncs = func();
    return true;
}

class EtsAppLibNamespaceMgr : public std::enable_shared_from_this<EtsAppLibNamespaceMgr>, public NoCopyable {
public:
    EtsAppLibNamespaceMgr(const AppLibPathMap& appLibPaths, bool isSystemApp)
        : isSystemApp_(isSystemApp), appLibPathMap_(appLibPaths)
    {
    }

    bool CreateNamespace(const std::string& bundleModuleName, std::string &nsName)
    {
        TAG_LOGD(AAFwkTag::ETSRUNTIME, "Create app ns: %{public}s", bundleModuleName.c_str());
        if (bundleModuleName.empty()) {
            TAG_LOGE(AAFwkTag::ETSRUNTIME, "empty bundleModuleName");
            return false;
        }
        auto appLibPath = appLibPathMap_.find(bundleModuleName);
        if (appLibPath == appLibPathMap_.end()) {
            TAG_LOGE(AAFwkTag::ETSRUNTIME, "not found app lib path: %{public}s", bundleModuleName.c_str());
            return false;
        }

        auto moduleManager = NativeModuleManager::GetInstance();
        if (moduleManager == nullptr) {
            TAG_LOGE(AAFwkTag::ETSRUNTIME, "null moduleManager");
            return false;
        }
        moduleManager->SetAppLibPath(appLibPath->first, appLibPath->second, isSystemApp_);
        return moduleManager->GetLdNamespaceName(appLibPath->first, nsName);
    }

private:
    bool isSystemApp_ = false;
    AppLibPathMap appLibPathMap_;
};
std::shared_ptr<EtsAppLibNamespaceMgr> g_etsAppLibNamespaceMgr;
} // namespace

std::unique_ptr<ETSRuntime> ETSRuntime::PreFork(const Options &options,
    std::unique_ptr<Runtime> &jsRuntime, bool isMove)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "PreFork begin");
    std::unique_ptr<ETSRuntime> instance = std::make_unique<ETSRuntime>();

    if (!instance->Initialize(options, jsRuntime, isMove)) {
        return std::unique_ptr<ETSRuntime>();
    }
    PreloadLibrary();
    return instance;
}

void ETSRuntime::PreloadLibrary()
{
    const char *ETS_ANI_LIBNAME_UI_ABILITY = "libui_ability_ani.z.so";
    const char *ETS_ANI_LIBNAME_ABILITY_STAGE = "libability_stage_ani.z.so";

    auto handleUIAbility = dlopen(ETS_ANI_LIBNAME_UI_ABILITY, RTLD_LAZY);
    if (handleUIAbility == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "dlopen failed %{public}s, %{public}s",
            ETS_ANI_LIBNAME_UI_ABILITY, dlerror());
    }

    auto handleAbilityStage = dlopen(ETS_ANI_LIBNAME_ABILITY_STAGE, RTLD_LAZY);
    if (handleAbilityStage == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "dlopen failed %{public}s, %{public}s",
            ETS_ANI_LIBNAME_ABILITY_STAGE, dlerror());
    }
}

std::string ETSRuntime::GetAotPath(const Options &options)
{
    std::vector<std::string> aotFiles;
    if (!options.arkNativeFilePath.empty()) {
        // Handle Hap and Inner Hsp
        // path: <sandbox_path>/arm64/<moduleName>.an
        for (const auto& status: options.aotCompileStatusMap) {
            if (status.second) {
                aotFiles.push_back(SANDBOX_ARK_CACHE_PATH + options.arkNativeFilePath + status.first + ".an");
            }
        }

        // Handle Outer Hsp
        for (const auto& bundleInfo: options.commonHspBundleInfos) {
            if (bundleInfo.moduleArkTSMode == AppExecFwk::Constants::ARKTS_MODE_DYNAMIC) {
                continue;
            }

            if (bundleInfo.hapPath.compare(0, SYS_HSP_FILE_PATH_PREFIX.size(), SYS_HSP_FILE_PATH_PREFIX) != 0 &&
                bundleInfo.hapPath.rfind('/') == std::string::npos) {
                continue;
            }

            // path: <sandbox_path>/<bundleName>/v<versionCode>/arm64/<moduleName>.an
            std::string outerHspAnPath = SANDBOX_SHARED_BUNDLE_ARK_CACHE_PATH + bundleInfo.bundleName +
                std::string(AbilityBase::Constants::FILE_SEPARATOR) + std::to_string(bundleInfo.versionCode) +
                std::string(AbilityBase::Constants::FILE_SEPARATOR) + options.arkNativeFilePath +
                bundleInfo.moduleName + ".an";
            aotFiles.push_back(outerHspAnPath);
        }
    }

    std::string aotFilePath;
    for (const auto& path: aotFiles) {
        if (!aotFilePath.empty()) {
            aotFilePath += ":";
        }
        aotFilePath += path;
    }
    return aotFilePath;
}

bool ETSRuntime::PostFork(const Options &options, std::unique_ptr<Runtime> &jsRuntime, bool isMove)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "PostFork begin");
    codePath_ = options.codePath;

    if (g_etsEnvFuncs == nullptr ||
        g_etsEnvFuncs->PostFork == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or PostFork");
        return false;
    }

    std::unique_ptr<Runtime> *jsRuntimeValid = &jsRuntime;
    if (isMove) {
        if (jsRuntime != nullptr) {
            jsRuntime_ = std::move(jsRuntime);
        }
        jsRuntimeValid = &jsRuntime_;
    }

    napi_env napiEnv = nullptr;
    if (jsRuntimeValid != nullptr && jsRuntimeValid->get() != nullptr) {
        napiEnv = static_cast<JsRuntime *>(jsRuntimeValid->get())->GetNapiEnv();
        auto vm = static_cast<JsRuntime *>(jsRuntimeValid->get())->GetEcmaVm();
        panda::JSNApi::SetHostResolveBufferTrackerForHybridApp(
            vm, HybridJsModuleReader(options.bundleName, options.hapPath, options.isUnique));
    }

    g_etsEnvFuncs->PostFork(reinterpret_cast<void *>(napiEnv), GetAotPath(options), options.appInnerHspPathList,
        options.commonHspBundleInfos, options.eventRunner);
    return true;
}

std::unique_ptr<ETSRuntime> ETSRuntime::Create(const Options &options,
    std::unique_ptr<Runtime> &jsRuntime, bool isMove)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "Create called");
    if (!RegisterETSEnvFuncs()) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "RegisterETSEnvFuncs failed");
        return std::unique_ptr<ETSRuntime>();
    }

    if (g_etsEnvFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs");
        return std::unique_ptr<ETSRuntime>();
    }

    if (jsRuntime == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null jsRuntime");
        return std::unique_ptr<ETSRuntime>();
    }
    std::unique_ptr<ETSRuntime> instance;
    auto preloadedInstance = Runtime::GetPreloaded(Language::ETS);
#ifdef SUPPORT_SCREEN
    // reload ace if compatible mode changes
    if (Ace::AceForwardCompatibility::PipelineChanged() && preloadedInstance) {
        preloadedInstance.reset();
    }
#endif
    if (preloadedInstance && preloadedInstance->GetLanguage() == Runtime::Language::ETS) {
        instance.reset(static_cast<ETSRuntime *>(preloadedInstance.release()));
    } else {
        instance = PreFork(options, jsRuntime, isMove);
    }

    if (instance != nullptr && !options.preload) {
        instance->PostFork(options, jsRuntime, isMove);
    }
    return instance;
}

void ETSRuntime::SetAppLibPath(const AppLibPathMap& appLibPaths,
    const std::map<std::string, std::string>& abcPathsToBundleModuleNameMap, bool isSystemApp)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "SetAppLibPath called");
    if (!RegisterETSEnvFuncs()) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "RegisterETSEnvFuncs failed");
        return;
    }

    if (g_etsEnvFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs");
        return;
    }

    if (g_etsEnvFuncs->InitETSSDKNS == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null InitETSSDKNS");
        return;
    }
    g_etsEnvFuncs->InitETSSDKNS(ETS_RT_PATH);

    if (g_etsEnvFuncs->InitETSSysNS == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null InitETSSysNS");
        return;
    }
    g_etsEnvFuncs->InitETSSysNS(ETS_SYSLIB_PATH);

    if (g_etsEnvFuncs->SetAppLibPath == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null SetAppLibPath");
        return;
    }
    g_etsAppLibNamespaceMgr = std::make_shared<EtsAppLibNamespaceMgr>(appLibPaths, isSystemApp);
    std::function<bool(const std::string &bundleModuleName, std::string &namespaceName)> cb =
        [weak = std::weak_ptr(g_etsAppLibNamespaceMgr)](const std::string &bundleModuleName, std::string &nsName) {
        auto appLibNamespaceMgr = weak.lock();
        if (appLibNamespaceMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ETSRUNTIME, "null appLibNamespaceMgr");
            return false;
        }
        return appLibNamespaceMgr->CreateNamespace(bundleModuleName, nsName);
    };
    g_etsEnvFuncs->SetAppLibPath(abcPathsToBundleModuleNameMap, cb);
}

void ETSRuntime::SetExtensionApiCheckCallback(
    std::function<bool(const std::string &className, const std::string &fileName)> &cb)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "SetExtensionApiCheckCallback called");
    if (!RegisterETSEnvFuncs()) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "RegisterETSEnvFuncs failed");
        return;
    }

    if (g_etsEnvFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs");
        return;
    }

    if (g_etsEnvFuncs->SetExtensionApiCheckCallback == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null SetExtensionApiCheckCallback");
        return;
    }
    g_etsEnvFuncs->SetExtensionApiCheckCallback(cb);
}

bool ETSRuntime::Initialize(const Options &options, std::unique_ptr<Runtime> &jsRuntime, bool isMove)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "Initialize called");
    if (options.lang != GetLanguage()) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "language mismatch");
        return false;
    }

    std::unique_ptr<Runtime> *jsRuntimeValid = &jsRuntime;
    if (isMove) {
        if (jsRuntime != nullptr) {
            jsRuntime_ = std::move(jsRuntime);
        }
        jsRuntimeValid = &jsRuntime_;
    }
    if (!CreateEtsEnv(options)) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "CreateEtsEnv failed");
        return false;
    }

    apiTargetVersion_ = options.apiTargetVersion;
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "Initialize: %{public}d", apiTargetVersion_);

#ifdef SUPPORT_SCREEN
    auto aniEngine = GetAniEnv();
    if (aniEngine == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "GetAniEnv failed");
        return false;
    }
    OHOS::Ace::ArkTSModulePreloader::Preload(aniEngine);
#endif
    if (!options.preload) {
        napi_env napiEnv = nullptr;
        if (jsRuntimeValid != nullptr && jsRuntimeValid->get() != nullptr) {
            napiEnv = static_cast<JsRuntime *>(jsRuntimeValid->get())->GetNapiEnv();
        }

        if (g_etsEnvFuncs == nullptr ||
            g_etsEnvFuncs->FinishPreload == nullptr) {
            TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or FinishPreload");
            return false;
        }
        g_etsEnvFuncs->FinishPreload(napiEnv);
    }
    return true;
}

void ETSRuntime::FinishPreload()
{
    if (jsRuntime_ != nullptr) {
        jsRuntime_->FinishPreload();
    }

    napi_env napiEnv = nullptr;
    if (jsRuntime_ != nullptr) {
        napiEnv = static_cast<AbilityRuntime::JsRuntime *>(jsRuntime_.get())->GetNapiEnv();
    }

    if (g_etsEnvFuncs == nullptr ||
        g_etsEnvFuncs->FinishPreload == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or FinishPreload");
        return;
    }
    g_etsEnvFuncs->FinishPreload(napiEnv);
}

void ETSRuntime::RegisterUncaughtExceptionHandler(const EtsEnv::ETSUncaughtExceptionInfo &uncaughtExceptionInfo)
{
    if (g_etsEnvFuncs == nullptr ||
        g_etsEnvFuncs->RegisterUncaughtExceptionHandler == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or RegisterUncaughtExceptionHandler");
        return;
    }
    g_etsEnvFuncs->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
}

ETSRuntime::~ETSRuntime()
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "~ETSRuntime called");
    Deinitialize();
    StopDebugMode();
}

void ETSRuntime::Deinitialize()
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "Deinitialize called");
}

bool ETSRuntime::CreateEtsEnv(const Options &options)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "CreateEtsEnv called");
    if (g_etsEnvFuncs == nullptr ||
        g_etsEnvFuncs->Initialize == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or Initialize");
        return false;
    }

    if (!g_etsEnvFuncs->Initialize(options.eventRunner, options.isStartWithDebug)) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "Initialize failed");
        return false;
    }
    return true;
}

ani_env *ETSRuntime::GetAniEnv() const
{
    if (g_etsEnvFuncs == nullptr ||
        g_etsEnvFuncs->GetAniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or GetAniEnv");
        return nullptr;
    }
    return g_etsEnvFuncs->GetAniEnv();
}

void ETSRuntime::PreloadModule(const std::string &moduleName, const std::string &hapPath,
    bool isEsMode, bool useCommonTrunk)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "moduleName: %{public}s", moduleName.c_str());
    if (g_etsEnvFuncs == nullptr ||
        g_etsEnvFuncs->PreloadModule == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or PreloadModule");
        return;
    }

    std::string modulePath = BUNDLE_INSTALL_PATH + moduleName + MERGE_ABC_PATH;
    if (!g_etsEnvFuncs->PreloadModule(modulePath)) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "PreloadModule failed");
    }
    return;
}

std::unique_ptr<AppExecFwk::ETSNativeReference> ETSRuntime::LoadModule(const std::string &moduleName,
    const std::string &modulePath, const std::string &hapPath, bool esmodule, bool useCommonChunk,
    const std::string &srcEntrance)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "LoadModule(%{public}s, %{public}s, %{public}s, %{public}s)",
        moduleName.c_str(), modulePath.c_str(), hapPath.c_str(), srcEntrance.c_str());

    std::string path = moduleName;
    auto pos = path.find("::");
    if (pos != std::string::npos) {
        path.erase(pos, path.size() - pos);
        moduleName_ = path;
    }
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "moduleName_(%{public}s, path %{public}s",
        moduleName_.c_str(), path.c_str());

    std::string fileName;
    if (!hapPath.empty()) {
        fileName.append(codePath_).append(Constants::FILE_SEPARATOR).append(modulePath);
        std::regex pattern(std::string(Constants::FILE_DOT) + std::string(Constants::FILE_SEPARATOR));
        fileName = std::regex_replace(fileName, pattern, "");
    } else {
        if (!MakeFilePath(codePath_, modulePath, fileName)) {
            TAG_LOGE(AAFwkTag::ETSRUNTIME, "make module file path: %{public}s failed", modulePath.c_str());
            return nullptr;
        }
    }
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsNativeReference = LoadEtsModule(moduleName, fileName,
        hapPath, srcEntrance);
    return etsNativeReference;
}

std::unique_ptr<AppExecFwk::ETSNativeReference> ETSRuntime::LoadEtsModule(const std::string &moduleName,
    const std::string &fileName, const std::string &hapPath, const std::string &srcEntrance)
{
    if (g_etsEnvFuncs == nullptr ||
        g_etsEnvFuncs->LoadModule == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or LoadModule");
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }

    std::string modulePath = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
    std::string entryPath = HandleOhmUrlSrcEntry(srcEntrance);
    void *cls = nullptr;
    void *obj = nullptr;
    void *ref = nullptr;
    if (!g_etsEnvFuncs->LoadModule(modulePath, entryPath, cls, obj, ref)) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "LoadModule failed");
        return std::unique_ptr<AppExecFwk::ETSNativeReference>();
    }
    auto etsNativeReference = std::make_unique<AppExecFwk::ETSNativeReference>();
    etsNativeReference->aniCls = reinterpret_cast<ani_class>(cls);
    etsNativeReference->aniObj = reinterpret_cast<ani_object>(obj);
    etsNativeReference->aniRef = reinterpret_cast<ani_ref>(ref);
    return etsNativeReference;
}

bool ETSRuntime::HandleUncaughtError()
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "HandleUncaughtError called");
    if (g_etsEnvFuncs == nullptr ||
        g_etsEnvFuncs->HandleUncaughtError == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or HandleUncaughtError");
        return false;
    }
    g_etsEnvFuncs->HandleUncaughtError();
    return true;
}

const std::unique_ptr<AbilityRuntime::Runtime> &ETSRuntime::GetJsRuntime() const
{
    return jsRuntime_;
}

std::unique_ptr<AbilityRuntime::Runtime> ETSRuntime::MoveJsRuntime()
{
    return std::move(jsRuntime_);
}

void ETSRuntime::PreloadSystemModule(const std::string &moduleName)
{
    if (jsRuntime_ != nullptr) {
        jsRuntime_->PreloadSystemModule(moduleName);
    }
}

bool ETSRuntime::PreloadSystemClass(const char *className)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "PreloadSystemClass called");
    if (g_etsEnvFuncs == nullptr ||
        g_etsEnvFuncs->PreloadSystemClass == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or PreloadSystemClass");
        return false;
    }
    g_etsEnvFuncs->PreloadSystemClass(className);
    return true;
}

void ETSRuntime::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate) const
{
    if (jsRuntime_ != nullptr) {
        jsRuntime_->SetModuleLoadChecker(moduleCheckerDelegate);
    }
}

static void HandleOhmUrlBase(std::string &base)
{
    // <dir1>/<dir2>/  =>  <dir1>.<dir2>.
    std::replace(base.begin(), base.end(), '/', '.');
}

std::string ETSRuntime::HandleOhmUrlSrcEntry(const std::string &srcEntry)
{
    size_t lastSlashPos = srcEntry.rfind('/');
    if (lastSlashPos == std::string::npos) {
        std::string fileName = srcEntry;
        // If there is no slash, the entire string is processed directly.
        HandleOhmUrlFileName(fileName);
        return fileName;
    }
    std::string base = srcEntry.substr(0, lastSlashPos + 1);
    HandleOhmUrlBase(base);
    std::string fileName = srcEntry.substr(lastSlashPos + 1);
    HandleOhmUrlFileName(fileName);
    return base + fileName;
}

void ETSRuntime::HandleOhmUrlFileName(std::string &fileName)
{
    size_t colonPos = fileName.rfind(':');
    if (colonPos != std::string::npos) {
        // <fileName>:<className>  =>  <fileName>.<className>
        fileName.replace(colonPos, 1, ".");
    } else {
        // <fileName>  =>  <fileName>.<fileName>
        fileName = fileName + "." + fileName;
    }
}

void ETSRuntime::StartDebugMode(const DebugOption dOption)
{
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "localDebug %{public}d", dOption.isDebugFromLocal);
    if (!dOption.isDebugFromLocal && !dOption.isDeveloperMode) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "developer Mode false");
        return;
    }
    // Set instance id to tid after the first instance.
    instanceId_ = static_cast<uint32_t>(getproctid());

    bool isStartWithDebug = dOption.isStartWithDebug;
    bool isDebugApp = dOption.isDebugApp;
    std::string appProvisionType = dOption.appProvisionType;
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "Ark VM is starting debug mode [%{public}s]", isStartWithDebug ? "break" : "normal");
    const std::string bundleName = dOption.bundleName;
    uint32_t instanceId = instanceId_;
    std::string inputProcessName = bundleName != dOption.processName ? dOption.processName : "";
    HdcRegister::DebugRegisterMode debugMode = HdcRegister::DebugRegisterMode::HDC_DEBUG_REG;
    HdcRegister::Get().StartHdcRegister(bundleName, inputProcessName, isDebugApp, debugMode,
        [bundleName, isStartWithDebug, instanceId, isDebugApp, appProvisionType]
        (int socketFd, std::string option) {
        TAG_LOGI(AAFwkTag::ETSRUNTIME, "HdcRegister msg, fd= %{public}d, option= %{public}s", socketFd, option.c_str());
        // system is debuggable when const.secure is false and const.debuggable is true
        bool isSystemDebuggable = system::GetBoolParameter("const.secure", true) == false &&
            system::GetBoolParameter("const.debuggable", false) == true;
        // Don't start any server if (system not in debuggable mode) and app is release version
        // Starting ConnectServer in release app on debuggable system is only for debug mode, not for profiling mode.
        if ((!isSystemDebuggable) && appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
            TAG_LOGE(AAFwkTag::ETSRUNTIME, "not support release app");
            return;
        }
        if (option.find(DEBUGGER) == std::string::npos) {
            // if has old connect server, stop it
            if (g_etsEnvFuncs == nullptr || g_etsEnvFuncs->BroadcastAndConnect == nullptr) {
                TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or BroadcastAndConnect");
                return;
            }
            g_etsEnvFuncs->BroadcastAndConnect(bundleName, socketFd);
        } else {
            if (appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE) {
                TAG_LOGE(AAFwkTag::ETSRUNTIME, "not support release app");
                return;
            }
            if (g_etsEnvFuncs == nullptr || g_etsEnvFuncs->StartDebuggerForSocketPair == nullptr) {
                TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or StartDebuggerForSocketPair");
                return;
            }
            g_etsEnvFuncs->StartDebuggerForSocketPair(option, socketFd);
        }
    });
    DebuggerConnectionHandler(isDebugApp, isStartWithDebug);
}

void ETSRuntime::DebuggerConnectionHandler(bool isDebugApp, bool isStartWithDebug)
{
    auto dTask = nullptr;
    if (jsRuntime_ == nullptr) {
        TAG_LOGD(AAFwkTag::ETSRUNTIME, "jsRuntime_ is nullptr");
        return;
    }
    if (g_etsEnvFuncs == nullptr || g_etsEnvFuncs->NotifyDebugMode == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs or NotifyDebugMode");
        return;
    }
    if (jsRuntime_ == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null js runtime");
        return;
    }
    auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime_));
    auto vm = jsRuntimePoint.GetEcmaVm();
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null js vm");
        return;
    }
    g_etsEnvFuncs->NotifyDebugMode(getproctid(), instanceId_, isStartWithDebug, vm);
}

void ETSRuntime::StopDebugMode()
{
    if (g_etsEnvFuncs == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null g_etsEnvFuncs");
        return;
    }
    if (g_etsEnvFuncs->RemoveInstance == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null RemoveInstance");
        return;
    }
    g_etsEnvFuncs->RemoveInstance(instanceId_);
    if (g_etsEnvFuncs->StopDebugMode == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null StopDebugMode");
        return;
    }
    if (jsRuntime_ == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null js runtime");
        return;
    }
    auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime_));
    auto vm = jsRuntimePoint.GetEcmaVm();
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "null js vm");
        return;
    }
    g_etsEnvFuncs->StopDebugMode(vm);
}

void ETSRuntime::SetJsRuntime(std::unique_ptr<AbilityRuntime::Runtime> &jsRuntime)
{
    jsRuntime_ = std::move(jsRuntime);
}
} // namespace AbilityRuntime
} // namespace OHOS
