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
#include "hisysevent_c.h"
#include "hisysevent_report.h"
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
constexpr size_t DEFAULT_RECOVERY_MAX_RESTORE_SIZE = 400 * 1024;
constexpr int32_t CALL_BACK_ERROR = -1;

// Report APP_RECOVERY hisysevent on ScheduleRestoreAbilityState success.
// APP_RECOVERY is the cloud-uploaded event also used by system-side
// AbilityManagerService::ReportAppRecoverResult. To keep cloud-side filtering unambiguous,
// the app-side RESTORE_RESULT values are prefixed with "SCHEDULE_RESTORE_" (mirroring the
// reporting function name) so they never collide with system-side result strings (SUCCESS /
// FAIL_*). Filter: WHERE EVENT_NAME='APP_RECOVERY' AND RECOVERY_RESULT LIKE 'SCHEDULE_RESTORE_%'.
constexpr const char* RECOVERY_EVENT_DOMAIN = "AAFWK";
constexpr const char* RECOVERY_EVENT_NAME = "APP_RECOVERY";
constexpr const char* RESTORE_RESULT_SUCCESS = "SCHEDULE_RESTORE_SUCCESS";

// Report APP_RECOVERY hisysevent on ScheduleRestoreAbilityState success.
// Field semantics are aligned with AbilityManagerService::ReportAppRecoverResult.
static void ReportRestoreAbilityStateResult(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::string &result)
{
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null abilityInfo");
        return;
    }
    auto report = std::make_shared<AAFwk::HisyseventReport>(6);
    report->InsertParam("APP_UID", static_cast<int32_t>(abilityInfo->applicationInfo.uid));
    report->InsertParam("VERSION_CODE", std::to_string(abilityInfo->applicationInfo.versionCode));
    report->InsertParam("VERSION_NAME", abilityInfo->applicationInfo.versionName);
    report->InsertParam("BUNDLE_NAME", abilityInfo->bundleName);
    report->InsertParam("ABILITY_NAME", abilityInfo->name);
    report->InsertParam("RECOVERY_RESULT", result);
    int32_t ret = report->Report(RECOVERY_EVENT_DOMAIN, RECOVERY_EVENT_NAME, HISYSEVENT_BEHAVIOR);
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::RECOVERY, "report APP_RECOVERY failed, ret=%{public}d, result=%{public}s",
            ret, result.c_str());
    }
}

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

void AbilityRecovery::SaveStateCallback(AppExecFwk::OnSaveStateResult result)
{
    if (result.status != AppExecFwk::OnSaveResult::ALL_AGREE &&
        result.status != AppExecFwk::OnSaveResult::RECOVERY_AGREE) {
        TAG_LOGE(AAFwkTag::RECOVERY, "save params failed");
        return;
    }
    auto ability = ability_.lock();
    if (ability == nullptr) {
        return;
    }
#ifdef SUPPORT_SCREEN
    std::string pageStack = DefaultRecovery() ? ability->GetContentInfoForDefaultRecovery() :
        ability->GetContentInfoForRecovery();
    if (!pageStack.empty()) {
        result.wantParams.SetParam("pageStack", AAFwk::String::Box(pageStack));
    } else {
        TAG_LOGE(AAFwkTag::RECOVERY, "get page stack failed");
    }
    TAG_LOGD(AAFwkTag::RECOVERY, "pageStack size: %{public}zu", pageStack.size());
#endif
    if (saveMode_ == SaveModeFlag::SAVE_WITH_FILE) {
        SerializeDataToFile(missionId_, result.wantParams);
    } else if (saveMode_ == SaveModeFlag::SAVE_WITH_SHARED_MEMORY) {
        params_ = result.wantParams;
    }
    auto token = token_.promote();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null token");
        return;
    }
    std::shared_ptr<AAFwk::AbilityManagerClient> abilityMgr = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null abilityMgr");
        return;
    }
    abilityMgr->EnableRecoverAbility(token);
    if (result.reason == StateReason::LIFECYCLE && DefaultRecovery()) {
        TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery ScheduleSaveAbilityState SubmitSaveRecoveryInfo");
        abilityMgr->SubmitSaveRecoveryInfo(token);
    }
}

bool AbilityRecovery::SaveAbilityState(StateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto ability = ability_.lock();
    auto abilityInfo = abilityInfo_.lock();
    if (ability == nullptr || abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null ability");
        return false;
    }

    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Create();
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo");
        return false;
    }

    auto weak = std::weak_ptr<AbilityRecovery>(shared_from_this());
    auto callback = [weak](AppExecFwk::OnSaveStateResult result) {
        auto recovery = weak.lock();
        if (recovery != nullptr) {
            recovery->SaveStateCallback(result);
        }
    };
    callbackInfo->Push(callback);
    AAFwk::WantParams wantParams;
    bool isAsync = false;
    int32_t status = ability->OnSaveState(AppExecFwk::StateType::APP_RECOVERY,
        wantParams, callbackInfo, isAsync, reason);
    if (status == CALL_BACK_ERROR) {
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult>::Destroy(callbackInfo);
    }
    return true;
}

bool AbilityRecovery::SerializeDataToFile(int32_t savedStateId, WantParams& params)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string file = GetSaveAppCachePath(savedStateId);
    if (file.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "persisted file path failed");
        return false;
    }
    Parcel parcel;
    if (!params.Marshalling(parcel)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "Marshalling want param failed");
        return false;
    }

    FILE *fileF = fopen(file.c_str(), "w+");
    if (fileF == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "errno: %{public}d", errno);
        return false;
    }
    size_t sz = parcel.GetDataSize();
    uintptr_t buf = parcel.GetData();
    if (sz == 0 || buf == 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "get parcel data failed");
        fclose(fileF);
        return false;
    }

    if (DefaultRecovery() && (sz > DEFAULT_RECOVERY_MAX_RESTORE_SIZE)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "data is too large, size: %{public}zu", sz);
        fclose(fileF);
        return false;
    }

    size_t nwrite = fwrite(reinterpret_cast<uint8_t*>(buf), 1, sz, fileF);
    if (ferror(fileF) != 0 || (nwrite != sz)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "persist parcel data failed %{public}d, wrote %{public}zu bytes "
            "instead of %{public}zu", errno, nwrite, sz);
    }
    TAG_LOGD(AAFwkTag::RECOVERY, "file size: %{public}zu", sz);
    fclose(fileF);
    return true;
}

bool AbilityRecovery::ReadSerializeDataFromFile(int32_t savedStateId, WantParams& params)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string file = GetSaveAppCachePath(savedStateId);
    if (file.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "persisted file path failed");
        return false;
    }

    TAG_LOGD(AAFwkTag::RECOVERY, "file path %{public}s", file.c_str());
    char path[PATH_MAX] = {0};
    if (realpath(file.c_str(), path) == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "errno is %{public}d", errno);
        return false;
    }

    FILE *fileF = fopen(path, "r");
    if (fileF == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "file open err: %{public}d", errno);
        remove(path);
        return false;
    }
    int fd = fileno(fileF);
    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        fclose(fileF);
        remove(path);
        return false;
    }

    auto mapFile = static_cast<uint8_t*>(mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (mapFile == MAP_FAILED) {
        fclose(fileF);
        remove(path);
        return false;
    }

    Parcel parcel(new AppRecoveryParcelAllocator()); // do not dealloc mmap area
    if (!parcel.ParseFrom(reinterpret_cast<uintptr_t>(mapFile), statbuf.st_size)) {
        munmap(mapFile, statbuf.st_size);
        fclose(fileF);
        remove(path);
        return false;
    }

    auto parsedParam = WantParams::Unmarshalling(parcel);
    if (parsedParam != nullptr) {
        params = *parsedParam;
        delete parsedParam;
    } else {
        munmap(mapFile, statbuf.st_size);
        fclose(fileF);
        remove(path);
        return false;
    }

    munmap(mapFile, statbuf.st_size);
    fclose(fileF);
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
        TAG_LOGE(AAFwkTag::RECOVERY, "invalid missionId_");
        return false;
    }

    if (!IsSaveAbilityState(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "not save ability state");
        return false;
    }

    return SaveAbilityState(reason);
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
        TAG_LOGE(AAFwkTag::RECOVERY, "null abilityMgr");
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
        TAG_LOGE(AAFwkTag::RECOVERY, "null abilityInfo");
        return false;
    }
    if (missionId_ <= 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "invalid missionId");
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
        TAG_LOGE(AAFwkTag::RECOVERY, "null abilityInfo");
        return false;
    }

    if (hasTryLoad_) {
        return hasLoaded_;
    }
    if (missionId_ <= 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "invalid missionId_");
        return false;
    }
    hasTryLoad_ = true;

    TAG_LOGD(AAFwkTag::RECOVERY, "missionId_:%{public}d", missionId_);
    if (!ReadSerializeDataFromFile(missionId_, params_)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "find record for id:%{public}d failed", missionId_);
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
    auto abilityInfo = abilityInfo_.lock();
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
    ReportRestoreAbilityStateResult(abilityInfo, RESTORE_RESULT_SUCCESS);
    return true;
}

std::string AbilityRecovery::GetSavedPageStack(StateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!LoadSavedState(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "no saved state");
        return "";
    }

    if (pageStack_.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "empty pageStack_");
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