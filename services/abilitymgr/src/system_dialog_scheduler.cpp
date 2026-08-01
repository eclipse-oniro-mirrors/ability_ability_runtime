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
#include "system_dialog_scheduler.h"

#include "ability_record.h"
#include "ability_util.h"
#include "app_utils.h"
#include "application_util.h"
#include "display_info.h"
#include "display_manager.h"
#include "hitrace_meter.h"
#include "scene_board_judgement.h"
#include "ui_extension_wrapper.h"

namespace OHOS {
namespace AAFwk {
const int32_t UI_SELECTOR_DIALOG_WIDTH = 328 * 2;
const int32_t UI_SELECTOR_DIALOG_HEIGHT = 350 * 2;
const int32_t UI_SELECTOR_DIALOG_HEIGHT_NARROW = 350 * 2;
const int32_t UI_SELECTOR_DIALOG_WIDTH_NARROW = 328 * 2;
const int32_t UI_SELECTOR_DIALOG_PHONE_H1 = 240 * 2;
const int32_t UI_SELECTOR_DIALOG_PHONE_H2 = 340 * 2;
const int32_t UI_SELECTOR_DIALOG_PHONE_H3 = 350 * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H0 = 1;
const int32_t UI_SELECTOR_DIALOG_PC_H2 = (64 * 2 + 56 + 48 + 54 + 64 + 48 + 2) * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H3 = (64 * 3 + 56 + 48 + 54 + 64 + 48 + 2) * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H4 = (64 * 4 + 56 + 48 + 54 + 64 + 48 + 2) * 2;
const int32_t UI_SELECTOR_DIALOG_PC_H5 = (64 * 4 + 56 + 48 + 54 + 64 + 48 + 58 + 2) * 2;

const int32_t UI_SELECTOR_PORTRAIT_PHONE_H1 = 280;
const int32_t UI_SELECTOR_PORTRAIT_PHONE_H2 = 400;
const int32_t UI_SELECTOR_PORTRAIT_PHONE_H3 = 410;
const int32_t UI_SELECTOR_LANDSCAPE_SIGNAL_BAR = 24;
const int32_t UI_SELECTOR_LANDSCAPE_HEIGHT = 350;
const int32_t UI_SELECTOR_LANDSCAPE_HEIGHT_NARROW = 350;
const int32_t UI_SELECTOR_LANDSCAPE_PHONE_H1 = 280;
const int32_t UI_SELECTOR_LANDSCAPE_PHONE_H2 = 400;
const int32_t UI_SELECTOR_LANDSCAPE_PHONE_H3 = 410;
const int32_t UI_SELECTOR_LANDSCAPE_COUNT_THREE = 3;
const int32_t UI_SELECTOR_LANDSCAPE_COUNT_FOUR = 4;
const float UI_SELECTOR_LANDSCAPE_GRILLE_LARGE = 0.107692;
const float UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL = 0.015385;
const float UI_SELECTOR_LANDSCAPE_MAX_RATIO = 0.9;
const float UI_SELECTOR_PORTRAIT_WIDTH_RATIO = 0.8;
const float UI_SELECTOR_PORTRAIT_WIDTH_EDGE_RATIO = 0.1;
const float UI_SELECTOR_PORTRAIT_HEIGHT_RATIO = 0.98;

const int32_t UI_TIPS_DIALOG_WIDTH = 328 * 2;
const int32_t UI_TIPS_DIALOG_HEIGHT = 135 * 2;
const int32_t UI_TIPS_DIALOG_HEIGHT_NARROW = 135 * 2;
const int32_t UI_TIPS_DIALOG_WIDTH_NARROW = 328 * 2;

const int32_t UI_JUMP_INTERCEPTOR_DIALOG_WIDTH = 328 * 2;
const int32_t UI_JUMP_INTERCEPTOR_DIALOG_HEIGHT = 135 * 2;
const int32_t UI_JUMP_INTERCEPTOR_DIALOG_HEIGHT_NARROW = 135 * 2;
const int32_t UI_JUMP_INTERCEPTOR_DIALOG_WIDTH_NARROW = 328 * 2;

const int32_t UI_ANR_DIALOG_WIDTH = 328 * 2;
const int32_t UI_ANR_DIALOG_HEIGHT = 192 * 2;
const std::string APP_NAME = "appName";
const std::string IS_DEFAULT_SELECTOR = "isDefaultSelector";
const std::string OFF_SET_X = "offsetX";
const std::string OFF_SET_Y = "offsetY";
const std::string WIDTH = "width";
const std::string HEIGHT = "height";
const std::string MODEL_FLAG = "modelFlag";
const std::string ACTION = "action";
const std::string OVERSIZE_HEIGHT = "oversizeHeight";

const int32_t UI_HALF = 2;
const int32_t UI_DEFAULT_BUTTOM_CLIP = 100;
const int32_t UI_WIDTH_780DP = 1560;
const int32_t UI_DEFAULT_WIDTH = 2560;
const int32_t UI_DEFAULT_HEIGHT = 1600;

const std::string STR_PHONE = "phone";
const std::string STR_DEFAULT = "default";
const std::string DIALOG_NAME_ANR = "dialog_anr_service";
const std::string DIALOG_NAME_TIPS = "dialog_tips_service";
const std::string DIALOG_SELECTOR_NAME = "dialog_selector_service";
const std::string DIALOG_JUMP_INTERCEPTOR_NAME = "dialog_jump_interceptor_service";

const std::string BUNDLE_NAME = "bundleName";
const std::string BUNDLE_NAME_DIALOG = "com.ohos.amsdialog";
const std::string DIALOG_PARAMS = "params";
const std::string DIALOG_POSITION = "position";
const std::string VERTICAL_SCREEN_DIALOG_POSITION = "landscapeScreen";
const std::string ABILITY_NAME_FREEZE_DIALOG = "SwitchUserDialog";
const std::string ABILITY_NAME_ASSERT_FAULT_DIALOG = "AssertFaultDialog";
const std::string ABILITY_NAME_TIPS_DIALOG = "TipsDialog";
const std::string ABILITY_NAME_SELECTOR_DIALOG = "SelectorDialog";
const std::string ABILITY_NAME_APPGALLERY_SELECTOR_DIALOG = "AppSelectorExtensionAbility";
const std::string UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
const std::string UIEXTENSION_SYS_COMMON_UI = "sys/commonUI";
const std::string CALLER_TOKEN = "callerToken";
const std::string ABILITY_NAME_JUMP_INTERCEPTOR_DIALOG = "JumpInterceptorDialog";
const std::string TYPE_ONLY_MATCH_WILDCARD = "reserved/wildcard";
const std::string ORIENTATION = "orientation";

const int32_t LINE_NUMS_ZERO = 0;
const int32_t LINE_NUMS_TWO = 2;
const int32_t LINE_NUMS_THREE = 3;
const int32_t LINE_NUMS_FOUR = 4;
const int32_t LINE_NUMS_EIGHT = 8;

const float WIDTH_MULTIPLE = 0.8;
const float HEIGHT_MULTIPLE = 0.3;
const float SETX_WIDTH_MULTIPLE = 0.1;

Want SystemDialogScheduler::GetTipsDialogWant(const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::DIALOG, "GetTipsDialogWant start");

    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_TIPS, position);

    nlohmann::json jsonObj;
    jsonObj[IS_DEFAULT_SELECTOR] = AppUtils::GetInstance().IsSelectorDialogDefaultPossion();
    const std::string params = jsonObj.dump();

    AAFwk::Want want;
    want.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_TIPS_DIALOG);
    want.SetParam(DIALOG_POSITION, GetDialogPositionParams(position));
    want.SetParam(DIALOG_PARAMS, params);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && !UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        want.SetParam(CALLER_TOKEN, callerToken);
    }
    return want;
}

Want SystemDialogScheduler::GetJumpInterceptorDialogWant(Want &targetWant)
{
    TAG_LOGD(AAFwkTag::DIALOG, "start");

    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_JUMP_INTERCEPTOR, position);

    nlohmann::json jsonObj;
    jsonObj[IS_DEFAULT_SELECTOR] = AppUtils::GetInstance().IsSelectorDialogDefaultPossion();
    auto wantEle = targetWant.GetElement();
    jsonObj["bundleName"] = wantEle.GetBundleName();
    jsonObj["abilityName"] = wantEle.GetAbilityName();
    jsonObj["moduleName"] = wantEle.GetModuleNameRef();
    const std::string params = jsonObj.dump();

    targetWant.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_JUMP_INTERCEPTOR_DIALOG);
    targetWant.SetParam(DIALOG_POSITION, GetDialogPositionParams(position));
    targetWant.SetParam(DIALOG_PARAMS, params);
    targetWant.GetStringParam(DIALOG_PARAMS);
    return targetWant;
}

void SystemDialogScheduler::DialogPortraitPositionAdaptive(
    DialogPosition &position, float densityPixels, int lineNums) const
{
    if (lineNums > LINE_NUMS_EIGHT) {
        position.height = static_cast<int32_t>(UI_SELECTOR_PORTRAIT_PHONE_H3 * densityPixels);
        return;
    } else if (lineNums > LINE_NUMS_FOUR) {
        position.height = static_cast<int32_t>(UI_SELECTOR_PORTRAIT_PHONE_H2 * densityPixels);
        return;
    } else if (lineNums > LINE_NUMS_ZERO) {
        position.height = static_cast<int32_t>(UI_SELECTOR_PORTRAIT_PHONE_H1 * densityPixels);
        return;
    }

    TAG_LOGD(AAFwkTag::DIALOG, "dialog portrait lineNums is zero");
}

void SystemDialogScheduler::GetSelectorDialogPortraitPosition(
    DialogPosition &position, int32_t height, int32_t width, int lineNums, float densityPixels) const
{
    TAG_LOGD(AAFwkTag::DIALOG, "PortraitPosition height %{public}d width %{public}d density %{public}f",
        height, width, densityPixels);
    position.width = static_cast<int32_t>(width * UI_SELECTOR_PORTRAIT_WIDTH_RATIO);
    position.height = static_cast<int32_t>(UI_SELECTOR_DIALOG_HEIGHT * densityPixels);
    position.width_narrow = static_cast<int32_t>(width * UI_SELECTOR_PORTRAIT_WIDTH_RATIO);
    position.height_narrow = static_cast<int32_t>(UI_SELECTOR_DIALOG_HEIGHT_NARROW * densityPixels);

    if (width < UI_WIDTH_780DP) {
        TAG_LOGI(AAFwkTag::DIALOG, "show dialog narrow");
        position.width = position.width_narrow;
        position.height = position.height_narrow;
    }

    DialogPortraitPositionAdaptive(position, densityPixels, lineNums);

    int32_t portraitMax = static_cast<int32_t>(height * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO);
    if (portraitMax < position.height) {
        position.oversizeHeight = true;
        position.height = static_cast<int32_t>(UI_SELECTOR_PORTRAIT_PHONE_H1 * densityPixels);
        TAG_LOGI(AAFwkTag::DIALOG, "portrait ratio 0.9 height: %{public}d", portraitMax);
    }

    position.offsetX = static_cast<int32_t>(width * UI_SELECTOR_PORTRAIT_WIDTH_EDGE_RATIO);
    position.offsetY = static_cast<int32_t>((height * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - position.height));
    TAG_LOGD(AAFwkTag::DIALOG, "dialog offset x:%{public}d y:%{public}d h:%{public}d w:%{public}d",
        position.offsetX, position.offsetY, position.height, position.width);
}

void SystemDialogScheduler::DialogLandscapePositionAdaptive(
    DialogPosition &position, float densityPixels, int lineNums) const
{
    if (lineNums > LINE_NUMS_EIGHT) {
        position.height = static_cast<int32_t>(UI_SELECTOR_LANDSCAPE_PHONE_H3 * densityPixels);
        return;
    } else if (lineNums > LINE_NUMS_FOUR) {
        position.height = static_cast<int32_t>(UI_SELECTOR_LANDSCAPE_PHONE_H2 * densityPixels);
        return;
    } else if (lineNums > LINE_NUMS_ZERO) {
        position.height = static_cast<int32_t>(UI_SELECTOR_LANDSCAPE_PHONE_H1 * densityPixels);
        return;
    }

    TAG_LOGD(AAFwkTag::DIALOG, "dialog landscape lineNums is zero");
}

void SystemDialogScheduler::GetSelectorDialogLandscapePosition(
    DialogPosition &position, int32_t height, int32_t width, int lineNums, float densityPixels) const
{
    TAG_LOGD(AAFwkTag::DIALOG, "LandscapePosition height %{public}d width %{public}d density %{public}f",
        height, width, densityPixels);
    position.width = static_cast<int32_t>(width *
        (UI_SELECTOR_LANDSCAPE_GRILLE_LARGE * UI_SELECTOR_LANDSCAPE_COUNT_FOUR +
        UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL * UI_SELECTOR_LANDSCAPE_COUNT_THREE));
    position.height = static_cast<int32_t>((UI_SELECTOR_LANDSCAPE_HEIGHT) * densityPixels);
    position.width_narrow = static_cast<int32_t>(width *
        (UI_SELECTOR_LANDSCAPE_GRILLE_LARGE * UI_SELECTOR_LANDSCAPE_COUNT_FOUR +
        UI_SELECTOR_LANDSCAPE_GRILLE_SAMLL * UI_SELECTOR_LANDSCAPE_COUNT_THREE));
    position.height_narrow = static_cast<int32_t>((UI_SELECTOR_LANDSCAPE_HEIGHT_NARROW) * densityPixels);
    DialogLandscapePositionAdaptive(position, densityPixels, lineNums);

    int32_t landscapeMax = static_cast<int32_t>(
        (height - UI_SELECTOR_LANDSCAPE_SIGNAL_BAR * densityPixels) * UI_SELECTOR_LANDSCAPE_MAX_RATIO);
    if (position.height > landscapeMax) {
        position.oversizeHeight = true;
        position.height = static_cast<int32_t>(UI_SELECTOR_LANDSCAPE_PHONE_H1 * densityPixels);
        TAG_LOGI(AAFwkTag::DIALOG, "landscape ratio 0.9 height:%{public}d", landscapeMax);
    }

    TAG_LOGD(AAFwkTag::DIALOG, "dialog height is %{public}d", position.height);
    position.offsetX = static_cast<int32_t>((width - position.width) / UI_HALF);
    position.offsetY = static_cast<int32_t>((height * UI_SELECTOR_PORTRAIT_HEIGHT_RATIO - position.height));
    TAG_LOGD(AAFwkTag::DIALOG, "dialog offset x:%{public}d y:%{public}d h:%{public}d w:%{public}d",
        position.offsetX, position.offsetY, position.height, position.width);
}

void SystemDialogScheduler::GetSelectorDialogPositionAndSize(
    DialogPosition &portraitPosition, DialogPosition &landscapePosition, int lineNums) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    portraitPosition.wideScreen = !AppUtils::GetInstance().IsSelectorDialogDefaultPossion();
    portraitPosition.align = AppUtils::GetInstance().IsSelectorDialogDefaultPossion() ?
        DialogAlign::BOTTOM : DialogAlign::CENTER;
    landscapePosition.wideScreen = portraitPosition.wideScreen;
    landscapePosition.align = portraitPosition.align;

    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "GetDefaultDisplay fail, try again");
        display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    }
    if (display == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "GetDefaultDisplay fail");
        return;
    }

    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "GetDisplayInfo fail");
        return;
    }

    TAG_LOGD(AAFwkTag::DIALOG, "GetOrientation, %{public}d %{public}f",
        displayInfo->GetDisplayOrientation(), display->GetVirtualPixelRatio());
    if (displayInfo->GetDisplayOrientation() == Rosen::DisplayOrientation::PORTRAIT ||
        displayInfo->GetDisplayOrientation() == Rosen::DisplayOrientation::PORTRAIT_INVERTED) {
        TAG_LOGI(AAFwkTag::DIALOG, "GetOrientation, PORTRAIT or PORTRAIT_INVERTED");
        GetSelectorDialogPortraitPosition(portraitPosition, display->GetHeight(), display->GetWidth(),
            lineNums, display->GetVirtualPixelRatio());
        GetSelectorDialogLandscapePosition(landscapePosition, display->GetWidth(), display->GetHeight(),
            lineNums, display->GetVirtualPixelRatio());
        return;
    }

    TAG_LOGI(AAFwkTag::DIALOG, "GetOrientation, LANDSCAPE or LANDSCAPE_INVERTED");
    GetSelectorDialogPortraitPosition(portraitPosition, display->GetWidth(), display->GetHeight(),
        lineNums, display->GetVirtualPixelRatio());
    GetSelectorDialogLandscapePosition(landscapePosition, display->GetHeight(), display->GetWidth(),
        lineNums, display->GetVirtualPixelRatio());
}

int SystemDialogScheduler::GetSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &requestWant,
    Want &targetWant, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::DIALOG, "start");
    DialogPosition portraitPosition;
    DialogPosition landscapePosition;
    GetSelectorDialogPositionAndSize(portraitPosition, landscapePosition, static_cast<int>(dialogAppInfos.size()));
    std::string params = GetSelectorParams(dialogAppInfos);

    requestWant.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_SELECTOR_DIALOG);
    requestWant.SetParam(DIALOG_POSITION, GetDialogPositionParams(portraitPosition));
    requestWant.SetParam(VERTICAL_SCREEN_DIALOG_POSITION, GetDialogPositionParams(landscapePosition));
    requestWant.SetParam(DIALOG_PARAMS, params);
    return GetSelectorDialogWantCommon(dialogAppInfos, requestWant, targetWant, callerToken);
}

const std::string SystemDialogScheduler::GetSelectorParams(const std::vector<DialogAppInfo> &infos) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (infos.empty()) {
        TAG_LOGW(AAFwkTag::DIALOG, "invalid abilityInfos");
        return {};
    }

    nlohmann::json jsonObject;
    jsonObject[IS_DEFAULT_SELECTOR] = AppUtils::GetInstance().IsSelectorDialogDefaultPossion();

    nlohmann::json hapListObj = nlohmann::json::array();
    for (const auto &aInfo : infos) {
        nlohmann::json aObj;
        aObj["label"] = std::to_string(aInfo.abilityLabelId);
        aObj["icon"] = std::to_string(aInfo.abilityIconId);
        aObj["bundle"] = aInfo.bundleName;
        aObj["ability"] = aInfo.abilityName;
        aObj["module"] = aInfo.moduleName;
        aObj["appIndex"] = std::to_string(aInfo.appIndex);
        aObj["bundleLabel"] = std::to_string(aInfo.bundleLabelId);
        aObj["bundleIcon"] = std::to_string(aInfo.bundleIconId);
        hapListObj.emplace_back(aObj);
    }
    jsonObject["hapList"] = hapListObj;

    return jsonObject.dump();
}

int SystemDialogScheduler::GetPcSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &requestWant,
    Want &targetWant, const std::string &type, int32_t userId, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::DIALOG, "start");
    DialogPosition position;
    GetDialogPositionAndSize(DialogType::DIALOG_SELECTOR, position, static_cast<int>(dialogAppInfos.size()));

    std::string params = GetPcSelectorParams(dialogAppInfos, type, userId, requestWant.GetActionRef());
    requestWant.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_SELECTOR_DIALOG);
    requestWant.SetParam(DIALOG_POSITION, GetDialogPositionParams(position));
    requestWant.SetParam(DIALOG_PARAMS, params);
    return GetSelectorDialogWantCommon(dialogAppInfos, requestWant, targetWant, callerToken);
}

const std::string SystemDialogScheduler::GetPcSelectorParams(const std::vector<DialogAppInfo> &infos,
    const std::string &type, int32_t userId, const std::string &action) const
{
    TAG_LOGD(AAFwkTag::DIALOG, "start");
    if (infos.empty()) {
        TAG_LOGW(AAFwkTag::DIALOG, "invalid abilityInfos");
        return {};
    }

    nlohmann::json jsonObject;
    jsonObject[IS_DEFAULT_SELECTOR] = AppUtils::GetInstance().IsSelectorDialogDefaultPossion();
    jsonObject[ACTION] = action;
    if (type == TYPE_ONLY_MATCH_WILDCARD) {
        jsonObject[MODEL_FLAG] = true;
    } else {
        jsonObject[MODEL_FLAG] = false;
    }

    nlohmann::json hapListObj = nlohmann::json::array();
    for (const auto &info : infos) {
        nlohmann::json aObj;
        aObj["label"] = std::to_string(info.abilityLabelId);
        aObj["icon"] = std::to_string(info.abilityIconId);
        aObj["bundle"] = info.bundleName;
        aObj["ability"] = info.abilityName;
        aObj["module"] = info.moduleName;
        aObj["type"] = type;
        aObj["userId"] = std::to_string(userId);
        aObj["appIndex"] = std::to_string(info.appIndex);
        aObj["bundleLabel"] = std::to_string(info.bundleLabelId);
        aObj["bundleIcon"] = std::to_string(info.bundleIconId);
        hapListObj.emplace_back(aObj);
    }
    jsonObject["hapList"] = hapListObj;

    return jsonObject.dump();
}

int SystemDialogScheduler::GetSelectorDialogWantCommon(const std::vector<DialogAppInfo> &dialogAppInfos,
    Want &requestWant, Want &targetWant, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::DIALOG, "start");
    bool isCallerStageBasedModel = true;
    if (callerToken != nullptr) {
        TAG_LOGD(AAFwkTag::DIALOG, "set callertoken to targetWant");
        auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
        if (abilityRecord && !abilityRecord->GetAbilityInfo().isStageBasedModel) {
            isCallerStageBasedModel = false;
        }
        if (abilityRecord && UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
            // SelectorDialog can't bind to the window of UIExtension, so set CALLER_TOKEN to null.
            requestWant.RemoveParam(CALLER_TOKEN);
        } else {
            requestWant.SetParam(CALLER_TOKEN, callerToken);
        }
    }
    if (ApplicationUtil::IsEnableAppGallerySelector() && Rosen::SceneBoardJudgement::IsSceneBoardEnabled() &&
        isCallerStageBasedModel) {
        auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
        if (bundleMgrHelper == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "bundleMgrHelper null");
            return INNER_ERR;
        }
        std::string bundleName;
        if (!IN_PROCESS_CALL(bundleMgrHelper->QueryAppGalleryBundleName(bundleName))) {
            TAG_LOGE(AAFwkTag::DIALOG, "QueryAppGalleryBundleName failed");
            return INNER_ERR;
        }
        targetWant.SetElementName(bundleName, ABILITY_NAME_APPGALLERY_SELECTOR_DIALOG);
        targetWant.SetParam(UIEXTENSION_TYPE_KEY, UIEXTENSION_SYS_COMMON_UI);
        targetWant.SetParam("isCreateAppGallerySelector", true);
        // app selectot not exist
#ifndef SUPPORT_APP_SELECTOR
        TAG_LOGI(AAFwkTag::DIALOG, "app selector not support");
        return ERR_APP_SELECTOR_NOT_EXISTS;
#endif
    }
    return ERR_OK;
}

const std::string SystemDialogScheduler::GetDialogPositionParams(const DialogPosition position) const
{
    nlohmann::json dialogPositionData;
    dialogPositionData[OFF_SET_X] = position.offsetX;
    dialogPositionData[OFF_SET_Y] = position.offsetY;
    dialogPositionData[WIDTH] = position.width;
    dialogPositionData[HEIGHT] = position.height;
    dialogPositionData[OVERSIZE_HEIGHT] = position.oversizeHeight;
    return dialogPositionData.dump();
}

void SystemDialogScheduler::InitDialogPosition(DialogType type, DialogPosition &position) const
{
    position.wideScreen = !AppUtils::GetInstance().IsSelectorDialogDefaultPossion();
    position.align = AppUtils::GetInstance().IsSelectorDialogDefaultPossion() ?
        DialogAlign::BOTTOM : DialogAlign::CENTER;
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();

    switch (type) {
        case DialogType::DIALOG_ANR:
            if (position.wideScreen) {
                position.width = UI_ANR_DIALOG_WIDTH;
                position.height = UI_ANR_DIALOG_HEIGHT;
                position.width_narrow = UI_ANR_DIALOG_WIDTH;
                position.height_narrow = UI_ANR_DIALOG_HEIGHT;
                position.align = DialogAlign::CENTER;
            } else {
                position.width =  display->GetWidth();
                position.height = display->GetHeight();
                position.width_narrow =  display->GetWidth();
                position.height_narrow = display->GetHeight();
                position.window_width = UI_ANR_DIALOG_WIDTH;
                position.window_height = UI_ANR_DIALOG_HEIGHT;
                position.align = DialogAlign::CENTER;
            }
            break;
        case DialogType::DIALOG_SELECTOR:
            position.width = UI_SELECTOR_DIALOG_WIDTH;
            position.height = UI_SELECTOR_DIALOG_HEIGHT;
            position.width_narrow = UI_SELECTOR_DIALOG_WIDTH_NARROW;
            position.height_narrow = UI_SELECTOR_DIALOG_HEIGHT_NARROW;
            break;
        case DialogType::DIALOG_TIPS:
            position.width = UI_TIPS_DIALOG_WIDTH;
            position.height = UI_TIPS_DIALOG_HEIGHT;
            position.width_narrow = UI_TIPS_DIALOG_WIDTH_NARROW;
            position.height_narrow = UI_TIPS_DIALOG_HEIGHT_NARROW;
            break;
        case DialogType::DIALOG_JUMP_INTERCEPTOR:
            position.width = UI_JUMP_INTERCEPTOR_DIALOG_WIDTH;
            position.height = UI_JUMP_INTERCEPTOR_DIALOG_HEIGHT;
            position.width_narrow = UI_JUMP_INTERCEPTOR_DIALOG_WIDTH_NARROW;
            position.height_narrow = UI_JUMP_INTERCEPTOR_DIALOG_HEIGHT_NARROW;
            break;
        default:
            position.width = UI_DEFAULT_WIDTH;
            position.height = UI_DEFAULT_HEIGHT;
            position.width_narrow = UI_DEFAULT_WIDTH;
            position.height_narrow = UI_DEFAULT_HEIGHT;
            break;
    }
}

void SystemDialogScheduler::DialogPositionAdaptive(DialogPosition &position, int lineNums) const
{
    if (position.wideScreen) {
        if (lineNums <= LINE_NUMS_TWO) {
            position.height = UI_SELECTOR_DIALOG_PC_H2;
        } else if (lineNums == LINE_NUMS_THREE) {
            position.height = UI_SELECTOR_DIALOG_PC_H3;
        } else if (lineNums == LINE_NUMS_FOUR) {
            position.height = UI_SELECTOR_DIALOG_PC_H4;
        } else if (lineNums > LINE_NUMS_FOUR) {
            position.height = UI_SELECTOR_DIALOG_PC_H5;
        } else {
            position.height = UI_SELECTOR_DIALOG_PC_H0;
        }
    } else {
        position.height = (lineNums > LINE_NUMS_EIGHT) ? UI_SELECTOR_DIALOG_PHONE_H3 :
            (lineNums > LINE_NUMS_THREE ? UI_SELECTOR_DIALOG_PHONE_H2 :
            (lineNums > LINE_NUMS_ZERO ? UI_SELECTOR_DIALOG_PHONE_H1 : position.height));
    }
}

void SystemDialogScheduler::GetDialogPositionAndSize(DialogType type, DialogPosition &position, int lineNums) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    InitDialogPosition(type, position);

    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        TAG_LOGW(AAFwkTag::DIALOG, "GetDefaultDisplay fail, try again");
        display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    }
    if (display != nullptr) {
        TAG_LOGI(AAFwkTag::DIALOG, "display width:%{public}d, height:%{public}d", display->GetWidth(),
            display->GetHeight());
        if (display->GetWidth() < UI_WIDTH_780DP) {
            TAG_LOGI(AAFwkTag::DIALOG, "show dialog narrow");
            position.width = position.width_narrow;
            position.height = position.height_narrow;
        }

        if (type == DialogType::DIALOG_SELECTOR) {
            DialogPositionAdaptive(position, lineNums);
        }
        switch (position.align) {
            case DialogAlign::CENTER:
                if (position.wideScreen) {
                    position.offsetX = (display->GetWidth() - position.width) / UI_HALF;
                    position.offsetY = (display->GetHeight() - position.height) / UI_HALF;
                } else {
                    position.window_width = position.window_width / UI_HALF;
                    position.window_height = position.window_height / UI_HALF;
                    position.offsetX = LINE_NUMS_ZERO;
                    position.offsetY = LINE_NUMS_ZERO;
                }
                break;
            case DialogAlign::BOTTOM:
                position.width = display->GetWidth() * WIDTH_MULTIPLE;
                position.height = display->GetHeight() * HEIGHT_MULTIPLE;
                position.offsetX = display->GetWidth() * SETX_WIDTH_MULTIPLE;
                position.offsetY = display->GetHeight() - position.height - UI_DEFAULT_BUTTOM_CLIP;
                break;
            default:
                position.offsetX = (display->GetWidth() - position.width) / UI_HALF;
                position.offsetY = (display->GetHeight() - position.height - UI_DEFAULT_BUTTOM_CLIP) / UI_HALF;
                break;
        }
    } else {
        TAG_LOGW(AAFwkTag::DIALOG, "fail, use default wide");
        if (type == DialogType::DIALOG_SELECTOR) {
            DialogPositionAdaptive(position, lineNums);
        }
        position.offsetX = (UI_DEFAULT_WIDTH - position.width) / UI_HALF;
        position.offsetY = UI_DEFAULT_HEIGHT - position.height - UI_DEFAULT_BUTTOM_CLIP;
    }
}

bool SystemDialogScheduler::GetAssertFaultDialogWant(Want &want)
{
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "Failed get bms");
        return false;
    }

    std::string bundleName;
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callingUid, bundleName)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "VerifyPermission get bundle name failed");
        return false;
    }

    want.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_ASSERT_FAULT_DIALOG);
    want.SetParam(BUNDLE_NAME, bundleName);
    want.SetParam(UIEXTENSION_TYPE_KEY, UIEXTENSION_SYS_COMMON_UI);
    return true;
}

Want SystemDialogScheduler::GetSwitchUserDialogWant()
{
    TAG_LOGD(AAFwkTag::DIALOG, "start");
    AAFwk::Want dialogWant;
    dialogWant.SetElementName(BUNDLE_NAME_DIALOG, ABILITY_NAME_FREEZE_DIALOG);

    return dialogWant;
}
}  // namespace AAFwk
}  // namespace OHOS
