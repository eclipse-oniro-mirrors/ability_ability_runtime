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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_FFI_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_FFI_H

#include <cstdint>

#include "ability_delegator_registry.h"
#include "cj_ability_monitor.h"
#include "cj_ability_stage_monitor.h"
#include "cj_macro.h"
#include "ffi_remote_data.h"
#include "cj_ability_delegator_impl.h"

using WantHandle = void*;
namespace OHOS {
namespace AbilityDelegatorCJ {

class CJAbilityDelegator : public FFI::FFIData {
    DECL_TYPE(CJAbilityDelegator, OHOS::FFI::FFIData)
public:
    explicit CJAbilityDelegator(const std::shared_ptr<AppExecFwk::CJAbilityDelegatorImpl>& abilityDelegator);

    int32_t StartAbility(const AAFwk::Want& want);
    std::shared_ptr<AppExecFwk::ShellCmdResult> ExecuteShellCommand(const char* cmd, int64_t timeoutSec);
    std::shared_ptr<AbilityRuntime::ApplicationContext> GetAppContext();
    void FinishTest(const char* msg, int64_t code);

    void AddAbilityMonitor(const std::shared_ptr<CJAbilityMonitor>& abilityMonitor);
    void RemoveAbilityMonitor(const std::shared_ptr<CJAbilityMonitor>& abilityMonitor);
    std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> WaitAbilityMonitor(
        const std::shared_ptr<CJAbilityMonitor>& abilityMonitor);
    std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> WaitAbilityMonitor(
        const std::shared_ptr<CJAbilityMonitor>& abilityMonitor, int64_t timeout);
    void AddAbilityStageMonitor(const std::shared_ptr<CJAbilityStageMonitor>& stageMonitor);
    void RemoveAbilityStageMonitor(const std::shared_ptr<CJAbilityStageMonitor>& stageMonitor);
    std::shared_ptr<AppExecFwk::CJDelegatorAbilityStageProperty> WaitAbilityStageMonitor(
        const std::shared_ptr<CJAbilityStageMonitor>& stageMonitor);
    std::shared_ptr<AppExecFwk::CJDelegatorAbilityStageProperty> WaitAbilityStageMonitor(
        const std::shared_ptr<CJAbilityStageMonitor>& stageMonitor, int64_t timeout);

    void Print(const std::string& msg);
    int64_t GetAbilityState(const sptr<OHOS::IRemoteObject>& remoteObject);
    std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> GetCurrentTopAbility();
    bool DoAbilityForeground(const sptr<OHOS::IRemoteObject>& remoteObject);
    bool DoAbilityBackground(const sptr<OHOS::IRemoteObject>& remoteObject);

private:
    std::shared_ptr<AppExecFwk::CJAbilityDelegatorImpl> delegator_;
};

class CJShellCmdResult : public FFI::FFIData {
    DECL_TYPE(CJShellCmdResult, OHOS::FFI::FFIData)
public:
    explicit CJShellCmdResult(std::shared_ptr<AppExecFwk::ShellCmdResult> shellCmdResult)
        : shellCmdResultr_(shellCmdResult) {};
    int32_t GetExitCode();
    std::string GetStdResult();
    std::string Dump();

private:
    std::shared_ptr<AppExecFwk::ShellCmdResult> shellCmdResultr_;
};

extern "C" {
struct CJAbilityInfo {
    const char* abilityName;
    const char* moduleName;
};

struct CJAbilityStageInfo {
    const char* moduleName;
    const char* srcEntrance;
};

CJ_EXPORT int32_t FFIAbilityDelegatorDoAbilityForeground(int64_t id, int64_t abilityId, bool* ret);
CJ_EXPORT int32_t FFIAbilityDelegatorDoAbilityBackground(int64_t id, int64_t abilityId, bool* ret);
CJ_EXPORT int32_t FFIAbilityDelegatorGetCurrentTopAbility(int64_t id, int64_t* abilityId);
CJ_EXPORT int32_t FFIAbilityDelegatorGetAbilityState(int64_t id, int64_t abilityId, int64_t* state);
CJ_EXPORT int32_t FFIAbilityDelegatorPrint(int64_t id, const char* msg);
CJ_EXPORT int32_t FFIAbilityDelegatorAddAbilityMonitor(
    int64_t id, int64_t monitorId, const char* abilityName, const char* moduleName);
CJ_EXPORT int32_t FFIAbilityDelegatorRemoveAbilityMonitor(
    int64_t id, int64_t monitorId, const char* abilityName, const char* moduleName);
CJ_EXPORT int32_t FFIAbilityDelegatorWaitAbilityMonitor(
    int64_t id, int64_t monitorId, CJAbilityInfo abilityInfo, int64_t* abilityId);
CJ_EXPORT int32_t FFIAbilityDelegatorWaitAbilityMonitorWithTimeout(
    int64_t id, int64_t monitorId, CJAbilityInfo abilityInfo, int64_t timeout, int64_t* abilityId);
CJ_EXPORT int32_t FFIAbilityDelegatorAddAbilityStageMonitor(
    int64_t id, int64_t stageMonitorId, const char* moduleName, const char* srcEntrance);
CJ_EXPORT int32_t FFIAbilityDelegatorRemoveAbilityStageMonitor(
    int64_t id, int64_t stageMonitorId, const char* moduleName, const char* srcEntrance);
CJ_EXPORT int32_t FFIAbilityDelegatorWaitAbilityStageMonitor(
    int64_t id, int64_t stageMonitorId, CJAbilityStageInfo abilityStageInfo, int64_t* abilityStageId);
CJ_EXPORT int32_t FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeout(
    int64_t id, int64_t stageMonitorId, CJAbilityStageInfo abilityStageInfo, int64_t timeout, int64_t* abilityStageId);

CJ_EXPORT int64_t FFIAbilityDelegatorRegistryGetAbilityDelegator();
CJ_EXPORT int32_t FFIAbilityDelegatorStartAbility(int64_t id, WantHandle want);
CJ_EXPORT int32_t FFIAbilityDelegatorExecuteShellCommand(int64_t id, const char* cmd, int64_t timeoutSec);
CJ_EXPORT int32_t FFIGetExitCode(int64_t id);
CJ_EXPORT const char* FFIGetStdResult(int64_t id);
CJ_EXPORT const char* FFIDump(int64_t id);
CJ_EXPORT int32_t FFIAbilityDelegatorApplicationContext(int64_t id);
CJ_EXPORT void FFIAbilityDelegatorFinishTest(int64_t id, const char* msg, int64_t code);
};
}
}
#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_FFI_H