/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_START_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_START_ABILITY_CONTEXT_H

#include <memory>

#include "ability_info.h"

namespace OHOS {
namespace AAFwk {
struct StartAbilityInfo;  // forward declaration (full type in start_ability_utils.h)

// Centralized ERMS constant (replaces the anonymous-namespace duplicates in
// start_ability_utils.cpp, ecological_rule_interceptor.cpp and dialog_session_manager.cpp).
inline constexpr int32_t ERMS_ISALLOW_RESULTCODE = 10;

// Request-scoped context replacing the former StartAbilityUtils thread_local state.
// One instance per start request; passed by reference through the start flow.
//
// CONTRACT (copy + SyncFrom):
// Interceptors do NOT hold this context by reference. InterceptorParamBuilder::StartAbilityCtx()
// copies it into AbilityInterceptorParam, interceptors mutate the copy, and the caller merges the
// copy back with SyncFrom() right after DoProcess(). Therefore:
//   - SyncFrom() below is the EXHAUSTIVE list of fields an interceptor may modify.
//     Any write to a field NOT listed there (e.g. startAbilityInfo, callerAbilityInfo,
//     isWantWithAppCloneIndex, isSandBoxClone) happens on the copy only and is SILENTLY LOST.
//     Those fields are read-only for interceptors by design.
//   - If a new interceptor needs to mutate a field of this struct, the field MUST be added
//     to SyncFrom() in the same change, otherwise the mutation never reaches the caller.
struct StartAbilityContext {
    // --- interceptor-writable (synced back by SyncFrom) ---
    bool skipCrowTest = false;               // consumed by CrowdTestInterceptor
    bool skipErms = false;                   // consumed by EcologicalRuleInterceptor::NoNeedErms
    bool ermsSupportBackToCallerFlag = false; // written by EcologicalRuleInterceptor
    int32_t ermsResultCode = ERMS_ISALLOW_RESULTCODE; // written by EcologicalRuleInterceptor

    // --- read-only for interceptors, NOT synced back ---
    bool isWantWithAppCloneIndex = false;    // consumed by DialogSessionManager::IsCreateCloneSelectorDialog
    bool isSandBoxClone = false;

    std::shared_ptr<StartAbilityInfo> startAbilityInfo;        // target: carries 6 sub-fields
    std::shared_ptr<AppExecFwk::AbilityInfo> callerAbilityInfo; // caller: only abilityInfo is used

    bool ConsumeSkipCrowTest()
    {
        bool v = skipCrowTest;
        skipCrowTest = false;
        return v;
    }
    bool ConsumeSkipErms()
    {
        bool v = skipErms;
        skipErms = false;
        return v;
    }

    bool ConsumeIsWantWithAppCloneIndex()
    {
        bool v = isWantWithAppCloneIndex;
        isWantWithAppCloneIndex = false;
        return v;
    }

    // Merge back the interceptor-writable fields from an AbilityInterceptorParam copy
    // after interceptorExecuter_->DoProcess(). Keep this list in sync with the contract above:
    // a field writable by interceptors but missing here will lose its mutations.
    void SyncFrom(const StartAbilityContext &other)
    {
        ermsResultCode = other.ermsResultCode;
        ermsSupportBackToCallerFlag = other.ermsSupportBackToCallerFlag;
        skipCrowTest = other.skipCrowTest;
        skipErms = other.skipErms;
    }
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_START_ABILITY_CONTEXT_H
