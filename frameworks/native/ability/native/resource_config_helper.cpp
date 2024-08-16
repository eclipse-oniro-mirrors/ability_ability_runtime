/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "resource_config_helper.h"
#include "hilog_tag_wrapper.h"
#include "configuration_convertor.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace AppExecFwk;

std::string ResourceConfigHelper::GetLanguage()
{
    return language_;
}
void ResourceConfigHelper::SetLanguage(std::string language)
{
    language_ = language;
}

std::string ResourceConfigHelper::GetColormode()
{
    return colormode_;
}
void ResourceConfigHelper::SetColormode(std::string colormode)
{
    colormode_ = colormode;
}
std::string ResourceConfigHelper::GetHasPointerDevice()
{
    return hasPointerDevice_;
}
void ResourceConfigHelper::SetHasPointerDevice(std::string hasPointerDevice)
{
    hasPointerDevice_ = hasPointerDevice;
}
std::string ResourceConfigHelper::GetMcc()
{
    return mcc_;
}
void ResourceConfigHelper::SetMcc(std::string mcc)
{
    mcc_ = mcc;
}
std::string ResourceConfigHelper::GetMnc()
{
    return mnc_;
}
void ResourceConfigHelper::SetMnc(std::string mnc)
{
    mnc_ = mnc;
}
void ResourceConfigHelper::SetThemeId(std::string themeId)
{
    themeId_ = themeId;
}

std::string ResourceConfigHelper::GetColorModeIsSetByApp()
{
    return colorModeIsSetByApp_;
}
void ResourceConfigHelper::SetColorModeIsSetByApp(std::string colorModeIsSetByApp)
{
    colorModeIsSetByApp_ = colorModeIsSetByApp;
}

void ResourceConfigHelper::UpdateResConfig(
    const AppExecFwk::Configuration &configuration, std::shared_ptr<Global::Resource::ResourceManager> resourceManager)
{
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Resource manager is invalid.");
        return;
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Create res config failed.");
        return;
    }
    resourceManager->GetResConfig(*resConfig);
#ifdef SUPPORT_GRAPHICS
    if (!language_.empty()) {
        UErrorCode status = U_ZERO_ERROR;
        icu::Locale locale = icu::Locale::forLanguageTag(language_, status);
        TAG_LOGD(AAFwkTag::ABILITY, "Get forLanguageTag return[%{public}d].", static_cast<int>(status));
        if (status == U_ZERO_ERROR) {
            resConfig->SetLocaleInfo(locale);
        }
        const icu::Locale *localeInfo = resConfig->GetLocaleInfo();
        if (localeInfo != nullptr) {
            TAG_LOGD(AAFwkTag::ABILITY, "Update config, language: %{public}s, script: %{public}s,"
            " region: %{public}s", localeInfo->getLanguage(), localeInfo->getScript(), localeInfo->getCountry());
        }
    }
#endif
    UpdateResConfig(resConfig);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "resourceManager->UpdateResConfig");
    Global::Resource::RState ret = resourceManager->UpdateResConfig(*resConfig);
    if (ret != Global::Resource::RState::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITY, "Update resource config failed with %{public}d.", static_cast<int>(ret));
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Current colorMode: %{public}d, hasPointerDevice: %{public}d.",
             resConfig->GetColorMode(), resConfig->GetInputDevice());
}

void ResourceConfigHelper::UpdateResConfig(std::unique_ptr<Global::Resource::ResConfig> &resConfig)
{
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "resConfig is nullptr!");
        return;
    }
    if (!colormode_.empty()) {
        resConfig->SetColorMode(AppExecFwk::ConvertColorMode(colormode_));
    }
    if (!hasPointerDevice_.empty()) {
        resConfig->SetInputDevice(AppExecFwk::ConvertHasPointerDevice(hasPointerDevice_));
    }
    if (!colorModeIsSetByApp_.empty()) {
        TAG_LOGD(AAFwkTag::ABILITY, "set app true");
        resConfig->SetAppColorMode(true);
    }
    if (!mcc_.empty()) {
        uint32_t mccNum = 0;
        if (ConvertStringToUint32(mcc_, mccNum)) {
            resConfig->SetMcc(mccNum);
            TAG_LOGD(AAFwkTag::ABILITY, "set mcc: %{public}u", resConfig->GetMcc());
        }
    }
    if (!mnc_.empty()) {
        uint32_t mncNum = 0;
        if (ConvertStringToUint32(mnc_, mncNum)) {
            resConfig->SetMnc(mncNum);
            TAG_LOGD(AAFwkTag::ABILITY, "set mnc: %{public}u", resConfig->GetMnc());
        }
    }
    if (!themeId_.empty()) {
        uint32_t themeId = 0;
        if (ConvertStringToUint32(themeId_, themeId)) {
            resConfig->SetThemeId(themeId);
            TAG_LOGD(AAFwkTag::ABILITY, "set themeId: %{public}u", resConfig->GetThemeId());
        }
    }
}

bool ResourceConfigHelper::ConvertStringToUint32(std::string source, uint32_t &result)
{
    try {
        result = static_cast<uint32_t>(std::stoi(source));
    } catch (...) {
        TAG_LOGW(AAFwkTag::ABILITY, "source:%{public}s is invalid.", source.c_str());
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS