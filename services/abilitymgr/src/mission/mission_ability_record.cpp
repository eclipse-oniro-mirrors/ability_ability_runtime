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

#include "mission_ability_record.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "configuration_convertor.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "mission_info_mgr.h"
#include "permission_verification.h"
#ifdef SUPPORT_SCREEN
#include "image_source.h"
#include "locale_config.h"
#endif

namespace OHOS {
namespace AAFwk {
using namespace AbilityRuntime::GlobalConstant;
namespace {
const std::string NEED_STARTINGWINDOW = "ohos.ability.NeedStartingWindow";
constexpr uint32_t RELEASE_STARTING_BG_TIMEOUT = 15000; // release starting window resource timeout.
}
MissionAbilityRecord::MissionAbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode)
    : AbilityRecord(want, abilityInfo, applicationInfo, requestCode) {}

std::shared_ptr<MissionAbilityRecord> MissionAbilityRecord::CreateAbilityRecord(const AbilityRequest &abilityRequest)
{
    auto abilityRecord = std::make_shared<MissionAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->Init(abilityRequest);
    return abilityRecord;
}

MissionAbilityRecordPtr MissionAbilityRecord::FromBaseRecord(std::shared_ptr<AbilityRecord> abilityRecord)
{
    if (abilityRecord == nullptr || abilityRecord->GetAbilityRecordType() != AbilityRecordType::MISSION_ABILITY) {
        return nullptr;
    }
    return std::static_pointer_cast<MissionAbilityRecord>(abilityRecord);
}

AbilityRecordType MissionAbilityRecord::GetAbilityRecordType()
{
    return AbilityRecordType::MISSION_ABILITY;
}

void MissionAbilityRecord::Dump(std::vector<std::string> &info)
{
    std::string dumpInfo = "      AbilityRecord ID #" + std::to_string(recordId_);
    info.push_back(dumpInfo);
    dumpInfo = "        app name [" + GetAbilityInfo().applicationName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        main name [" + GetAbilityInfo().name + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        bundle name [" + GetAbilityInfo().bundleName + "]";
    info.push_back(dumpInfo);
    std::string isKeepAlive = GetKeepAlive() ? "true" : "false";
    dumpInfo = "        isKeepAlive: " + isKeepAlive;
    info.push_back(dumpInfo);
    // get ability type(unknown/page/service/provider)
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);
    info.push_back(DumpPreAbility());
    info.push_back(DumpNextAbility());
    dumpInfo = "        state #" + AbilityRecord::ConvertAbilityState(GetAbilityState()) + "  start time [" +
               std::to_string(startTime_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        app state #" + AbilityRecord::ConvertAppState(appState_);
    info.push_back(dumpInfo);
    dumpInfo = "        ready #" + std::to_string(isReady_) + "  window attached #" +
               std::to_string(isWindowAttached_) + "  launcher #" + std::to_string(isLauncherAbility_);
    info.push_back(dumpInfo);

    if (isLauncherRoot_) {
        dumpInfo = "        can restart num #" + std::to_string(restartCount_);
        info.push_back(dumpInfo);
    }
}

std::string MissionAbilityRecord::DumpPreAbility() const
{
    std::string dumpInfo;
    auto preAbility = GetPreAbilityRecord();
    if (preAbility == nullptr) {
        dumpInfo = "        previous ability app name [NULL]";
        dumpInfo.append("\n");
        dumpInfo += "        previous ability file name [NULL]";
    } else {
        dumpInfo =
            "        previous ability app name [" + preAbility->GetAbilityInfo().applicationName + "]";
        dumpInfo.append("\n");
        dumpInfo += "        previous ability file name [" + preAbility->GetAbilityInfo().name + "]";
    }
    return dumpInfo;
}

std::string MissionAbilityRecord::DumpNextAbility() const
{
    std::string dumpInfo;
    auto nextAbility = GetNextAbilityRecord();
    if (nextAbility == nullptr) {
        dumpInfo = "        next ability app name [NULL]";
        dumpInfo.append("\n");
        dumpInfo += "        next ability file name [NULL]";
    } else {
        dumpInfo =
            "        next ability app name [" + nextAbility->GetAbilityInfo().applicationName + "]";
        dumpInfo.append("\n");
        dumpInfo += "        next ability main name [" + nextAbility->GetAbilityInfo().name + "]";
    }
    return dumpInfo;
}

void MissionAbilityRecord::SetAbilityForegroundingFlag()
{
    isAbilityForegrounding_ = true;
    DelayedSingleton<AppScheduler>::GetInstance()->SetAbilityForegroundingFlagToAppRecord(pid_);
}

std::string MissionAbilityRecord::GetLabel()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::string strLabel = abilityInfo_.applicationInfo.label;

    if (abilityInfo_.resourcePath.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "resource path empty");
        return strLabel;
    }

#ifdef SUPPORT_SCREEN
    auto resourceMgr = CreateResourceManager();
    if (!resourceMgr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "CreateResourceManager empty");
        return strLabel;
    }

    auto result = resourceMgr->GetStringById(abilityInfo_.applicationInfo.labelId, strLabel);
    if (result != OHOS::Global::Resource::RState::SUCCESS) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, fail", __func__);
    }

    InitColdStartingWindowResource(resourceMgr);
#endif

    return strLabel;
}

#ifdef SUPPORT_SCREEN
void MissionAbilityRecord::NotifyAnimationFromMinimizeAbility(bool& animaEnabled)
{
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get WMS failed");
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Notify Animation From MinimizeAbility");
    sptr<AbilityTransitionInfo> fromInfo = new AbilityTransitionInfo();
    SetAbilityTransitionInfo(fromInfo);
    fromInfo->reason_ = TransitionReason::MINIMIZE;
    windowHandler->NotifyWindowTransition(fromInfo, nullptr, animaEnabled);
}

void MissionAbilityRecord::ProcessForegroundAbility(const std::shared_ptr<AbilityRecord> &callerAbility, bool needExit,
    uint32_t sceneFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SUPPORT_GRAPHICS: ability record: %{public}s/%{public}s",
        GetBundleName().c_str(), GetAbilityName().c_str());

    StartingWindowHot();
    auto flag = !IsForeground();
    NotifyAnimationFromTerminatingAbility(callerAbility, needExit, flag);
    PostCancelStartingWindowHotTask();

    PostForegroundTimeoutTask();
    if (IsAbilityState(AbilityState::FOREGROUND)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Activate %{public}s/%{public}s",
            GetBundleName().c_str(), GetAbilityName().c_str());
        ForegroundAbility(sceneFlag);
    } else {
        // background to active state
        TAG_LOGD(AAFwkTag::ABILITYMGR, "MoveToForeground, %{public}s/%{public}s",
            GetBundleName().c_str(), GetAbilityName().c_str());
        lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token_);
    }
}

void MissionAbilityRecord::ProcessForegroundAbility(bool isRecent, const AbilityRequest &abilityRequest,
    std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<AbilityRecord> &callerAbility,
    uint32_t sceneFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SUPPORT_GRAPHICS: ability record: %{public}s/%{public}s",
        GetBundleName().c_str(), GetAbilityName().c_str());
#ifdef SUPPORT_UPMS
    {
        std::lock_guard guard(wantLock_);
        GrantUriPermission(want_, abilityInfo_.applicationInfo.bundleName, false, 0, false);
    }
#endif // SUPPORT_UPMS

    if (isReady_ && !GetRestartAppFlag()) {
        auto handler = AbilityManagerService::GetPubInstance()->GetTaskHandler();
        if (!handler) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "fail get AbilityEventHandler");
            return;
        }
        auto taskName = std::to_string(missionId_) + "_hot";
        handler->CancelTask(taskName);

        StartingWindowTask(isRecent, !isWindowStarted_, abilityRequest, startOptions);
        AnimationTask(isRecent, abilityRequest, startOptions, callerAbility);
        if (isWindowStarted_) {
            PostCancelStartingWindowHotTask();
        } else {
            PostCancelStartingWindowColdTask();
        }
        PostForegroundTimeoutTask();
        if (IsAbilityState(AbilityState::FOREGROUND)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Activate %{public}s/%{public}s",
                GetBundleName().c_str(), GetAbilityName().c_str());
            ForegroundAbility(sceneFlag);
        } else {
            // background to active state
            TAG_LOGD(AAFwkTag::ABILITYMGR, "MoveToForeground, %{public}s/%{public}s",
                GetBundleName().c_str(), GetAbilityName().c_str());
            lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token_);
        }
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "SUPPORT_GRAPHICS: to load ability.");
        lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
        auto isSaCall = PermissionVerification::GetInstance()->IsSACall();
        auto needStartingWindow = abilityRequest.want.GetBoolParam(NEED_STARTINGWINDOW, true);
        if (!isSaCall || needStartingWindow) {
            StartingWindowTask(isRecent, true, abilityRequest, startOptions);
            AnimationTask(isRecent, abilityRequest, startOptions, callerAbility);
            PostCancelStartingWindowColdTask();
        }
        LoadAbility();
    }
}

void MissionAbilityRecord::PostCancelStartingWindowHotTask()
{
    if (IsDebug()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "debug mode, just return");
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    auto handler = AbilityManagerService::GetPubInstance()->GetTaskHandler();
    CHECK_POINTER_LOG(handler, "Fail to get TaskHandler.");

    auto windowHandler = GetWMSHandler();
    CHECK_POINTER_LOG(windowHandler, "PostCancelStartingWindowColdTask, Get WMS handler failed.");

    auto abilityRecord(shared_from_this());
    auto delayTask = [windowHandler, abilityRecord] {
        if (windowHandler && abilityRecord && abilityRecord->IsStartingWindow() &&
            abilityRecord->GetAbilityState() != AbilityState::FOREGROUNDING) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "PostCancelStartingWindowHotTask, call windowHandler CancelStartingWindow.");
            windowHandler->CancelStartingWindow(abilityRecord->GetToken());
            abilityRecord->SetStartingWindow(false);
        }
    };
    auto taskName = std::to_string(missionId_) + "_hot";
    int foregroundTimeout =
        AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * FOREGROUND_TIMEOUT_MULTIPLE;
    handler->SubmitTask(delayTask, taskName, foregroundTimeout);
}

void MissionAbilityRecord::NotifyAnimationFromTerminatingAbility(const std::shared_ptr<AbilityRecord>& callerAbility,
    bool needExit, bool flag)
{
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get WMS failed");
        return;
    }

    sptr<AbilityTransitionInfo> fromInfo = new AbilityTransitionInfo();
    if (callerAbility) {
        auto callerAbilityInfo = callerAbility->GetAbilityInfo();
        SetAbilityTransitionInfo(callerAbilityInfo, fromInfo);
        fromInfo->abilityToken_ = callerAbility->GetToken();
    }

    if (flag && needExit) {
        fromInfo->reason_ = TransitionReason::BACK_TRANSITION;
    } else if (flag && !needExit) {
        fromInfo->reason_ = TransitionReason::BACKGROUND_TRANSITION;
    } else {
        fromInfo->reason_ = TransitionReason::CLOSE;
    }

    auto toInfo = CreateAbilityTransitionInfo();
    SetAbilityTransitionInfo(abilityInfo_, toInfo);
    bool animaEnabled = false;
    windowHandler->NotifyWindowTransition(fromInfo, toInfo, animaEnabled);
}

void MissionAbilityRecord::NotifyAnimationFromTerminatingAbility() const
{
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get WMS failed");
        return;
    }

    sptr<AbilityTransitionInfo> fromInfo = new AbilityTransitionInfo();
    SetAbilityTransitionInfo(fromInfo);
    fromInfo->reason_ = TransitionReason::CLOSE;
    bool animaEnabled = false;
    windowHandler->NotifyWindowTransition(fromInfo, nullptr, animaEnabled);
}

void MissionAbilityRecord::AnimationTask(bool isRecent, const AbilityRequest &abilityRequest,
    const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<AbilityRecord> &callerAbility)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (isRecent) {
        auto want = GetWantFromMission();
        NotifyAnimationFromRecentTask(startOptions, want);
    } else if (!IsForeground()) {
        NotifyAnimationFromStartingAbility(callerAbility, abilityRequest);
    }
}

void MissionAbilityRecord::NotifyAnimationFromRecentTask(const std::shared_ptr<StartOptions> &startOptions,
    const std::shared_ptr<Want> &want) const
{
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, failed", __func__);
        return;
    }

    auto toInfo = CreateAbilityTransitionInfo(startOptions, want);
    toInfo->abilityToken_ = token_;
    toInfo->missionId_ = missionId_;
    SetAbilityTransitionInfo(abilityInfo_, toInfo);
    sptr<AbilityTransitionInfo> fromInfo = new AbilityTransitionInfo();
    fromInfo->isRecent_ = true;
    bool animaEnabled = false;
    windowHandler->NotifyWindowTransition(fromInfo, toInfo, animaEnabled);
}

void MissionAbilityRecord::NotifyAnimationFromStartingAbility(const std::shared_ptr<AbilityRecord> &callerAbility,
    const AbilityRequest &abilityRequest) const
{
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, failed", __func__);
        return;
    }

    sptr<AbilityTransitionInfo> fromInfo = new AbilityTransitionInfo();
    if (callerAbility) {
        auto callerAbilityInfo = callerAbility->GetAbilityInfo();
        SetAbilityTransitionInfo(callerAbilityInfo, fromInfo);
        fromInfo->abilityToken_ = callerAbility->GetToken();
    } else {
        fromInfo->abilityToken_ = abilityRequest.callerToken;
    }

    auto toInfo = CreateAbilityTransitionInfo(abilityRequest);
    toInfo->abilityToken_ = token_;
    toInfo->missionId_ = missionId_;
    SetAbilityTransitionInfo(abilityInfo_, toInfo);
    bool animaEnabled = false;
    windowHandler->NotifyWindowTransition(fromInfo, toInfo, animaEnabled);
}

void MissionAbilityRecord::StartingWindowTask(bool isRecent, bool isCold, const AbilityRequest &abilityRequest,
    std::shared_ptr<StartOptions> &startOptions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (isRecent) {
        auto want = GetWantFromMission();
        if (isCold) {
            StartingWindowCold(startOptions, want, abilityRequest);
        } else {
            StartingWindowHot(startOptions, want, abilityRequest);
        }
    } else {
        std::shared_ptr<Want> want = nullptr;
        if (isCold) {
            StartingWindowCold(startOptions, want, abilityRequest);
        } else {
            StartingWindowHot(startOptions, want, abilityRequest);
        }
    }
}

void MissionAbilityRecord::PostCancelStartingWindowColdTask()
{
    if (IsDebug()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "debug mode, just return");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto handler = AbilityManagerService::GetPubInstance()->GetTaskHandler();
    CHECK_POINTER_LOG(handler, "Fail to get TaskHandler.");

    auto windowHandler = GetWMSHandler();
    CHECK_POINTER_LOG(windowHandler, "PostCancelStartingWindowColdTask, Get WMS handler failed.");

    auto abilityRecord(shared_from_this());
    auto delayTask = [windowHandler, abilityRecord] {
        if (windowHandler && abilityRecord && abilityRecord->IsStartingWindow() &&
            (abilityRecord->GetScheduler() == nullptr ||
            abilityRecord->GetAbilityState() != AbilityState::FOREGROUNDING)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR,
                "PostCancelStartingWindowColdTask, call windowHandler CancelStartingWindow.");
            windowHandler->CancelStartingWindow(abilityRecord->GetToken());
            abilityRecord->SetStartingWindow(false);
        }
    };
    auto taskName = std::to_string(missionId_) + "_cold";
    int loadTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * LOAD_TIMEOUT_MULTIPLE;
    handler->SubmitTask(delayTask, taskName, loadTimeout);
}

void MissionAbilityRecord::StartingWindowHot()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get WMS failed");
        return;
    }

    auto pixelMap = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetSnapshot(missionId_);
    if (!pixelMap) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get snapshot failed");
    }

    auto info = CreateAbilityTransitionInfo();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "notify wms to start StartingWindow");
    windowHandler->StartingWindow(info, pixelMap);
}

void MissionAbilityRecord::StartingWindowHot(const std::shared_ptr<StartOptions> &startOptions,
    const std::shared_ptr<Want> &want, const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get WMS failed");
        return;
    }

    auto pixelMap = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetSnapshot(missionId_);
    if (!pixelMap) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, failed", __func__);
    }

    auto info = CreateAbilityTransitionInfo(startOptions, want, abilityRequest);
    windowHandler->StartingWindow(info, pixelMap);
}

void MissionAbilityRecord::StartingWindowCold(const std::shared_ptr<StartOptions> &startOptions,
    const std::shared_ptr<Want> &want, const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, get WMS failed", __func__);
        return;
    }

    // get bg pixelmap and color.
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    uint32_t bgColor = 0;
    GetColdStartingWindowResource(pixelMap, bgColor);

    // start window
    auto info = CreateAbilityTransitionInfo(startOptions, want, abilityRequest);
    windowHandler->StartingWindow(info, pixelMap, bgColor);
    startingWindowBg_.reset();
}

std::shared_ptr<Global::Resource::ResourceManager> MissionAbilityRecord::CreateResourceManager() const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetEffectiveLanguage(), status);
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resConfig->SetLocaleInfo(locale);
    AppExecFwk::Configuration cfg;
    if (AbilityManagerService::GetPubInstance()->GetConfiguration(cfg) == 0) {
        std::string colormode = cfg.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "getcolormode is %{public}s.", colormode.c_str());
        resConfig->SetColorMode(AppExecFwk::ConvertColorMode(colormode));
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "getcolormode failed");
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr(Global::Resource::CreateResourceManager(false));
    resourceMgr->UpdateResConfig(*resConfig);

    std::string loadPath;
    if (!abilityInfo_.hapPath.empty()) {
        loadPath = abilityInfo_.hapPath;
    } else {
        loadPath = abilityInfo_.resourcePath;
    }

    if (loadPath.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid app resource");
        return nullptr;
    }

    if (!resourceMgr->AddResource(loadPath.c_str())) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, failed", __func__);
        return nullptr;
    }
    return resourceMgr;
}

std::shared_ptr<Media::PixelMap> MissionAbilityRecord::GetPixelMap(const uint32_t windowIconId,
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (resourceMgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, null resourceMgr", __func__);
        return nullptr;
    }

    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource;
    if (!abilityInfo_.hapPath.empty()) { // hap is not unzip
        std::unique_ptr<uint8_t[]> iconOut;
        size_t len = 0;
        if (resourceMgr->GetMediaDataById(windowIconId, len, iconOut) != Global::Resource::RState::SUCCESS) {
            return nullptr;
        }
        imageSource = Media::ImageSource::CreateImageSource(iconOut.get(), len, opts, errorCode);
    } else { // already unzip hap
        std::string iconPath;
        if (resourceMgr->GetMediaById(windowIconId, iconPath) != Global::Resource::RState::SUCCESS) {
            return nullptr;
        }
        imageSource = Media::ImageSource::CreateImageSource(iconPath, opts, errorCode);
    }

    if (errorCode != 0 || imageSource == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, id %{private}d err %{public}d", windowIconId, errorCode);
        return nullptr;
    }

    Media::DecodeOptions decodeOpts;
    auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        TAG_LOGE(
            AAFwkTag::ABILITYMGR, "failed, id %{private}d err %{public}d", windowIconId, errorCode);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OUT.");
    return std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
}

void MissionAbilityRecord::GetColdStartingWindowResource(std::shared_ptr<Media::PixelMap> &bg, uint32_t &bgColor)
{
    bg = startingWindowBg_;
    bgColor = bgColor_;
    if (bg) {
        return;
    }
    auto resourceMgr = CreateResourceManager();
    if (!resourceMgr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get resourceMgr failed");
        return;
    }

    auto windowIconId = static_cast<uint32_t>(abilityInfo_.startWindowIconId);
    bg = GetPixelMap(windowIconId, resourceMgr);

    auto colorId = static_cast<uint32_t>(abilityInfo_.startWindowBackgroundId);
    auto colorErrval = resourceMgr->GetColorById(colorId, bgColor);
    if (colorErrval != OHOS::Global::Resource::RState::SUCCESS) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "failed to GetColorById");
        bgColor = 0xdfffffff;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "colorId is %{public}u, bgColor is %{public}u.", colorId, bgColor);
}

void MissionAbilityRecord::InitColdStartingWindowResource(
    const std::shared_ptr<Global::Resource::ResourceManager> &resourceMgr)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!resourceMgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid resourceManager");
        return;
    }

    startingWindowBg_ = GetPixelMap(static_cast<uint32_t>(abilityInfo_.startWindowIconId), resourceMgr);
    if (resourceMgr->GetColorById(static_cast<uint32_t>(abilityInfo_.startWindowBackgroundId), bgColor_) !=
        OHOS::Global::Resource::RState::SUCCESS) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "getColorById failed");
        bgColor_ = 0xdfffffff;
    }

    auto handler = AbilityManagerService::GetPubInstance()->GetTaskHandler();
    if (startingWindowBg_ && handler) {
        auto delayTask = [me = weak_from_this()] {
            auto self = FromBaseRecord(me.lock());
            if (!self || !self->startingWindowBg_) {
                return;
            }
            self->startingWindowBg_.reset();
        };
        handler->SubmitTask(delayTask, "release_bg", RELEASE_STARTING_BG_TIMEOUT);
    }
}
#endif
}  // namespace AAFwk
}  // namespace OHOS