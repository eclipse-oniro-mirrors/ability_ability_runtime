/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cj_ability_stage_object.h"
#include "cj_ability_stage_context.h"

#include "hilog_tag_wrapper.h"
#include "res_common.h"
#include "securec.h"

using namespace OHOS::AbilityRuntime;

namespace {
// g_cjAbilityStageFuncs is used to save cj functions.
// It is assigned by the global variable REGISTER_ABILITY_STAGE on the cj side which invokes
// RegisterCJAbilityStageFuncs.
CJAbilityStageFuncs g_cjAbilityStageFuncs {};
} // namespace

void RegisterCJAbilityStageFuncs(void (*registerFunc)(CJAbilityStageFuncs* result))
{
    if (g_cjAbilityStageFuncs.LoadAbilityStage != nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Repeated registration for cj functions of CJAbilityStage.");
        return;
    }

    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "RegisterCJAbilityStageFuncs failed, registerFunc is nullptr.");
        return;
    }

    registerFunc(&g_cjAbilityStageFuncs);
}

std::shared_ptr<CJAbilityStageObject> CJAbilityStageObject::LoadModule(const std::string& moduleName)
{
    if (g_cjAbilityStageFuncs.LoadAbilityStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "cj functions for CJAbilityStage.LoadAbilityStage are not registered");
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "CJAbilityStageObject::LoadModule");
    auto handle = g_cjAbilityStageFuncs.LoadAbilityStage(moduleName.c_str());
    if (!handle) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to invoke CJAbilityStageObject::LoadModule. AbilityStage"
            " is not registered: %{public}s.", moduleName.c_str());
        return nullptr;
    }

    return std::make_shared<CJAbilityStageObject>(handle);
}

CJAbilityStageObject::~CJAbilityStageObject()
{
    if (g_cjAbilityStageFuncs.ReleaseAbilityStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "cj functions for CJAbilityStage.Stage are not registered");
    } else {
        g_cjAbilityStageFuncs.ReleaseAbilityStage(id_);
        id_ = 0;
    }
}

void CJAbilityStageObject::Init(AbilityStageHandle abilityStage) const
{
    if (g_cjAbilityStageFuncs.AbilityStageInit == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "cj functions for CJAbilityStage.Init are not registered");
        return;
    }
    g_cjAbilityStageFuncs.AbilityStageInit(id_, abilityStage);
}

void CJAbilityStageObject::OnCreate() const
{
    if (g_cjAbilityStageFuncs.AbilityStageOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "cj functions for CJAbilityStage.OnCreate are not registered");
        return;
    }
    g_cjAbilityStageFuncs.AbilityStageOnCreate(id_);
}

void CJAbilityStageObject::OnDestroy() const
{
    if (g_cjAbilityStageFuncs.AbilityStageOnDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "cj functions for CJAbilityStage.OnDestroy are not registered");
        return;
    }
    g_cjAbilityStageFuncs.AbilityStageOnDestroy(id_);
}

std::string CJAbilityStageObject::OnAcceptWant(const AAFwk::Want& want) const
{
    if (g_cjAbilityStageFuncs.AbilityStageOnAcceptWant == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "cj functions for CJAbilityStage.OnAcceptWant are not registered");
        return "";
    }

    auto wantHandle = const_cast<AAFwk::Want*>(&want);
    auto unsafeStr = g_cjAbilityStageFuncs.AbilityStageOnAcceptWant(id_, wantHandle);
    std::string result = unsafeStr == nullptr ? "" : unsafeStr;
    if (unsafeStr != nullptr) {
        free(static_cast<void*>(unsafeStr));
    }
    return result;
}

std::string CJAbilityStageObject::OnNewProcessRequest(const AAFwk::Want& want) const
{
    if (g_cjAbilityStageFuncs.AbilityStageOnNewProcessRequest == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "cj functions for CJAbilityStage.OnNewProcessRequest are not registered");
        return "";
    }

    auto wantHandle = const_cast<AAFwk::Want*>(&want);
    auto unsafeStr = g_cjAbilityStageFuncs.AbilityStageOnNewProcessRequest(id_, wantHandle);
    std::string result = unsafeStr == nullptr ? "" : unsafeStr;
    if (unsafeStr != nullptr) {
        free(static_cast<void*>(unsafeStr));
    }
    return result;
}

void CJAbilityStageObject::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration>& configuration) const
{
    if (g_cjAbilityStageFuncs.AbilityStageOnConfigurationUpdated2 == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "cj functions for CJAbilityStage.OnConfigurationUpdated2 are not registered");
        return;
    }

    g_cjAbilityStageFuncs.AbilityStageOnConfigurationUpdated2(id_, ConvertConfiguration(*configuration));
}

void CJAbilityStageObject::OnMemoryLevel(int32_t level) const
{
    if (g_cjAbilityStageFuncs.AbilityStageOnMemoryLevel == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "cj functions for CJAbilityStage.OnMemoryLevel are not registered");
        return;
    }
    g_cjAbilityStageFuncs.AbilityStageOnMemoryLevel(id_, level);
}
