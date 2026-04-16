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

#include "ability_stage.h"

#include "ability_runtime/context/context.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#ifdef CJ_FRONTEND
#include "cj_ability_stage.h"
#endif
#include "js_ability_stage.h"
#include "ets_ability_stage_instance.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<AbilityStage> AbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!runtime) {
        return std::make_shared<AbilityStage>();
    }

    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsAbilityStage::Create(runtime, hapModuleInfo);
#ifdef CJ_FRONTEND
        case Runtime::Language::CJ:
            return CJAbilityStage::Create(runtime, hapModuleInfo);
#endif
        case Runtime::Language::ETS:
            return std::shared_ptr<AbilityStage>(CreateETSAbilityStage(runtime, hapModuleInfo));
        default:
            return std::make_shared<AbilityStage>();
    }
}

void AbilityStage::OnCreate(const AAFwk::Want &want) const
{}

void AbilityStage::OnDestroy() const
{}

bool AbilityStage::OnPrepareTerminate(
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
    bool &isAsync) const
{
    return false;
}

std::shared_ptr<Context> AbilityStage::GetContext() const
{
    return context_;
}

void AbilityStage::Init(const std::shared_ptr<Context>& context,
    const std::weak_ptr<AppExecFwk::OHOSApplication> application)
{
    context_ = context;
    application_ = application;
}

void AbilityStage::LoadModule(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{}

void AbilityStage::AddAbility(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null token");
        return;
    }

    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityRecord");
        return;
    }

    abilityRecords_[token] = abilityRecord;
}

void AbilityStage::RemoveAbility(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null token");
        return;
    }
    abilityRecords_.erase(token);
}

bool AbilityStage::ContainsAbility() const
{
    return !abilityRecords_.empty();
}

std::string AbilityStage::OnAcceptWant(const AAFwk::Want &want)
{
    return "";
}

std::string AbilityStage::OnNewProcessRequest(const AAFwk::Want &want)
{
    return "";
}

std::string AbilityStage::OnAcceptWant(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync)
{
    return "";
}

std::string AbilityStage::OnNewProcessRequest(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync)
{
    return "";
}

void AbilityStage::OnLaunchFromHyperSnap()
{}

void AbilityStage::OnAboutToCreateAbility()
{}

void AbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{}

void AbilityStage::OnMemoryLevel(int level)
{}

int32_t AbilityStage::RunAutoStartupTask(const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
    bool &isAsyncCallback, const std::shared_ptr<Context> &stageContext, bool preAbilityStageLoad)
{
    isAsyncCallback = false;
    return ERR_OK;
}

bool AbilityStage::IsAbilityCreated() const
{
    return isAbilityCreate_;
}

void AbilityStage::MarkAbilityCreated()
{
    isAbilityCreate_ = true;
}

void AbilityStage::SetSkipAbilityStageLifecycle(bool skipAbilityStageLifecycle)
{
    skipAbilityStageLifecycle_ = skipAbilityStageLifecycle;
}

bool AbilityStage::IsSkipAbilityStageLifecycle() const
{
    return skipAbilityStageLifecycle_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
