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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_STAGE_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_STAGE_H

#include "ability_delegator_infos.h"

#include <memory>
#include <vector>

#include "ability_stage.h"
#include "configuration.h"
#include "js_startup_task.h"
#include "resource_manager.h"
#include "nlohmann/json.hpp"
#include "native_engine/native_value.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class JsRuntime;
class JsAbilityStage : public AbilityStage {
public:
    static std::shared_ptr<AbilityStage> Create(
        const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo);

    JsAbilityStage(JsRuntime& jsRuntime);
    ~JsAbilityStage() override;

    void Init(const std::shared_ptr<Context> &context,
        const std::weak_ptr<AppExecFwk::OHOSApplication> application) override;

    void LoadModule(const AppExecFwk::HapModuleInfo &hapModuleInfo) override;

    void OnCreate(const AAFwk::Want &want) const override;

    void OnDestroy() const override;

    bool OnPrepareTerminate(
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
        bool &isAsync) const override;

    std::string OnAcceptWant(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsyn) override;

    std::string OnNewProcessRequest(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync) override;

    void OnLaunchFromHyperSnap() override;

    void OnAboutToCreateAbility() override;

    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;

    void OnMemoryLevel(int32_t level) override;

    int32_t RunAutoStartupTask(const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
        bool &isAsyncCallback, const std::shared_ptr<Context> &stageContext, bool preAbilityStageLoad) override;

private:
    napi_value CallObjectMethod(
        const char* name, bool &hasCaughtException, napi_value const * argv = nullptr, size_t argc = 0) const;

    std::shared_ptr<AppExecFwk::DelegatorAbilityStageProperty> CreateStageProperty() const;

    std::string GetHapModuleProp(const std::string &propName) const;

    static bool UseCommonChunk(const AppExecFwk::HapModuleInfo& hapModuleInfo);

    bool LoadJsStartupConfig(const std::pair<std::string, std::string> &configEntry, std::shared_ptr<AAFwk::Want> want,
        const std::string &moduleName, AppExecFwk::ModuleType moduleType);

    bool CallOnPrepareTerminate(napi_env env,
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo) const;

    bool CallOnPrepareTerminateAsync(napi_env env,
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
        bool &isAsync) const;
    
    bool CallAcceptOrRequestSync(napi_env env, const AAFwk::Want &want, std::string &methodName,
        AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo) const;

    bool CallAcceptOrRequestAsync(napi_env env, const AAFwk::Want &want, std::string &methodName,
        AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo,
        bool &isAsync) const;

    int32_t RegisterAppStartupTask(const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo,
        std::shared_ptr<AAFwk::Want> want);
    
    int32_t RegisterJsStartupTask(std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo);

    int32_t RunAutoStartupTaskInner(const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
        bool &isAsyncCallback, const std::string &moduleName, bool preAbilityStageLoad);
    
    void SetShellContextRef(std::shared_ptr<Context> context);

    void SetJsAbilityStage();

    bool isStartupTaskRegistered_ = false;
    JsRuntime& jsRuntime_;
    std::shared_ptr<NativeReference> jsAbilityStageObj_;
    std::shared_ptr<NativeReference> shellContextRef_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_ABILITY_STAGE_H
