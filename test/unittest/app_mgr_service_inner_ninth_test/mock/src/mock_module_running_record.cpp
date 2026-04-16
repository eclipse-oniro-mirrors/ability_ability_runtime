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

#include "module_running_record.h"
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "app_utils.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ui_extension_wrapper.h"
#include "cache_process_manager.h"
#include "mock_my_status.h"
namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string ABILITY_OWNER_USERID = "AbilityMS_Owner_UserId";
}
ModuleRunningRecord::ModuleRunningRecord(
    const std::shared_ptr<ApplicationInfo> &info, const std::shared_ptr<AMSEventHandler> &eventHandler)
    : appInfo_(info), eventHandler_(eventHandler)
{}

ModuleRunningRecord::~ModuleRunningRecord()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void ModuleRunningRecord::Init(const HapModuleInfo &info, int32_t appIndex,
    std::weak_ptr<AppMgrServiceInner> appMgrService, std::shared_ptr<AppLifeCycleDeal> appLifeCycleDeal)
{
    moduleName_ = info.moduleName;
    appIndex_ = appIndex;
    appMgrServiceInner_ = appMgrService;
    appLifeCycleDeal_ = appLifeCycleDeal;
    owenState_ = ModuleRecordState::INITIALIZED_STATE;
}

const std::string &ModuleRunningRecord::GetModuleName() const
{
    return moduleName_;
}

const std::shared_ptr<ApplicationInfo> ModuleRunningRecord::GetAppInfo()
{
    return appInfo_;
}

std::shared_ptr<AbilityRunningRecord> ModuleRunningRecord::GetAbilityRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenCalled_ = true;
    return AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByTokenModule_;
}

std::shared_ptr<AbilityRunningRecord> ModuleRunningRecord::AddAbility(sptr<IRemoteObject> token,
    std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Add ability.");
    if (!token || !abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityInfo or token");
        return nullptr;
    }
    if (GetAbilityRunningRecordByToken(token)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AbilityRecord no need to add");
        return nullptr;
    }
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    abilityRecord->SetWant(want);
    if (appInfo_) {
        abilityRecord->SetIsSingleUser(appInfo_->singleton);
    }
    if (want) {
        abilityRecord->SetOwnerUserId(want->GetIntParam(ABILITY_OWNER_USERID, -1));
    }
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    abilities_.emplace(token, abilityRecord);
    return abilityRecord;
}

bool ModuleRunningRecord::IsLastAbilityRecord(const sptr<IRemoteObject> &token)
{
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "null token");
        return false;
    }
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    return ((abilities_.size() == 1) && (abilities_.find(token) != abilities_.end()));
}

int32_t ModuleRunningRecord::GetPageAbilitySize()
{
    int pageAbilitySize = 0;
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    for (auto it : abilities_) {
        std::shared_ptr<AbilityRunningRecord> abilityRunningRecord = it.second;
        std::shared_ptr<AbilityInfo> abilityInfo = abilityRunningRecord->GetAbilityInfo();
        if (abilityInfo && (abilityInfo->type == AbilityType::PAGE ||
            abilityInfo->extensionAbilityType == ExtensionAbilityType::EMBEDDED_UI)) {
            pageAbilitySize++;
        }
    }

    return pageAbilitySize;
}

bool ModuleRunningRecord::ExtensionAbilityRecordExists()
{
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    for (auto it : abilities_) {
        std::shared_ptr<AbilityRunningRecord> abilityRunningRecord = it.second;
        std::shared_ptr<AbilityInfo> abilityInfo = abilityRunningRecord->GetAbilityInfo();
        if (abilityInfo && abilityInfo->type != AbilityType::PAGE) {
            return true;
        }
    }
    return false;
}

const std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> ModuleRunningRecord::GetAbilities()
    const
{
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    return abilities_;
}

std::shared_ptr<AbilityRunningRecord> ModuleRunningRecord::GetAbilityByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "null token");
        return nullptr;
    }
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    const auto &iter = terminateAbilities_.find(token);
    if (iter != terminateAbilities_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> ModuleRunningRecord::GetAbilityRunningRecord(const int64_t eventId) const
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    const auto &iter = std::find_if(abilities_.begin(), abilities_.end(), [eventId](const auto &pair) {
        return pair.second->GetEventId() == eventId;
    });
    if (iter != abilities_.end()) {
        return iter->second;
    }

    const auto &finder = std::find_if(terminateAbilities_.begin(),
        terminateAbilities_.end(),
        [eventId](const auto &pair) { return pair.second->GetEventId() == eventId; });
    if (finder != terminateAbilities_.end()) {
        return finder->second;
    }
    return nullptr;
}

void ModuleRunningRecord::OnAbilityStateChanged(
    const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ability");
        return;
    }
    AbilityState oldState = ability->GetState();
    ability->SetState(state);
    TAG_LOGI(AAFwkTag::APPMGR,
        "change from %{public}d to %{public}d. bundle: %{public}s, ability: %{public}s.", oldState, state,
        ability->GetAbilityInfo()->bundleName.c_str(), ability->GetName().c_str());
    auto serviceInner = appMgrServiceInner_.lock();
    if (serviceInner) {
        serviceInner->OnAbilityStateChanged(ability, state);
    }
}

void ModuleRunningRecord::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability,
    std::shared_ptr<AppUpdateInfo> updateInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!ability || !ability->GetToken()) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityRecord or abilityToken");
        return;
    }
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    const auto &iter = abilities_.find(ability->GetToken());
    if (iter != abilities_.end() && appLifeCycleDeal_->GetApplicationClient()) {
        TAG_LOGD(AAFwkTag::APPMGR, "Schedule launch ability, name is %{public}s.", ability->GetName().c_str());
        appLifeCycleDeal_->LaunchAbility(ability, updateInfo);
        ability->SetState(AbilityState::ABILITY_STATE_READY);
        SetLoaded();
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread or ability");
    }
}

void ModuleRunningRecord::LaunchPendingAbilities()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Launch pending abilities.");
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    if (abilities_.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilities_ is empty");
        return;
    }

    for (const auto &item : abilities_) {
        const auto &ability = item.second;
        TAG_LOGD(AAFwkTag::APPMGR, "state : %{public}d", ability->GetState());
        if (ability->GetState() == AbilityState::ABILITY_STATE_CREATE && ability->GetToken() &&
            appLifeCycleDeal_->GetApplicationClient()) {
            TAG_LOGD(AAFwkTag::APPMGR, "name is %{public}s.", ability->GetName().c_str());
            appLifeCycleDeal_->LaunchAbility(ability);
            ability->SetState(AbilityState::ABILITY_STATE_READY);
            SetLoaded();
        }
    }
}

void ModuleRunningRecord::TerminateAbility(const std::shared_ptr<AppRunningRecord> &appRecord,
    const sptr<IRemoteObject> &token, const bool isForce)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityRecord");
        return;
    }

    {
        std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
        terminateAbilities_.emplace(token, abilityRecord);
        abilities_.erase(token);
    }

    if (!isForce) {
        auto curAbilityState = abilityRecord->GetState();
        auto curAbilityType = abilityRecord->GetAbilityInfo()->type;
        if (curAbilityState != AbilityState::ABILITY_STATE_BACKGROUND &&
            curAbilityType == AppExecFwk::AbilityType::PAGE) {
            TAG_LOGE(AAFwkTag::APPMGR, "current state(%{public}d) error", static_cast<int32_t>(curAbilityState));
            return;
        }
    }

    if (appLifeCycleDeal_) {
        if (!(appRecord->IsDebugApp() || appRecord->IsAttachDebug())) {
            SendEvent(AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG,
                AMSEventHandler::TERMINATE_ABILITY_TIMEOUT * AAFwk::AppUtils::GetInstance().GetTimeoutUnitTimeRatio(),
                abilityRecord);
        }
        bool isCachedProcess = DelayedSingleton<CacheProcessManager>::GetInstance()->IsAppShouldCache(appRecord);
        appLifeCycleDeal_->ScheduleCleanAbility(token, isCachedProcess);
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        auto serviceInner = appMgrServiceInner_.lock();
        if (serviceInner) {
            serviceInner->TerminateApplication(appRecord);
        }
    }

    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void ModuleRunningRecord::SendEvent(
    uint32_t msg, int64_t timeOut, const std::shared_ptr<AbilityRunningRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Send event");
    if (!eventHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null eventHandler_");
        return;
    }

    AppRunningRecord::appEventId_++;
    abilityRecord->SetEventId(AppRunningRecord::appEventId_);
    eventHandler_->SendEvent(AAFwk::EventWrap(msg, AppRunningRecord::appEventId_), timeOut);
}

void ModuleRunningRecord::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "null token");
        return;
    }

    if (RemoveTerminateAbilityTimeoutTask(token)) {
        std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
        terminateAbilities_.erase(token);
    }
}

bool ModuleRunningRecord::RemoveTerminateAbilityTimeoutTask(const sptr<IRemoteObject>& token) const
{
    auto abilityRecord = GetAbilityByTerminateLists(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityRecord");
        return false;
    }
    if (!eventHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null eventHandler_");
        return false;
    }
    eventHandler_->RemoveEvent(AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG, abilityRecord->GetEventId());
    return true;
}

bool ModuleRunningRecord::IsAbilitiesBackgrounded()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    for (const auto &iter : abilities_) {
        const auto &ability = iter.second;
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null ability");
            continue;
        }
        const auto &abilityInfo = ability->GetAbilityInfo();
        // uiextensionability also has foreground and background states.
        if (abilityInfo != nullptr && abilityInfo->type != AbilityType::PAGE &&
            !AAFwk::UIExtensionWrapper::IsUIExtension(abilityInfo->extensionAbilityType)) {
            continue;
        }

        const auto &state = ability->GetState();
        if (state != AbilityState::ABILITY_STATE_BACKGROUND &&
            state != AbilityState::ABILITY_STATE_TERMINATED &&
            state != AbilityState::ABILITY_STATE_END) {
            return false;
        }
    }
    return true;
}

void ModuleRunningRecord::SetAppMgrServiceInner(const std::weak_ptr<AppMgrServiceInner> &inner)
{
    appMgrServiceInner_ = inner;
}

ModuleRecordState ModuleRunningRecord::GetModuleRecordState()
{
    return owenState_;
}

void ModuleRunningRecord::SetModuleRecordState(const ModuleRecordState &state)
{
    owenState_ = state;
}

void ModuleRunningRecord::GetHapModuleInfo(HapModuleInfo &info)
{
    if (appInfo_ == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "appInfo null");
        return;
    }
    BundleInfo bundleInfo;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    auto userId = appInfo_->uid / BASE_USER_RANGE;
    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s, appIndex: %{public}d", userId,
        appInfo_->bundleName.c_str(), appIndex_);
    int32_t bundleMgrResult;
    auto flag = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
    if (appIndex_ <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetCloneBundleInfo(appInfo_->bundleName,
            flag, appIndex_, bundleInfo, userId));
    } else {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetSandboxBundleInfo(appInfo_->bundleName,
            appIndex_, userId, bundleInfo));
    }

    if (bundleMgrResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBundleInfo fail");
        return;
    }

    bool found = false;
    for (const auto &moduleIno : bundleInfo.hapModuleInfos) {
        if (moduleIno.moduleName == moduleName_) {
            info = moduleIno;
            found = true;
            break;
        }
    }

    if (!found) {
        TAG_LOGW(AAFwkTag::APPMGR, "not found userId: %{public}d, bundleName: %{public}s, appIndex: %{public}d, "
            "name: %{public}s", userId, appInfo_->bundleName.c_str(), appIndex_, moduleName_.c_str());
    }
}

void ModuleRunningRecord::SetApplicationClient(std::shared_ptr<AppLifeCycleDeal> &appLifeCycleDeal)
{
    appLifeCycleDeal_ = appLifeCycleDeal;
}

ModuleRecordState ModuleRunningRecord::GetState() const
{
    return owenState_;
}

bool ModuleRunningRecord::IsAllAbilityReadyToCleanedByUserRequest()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard<ffrt::mutex> lock(abilitiesMutex_);
    for (const auto &iter : abilities_) {
        const auto &ability = iter.second;
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null ability");
            continue;
        }
        const auto &abilityInfo = ability->GetAbilityInfo();
        if (abilityInfo != nullptr && abilityInfo->type != AbilityType::PAGE) {
            continue;
        }

        if (!ability->IsUserRequestCleaning()) {
            return false;
        }

        const auto &state = ability->GetState();
        if (state != AbilityState::ABILITY_STATE_BACKGROUND &&
            state != AbilityState::ABILITY_STATE_TERMINATED &&
            state != AbilityState::ABILITY_STATE_END) {
            return false;
        }
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
