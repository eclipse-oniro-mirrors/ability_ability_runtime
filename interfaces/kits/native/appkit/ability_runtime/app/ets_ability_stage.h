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

#ifndef OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_H
#define OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_H

#include <memory>
#include <vector>

#include "ability_delegator_infos.h"
#include "app_startup_task.h"
#include "ability_stage.h"
#include "configuration.h"
#include "ets_native_reference.h"
#include "ets_runtime.h"
#include "resource_manager.h"

namespace OHOS {
namespace AbilityRuntime {
class ETSAbilityStage : public AbilityStage {
public:
    static AbilityStage *Create(
        const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo);

    ETSAbilityStage(ETSRuntime &etsRuntime);
    ~ETSAbilityStage() override;

    void Init(const std::shared_ptr<Context> &context,
        const std::weak_ptr<AppExecFwk::OHOSApplication> application) override;

    void LoadModule(const AppExecFwk::HapModuleInfo &hapModuleInfo) override;

    void OnCreate(const AAFwk::Want &want) const override;

    void OnDestroy() const override;

    std::string OnAcceptWant(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync) override;

    std::string OnNewProcessRequest(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync) override;

    void OnLaunchFromHyperSnap() override;

    void OnAboutToCreateAbility(bool &isAsyncCallback,
        const std::function<void(const std::shared_ptr<AbilityRuntime::Context> &)> &callback) const override;

    static void OnAcceptWantCallback(ani_env *env, ani_object aniObj, ani_string aniResult);

    static void OnNewProcessRequestCallback(ani_env *env, ani_object aniObj, ani_string aniResult);

    void OnConfigurationUpdated(const AppExecFwk::Configuration &configuration) override;

    void OnMemoryLevel(int32_t level) override;

    int32_t RunAutoStartupTask(const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
        bool &isAsyncCallback, const std::shared_ptr<Context> &stageContext, bool preAbilityStageLoad) override;

    bool OnPrepareTerminate(
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
        bool &isAsync) const override;
private:
    bool CallObjectMethod(bool withResult, const char *name, const char *signature, ...) const;

    ani_object CallObjectMethod(const char *name, const char *signature, ...) const;

    std::shared_ptr<AppExecFwk::EtsDelegatorAbilityStageProperty> CreateStageProperty() const;

    std::string GetHapModuleProp(const std::string &propName) const;

    void SetShellContextRef(std::shared_ptr<Context> context);

    void SetEtsAbilityStage();

    int32_t RegisterAppStartupTask(std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo,
        std::shared_ptr<AAFwk::Want> want);
    
    bool LoadEtsStartupConfig(const std::pair<std::string, std::string> &configEntry,
        std::shared_ptr<AAFwk::Want> want, const std::string &moduleName, AppExecFwk::ModuleType moduleType);

    int32_t RegisterEtsStartupTask(std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo);

    int32_t RunAutoStartupTaskInner(const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
        bool &isAsyncCallback, const std::string &moduleName, bool preAbilityStageLoad);
    
    void UpdateStartupTasks(std::map<std::string, std::shared_ptr<StartupTask>> &tasks);

    napi_env GetNapiEnv();

    bool isStartupTaskRegistered_ = false;
    bool CallAcceptOrRequestSync(ani_env *env, const AAFwk::Want &want, std::string &methodName,
        AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo) const;

    bool CallAcceptOrRequestAsync(ani_env *env, const AAFwk::Want &want, std::string &methodName, bool &isAsync) const;

    void SetEtsAbilityStage(const std::shared_ptr<Context> &context);

    bool CallOnPrepareTerminate(AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>
         *callbackInfo) const;

    bool CallOnPrepareTerminateAsync(AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>
        *callbackInfo, bool &isAsync) const;

    bool BindNativeMethods();

    ETSRuntime& etsRuntime_;
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsAbilityStageObj_ = nullptr;
    std::shared_ptr<AppExecFwk::ETSNativeReference> shellContextRef_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_H
