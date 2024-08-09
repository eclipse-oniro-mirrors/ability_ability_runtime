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

#include "ability_recovery.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ability_manager_client.h"
#include "app_recovery_parcel_allocator.h"
#include "context/application_context.h"
#include "file_ex.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "parcel.h"
#include "recovery_param.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr size_t DEFAULT_RECOVERY_MAX_RESTORE_SIZE = 10 * 1024;

static std::string GetSaveAppCachePath(int32_t savedStateId)
{
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        return "";
    }

    std::string fileDir = context->GetFilesDir();
    TAG_LOGD(AAFwkTag::RECOVERY, "fileDir %{public}s", fileDir.c_str());
    if (fileDir.empty() || !OHOS::FileExists(fileDir)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "empty fileDir or fileDir not exist");
        return "";
    }

    std::string fileName = std::to_string(savedStateId) + ".state";
    return fileDir + "/" + fileName;
}
}

AbilityRecovery::AbilityRecovery() : isEnable_(false), restartFlag_(RestartFlag::ALWAYS_RESTART),
    saveOccasion_(SaveOccasionFlag::SAVE_WHEN_ERROR), saveMode_(SaveModeFlag::SAVE_WITH_FILE)
{
}

AbilityRecovery::~AbilityRecovery()
{
}

bool AbilityRecovery::InitAbilityInfo(const std::shared_ptr<AbilityRuntime::UIAbility> ability,
    const std::shared_ptr<AbilityInfo>& abilityInfo, const sptr<IRemoteObject>& token)
{
    isEnable_ = true;
    ability_ = ability;
    abilityInfo_ = abilityInfo;
    token_ = token;
    auto abilityContext = ability->GetAbilityContext();
    if (abilityContext != nullptr) {
        abilityContext->GetMissionId(missionId_);
    }
    return true;
}

void AbilityRecovery::EnableAbilityRecovery(bool useAppSettedValue, uint16_t restartFlag, uint16_t saveFlag,
    uint16_t saveMode)
{
    isEnable_ = true;
    restartFlag_ = restartFlag;
    useAppSettedValue_.store(useAppSettedValue);
    saveOccasion_ = useAppSettedValue ? saveFlag : SaveOccasionFlag::SAVE_WHEN_BACKGROUND;
    saveMode_ = saveMode;
}

bool AbilityRecovery::IsSameAbility(uintptr_t ability)
{
    return ability == jsAbilityPtr_;
}

void AbilityRecovery::SetJsAbility(uintptr_t ability)
{
    jsAbilityPtr_ = ability;
}

bool AbilityRecovery::SaveAbilityState()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto ability = ability_.lock();
    auto abilityInfo = abilityInfo_.lock();
    if (ability == nullptr || abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "ability is nullptr");
        return false;
    }

    AAFwk::WantParams wantParams;
    int32_t status = ability->OnSaveState(AppExecFwk::StateType::APP_RECOVERY, wantParams);
    if (!(status == AppExecFwk::OnSaveResult::ALL_AGREE || status == AppExecFwk::OnSaveResult::RECOVERY_AGREE)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "Failed save params");
        return false;
    }

#ifdef SUPPORT_SCREEN
    std::string pageStack = DefaultRecovery() ? ability->GetContentInfoForDefaultRecovery() :
        ability->GetContentInfoForRecovery();
    if (!pageStack.empty()) {
        wantParams.SetParam("pageStack", AAFwk::String::Box(pageStack));
    } else {
        TAG_LOGE(AAFwkTag::RECOVERY, "Failed to get page stack");
    }
    TAG_LOGD(AAFwkTag::RECOVERY, "pageStack size: %{public}zu", pageStack.size());
#endif
    if (saveMode_ == SaveModeFlag::SAVE_WITH_FILE) {
        SerializeDataToFile(missionId_, wantParams);
    } else if (saveMode_ == SaveModeFlag::SAVE_WITH_SHARED_MEMORY) {
        params_ = wantParams;
    }
    return true;
}

bool AbilityRecovery::SerializeDataToFile(int32_t savedStateId, WantParams& params)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string file = GetSaveAppCachePath(savedStateId);
    if (file.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "failed to persisted file path");
        return false;
    }
    Parcel parcel;
    if (!params.Marshalling(parcel)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "failed to Marshalling want param");
        return false;
    }
    int fd = open(file.c_str(), O_RDWR | O_CREAT, (mode_t)0600);
    if (fd <= 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "failed to open %{public}s", file.c_str());
        return false;
    }
    size_t sz = parcel.GetDataSize();
    uintptr_t buf = parcel.GetData();
    if (sz == 0 || buf == 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "failed to get parcel data");
        close(fd);
        return false;
    }

    if (DefaultRecovery() && (sz > DEFAULT_RECOVERY_MAX_RESTORE_SIZE)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "data is too large, size: %{public}zu", sz);
        close(fd);
        return false;
    }

    ssize_t nwrite = write(fd, reinterpret_cast<uint8_t*>(buf), sz);
    if (nwrite <= 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "failed to persist parcel data %{public}d", errno);
    }
    TAG_LOGD(AAFwkTag::RECOVERY, "file size: %{public}zu", sz);
    close(fd);
    return true;
}

bool AbilityRecovery::ReadSerializeDataFromFile(int32_t savedStateId, WantParams& params)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string file = GetSaveAppCachePath(savedStateId);
    if (file.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "failed to persisted file path");
        return false;
    }

    TAG_LOGD(AAFwkTag::RECOVERY, "file path %{public}s", file.c_str());
    char path[PATH_MAX] = {0};
    if (realpath(file.c_str(), path) == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "errno is %{public}d", errno);
        return false;
    }

    int32_t fd = open(path, O_RDONLY);
    if (fd <= 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "fd open error");
        remove(path);
        return false;
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        close(fd);
        remove(path);
        return false;
    }

    auto mapFile = static_cast<uint8_t*>(mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (mapFile == MAP_FAILED) {
        close(fd);
        remove(path);
        return false;
    }

    Parcel parcel(new AppRecoveryParcelAllocator()); // do not dealloc mmap area
    if (!parcel.ParseFrom(reinterpret_cast<uintptr_t>(mapFile), statbuf.st_size)) {
        munmap(mapFile, statbuf.st_size);
        close(fd);
        remove(path);
        return false;
    }

    auto parsedParam = WantParams::Unmarshalling(parcel);
    if (parsedParam != nullptr) {
        params = *parsedParam;
        delete parsedParam;
    } else {
        munmap(mapFile, statbuf.st_size);
        close(fd);
        remove(path);
        return false;
    }

    munmap(mapFile, statbuf.st_size);
    close(fd);
    remove(path);
    return true;
}

bool AbilityRecovery::ScheduleSaveAbilityState(StateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not enable");
        return false;
    }

    if (missionId_ <= 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not save ability missionId_ is invalid");
        return false;
    }

    if (!IsSaveAbilityState(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not save ability state");
        return false;
    }

    bool ret = SaveAbilityState();
    if (ret) {
        auto token = token_.promote();
        if (token == nullptr) {
            TAG_LOGE(AAFwkTag::RECOVERY, "token is nullptr");
            return false;
        }

        std::shared_ptr<AAFwk::AbilityManagerClient> abilityMgr = AAFwk::AbilityManagerClient::GetInstance();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::RECOVERY, "abilityMgr client is not exist");
            return false;
        }
        abilityMgr->EnableRecoverAbility(token);
        if (reason == StateReason::LIFECYCLE && DefaultRecovery()) {
            TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery ScheduleSaveAbilityState SubmitSaveRecoveryInfo");
            abilityMgr->SubmitSaveRecoveryInfo(token);
        }
    }
    return ret;
}

bool AbilityRecovery::ScheduleRecoverAbility(StateReason reason, const Want *want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not enable");
        return false;
    }

    std::shared_ptr<AAFwk::AbilityManagerClient> abilityMgr = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "abilityMgr client is not exist");
        return false;
    }

    auto token = token_.promote();
    if (token == nullptr) {
        return false;
    }
    abilityMgr->ScheduleRecoverAbility(token, reason, want);
    return true;
}

bool AbilityRecovery::PersistState()
{
    auto abilityInfo = abilityInfo_.lock();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "ability is nullptr");
        return false;
    }
    if (missionId_ <= 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "missionId is Invalid");
        return false;
    }
    if (!params_.IsEmpty()) {
        SerializeDataToFile(missionId_, params_);
    }
    return true;
}

bool AbilityRecovery::IsOnForeground()
{
    auto ability = ability_.lock();
    if (ability == nullptr) {
        return false;
    }
    AbilityLifecycleExecutor::LifecycleState state = ability->GetState();
    TAG_LOGI(AAFwkTag::RECOVERY, "state: %{public}d", state);
    if (state == AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW) {
        return true;
    }
    return false;
}

bool AbilityRecovery::LoadSavedState(StateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityInfo = abilityInfo_.lock();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "abilityInfo is nullptr");
        return false;
    }

    if (hasTryLoad_) {
        return hasLoaded_;
    }
    if (missionId_ <= 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "missionId_ is invalid");
        return false;
    }
    hasTryLoad_ = true;

    TAG_LOGD(AAFwkTag::RECOVERY, "missionId_:%{public}d", missionId_);
    if (!ReadSerializeDataFromFile(missionId_, params_)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "failed to find record for id:%{public}d", missionId_);
        hasLoaded_ = false;
        return hasLoaded_;
    }

    auto stringObj = AAFwk::IString::Query(params_.GetParam("pageStack"));
    if (stringObj != nullptr) {
        pageStack_ = AAFwk::String::Unbox(stringObj);
    }
    hasLoaded_ = true;
    return hasLoaded_;
}

bool AbilityRecovery::ScheduleRestoreAbilityState(StateReason reason, const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not enable");
        return false;
    }

    if (!IsSaveAbilityState(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not save ability state");
        return false;
    }

    if (!LoadSavedState(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "no saved state ");
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    WantParams &wantCurrent = const_cast<WantParams&>(wantParams);
    for (auto& i : params_.GetParams()) {
        wantCurrent.SetParam(i.first, i.second.GetRefPtr());
    }
    return true;
}

std::string AbilityRecovery::GetSavedPageStack(StateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!LoadSavedState(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "no saved state ");
        return "";
    }

    if (pageStack_.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "pageStack_ is empty");
    }
    return pageStack_;
}

bool AbilityRecovery::IsSaveAbilityState(StateReason reason)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "enter");
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
        case StateReason::APP_FREEZE:
            if ((saveOccasion_ & SaveOccasionFlag::SAVE_WHEN_ERROR) != 0) {
                ret = true;
            }
            break;

        default:
            ret = false;
            break;
    }
    return ret;
}

uint16_t AbilityRecovery::GetRestartFlag() const
{
    return restartFlag_;
}

uint16_t AbilityRecovery::GetSaveOccasionFlag() const
{
    return saveOccasion_;
}

uint16_t AbilityRecovery::GetSaveModeFlag() const
{
    return saveMode_;
}

bool AbilityRecovery::DefaultRecovery() const
{
    return !(useAppSettedValue_.load());
}
}  // namespace AbilityRuntime
}  // namespace OHOS