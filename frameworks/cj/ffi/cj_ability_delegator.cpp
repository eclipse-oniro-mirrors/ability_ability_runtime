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

#include "cj_ability_delegator.h"

#include "ability_delegator_registry.h"
#include "application_context.h"
#include "cj_application_context.h"
#include "cj_utils_ffi.h"
#include "cj_common_ffi.h"
#include "hilog_tag_wrapper.h"
#include "shell_cmd_result.h"
#include <mutex>

namespace OHOS {
namespace AbilityDelegatorCJ {
using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;

constexpr int COMMON_FAILED = 16000100;
constexpr int INNER_ERROR = 16000050;
constexpr int INCORRECT_PARAMETERS = 401;

std::map<int64_t, std::shared_ptr<CJAbilityMonitor>> g_monitorRecord;
std::map<int64_t, std::shared_ptr<CJAbilityStageMonitor>> g_stageMonitorRecord;
std::map<int64_t, sptr<OHOS::IRemoteObject>> g_abilityRecord;
std::mutex g_mutexAbilityRecord;
std::mutex g_mtxMonitorRecord;
std::mutex g_mtxStageMonitorRecord;

CJAbilityDelegator::CJAbilityDelegator(const std::shared_ptr<AppExecFwk::CJAbilityDelegatorImpl>& abilityDelegator)
    : delegator_(abilityDelegator)
{
    auto clearFunc = [](const std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty>& property) {
        TAG_LOGD(AAFwkTag::DELEGATOR, "clearFunc called");
        if (!property) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "property is null");
            return;
        }

        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        for (auto it = g_abilityRecord.begin(); it != g_abilityRecord.end();) {
            if (it->second == property->token_) {
                it = g_abilityRecord.erase(it);
                continue;
            }
            ++it;
        }
    };
    delegator_->RegisterClearFunc(clearFunc);
}

int32_t CJAbilityDelegator::StartAbility(const AAFwk::Want& want)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CJAbilityDelegator::StartAbility called");
    return delegator_->StartAbility(want);
}

void CJAbilityDelegator::AddAbilityMonitor(const std::shared_ptr<CJAbilityMonitor>& abilityMonitor)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CJAbilityDelegator::AddAbilityMonitor called");
    delegator_->AddAbilityMonitor(abilityMonitor);
}

void CJAbilityDelegator::RemoveAbilityMonitor(const std::shared_ptr<CJAbilityMonitor>& abilityMonitor)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CJAbilityDelegator::RemoveAbilityMonitor called");
    delegator_->RemoveAbilityMonitor(abilityMonitor);
}

std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> CJAbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<CJAbilityMonitor>& abilityMonitor)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CJAbilityDelegator::WaitAbilityMonitor called");
    std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> property = delegator_->WaitAbilityMonitor(abilityMonitor);
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "property is null");
        return nullptr;
    }
    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    g_abilityRecord.emplace(property->cjObject_, property->token_);
    return property;
}

std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> CJAbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<CJAbilityMonitor>& abilityMonitor, int64_t timeout)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CJAbilityDelegator::WaitAbilityMonitor called");
    std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> property =
        delegator_->WaitAbilityMonitor(abilityMonitor, timeout);
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "property is null");
        return nullptr;
    }
    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    g_abilityRecord.emplace(property->cjObject_, property->token_);
    return property;
}

void CJAbilityDelegator::AddAbilityStageMonitor(const std::shared_ptr<CJAbilityStageMonitor>& stageMonitor)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CJAbilityDelegator::AddAbilityStageMonitor called");
    delegator_->AddAbilityStageMonitor(stageMonitor);
}

void CJAbilityDelegator::RemoveAbilityStageMonitor(const std::shared_ptr<CJAbilityStageMonitor>& stageMonitor)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CJAbilityDelegator::RemoveAbilityStageMonitor called");
    delegator_->RemoveAbilityStageMonitor(stageMonitor);
}

std::shared_ptr<AppExecFwk::CJDelegatorAbilityStageProperty> CJAbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<CJAbilityStageMonitor>& stageMonitor)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CJAbilityDelegator::WaitAbilityStageMonitor called");
    std::shared_ptr<AppExecFwk::CJDelegatorAbilityStageProperty> property =
        delegator_->WaitAbilityStageMonitor(stageMonitor);
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "property is null");
        return nullptr;
    }
    return property;
}

std::shared_ptr<AppExecFwk::CJDelegatorAbilityStageProperty> CJAbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<CJAbilityStageMonitor>& stageMonitor, int64_t timeout)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CJAbilityDelegator::WaitAbilityStageMonitor called");
    std::shared_ptr<AppExecFwk::CJDelegatorAbilityStageProperty> property =
        delegator_->WaitAbilityStageMonitor(stageMonitor, timeout);
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "property is null");
        return nullptr;
    }
    return property;
}

void CJAbilityDelegator::Print(const std::string& msg)
{
    return delegator_->Print(msg);
}

int64_t CJAbilityDelegator::GetAbilityState(const sptr<OHOS::IRemoteObject>& remoteObject)
{
    return static_cast<int64_t>(delegator_->GetAbilityState(remoteObject));
}

std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> CJAbilityDelegator::GetCurrentTopAbility()
{
    auto property = delegator_->GetCurrentTopAbility();
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "property is null");
        return nullptr;
    }
    return property;
}

bool CJAbilityDelegator::DoAbilityForeground(const sptr<OHOS::IRemoteObject>& remoteObject)
{
    return delegator_->DoAbilityForeground(remoteObject);
}

bool CJAbilityDelegator::DoAbilityBackground(const sptr<OHOS::IRemoteObject>& remoteObject)
{
    return delegator_->DoAbilityBackground(remoteObject);
}

std::shared_ptr<AppExecFwk::ShellCmdResult> CJAbilityDelegator::ExecuteShellCommand(const char* cmd, int64_t timeoutSec)
{
    auto shellCmd = delegator_->ExecuteShellCommand(cmd, timeoutSec);
    std::shared_ptr<AppExecFwk::ShellCmdResult> ret = std::move(shellCmd);
    return ret;
}

std::shared_ptr<AbilityRuntime::ApplicationContext> CJAbilityDelegator::GetAppContext()
{
    auto context = delegator_->GetAppContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null context");
        return nullptr;
    }
    return context->GetApplicationContext();
}

int32_t CJShellCmdResult::GetExitCode()
{
    return shellCmdResultr_->GetExitCode();
}

std::string CJShellCmdResult::GetStdResult()
{
    return shellCmdResultr_->GetStdResult();
}

std::string CJShellCmdResult::Dump()
{
    return shellCmdResultr_->Dump();
}

void CJAbilityDelegator::FinishTest(const char* msg, int64_t code)
{
    delegator_->FinishUserTest(msg, code);
}

enum class MonitorRecordOp {
    ADD,
    REMOVE,
};

std::shared_ptr<CJAbilityMonitor> GetOrCreateMonitor(
    int64_t monitorId, const std::string& abilityName, const std::string& moduleName, MonitorRecordOp op)
{
    std::unique_lock<std::mutex> lck(g_mtxMonitorRecord);
    auto iter = g_monitorRecord.find(monitorId);
    if (iter != g_monitorRecord.end()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "monitor existed");
        auto existing = iter->second;
        if (op == MonitorRecordOp::REMOVE) {
            g_monitorRecord.erase(iter);
        }
        return existing;
    }
    if (op == MonitorRecordOp::REMOVE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "monitor not existed");
        return nullptr;
    }
    auto cjMonitorObj = std::make_shared<CJMonitorObject>(monitorId);
    std::shared_ptr<CJAbilityMonitor> cjMonitor;
    if (moduleName.empty()) {
        cjMonitor = std::make_shared<CJAbilityMonitor>(abilityName, cjMonitorObj);
    } else {
        cjMonitor = std::make_shared<CJAbilityMonitor>(abilityName, moduleName, cjMonitorObj);
    }
    g_monitorRecord.emplace(monitorId, cjMonitor);
    return cjMonitor;
}

std::shared_ptr<CJAbilityStageMonitor> GetOrCreateStageMonitor(
    int64_t stageMonitorId, const std::string& moduleName, const std::string& srcEntrance, MonitorRecordOp op)
{
    std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
    auto iter = g_stageMonitorRecord.find(stageMonitorId);
    if (iter != g_stageMonitorRecord.end()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "stageMonitor existed");
        auto existing = iter->second;
        if (op == MonitorRecordOp::REMOVE) {
            g_stageMonitorRecord.erase(iter);
        }
        return existing;
    }
    if (op == MonitorRecordOp::REMOVE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "stageMonitor not existed");
        return nullptr;
    }
    auto cjStageMonitor = std::make_shared<CJAbilityStageMonitor>(moduleName, srcEntrance, stageMonitorId);
    g_stageMonitorRecord.emplace(stageMonitorId, cjStageMonitor);
    return cjStageMonitor;
}

extern "C" {
int32_t FFIAbilityDelegatorDoAbilityForeground(int64_t id, int64_t abilityId, bool* ret)
{
    if (!ret) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "receiver is nullptr");
        return COMMON_FAILED;
    }
    
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        *ret = false;
        return COMMON_FAILED;
    }

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    {
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        auto iter = g_abilityRecord.find(abilityId);
        if (iter != g_abilityRecord.end()) {
            remoteObject = iter->second;
        }
    }
    if (!remoteObject) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parase remoteObject failed");
        *ret = false;
        return INCORRECT_PARAMETERS;
    }
    if (!cjDelegator->DoAbilityForeground(remoteObject)) {
        *ret = false;
        return COMMON_FAILED;
    }
    *ret = true;
    return 0;
}

int32_t FFIAbilityDelegatorDoAbilityBackground(int64_t id, int64_t abilityId, bool* ret)
{
    if (!ret) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "receiver is nullptr");
        return COMMON_FAILED;
    }

    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        *ret = false;
        return COMMON_FAILED;
    }

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    {
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        auto iter = g_abilityRecord.find(abilityId);
        if (iter != g_abilityRecord.end()) {
            remoteObject = iter->second;
        }
    }
    if (!remoteObject) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parase remoteObject failed");
        *ret = false;
        return INCORRECT_PARAMETERS;
    }
    if (!cjDelegator->DoAbilityBackground(remoteObject)) {
        *ret = false;
        return COMMON_FAILED;
    }
    *ret = true;
    return 0;
}

int32_t FFIAbilityDelegatorGetCurrentTopAbility(int64_t id, int64_t* abilityId)
{
    if (!abilityId) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "receiver is nullptr");
        return COMMON_FAILED;
    }

    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        *abilityId = 0;
        return COMMON_FAILED;
    }

    auto property = cjDelegator->GetCurrentTopAbility();
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null property");
        *abilityId = 0;
        return COMMON_FAILED;
    }
    {
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        g_abilityRecord.emplace(property->cjObject_, property->token_);
    }
    *abilityId =  property->cjObject_;
    return 0;
}

int32_t FFIAbilityDelegatorPrint(int64_t id, const char* msg)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        return COMMON_FAILED;
    }
    cjDelegator->Print(msg);
    return 0;
}
int32_t FFIAbilityDelegatorGetAbilityState(int64_t id, int64_t abilityId, int64_t* state)
{
    if (!state) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "receiver is nullptr");
        return COMMON_FAILED;
    }
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        *state = -1;
        return COMMON_FAILED;
    }

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    {
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        auto iter = g_abilityRecord.find(abilityId);
        if (iter != g_abilityRecord.end()) {
            remoteObject = iter->second;
        }
    }
    if (!remoteObject) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parase remoteObject failed");
        *state = -1;
        return INCORRECT_PARAMETERS;
    }
    *state = cjDelegator->GetAbilityState(remoteObject);
    return 0;
}

int32_t FFIAbilityDelegatorAddAbilityMonitor(
    int64_t id, int64_t monitorId, const char* abilityName, const char* moduleName)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        return COMMON_FAILED;
    }
    auto cjMonitor = GetOrCreateMonitor(monitorId, abilityName, moduleName, MonitorRecordOp::ADD);
    if (cjMonitor == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parase cj monitor failed");
        return INCORRECT_PARAMETERS;
    }
    cjDelegator->AddAbilityMonitor(cjMonitor);
    return 0;
}

int32_t FFIAbilityDelegatorRemoveAbilityMonitor(
    int64_t id, int64_t monitorId, const char* abilityName, const char* moduleName)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        return COMMON_FAILED;
    }
    auto cjMonitor = GetOrCreateMonitor(monitorId, abilityName, moduleName, MonitorRecordOp::REMOVE);
    if (cjMonitor == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "monitor not existed");
        return INCORRECT_PARAMETERS;
    }
    cjDelegator->RemoveAbilityMonitor(cjMonitor);
    return 0;
}

int32_t FFIAbilityDelegatorWaitAbilityMonitor(
    int64_t id, int64_t monitorId, CJAbilityInfo abilityInfo, int64_t* abilityId)
{
    if (!abilityId) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "receiver is nullptr");
        return COMMON_FAILED;
    }
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        *abilityId = 0;
        return COMMON_FAILED;
    }

    const char* abilityName = abilityInfo.abilityName;
    const char* moduleName = abilityInfo.moduleName;
    auto cjMonitor = GetOrCreateMonitor(monitorId, abilityName, moduleName, MonitorRecordOp::ADD);
    if (cjMonitor == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parase cj monitor failed");
        *abilityId = 0;
        return INCORRECT_PARAMETERS;
    }

    auto property = cjDelegator->WaitAbilityMonitor(cjMonitor);
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "property is null");
        *abilityId = 0;
        return COMMON_FAILED;
    }

    {
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        g_abilityRecord.emplace(property->cjObject_, property->token_);
    }
    *abilityId = property->cjObject_;
    return 0;
}

int32_t FFIAbilityDelegatorWaitAbilityMonitorWithTimeout(
    int64_t id, int64_t monitorId, CJAbilityInfo abilityInfo, int64_t timeout, int64_t* abilityId)
{
    if (!abilityId) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "receiver is nullptr");
        return COMMON_FAILED;
    }
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        *abilityId = 0;
        return COMMON_FAILED;
    }

    const char* abilityName = abilityInfo.abilityName;
    const char* moduleName = abilityInfo.moduleName;
    auto cjMonitor = GetOrCreateMonitor(monitorId, abilityName, moduleName, MonitorRecordOp::ADD);
    if (cjMonitor == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parase cj monitor failed");
        *abilityId = 0;
        return INCORRECT_PARAMETERS;
    }

    auto property = cjDelegator->WaitAbilityMonitor(cjMonitor, timeout);
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "property is null");
        *abilityId = 0;
        return COMMON_FAILED;
    }

    {
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        g_abilityRecord.emplace(property->cjObject_, property->token_);
    }
    *abilityId = property->cjObject_;
    return 0;
}

int32_t FFIAbilityDelegatorAddAbilityStageMonitor(
    int64_t id, int64_t stageMonitorId, const char* moduleName, const char* srcEntrance)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        return COMMON_FAILED;
    }
    auto cjStageMonitor = GetOrCreateStageMonitor(stageMonitorId, moduleName, srcEntrance, MonitorRecordOp::ADD);
    if (cjStageMonitor == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parse cj stageMonitor failed");
        return INCORRECT_PARAMETERS;
    }
    cjDelegator->AddAbilityStageMonitor(cjStageMonitor);
    return 0;
}

int32_t FFIAbilityDelegatorRemoveAbilityStageMonitor(
    int64_t id, int64_t stageMonitorId, const char* moduleName, const char* srcEntrance)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        return COMMON_FAILED;
    }
    auto cjStageMonitor = GetOrCreateStageMonitor(stageMonitorId, moduleName, srcEntrance, MonitorRecordOp::REMOVE);
    if (cjStageMonitor == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "stageMonitor not existed");
        return INCORRECT_PARAMETERS;
    }
    cjDelegator->RemoveAbilityStageMonitor(cjStageMonitor);
    return 0;
}

int32_t FFIAbilityDelegatorWaitAbilityStageMonitor(
    int64_t id, int64_t stageMonitorId, CJAbilityStageInfo abilityStageInfo, int64_t* abilityStageId)
{
    if (!abilityStageId) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "receiver is nullptr");
        return COMMON_FAILED;
    }
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        *abilityStageId = 0;
        return COMMON_FAILED;
    }
    const char* moduleName = abilityStageInfo.moduleName;
    const char* srcEntrance = abilityStageInfo.srcEntrance;
    auto cjStageMonitor = GetOrCreateStageMonitor(stageMonitorId, moduleName, srcEntrance, MonitorRecordOp::ADD);
    if (cjStageMonitor == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parse cj stageMonitor failed");
        *abilityStageId = 0;
        return INCORRECT_PARAMETERS;
    }

    auto property = cjDelegator->WaitAbilityStageMonitor(cjStageMonitor);
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "stageProperty is null");
        *abilityStageId = 0;
        return COMMON_FAILED;
    }

    *abilityStageId = property->cjStageObject_;
    return 0;
}

int32_t FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeout(
    int64_t id, int64_t stageMonitorId, CJAbilityStageInfo abilityStageInfo, int64_t timeout, int64_t* abilityStageId)
{
    if (!abilityStageId) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "receiver is nullptr");
        return COMMON_FAILED;
    }
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        *abilityStageId = 0;
        return COMMON_FAILED;
    }
    const char* moduleName = abilityStageInfo.moduleName;
    const char* srcEntrance = abilityStageInfo.srcEntrance;
    auto cjStageMonitor = GetOrCreateStageMonitor(stageMonitorId, moduleName, srcEntrance, MonitorRecordOp::ADD);
    if (cjStageMonitor == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parse cj stageMonitor failed");
        *abilityStageId = 0;
        return INCORRECT_PARAMETERS;
    }

    auto property = cjDelegator->WaitAbilityStageMonitor(cjStageMonitor, timeout);
    if (!property) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "stageProperty is null");
        *abilityStageId = 0;
        return COMMON_FAILED;
    }

    *abilityStageId = property->cjStageObject_;
    return 0;
}

int64_t FFIAbilityDelegatorRegistryGetAbilityDelegator()
{
    auto delegator = OHOS::AppExecFwk::AbilityDelegatorRegistry::GetCJAbilityDelegator();
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "cjDelegatorImpl is null");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjDelegator = FFI::FFIData::Create<CJAbilityDelegator>(delegator);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null cj delegator");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjDelegator->GetID();
}

int32_t FFIAbilityDelegatorStartAbility(int64_t id, WantHandle want)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        return INNER_ERROR;
    }
    auto actualWant = reinterpret_cast<AAFwk::Want*>(want);
    return cjDelegator->StartAbility(*actualWant);
}

int32_t FFIAbilityDelegatorExecuteShellCommand(int64_t id, const char* cmd, int64_t timeoutSec)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cJShellCmdResult = FFI::FFIData::Create<CJShellCmdResult>(cjDelegator->ExecuteShellCommand(cmd, timeoutSec));
    if (cJShellCmdResult == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj shell command result is null.");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cJShellCmdResult->GetID();
}

int32_t FFIGetExitCode(int64_t id)
{
    auto cjShellCmdResult = FFI::FFIData::GetData<CJShellCmdResult>(id);
    if (cjShellCmdResult == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj shellcommand result");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjShellCmdResult->GetExitCode();
}

const char* FFIGetStdResult(int64_t id)
{
    auto cjShellCmdResult = FFI::FFIData::GetData<CJShellCmdResult>(id);
    if (cjShellCmdResult == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj shellcommand result");
        return nullptr;
    }
    const char* res = CreateCStringFromString(cjShellCmdResult->GetStdResult());
    return res;
}

const char* FFIDump(int64_t id)
{
    auto cjShellCmdResult = FFI::FFIData::GetData<CJShellCmdResult>(id);
    if (cjShellCmdResult == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj shellcommand result");
        return nullptr;
    }
    const char* dump = CreateCStringFromString(cjShellCmdResult->Dump());
    return dump;
}

int32_t FFIAbilityDelegatorApplicationContext(int64_t id)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto appContext = ApplicationContextCJ::CJApplicationContext::GetCJApplicationContext(cjDelegator->GetAppContext());
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null app context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return appContext->GetID();
}

void FFIAbilityDelegatorFinishTest(int64_t id, const char* msg, int64_t code)
{
    auto cjDelegator = FFI::FFIData::GetData<CJAbilityDelegator>(id);
    if (cjDelegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cj delegator");
        return;
    }
    cjDelegator->FinishTest(msg, code);
}
}
}
}