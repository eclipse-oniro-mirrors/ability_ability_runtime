/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "simulator.h"

#include <condition_variable>
#include <fstream>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "ability_context.h"
#include "ability_stage_context.h"
#include "bundle_container.h"
#include "console.h"
#include "declarative_module_preloader.h"
#include "hilog_tag_wrapper.h"
#include "js_ability_context.h"
#include "js_ability_stage_context.h"
#include "js_console_log.h"
#include "js_data_converter.h"
#include "js_module_searcher.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_timer.h"
#include "js_window_stage.h"
#include "json_serializer.h"
#include "JsMockUtil.h"
#include "launch_param.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "res_config.h"
#include "resource_manager_helper.h"
#include "resource_manager.h"
#include "window_scene.h"
#include "sys_timer.h"
#include "source_map.h"


namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
constexpr int32_t DEFAULT_ARK_PROPERTIES = -1;
constexpr size_t DEFAULT_GC_THREAD_NUM = 7;
constexpr size_t DEFAULT_LONG_PAUSE_TIME = 40;

constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
constexpr char SOURCE_MAPS_PATH[] = "/ets/sourceMaps.map";
const std::string PACKAGE_NAME = "packageName";
const std::string BUNDLE_NAME = "bundleName";
const std::string MODULE_NAME = "moduleName";
const std::string VERSION = "version";
const std::string ENTRY_PATH = "entryPath";
const std::string IS_SO = "isSO";
const std::string DEPENDENCY_ALIAS = "dependencyAlias";

#if defined(WINDOWS_PLATFORM)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_inspector.dll";
#elif defined(MAC_PLATFORM)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_inspector.dylib";
#else
#error "Unsupported platform"
#endif

int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char *message)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "ArkLog:%{public}s", message);
    return 0;
}

template<typename T, size_t N>
inline constexpr size_t ArraySize(T (&)[N]) noexcept
{
    return N;
}

struct DebuggerTask {
    void OnPostTask(std::function<void()> &&task);

    static void HandleTask(const uv_async_t *req);

    uv_async_t onPostTaskSignal {};
    std::function<void()> func;
};

class SimulatorImpl : public Simulator, public std::enable_shared_from_this<SimulatorImpl> {
public:
    SimulatorImpl() = default;
    ~SimulatorImpl();

    bool Initialize(const Options &options);

    int64_t StartAbility(
        const std::string &abilitySrcPath, TerminateCallback callback, const std::string &abilityName = "") override;
    void TerminateAbility(int64_t abilityId) override;
    void UpdateConfiguration(const AppExecFwk::Configuration &config) override;
    void SetMockList(const std::map<std::string, std::string> &mockList) override;
    void SetHostResolveBufferTracker(ResolveBufferTrackerCallback cb) override;
private:
    bool OnInit();
    void Run();
    napi_value LoadScript(const std::string &srcPath);
    void InitResourceMgr();
    void InitJsAbilityContext(napi_env env, napi_value instanceValue);
    void DispatchStartLifecycle(napi_value instanceValue);
    std::unique_ptr<NativeReference> CreateJsWindowStage(const std::shared_ptr<Rosen::WindowScene> &windowScene);
    napi_value CreateJsWant(napi_env env);
    bool LoadAbilityStage(uint8_t *buffer, size_t len);
    void InitJsAbilityStageContext(napi_value instanceValue);
    napi_value CreateJsLaunchParam(napi_env env);
    bool ParseBundleAndModuleInfo();
    bool ParseAbilityInfo(const std::string &abilitySrcPath, const std::string &abilityName = "");
    bool LoadRuntimeEnv(napi_env env, napi_value globalObject);
    static napi_value RequireNapi(napi_env env, napi_callback_info info);
    inline void SetHostResolveBufferTracker();
    void LoadJsMock(const std::string &fileName);
    void ReportJsError(napi_value obj);
    std::string GetNativeStrFromJsTaggedObj(napi_value obj, const char* key);
    void CreateStageContext();
    std::string ReadSourceMap();

    panda::ecmascript::EcmaVM *CreateJSVM();
    Options options_;
    std::string abilityPath_;
    panda::ecmascript::EcmaVM *vm_ = nullptr;
    DebuggerTask debuggerTask_;
    napi_env nativeEngine_ = nullptr;
    TerminateCallback terminateCallback_;
    bool isOhmUrl_ = false;

    int64_t currentId_ = 0;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> abilities_;
    std::unordered_map<int64_t, std::shared_ptr<Rosen::WindowScene>> windowScenes_;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> jsWindowStages_;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> jsContexts_;
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr_;
    std::shared_ptr<AbilityContext> context_;
    std::shared_ptr<NativeReference> abilityStage_;
    std::shared_ptr<AbilityStageContext> stageContext_;
    std::shared_ptr<NativeReference> jsStageContext_;
    std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo_;
    std::shared_ptr<AppExecFwk::HapModuleInfo> moduleInfo_;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_;
    std::shared_ptr<JsEnv::SourceMap> sourceMapPtr_;
    CallbackTypePostTask postTask_ = nullptr;
    void GetPkgContextInfoListMap(const std::map<std::string, std::string> &contextInfoMap,
        std::map<std::string, std::vector<std::vector<std::string>>> &pkgContextInfoMap,
        std::map<std::string, std::string> &pkgAliasMap);
    void GetPkgContextInfoListInner(nlohmann::json &itemObject, std::vector<std::string> &items,
        std::map<std::string, std::string> &pkgAliasMap, std::string &pkgName);
};

void DebuggerTask::HandleTask(const uv_async_t *req)
{
    auto *debuggerTask = reinterpret_cast<DebuggerTask*>(req->data);
    if (debuggerTask == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null HandleTask debuggerTask");
        return;
    }
    debuggerTask->func();
}

void DebuggerTask::OnPostTask(std::function<void()> &&task)
{
    if (uv_is_active((uv_handle_t*)&onPostTaskSignal)) {
        func = std::move(task);
        onPostTaskSignal.data = static_cast<void*>(this);
        uv_async_send(&onPostTaskSignal);
    }
}

SimulatorImpl::~SimulatorImpl()
{
    if (context_ != nullptr) {
        context_->Unbind();
    }
    if (stageContext_ != nullptr) {
        stageContext_->Unbind();
    }
    if (nativeEngine_) {
        uv_close(reinterpret_cast<uv_handle_t*>(&debuggerTask_.onPostTaskSignal), nullptr);
        uv_loop_t* uvLoop = nullptr;
        napi_get_uv_event_loop(nativeEngine_, &uvLoop);
        if (uvLoop != nullptr) {
            uv_work_t work;
            uv_queue_work(uvLoop, &work, [](uv_work_t*) {}, [](uv_work_t *work, int32_t status) {
                TAG_LOGE(AAFwkTag::ABILITY_SIM, "Simulator stop uv loop");
                uv_stop(work->loop);
            });
        }
    }

    panda::JSNApi::StopDebugger(vm_);

    abilities_.clear();
    nativeEngine_ = nullptr;
    panda::JSNApi::DestroyJSVM(vm_);
    vm_ = nullptr;
}

bool SimulatorImpl::Initialize(const Options &options)
{
    if (nativeEngine_) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "initialized");
        return true;
    }

    options_ = options;
    sourceMapPtr_ = std::make_shared<JsEnv::SourceMap>();
    auto content = ReadSourceMap();
    sourceMapPtr_->SplitSourceMap(content);

    postTask_ = options.postTask;
    if (!OnInit()) {
        return false;
    }

    uv_loop_t* uvLoop = nullptr;
    napi_get_uv_event_loop(nativeEngine_, &uvLoop);
    if (uvLoop == nullptr) {
        return false;
    }

    uv_async_init(uvLoop, &debuggerTask_.onPostTaskSignal,
        reinterpret_cast<uv_async_cb>(DebuggerTask::HandleTask));

    Run();
    return true;
}

void CallObjectMethod(napi_env env, napi_value obj, const char *name, napi_value const *argv, size_t argc)
{
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "get Ability object failed");
        return;
    }
    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "get '%{public}s' failed", name);
        return;
    }
    napi_status status = napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "napi call function failed");
    }
}

napi_value SimulatorImpl::LoadScript(const std::string &srcPath)
{
    panda::Local<panda::ObjectRef> objRef;
    if (isOhmUrl_) {
        objRef = panda::JSNApi::GetExportObjectFromOhmUrl(vm_, srcPath, "default");
    } else {
        objRef = panda::JSNApi::GetExportObject(vm_, srcPath, "default");
    }

    if (objRef->IsNull()) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Get export object failed");
        return nullptr;
    }

    auto obj = ArkNativeEngine::ArkValueToNapiValue(nativeEngine_, objRef);
    napi_value instanceValue = nullptr;
    napi_new_instance(nativeEngine_, obj, 0, nullptr, &instanceValue);
    return instanceValue;
}

bool SimulatorImpl::ParseBundleAndModuleInfo()
{
    AppExecFwk::BundleContainer::GetInstance().LoadBundleInfos(options_.moduleJsonBuffer, options_.resourcePath);
    appInfo_ = AppExecFwk::BundleContainer::GetInstance().GetApplicationInfo();
    if (appInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "appinfo parse failed");
        return false;
    }
    nlohmann::json appInfoJson;
    to_json(appInfoJson, *appInfo_);
    std::cout << "appinfo : " << appInfoJson.dump() << std::endl;

    AppExecFwk::BundleContainer::GetInstance().LoadDependencyHspInfo(appInfo_->bundleName, options_.dependencyHspInfos);
    AppExecFwk::BundleContainer::GetInstance().SetBundleCodeDir(options_.previewPath);

    options_.bundleName = appInfo_->bundleName;
    options_.compatibleVersion = appInfo_->apiCompatibleVersion;
    options_.installationFree = (appInfo_->bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE ? true : false);
    options_.targetVersion = appInfo_->apiTargetVersion;
    options_.releaseType = appInfo_->apiReleaseType;
    options_.compileMode = "esmodule";

    if (appInfo_->moduleInfos.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "module name not exist");
        return false;
    }
    options_.moduleName = appInfo_->moduleInfos[0].moduleName;
    std::cout << "module name is " << options_.moduleName << std::endl;

    moduleInfo_ = AppExecFwk::BundleContainer::GetInstance().GetHapModuleInfo(options_.moduleName);
    if (moduleInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "module info parse failed");
        return false;
    }
    nlohmann::json moduleInfoJson;
    to_json(moduleInfoJson, *moduleInfo_);
    std::cout << "moduleInfo : " << moduleInfoJson.dump() << std::endl;

    options_.pageProfile = moduleInfo_->pages;
    options_.enablePartialUpdate = true;
    for (auto iter : moduleInfo_->metadata) {
        if (iter.name == "ArkTSPartialUpdate" && iter.value == "false") {
            options_.enablePartialUpdate = false;
            break;
        }
    }
    return true;
}

bool SimulatorImpl::ParseAbilityInfo(const std::string &abilitySrcPath, const std::string &abilityName)
{
    if (!abilityName.empty()) {
        abilityInfo_ = AppExecFwk::BundleContainer::GetInstance().GetAbilityInfo(options_.moduleName, abilityName);
    } else {
        auto path = abilitySrcPath;
        path.erase(path.rfind("."));
        auto abilityNameFromPath = path.substr(path.rfind('/') + 1, path.length());
        abilityInfo_ = AppExecFwk::BundleContainer::GetInstance().GetAbilityInfo(
            options_.moduleName, abilityNameFromPath);
    }

    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "ability info parse failed");
        return false;
    }
    nlohmann::json json;
    to_json(json, *abilityInfo_);
    std::cout << "abilityInfo : " << json.dump() << std::endl;

    options_.labelId = abilityInfo_->labelId;
    return true;
}

int64_t SimulatorImpl::StartAbility(
    const std::string &abilitySrcPath, TerminateCallback callback, const std::string &abilityName)
{
    if (!ParseAbilityInfo(abilitySrcPath, abilityName)) {
        return -1;
    }

    CreateStageContext();
    std::ifstream stream(options_.modulePath, std::ios::ate | std::ios::binary);
    if (!stream.is_open()) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "open:%{public}s failed", options_.modulePath.c_str());
        return -1;
    }

    size_t len = stream.tellg();
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(len);
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(buffer.get()), len);
    stream.close();

    auto buf = buffer.release();
    if (!LoadAbilityStage(buf, len)) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Load ability stage failed");
        return -1;
    }

    isOhmUrl_ = panda::JSNApi::IsOhmUrl(abilitySrcPath);
    napi_value instanceValue = nullptr;
    if (isOhmUrl_) {
        std::string srcFilename = "";
        srcFilename = BUNDLE_INSTALL_PATH + options_.moduleName + MERGE_ABC_PATH;
        if (!panda::JSNApi::ExecuteSecureWithOhmUrl(vm_, buf, len, srcFilename, abilitySrcPath)) {
            return -1;
        }
        instanceValue = LoadScript(abilitySrcPath);
    } else {
        abilityPath_ = BUNDLE_INSTALL_PATH + options_.moduleName + "/" + abilitySrcPath;
        if (!reinterpret_cast<NativeEngine*>(nativeEngine_)->RunScriptBuffer(abilityPath_, buf, len, false)) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "run script:%{public}s failed", abilityPath_.c_str());
            return -1;
        }
        instanceValue = LoadScript(abilityPath_);
    }

    if (instanceValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "create object instance failed");
        return -1;
    }

    ++currentId_;
    terminateCallback_ = callback;
    InitResourceMgr();
    InitJsAbilityContext(nativeEngine_, instanceValue);
    DispatchStartLifecycle(instanceValue);
    napi_ref ref = nullptr;
    napi_create_reference(nativeEngine_, instanceValue, 1, &ref);
    abilities_.emplace(currentId_, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    return currentId_;
}

bool SimulatorImpl::LoadAbilityStage(uint8_t *buffer, size_t len)
{
    if (moduleInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null moduleInfo");
        return false;
    }

    if (moduleInfo_->srcEntrance.empty()) {
        TAG_LOGD(AAFwkTag::ABILITY_SIM, "module src path empty");
        return true;
    }

    if (nativeEngine_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null nativeEngine_");
        return false;
    }
    std::string srcEntrance = moduleInfo_->srcEntrance;
    srcEntrance.erase(srcEntrance.rfind("."));
    srcEntrance.append(".abc");
    srcEntrance = srcEntrance.substr(srcEntrance.find('/') + 1, srcEntrance.length());

    auto moduleSrcPath = BUNDLE_INSTALL_PATH + options_.moduleName + "/" + srcEntrance;
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "moduleSrcPath is %{public}s", moduleSrcPath.c_str());
    if (!reinterpret_cast<NativeEngine*>(nativeEngine_)->RunScriptBuffer(moduleSrcPath, buffer, len, false)) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "run ability stage script:%{public}s failed", moduleSrcPath.c_str());
        return false;
    }

    napi_value instanceValue = LoadScript(moduleSrcPath);
    if (instanceValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "create ability stage instance failed");
        return false;
    }

    InitJsAbilityStageContext(instanceValue);
    CallObjectMethod(nativeEngine_, instanceValue, "onCreate", nullptr, 0);

    napi_value wantArgv[] = {
        CreateJsWant(nativeEngine_)
    };
    CallObjectMethod(nativeEngine_, instanceValue, "onAcceptWant", wantArgv, ArraySize(wantArgv));
    napi_ref ref = nullptr;
    napi_create_reference(nativeEngine_, instanceValue, 1, &ref);
    abilityStage_ = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    return true;
}

void SimulatorImpl::InitJsAbilityStageContext(napi_value obj)
{
    napi_value contextObj = CreateJsAbilityStageContext(nativeEngine_, stageContext_);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null contextObj");
        return;
    }

    jsStageContext_ = std::shared_ptr<NativeReference>(
        JsRuntime::LoadSystemModuleByEngine(nativeEngine_, "application.AbilityStageContext", &contextObj, 1));
    if (jsStageContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null get LoadSystemModuleByEngine failed");
        return;
    }

    contextObj = jsStageContext_->GetNapiValue();
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null contextObj");
        return;
    }

    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null obj");
        return;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<AbilityStageContext>(stageContext_);
    auto status = napi_wrap(nativeEngine_, contextObj, workContext,
        [](napi_env, void *data, void*) {
            TAG_LOGD(AAFwkTag::ABILITY_SIM, "finalizer for weak_ptr ui ability context");
            delete static_cast<std::weak_ptr<AbilityStageContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGW(AAFwkTag::ABILITY_SIM, "wrap ability context failed: %{public}d", status);
        delete workContext;
        return;
    }

    JsRuntime jsRuntime;
    stageContext_->Bind(jsRuntime, jsStageContext_.get());
    napi_set_named_property(nativeEngine_, obj, "context", contextObj);
}

void SimulatorImpl::TerminateAbility(int64_t abilityId)
{
    if (abilityId == 0 && abilities_.begin() != abilities_.end()) {
        TerminateAbility(abilities_.begin()->first);
        return;
    }

    auto it = abilities_.find(abilityId);
    if (it == abilities_.end()) {
        return;
    }

    std::shared_ptr<NativeReference> ref = it->second;
    abilities_.erase(it);

    auto instanceValue = ref->GetNapiValue();
    if (instanceValue == nullptr) {
        return;
    }

    CallObjectMethod(nativeEngine_, instanceValue, "onBackground", nullptr, 0);
    CallObjectMethod(nativeEngine_, instanceValue, "onWindowStageDestroy", nullptr, 0);
    CallObjectMethod(nativeEngine_, instanceValue, "onDestroy", nullptr, 0);

    auto windowSceneIter = windowScenes_.find(abilityId);
    if (windowSceneIter != windowScenes_.end()) {
        windowScenes_.erase(windowSceneIter);
    }

    auto windowStageIter = jsWindowStages_.find(abilityId);
    if (windowStageIter != jsWindowStages_.end()) {
        jsWindowStages_.erase(windowStageIter);
    }

    auto jsContextIter = jsContexts_.find(abilityId);
    if (jsContextIter != jsContexts_.end()) {
        jsContexts_.erase(jsContextIter);
    }
}

void SimulatorImpl::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    if (abilityStage_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null abilityStage_");
        return;
    }

    auto configuration = std::make_shared<AppExecFwk::Configuration>(config);
    if (configuration == nullptr) {
        return;
    }

    if (stageContext_) {
        stageContext_->SetConfiguration(configuration);
    }

    napi_value configArgv[] = {
        CreateJsConfiguration(nativeEngine_, config)
    };

    auto abilityStage = abilityStage_->GetNapiValue();
    if (abilityStage == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null abilityStage");
        return;
    }
    CallObjectMethod(nativeEngine_, abilityStage, "onConfigurationUpdated", configArgv, ArraySize(configArgv));
    CallObjectMethod(nativeEngine_, abilityStage, "onConfigurationUpdate", configArgv, ArraySize(configArgv));
    JsAbilityStageContext::ConfigurationUpdated(nativeEngine_, jsStageContext_, configuration);

    for (auto iter = abilities_.begin(); iter != abilities_.end(); iter++) {
        auto ability = iter->second->GetNapiValue();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "null ability");
            continue;
        }

        CallObjectMethod(nativeEngine_, ability, "onConfigurationUpdated", configArgv, ArraySize(configArgv));
        CallObjectMethod(nativeEngine_, ability, "onConfigurationUpdate", configArgv, ArraySize(configArgv));
        JsAbilityContext::ConfigurationUpdated(nativeEngine_, iter->second, configuration);
    }
}

void SimulatorImpl::SetMockList(const std::map<std::string, std::string> &mockList)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called. mockList size: %{public}zu", mockList.size());
    panda::JSNApi::SetMockModuleList(vm_, mockList);
}

void SimulatorImpl::InitResourceMgr()
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    resourceMgr_ = std::shared_ptr<Global::Resource::ResourceManager>(Global::Resource::CreateResourceManager());
    if (resourceMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "resourceMgr");
        return;
    }

    if (!resourceMgr_->AddResource(options_.resourcePath.c_str())) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Add app resource failed");
    }
    ResourceManagerHelper::GetInstance().Init(options_);
    ResourceManagerHelper::GetInstance().AddSystemResource(resourceMgr_);
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null resConfig");
        return;
    }
    ResourceManagerHelper::GetInstance().GetResConfig(*resConfig);
    resourceMgr_->UpdateResConfig(*resConfig);
}

void SimulatorImpl::InitJsAbilityContext(napi_env env, napi_value obj)
{
    if (context_ == nullptr) {
        context_ = std::make_shared<AbilityContext>();
        context_->SetSimulator(static_cast<Simulator*>(this));
        context_->SetOptions(options_);
        context_->SetAbilityStageContext(stageContext_);
        context_->SetResourceManager(resourceMgr_);
        context_->SetAbilityInfo(abilityInfo_);
    }
    napi_value contextObj = CreateJsAbilityContext(nativeEngine_, context_);
    auto systemModule = std::shared_ptr<NativeReference>(
        JsRuntime::LoadSystemModuleByEngine(nativeEngine_, "application.AbilityContext", &contextObj, 1));
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null systemModule");
        return;
    }
    contextObj = systemModule->GetNapiValue();
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null contextObj");
        return;
    }

    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null obj");
        return;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<AbilityContext>(context_);
    auto status = napi_wrap(nativeEngine_, contextObj, workContext,
        [](napi_env, void *data, void*) {
            TAG_LOGD(AAFwkTag::ABILITY_SIM, "finalizer for weak_ptr ui ability context");
            delete static_cast<std::weak_ptr<AbilityContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGW(AAFwkTag::ABILITY_SIM, "wrap ability context failed: %{public}d", status);
        delete workContext;
        return;
    }

    JsRuntime jsRuntime;
    context_->Bind(jsRuntime, systemModule.get());
    napi_set_named_property(env, obj, "context", contextObj);
    jsContexts_.emplace(currentId_, systemModule);
}

napi_value SimulatorImpl::CreateJsWant(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "deviceId", CreateJsValue(env, std::string("")));
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, options_.bundleName));
    if (abilityInfo_) {
        napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, abilityInfo_->name));
    }
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, options_.moduleName));

    napi_set_named_property(env, objValue, "uri", CreateJsValue(env, std::string("")));
    napi_set_named_property(env, objValue, "type", CreateJsValue(env, std::string("")));
    napi_set_named_property(env, objValue, "flags", CreateJsValue(env, 0));
    napi_set_named_property(env, objValue, "type", CreateJsValue(env, std::string("")));
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_set_named_property(env, objValue, "parameters", object);
    napi_value array = nullptr;
    napi_create_array_with_length(env, 0, &array);
    napi_set_named_property(env, objValue, "entities", array);
    return objValue;
}

napi_value SimulatorImpl::CreateJsLaunchParam(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "launchReason", CreateJsValue(env, AAFwk::LAUNCHREASON_UNKNOWN));
    napi_set_named_property(env, objValue, "lastExitReason", CreateJsValue(env, AAFwk::LASTEXITREASON_UNKNOWN));
    napi_set_named_property(env, objValue, "lastExitMessage", CreateJsValue(env, std::string("")));
    return objValue;
}

void SimulatorImpl::DispatchStartLifecycle(napi_value instanceValue)
{
    napi_value wantArgv[] = {
        CreateJsWant(nativeEngine_),
        CreateJsLaunchParam(nativeEngine_)
    };
    CallObjectMethod(nativeEngine_, instanceValue, "onCreate", wantArgv, ArraySize(wantArgv));
    auto windowScene = std::make_shared<Rosen::WindowScene>();
    if (windowScene == nullptr) {
        return;
    }
    sptr<Rosen::IWindowLifeCycle> listener = nullptr;
    windowScene->Init(-1, context_, listener);
    auto jsWindowStage = CreateJsWindowStage(windowScene);
    if (jsWindowStage == nullptr) {
        return;
    }
    napi_value argv[] = { jsWindowStage->GetNapiValue() };
    CallObjectMethod(nativeEngine_, instanceValue, "onWindowStageCreate", argv, ArraySize(argv));

    CallObjectMethod(nativeEngine_, instanceValue, "onForeground", nullptr, 0);

    windowScenes_.emplace(currentId_, windowScene);
    jsWindowStages_.emplace(currentId_, std::shared_ptr<NativeReference>(jsWindowStage.release()));
}

std::unique_ptr<NativeReference> SimulatorImpl::CreateJsWindowStage(
    const std::shared_ptr<Rosen::WindowScene> &windowScene)
{
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(nativeEngine_, windowScene);
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null jsWindowSatge");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(nativeEngine_, "application.WindowStage", &jsWindowStage, 1);
}

panda::ecmascript::EcmaVM *SimulatorImpl::CreateJSVM()
{
    panda::RuntimeOption pandaOption;
    pandaOption.SetArkProperties(DEFAULT_ARK_PROPERTIES);
    pandaOption.SetGcThreadNum(DEFAULT_GC_THREAD_NUM);
    pandaOption.SetLongPauseTime(DEFAULT_LONG_PAUSE_TIME);
    pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
    pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::FOLLOW);
    pandaOption.SetLogBufPrint(PrintVmLog);
    pandaOption.SetEnableAsmInterpreter(true);
    pandaOption.SetAsmOpcodeDisableRange("");
    return panda::JSNApi::CreateJSVM(pandaOption);
}

std::string SimulatorImpl::ReadSourceMap()
{
    std::string normalizedPath = options_.modulePath;
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "modulePath:%{public}s", normalizedPath.c_str());
    std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
    auto sourceMapPath = std::regex_replace(normalizedPath, std::regex(MERGE_ABC_PATH), SOURCE_MAPS_PATH);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "is mac sourceMapPath:%{public}s", sourceMapPath.c_str());

#if defined(WINDOWS_PLATFORM)
    std::replace(sourceMapPath.begin(), sourceMapPath.end(), '/', '\\');
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "is windows sourceMapPath:%{public}s", sourceMapPath.c_str());
#endif
    std::ifstream stream(sourceMapPath, std::ios::ate | std::ios::binary);
    if (!stream.is_open()) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "open:%{public}s failed", sourceMapPath.c_str());
        return "";
    }

    size_t len = stream.tellg();
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(len);
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(buffer.get()), len);
    stream.close();
    std::string content;
    content.assign(reinterpret_cast<char*>(buffer.get()), len);
    return content;
}

bool SimulatorImpl::OnInit()
{
    if (!ParseBundleAndModuleInfo()) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "failed");
        return false;
    }

    vm_ = CreateJSVM();
    if (vm_ == nullptr) {
        return false;
    }

    panda::JSNApi::DebugOption debugOption = {ARK_DEBUGGER_LIB_PATH, (options_.debugPort != 0), options_.debugPort};
    panda::JSNApi::StartDebugger(vm_, debugOption, 0, [this](std::function<void()> &&arg) {
        debuggerTask_.OnPostTask(std::move(arg));
    });

    auto nativeEngine = new (std::nothrow) ArkNativeEngine(vm_, nullptr);
    if (nativeEngine == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null nativeEngine");
        return false;
    }
    napi_env env = reinterpret_cast<napi_env>(nativeEngine);
    auto uncaughtTask = [weak = weak_from_this()](napi_value value) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "uncaught exception");
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "null SimulatorImpl");
            return;
        }
        self->ReportJsError(value);
        if (self->terminateCallback_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "null terminateCallback");
            return;
        }
        self->terminateCallback_(self->currentId_);
    };
    nativeEngine->RegisterNapiUncaughtExceptionHandler(uncaughtTask);
    Ace::DeclarativeModulePreloader::Preload(*nativeEngine);

    napi_value globalObj;
    napi_get_global(env, &globalObj);
    if (globalObj == nullptr) {
        delete nativeEngine;
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "null global object");
        return false;
    }

#if defined(PREVIEW)
    ArkNativeEngine::SetCurrentPreviewenv(options_.enableFileOperation);
#endif

    if (!LoadRuntimeEnv(env, globalObj)) {
        delete nativeEngine;
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Load runtime env failed");
        return false;
    }

    panda::JSNApi::SetBundle(vm_, false);
    panda::JSNApi::SetBundleName(vm_, options_.bundleName);
    panda::JSNApi::SetModuleName(vm_, options_.moduleName);
    panda::JSNApi::SetAssetPath(vm_, options_.modulePath);
    std::map<std::string, std::vector<std::vector<std::string>>> pkgContextInfoMap;
    std::map<std::string, std::string> pkgAliasMap;
    GetPkgContextInfoListMap(options_.pkgContextInfoJsonStringMap, pkgContextInfoMap, pkgAliasMap);
    panda::JSNApi::SetpkgContextInfoList(vm_, pkgContextInfoMap);
    panda::JSNApi::SetPkgAliasList(vm_, pkgAliasMap);
    panda::JSNApi::SetPkgNameList(vm_, options_.packageNameList);

    nativeEngine_ = env;
    return true;
}

napi_value SimulatorImpl::RequireNapi(napi_env env, napi_callback_info info)
{
    napi_value globalObj;
    napi_get_global(env, &globalObj);
    napi_value requireNapi = nullptr;
    napi_get_named_property(env, globalObj, "requireNapiPreview", &requireNapi);
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    napi_value result = nullptr;
    napi_call_function(env, CreateJsUndefined(env), requireNapi, argc, argv, &result);
    if (!CheckTypeForNapiValue(env, result, napi_undefined)) {
        return result;
    }
    napi_value mockRequireNapi = nullptr;
    napi_get_named_property(env, globalObj, "mockRequireNapi", &mockRequireNapi);
    napi_call_function(env, CreateJsUndefined(env), mockRequireNapi, argc, argv, &result);
    return result;
}

void SimulatorImpl::LoadJsMock(const std::string &fileName)
{
    std::ifstream stream(fileName, std::ios::ate | std::ios::binary);
    if (!stream.is_open()) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "open: %{public}s failed", fileName.c_str());
        return;
    }
    size_t len = stream.tellg();
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(len);
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(buffer.get()), len);
    stream.close();
    panda::JSNApi::Execute(vm_, buffer.get(), len, "_GLOBAL::func_main_0");
}

bool SimulatorImpl::LoadRuntimeEnv(napi_env env, napi_value globalObj)
{
    JsSysModule::Console::InitConsoleModule(env);
    auto ret = JsSysModule::Timer::RegisterTime(env);
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Register timer failed");
    }
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_set_named_property(env, globalObj, "group", object);

    const OHOS::Ide::JsMockUtil::AbcInfo info = OHOS::Ide::JsMockUtil::GetAbcBufferInfo();
    const uint8_t *buffer = info.buffer;
    std::size_t size = info.bufferSize;
    panda::JSNApi::Execute(vm_, buffer, size, "_GLOBAL::func_main_0");

    napi_value mockRequireNapi = nullptr;
    napi_get_named_property(env, globalObj, "requireNapi", &mockRequireNapi);
    napi_set_named_property(env, globalObj, "mockRequireNapi", mockRequireNapi);
    auto* moduleManager = reinterpret_cast<NativeEngine*>(env)->GetModuleManager();
    if (moduleManager != nullptr) {
        TAG_LOGE(
            AAFwkTag::ABILITY_SIM, "moduleManager SetPreviewSearchPath: %{public}s", options_.containerSdkPath.c_str());
        moduleManager->SetPreviewSearchPath(options_.containerSdkPath);
    }

    std::string fileSeparator = "/";
    auto pos = options_.containerSdkPath.find(fileSeparator);
    if (pos == std::string::npos) {
        fileSeparator = "\\";
    }

    std::string fileName = options_.containerSdkPath + fileSeparator + "apiMock" + fileSeparator + "jsMockHmos.abc";
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "file name:%{public}s", fileName.c_str());
    if (!fileName.empty() && AbilityStageContext::Access(fileName)) {
        LoadJsMock(fileName);
    }

    const char *moduleName = "SimulatorImpl";
    BindNativeFunction(env, globalObj, "requireNapi", moduleName, SimulatorImpl::RequireNapi);
    return true;
}

void SimulatorImpl::Run()
{
    uv_loop_t* uvLoop = nullptr;
    napi_get_uv_event_loop(nativeEngine_, &uvLoop);
    if (uvLoop != nullptr) {
        uv_run(uvLoop, UV_RUN_NOWAIT);
    }

    if (postTask_ != nullptr) {
        postTask_([this]() { Run(); }, 0);
    }
}
}

std::shared_ptr<Simulator> Simulator::Create(const Options &options)
{
    auto simulator = std::make_shared<SimulatorImpl>();
    if (simulator->Initialize(options)) {
        return simulator;
    }
    return nullptr;
}

void SimulatorImpl::SetHostResolveBufferTracker(ResolveBufferTrackerCallback cb)
{
    if (vm_ == nullptr || cb == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Params invalid");
        return;
    }
    panda::JSNApi::SetHostResolveBufferTracker(vm_, cb);
}

void SimulatorImpl::GetPkgContextInfoListMap(const std::map<std::string, std::string> &contextInfoMap,
    std::map<std::string, std::vector<std::vector<std::string>>> &pkgContextInfoMap,
    std::map<std::string, std::string> &pkgAliasMap)
{
    for (auto it = contextInfoMap.begin(); it != contextInfoMap.end(); it++) {
        std::vector<std::vector<std::string>> pkgContextInfoList;
        auto jsonObject = nlohmann::json::parse(it->second, nullptr, false);
        if (jsonObject.is_discarded()) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "moduleName: %{public}s parse json error", it->first.c_str());
            continue;
        }
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

            GetPkgContextInfoListInner(itemObject, items, pkgAliasMap, pkgName);
            pkgContextInfoList.emplace_back(items);
        }
        TAG_LOGI(AAFwkTag::JSRUNTIME, "moduleName: %{public}s parse json success", it->first.c_str());
        pkgContextInfoMap[it->first] = pkgContextInfoList;
    }
}

void SimulatorImpl::GetPkgContextInfoListInner(nlohmann::json &itemObject, std::vector<std::string> &items,
    std::map<std::string, std::string> &pkgAliasMap, std::string &pkgName)
{
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
}

std::string SimulatorImpl::GetNativeStrFromJsTaggedObj(napi_value obj, const char* key)
{
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "get value failed");
        return "";
    }

    napi_value valueStr = nullptr;
    napi_get_named_property(nativeEngine_, obj, key, &valueStr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(nativeEngine_, valueStr, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "convert value failed");
        return "";
    }

    size_t valueStrBufLength = 0;
    napi_get_value_string_utf8(nativeEngine_, valueStr, nullptr, 0, &valueStrBufLength);
    auto valueCStr = std::make_unique<char[]>(valueStrBufLength + 1);
    size_t valueStrLength = 0;
    napi_get_value_string_utf8(nativeEngine_, valueStr, valueCStr.get(), valueStrBufLength + 1, &valueStrLength);
    std::string ret(valueCStr.get(), valueStrLength);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "GetNativeStrFromJsTaggedObj Success");
    return ret;
}

void SimulatorImpl::ReportJsError(napi_value obj)
{
    std::string errorMsg = GetNativeStrFromJsTaggedObj(obj, "message");
    std::string errorName = GetNativeStrFromJsTaggedObj(obj, "name");
    std::string errorStack = GetNativeStrFromJsTaggedObj(obj, "stack");
    std::string topStack = GetNativeStrFromJsTaggedObj(obj, "topstack");
    std::string summary = "Error name:" + errorName + "\n";
    summary += "Error message:" + errorMsg + "\n";
    bool hasProperty = false;
    napi_has_named_property(nativeEngine_, obj, "code", &hasProperty);
    if (hasProperty) {
        std::string errorCode = GetNativeStrFromJsTaggedObj(obj, "code");
        summary += "Error code:" + errorCode + "\n";
    }
    if (errorStack.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "errorStack empty");
        return;
    }
    auto newErrorStack = sourceMapPtr_->TranslateBySourceMap(errorStack);
    summary += "Stacktrace:\n" + newErrorStack;

    std::stringstream summaryBody(summary);
    std::string line;
    std::string formattedSummary;
    while (std::getline(summaryBody, line)) {
        formattedSummary += "[Simulator Log]" + line + "\n";
    }

    TAG_LOGE(AAFwkTag::ABILITY_SIM, "summary:\n%{public}s", formattedSummary.c_str());
}

void SimulatorImpl::CreateStageContext()
{
    if (stageContext_ == nullptr) {
        stageContext_ = std::make_shared<AbilityStageContext>();
        stageContext_->SetOptions(options_);
        stageContext_->SetConfiguration(options_.configuration);
        stageContext_->SetApplicationInfo(appInfo_);
        stageContext_->SetHapModuleInfo(moduleInfo_);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
