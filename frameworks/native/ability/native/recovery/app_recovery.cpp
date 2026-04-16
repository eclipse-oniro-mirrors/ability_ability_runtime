/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "app_recovery.h"

#include <csignal>
#include <mutex>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <syscall.h>
#include <unistd.h>

#include "ability_manager_client.h"
#include "context/application_context.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_ability.h"
#include "mission_info.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "ohos_application.h"
#include "parcel.h"
#include "recovery_param.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {
std::mutex g_mutex;
std::atomic<bool> g_blocked = false;
const int DELAY_TIME = 1000;

AppRecovery::AppRecovery() : isEnable_(false), restartFlag_(RestartFlag::ALWAYS_RESTART),
    saveOccasion_(SaveOccasionFlag::SAVE_WHEN_ERROR), saveMode_(SaveModeFlag::SAVE_WITH_FILE),
    crashFlag_(RestartFlag::NO_RESTART)
{
}

AppRecovery::~AppRecovery()
{
}

static void SigQuitHandler(int signal)
{
    g_blocked = true;
    std::lock_guard<std::mutex> lock(g_mutex);
    g_blocked = false;
}

static bool BlockMainThreadLocked()
{
    struct sigaction action;
    (void)memset_s(&action, sizeof(action), 0, sizeof(action));
    sigfillset(&action.sa_mask);
    action.sa_handler = SigQuitHandler;
    action.sa_flags = 0;
    if (sigaction(SIGQUIT, &action, nullptr) != 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "register signal failed");
        return false;
    }

    if (syscall(SYS_tgkill, getpid(), getpid(), SIGQUIT) != 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "send SIGQUIT failed, errno(%{public}d)", errno);
        return false;
    }
    int left = 1000000; // 1s
    constexpr int pollTime = 100; // 100us
    while (left > 0) {
        int ret = usleep(pollTime);
        if (ret == 0) {
            left -= pollTime;
        } else {
            left -= ret;
        }

        if (g_blocked) {
            return true;
        }
    }
    return false;
}

AppRecovery& AppRecovery::GetInstance()
{
    static AppRecovery instance;
    return instance;
}

bool AppRecovery::InitApplicationInfo(const std::shared_ptr<EventHandler>& mainHandler,
    const std::shared_ptr<ApplicationInfo>& applicationInfo)
{
    mainHandler_ = mainHandler;
    applicationInfo_ = applicationInfo;
    return true;
}

bool AppRecovery::AddAbility(std::shared_ptr<AbilityRuntime::UIAbility> ability,
    const std::shared_ptr<AbilityInfo>& abilityInfo, const sptr<IRemoteObject>& token)
{
    if (abilityInfo == nullptr || ability == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null abilityInfo or ability");
        return false;
    }

    token_ = token;
    mainWant_.SetElementName(abilityInfo->bundleName, abilityInfo->name);
    auto handler = mainHandler_.lock();
    if (handler != nullptr) {
        auto task = []() {
            AppRecovery::GetInstance().DeleteInValidMissionFiles();
        };
        if (!handler->PostTask(task, "AppRecovery:AddAbility", DELAY_TIME)) {
            TAG_LOGE(AAFwkTag::RECOVERY, "DeleteInValidMissionFiles failed");
        }
    }
 
    if (isEnable_ && !abilityRecoverys_.empty() && !abilityInfo->recoverable) {
        TAG_LOGE(AAFwkTag::RECOVERY, "recoverable is false");
        return false;
    }
    ability_ = ability;
    std::shared_ptr<AbilityRecovery> abilityRecovery = std::make_shared<AbilityRecovery>();
    abilityRecovery->InitAbilityInfo(ability, abilityInfo, token);
    abilityRecovery->EnableAbilityRecovery(useAppSettedValue_.load(), restartFlag_, saveOccasion_, saveMode_);
    ability->EnableAbilityRecovery(abilityRecovery, useAppSettedValue_.load());
    abilityRecoverys_.push_back(abilityRecovery);
    std::shared_ptr<AAFwk::AbilityManagerClient> abilityMgr = AAFwk::AbilityManagerClient::GetInstance();
    abilityMgr->SetAppRecoveryFlag(token_.promote(), crashFlag_);
    return true;
}

bool AppRecovery::RemoveAbility(const sptr<IRemoteObject>& tokenId)
{
    if (!tokenId) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null tokenId");
        return false;
    }
    TAG_LOGD(AAFwkTag::RECOVERY, "start");
    auto itr = std::find_if(abilityRecoverys_.begin(), abilityRecoverys_.end(),
        [&tokenId](std::shared_ptr<AbilityRecovery> &abilityRecovery) {
        return (abilityRecovery && abilityRecovery->GetToken() == tokenId);
    });
    if (itr != abilityRecoverys_.end()) {
        abilityRecoverys_.erase(itr);
    }
    return true;
}

void AppRecovery::SetFreezeCallback(FreezeFunction freezeCallback)
{
    if (!this->freezeCallback) {
        this->freezeCallback = freezeCallback;
    }
}

bool AppRecovery::IsNeedSaveAppState(StateReason reason)
{
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not enabled");
        return false;
    }

    if (!ShouldSaveAppState(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not save ability state");
        return false;
    }
    return true;
}

bool AppRecovery::ScheduleSaveAppState(StateReason reason, uintptr_t ability)
{
    TAG_LOGI(AAFwkTag::RECOVERY, "begin");
    bool ret = IsNeedSaveAppState(reason);
    if (!ret && this->freezeCallback ==  nullptr) {
        return false;
    }

    if (reason == StateReason::APP_FREEZE) {
        auto abilityPtr = ability_.lock();
        if (!abilityPtr || !abilityPtr->GetAbilityContext()) {
            TAG_LOGE(AAFwkTag::RECOVERY, "null ability or context");
            return false;
        }
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!BlockMainThreadLocked()) {
            TAG_LOGE(AAFwkTag::RECOVERY, "block main thread failed");
            return false;
        }
        if (!ExecuteFreezeCallbackWithVMSafety(abilityPtr)) {
            return false;
        }
        if (!ret) {
            return false;
        }
        AppRecovery::GetInstance().DoSaveAppState(reason, ability);
        return true;
    }

    auto handler = mainHandler_.lock();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null handler");
        return false;
    }

    auto task = [reason, ability]() {
        AppRecovery::GetInstance().DoSaveAppState(reason, ability);
    };
    if (!handler->PostTask(task, "AppRecovery:SaveAppState")) {
        TAG_LOGE(AAFwkTag::RECOVERY, "schedule save app state failed");
        return false;
    }

    return true;
}

bool AppRecovery::ExecuteFreezeCallbackWithVMSafety(const std::shared_ptr<OHOS::AbilityRuntime::UIAbility>& abilityPtr)
{
    if (!abilityPtr) {
        return false;
    }
    if (IsEtsAPP() && this->freezeCallback) {
        this->freezeCallback();
        TAG_LOGW(AAFwkTag::RECOVERY, "Freeze callback execution completed");
        return true;
    }
#ifdef SUPPORT_SCREEN
    panda::ecmascript::EcmaVM* vm = GetVMFromAbility(abilityPtr);
    if (!panda::JSNApi::CheckAndSetAllowCrossThreadExecution(vm)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "VM is in gc process");
        return false;
    }
#endif
    if (this->freezeCallback) {
        this->freezeCallback();
        TAG_LOGW(AAFwkTag::RECOVERY, "Freeze callback execution completed");
    }
    return true;
}

void AppRecovery::SetRestartWant(std::shared_ptr<AAFwk::Want> want)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "begin");
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not enabled");
        return;
    }
    want_ = want;
}

bool AppRecovery::ScheduleRecoverApp(StateReason reason)
{
    TAG_LOGI(AAFwkTag::RECOVERY, "scheduleRecoverApp  begin");
    if (!ShouldRecoverApp(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not recover app");
        return false;
    }
    
    if (!isEnable_) {
        if (OHOS::system::GetParameter("const.dfx.sub_health_recovery.enable", "") == "true") {
            DoRecoverMainApp(reason);
            return true;
        }
        return false;
    }

    if (abilityRecoverys_.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null ability");
        return false;
    }

    if (reason == StateReason::APP_FREEZE) {
        DoRecoverApp(reason);
        return true;
    }

    auto handler = mainHandler_.lock();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null handler");
        return false;
    }

    // may we save state in other thread or just restart.
    // 1. check whether main handler is still avaliable
    // 2. do state saving in main thread or just restart app with no state?
    // 3. create an recovery thread for saving state, just block jsvm mult-thread checking mechaism
    if (getpid() == gettid()) {
        DoRecoverApp(reason);
        return true;
    }
    auto task = [reason]() {
        AppRecovery::GetInstance().DoRecoverApp(reason);
    };
    if (!handler->PostTask(task, "AppRecovery:RecoverApp")) {
        TAG_LOGE(AAFwkTag::RECOVERY, "schedule recover restart app failed");
    }

    return true;
}

bool AppRecovery::TryRecoverApp(StateReason reason)
{
    if (isEnable_) {
        ScheduleSaveAppState(reason);
        DisallowCrossThreadExecutionInRecovery();
        PersistAppState();
    }
    return ScheduleRecoverApp(reason);
}

void AppRecovery::DoRecoverApp(StateReason reason)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "begin");
    if (abilityRecoverys_.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "no ability exist");
        return;
    }
    AAFwk::Want *want = nullptr;
    if (want_ != nullptr) {
        want = want_.get();
    }

    if (abilityRecoverys_.size() == 1) {
        if (abilityRecoverys_.front()->IsOnForeground()) {
            abilityRecoverys_.front()->ScheduleRecoverAbility(reason, want);
            return;
        }
    }

    for (auto itr = abilityRecoverys_.rbegin(); itr != abilityRecoverys_.rend(); itr++) {
        if ((*itr)->IsOnForeground()) {
            (*itr)->ScheduleRecoverAbility(reason, want);
            break;
        }
    }
}

void AppRecovery::DoRecoverMainApp(StateReason reason)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "DoReciverMainApp begin");
    AAFwk::Want *want = &mainWant_;
    want->SetParam(AAFwk::Want::PARAM_ABILITY_RECOVERY_RESTART, false);
    auto token = token_.promote();
    if (token == nullptr) {
        return;
    }
    auto abilityMgr = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null abilityMgr");
        return;
    }
    abilityMgr->ScheduleRecoverAbility(token, reason, want);
}
 
void AppRecovery::DoSaveAppState(StateReason reason, uintptr_t ability)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "begin");
    auto appInfo = applicationInfo_.lock();
    if (appInfo == nullptr || abilityRecoverys_.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not exist application or ability info");
        return;
    }

    bool onlySaveTargetAbility = (ability != 0);
    for (auto& abilityRecoveryRecord : abilityRecoverys_) {
        if (!onlySaveTargetAbility) {
            abilityRecoveryRecord->ScheduleSaveAbilityState(reason);
            TAG_LOGD(AAFwkTag::RECOVERY, "not onlySaveTargetAbility");
            continue;
        }
        if (abilityRecoveryRecord->IsSameAbility(ability)) {
            abilityRecoveryRecord->ScheduleSaveAbilityState(reason);
            TAG_LOGD(AAFwkTag::RECOVERY, "IsSameAbility");
            break;
        }
    }
}

void AppRecovery::EnableAppRecovery(uint16_t restartFlag, uint16_t saveFlag, uint16_t saveMode)
{
    isEnable_ = true;
    restartFlag_ = restartFlag;
    crashFlag_ = restartFlag;
    saveOccasion_ = saveFlag;
    saveMode_ = saveMode;
    useAppSettedValue_.store(true);
}

bool AppRecovery::ShouldSaveAppState(StateReason reason)
{
    bool ret = false;
    switch (reason) {
        case StateReason::DEVELOPER_REQUEST:
            ret = true;
            break;

        case StateReason::LIFECYCLE:
            if ((saveOccasion_ & SaveOccasionFlag::SAVE_WHEN_BACKGROUND) != 0) {
                ret = true;
            }
            break;

        case StateReason::CPP_CRASH:
        case StateReason::JS_ERROR:
        case StateReason::CJ_ERROR:
        case StateReason::APP_FREEZE: // appfreeze could not callback to js function safely.
            if ((saveOccasion_ & SaveOccasionFlag::SAVE_WHEN_ERROR) != 0) {
                ret = true;
            }
            break;
    }
    return ret;
}

bool AppRecovery::ShouldRecoverApp(StateReason reason)
{
    if (restartFlag_ == RestartFlag::NO_RESTART) {
        return false;
    }

    bool ret = false;
    bool isAlwaysStart = false;
    if (restartFlag_ == RestartFlag::ALWAYS_RESTART) {
        isAlwaysStart = true;
    }
    switch (reason) {
        case StateReason::DEVELOPER_REQUEST:
            ret = true;
            break;

        case StateReason::LIFECYCLE:
            ret = false;
            break;

        case StateReason::CPP_CRASH:
            ret = false;
            break;

        case StateReason::JS_ERROR:
            if (isAlwaysStart || (restartFlag_ & RestartFlag::RESTART_WHEN_JS_CRASH) != 0) {
                ret = true;
            }
            break;

        case StateReason::CJ_ERROR:
            if (isAlwaysStart || (restartFlag_ & RestartFlag::RESTART_WHEN_CJ_CRASH) != 0) {
                ret = true;
            }
            break;

        case StateReason::APP_FREEZE:
            if (isAlwaysStart || (restartFlag_ & RestartFlag::RESTART_WHEN_APP_FREEZE) != 0) {
                ret = true;
            }
            break;
    }
    return ret;
}

void AppRecovery::DeleteInValidMissionFiles()
{
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        return;
    }

    std::string fileDir = context->GetFilesDir();
    TAG_LOGI(AAFwkTag::RECOVERY, "fileDir: %{public}s", fileDir.c_str());
    if (fileDir.empty() || !OHOS::FileExists(fileDir)) {
        TAG_LOGD(AAFwkTag::RECOVERY, "empty fileDir or not exist fileDir");
        return;
    }
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;

    if (!GetMissionIds(fileDir, missionIds)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "get mission id failed");
        return;
    }
    if (missionIds.empty()) {
        TAG_LOGD(AAFwkTag::RECOVERY, "missionIds empty");
        return;
    }
    std::shared_ptr<AAFwk::AbilityManagerClient> abilityMgr = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "abilityMgr client is not exist");
        return;
    }
    abilityMgr->IsValidMissionIds(missionIds, results);
    if (results.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "empty results");
        return;
    }
    for (auto& item : results) {
        TAG_LOGD(AAFwkTag::RECOVERY, "missionId: %{public}d, isValid: %{public}d",
            item.missionId, item.isValid);
        if (!item.isValid) {
            DeleteInValidMissionFileById(fileDir, item.missionId);
        }
    }
}

void AppRecovery::DeleteInValidMissionFileById(std::string fileDir, int32_t missionId)
{
    std::string fileName = std::to_string(missionId) + ".state";
    std::string file = fileDir + "/" + fileName;
    bool ret = OHOS::RemoveFile(file);
    if (!ret) {
        TAG_LOGE(AAFwkTag::RECOVERY, "file: %{public}s failed", file.c_str());
    }
}

void AppRecovery::ClearPageStack(std::string bundleName)
{
    DeleteInValidMissionFiles();
    std::shared_ptr<AAFwk::AbilityManagerClient> abilityMgr = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null abilityMgr");
        return;
    }
    abilityMgr->ScheduleClearRecoveryPageStack();
}

bool AppRecovery::GetMissionIds(std::string path, std::vector<int32_t> &missionIds)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null dir");
        return false;
    }
    struct dirent *ptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::RECOVERY, "null ptr");
            closedir(dir);
            return false;
        }
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (ptr->d_type == DT_REG) {
            std::string fileName = ptr->d_name;
            auto pos = fileName.find_first_of(".");
            if (pos != std::string::npos) {
                std::string missionIdStr = fileName.substr(0, pos);
                missionIds.push_back(atoi(missionIdStr.c_str()));
            }
        } else {
            continue;
        }
    }
    closedir(dir);
    return true;
}

bool AppRecovery::PersistAppState()
{
    if (saveMode_ == SaveModeFlag::SAVE_WITH_FILE) {
        return true;
    }

    bool ret = true;
    for (auto& abilityRecovery : abilityRecoverys_) {
        ret = ret && abilityRecovery->PersistState();
    }
    return ret;
}

bool AppRecovery::IsEnabled() const
{
    return isEnable_;
}

uint16_t AppRecovery::GetRestartFlag() const
{
    return restartFlag_;
}

uint16_t AppRecovery::GetSaveOccasionFlag() const
{
    return saveOccasion_;
}

uint16_t AppRecovery::GetSaveModeFlag() const
{
    return saveMode_;
}

bool AppRecovery::IsEtsAPP()
{
    TAG_LOGD(AAFwkTag::RECOVERY, "begin");
    auto appInfo = applicationInfo_.lock();
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not exist application info");
        return true;
    }
    return appInfo->arkTSMode == OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2 ||
        appInfo->arkTSMode == OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_HYBRID;
}

panda::ecmascript::EcmaVM* AppRecovery::GetVMFromAbility(const std::shared_ptr<AbilityRuntime::UIAbility>& abilityPtr)
{
    if (!abilityPtr) {
        return nullptr;
    }
    OHOS::AbilityRuntime::JsUIAbility& jsAbility = static_cast<AbilityRuntime::JsUIAbility&>(*abilityPtr);
    AbilityRuntime::JsRuntime& runtime = const_cast<AbilityRuntime::JsRuntime&>(jsAbility.GetJsRuntime());
    return runtime.GetEcmaVm();
}

void AppRecovery::DisallowCrossThreadExecutionInRecovery()
{
#ifdef SUPPORT_SCREEN
    auto abilityPtr = ability_.lock();
    if (!abilityPtr || !abilityPtr->GetAbilityContext()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null ability or context");
        return;
    }
    panda::ecmascript::EcmaVM* vm = GetVMFromAbility(abilityPtr);
    if (vm != nullptr) {
        panda::JSNApi::DisallowCrossThreadExecution(vm);
    }
#endif
}
}  // namespace AbilityRuntime
}  // namespace OHOS
