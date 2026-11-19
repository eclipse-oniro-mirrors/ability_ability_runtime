/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_OBJECT_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_OBJECT_H

#include <memory>

#include "configuration.h"
#include "want.h"
#include "cj_utils_ffi.h"

#ifdef WINDOWS_PLATFORM
#define CJ_EXPORT __declspec(dllexport)
#else
#define CJ_EXPORT __attribute__((visibility("default")))
#endif

using AbilityStageHandle = void*;

extern "C" {
struct CJConfiguration {
    int32_t colorMode;
    int32_t direction;
    int32_t displayId;
    int32_t screenDensity;
    bool isValid;
    const char* language;
};

struct CJAbilityStageFuncs {
    int64_t (*LoadAbilityStage)(const char* moduleName);
    void (*ReleaseAbilityStage)(int64_t handle);
    void (*AbilityStageOnCreate)(int64_t handle);
    char* (*AbilityStageOnAcceptWant)(int64_t handle, OHOS::AAFwk::Want* want);
    void (*AbilityStageOnConfigurationUpdated)(int64_t id, CJConfiguration configuration);
    void (*AbilityStageOnMemoryLevel)(int64_t id, int32_t level);
    void (*AbilityStageInit)(int64_t id, void* abilityStage);
    void (*AbilityStageOnConfigurationUpdated2)(int64_t id, OHOS::AbilityRuntime::CConfiguration configuration);
    void (*AbilityStageOnDestroy)(int64_t id);
    char* (*AbilityStageOnNewProcessRequest)(int64_t id, OHOS::AAFwk::Want* want);
};

struct CJAbilityStageFuncsV3 {
    void (*AbilityStageOnConfigurationUpdatedV2)(int64_t id, OHOS::AbilityRuntime::CConfigurationV2 configuration);
};

CJ_EXPORT void RegisterCJAbilityStageFuncs(void (*registerFunc)(CJAbilityStageFuncs* result));
CJ_EXPORT void RegisterCJAbilityStageFuncsV3(void (*registerFunc)(CJAbilityStageFuncsV3* result));
}

namespace OHOS {
namespace AbilityRuntime {
class CJAbilityStageObject {
public:
    static std::shared_ptr<CJAbilityStageObject> LoadModule(const std::string& moduleName);

    explicit CJAbilityStageObject(int64_t id) : id_(id) {}
    ~CJAbilityStageObject();

    void Init(AbilityStageHandle abilityStage) const;
    void OnCreate() const;
    void OnDestroy() const;
    std::string OnAcceptWant(const AAFwk::Want& want) const;
    std::string OnNewProcessRequest(const AAFwk::Want& want) const;
    void OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration>& configuration) const;
    void OnMemoryLevel(int32_t level) const;
    int64_t GetId() const { return id_; }

private:
    int64_t id_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_OBJECT_H
