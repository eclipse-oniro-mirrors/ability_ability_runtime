/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "start_options.h"

#include "hilog_tag_wrapper.h"
#include "process_options.h"
#include "start_window_option.h"
#include "wm_animation_common.h"

namespace OHOS {
namespace AAFwk {
constexpr int MAX_SUPPOPRT_WINDOW_MODES_SIZE = 10;

StartOptions::StartOptions(const StartOptions &other)
{
    windowMode_ = other.windowMode_;
    displayId_ = other.displayId_;
    withAnimation_ = other.withAnimation_;
    windowLeft_ = other.windowLeft_;
    windowTop_ = other.windowTop_;
    windowWidth_ = other.windowWidth_;
    windowHeight_ = other.windowHeight_;
    minWindowWidth_ = other.minWindowWidth_;
    minWindowHeight_ = other.minWindowHeight_;
    maxWindowWidth_ = other.maxWindowWidth_;
    maxWindowHeight_ = other.maxWindowHeight_;
    windowLeftUsed_ = other.windowLeftUsed_;
    windowTopUsed_ = other.windowTopUsed_;
    windowWidthUsed_ = other.windowWidthUsed_;
    windowHeightUsed_ = other.windowHeightUsed_;
    minWindowWidthUsed_ = other.minWindowWidthUsed_;
    minWindowHeightUsed_ = other.minWindowHeightUsed_;
    maxWindowWidthUsed_ = other.maxWindowWidthUsed_;
    maxWindowHeightUsed_ = other.maxWindowHeightUsed_;
    processOptions = other.processOptions;
    windowFocused_ = other.windowFocused_;
    hideStartWindow_ = other.hideStartWindow_;
    startWindowOption = other.startWindowOption;
    supportWindowModes_ = other.supportWindowModes_;
    requestId_ = other.requestId_;
    windowCreateParams_ = other.windowCreateParams_;
    splitRatioPreference_ = other.splitRatioPreference_;
    currentProcessName_ = other.currentProcessName_;
}

StartOptions &StartOptions::operator=(const StartOptions &other)
{
    if (this != &other) {
        windowMode_ = other.windowMode_;
        displayId_ = other.displayId_;
        withAnimation_ = other.withAnimation_;
        windowLeft_ = other.windowLeft_;
        windowTop_ = other.windowTop_;
        windowWidth_ = other.windowWidth_;
        windowHeight_ = other.windowHeight_;
        minWindowWidth_ = other.minWindowWidth_;
        minWindowHeight_ = other.minWindowHeight_;
        maxWindowWidth_ = other.maxWindowWidth_;
        maxWindowHeight_ = other.maxWindowHeight_;
        windowLeftUsed_ = other.windowLeftUsed_;
        windowTopUsed_ = other.windowTopUsed_;
        windowWidthUsed_ = other.windowWidthUsed_;
        windowHeightUsed_ = other.windowHeightUsed_;
        minWindowWidthUsed_ = other.minWindowWidthUsed_;
        minWindowHeightUsed_ = other.minWindowHeightUsed_;
        maxWindowWidthUsed_ = other.maxWindowWidthUsed_;
        maxWindowHeightUsed_ = other.maxWindowHeightUsed_;
        processOptions = other.processOptions;
        windowFocused_ = other.windowFocused_;
        hideStartWindow_ = other.hideStartWindow_;
        startWindowOption = other.startWindowOption;
        supportWindowModes_ = other.supportWindowModes_;
        requestId_ = other.requestId_;
        windowCreateParams_ = other.windowCreateParams_;
        splitRatioPreference_ = other.splitRatioPreference_;
        currentProcessName_ = other.currentProcessName_;
    }
    return *this;
}

bool StartOptions::ReadFromParcel(Parcel &parcel)
{
    SetWindowMode(parcel.ReadInt32());
    SetDisplayID(parcel.ReadInt32());
    SetWithAnimation(parcel.ReadBool());
    SetWindowLeft(parcel.ReadInt32());
    SetWindowTop(parcel.ReadInt32());
    SetWindowWidth(parcel.ReadInt32());
    SetWindowHeight(parcel.ReadInt32());
    SetMinWindowWidth(parcel.ReadInt32());
    SetMinWindowHeight(parcel.ReadInt32());
    SetMaxWindowWidth(parcel.ReadInt32());
    SetMaxWindowHeight(parcel.ReadInt32());
    SetWindowFocused(parcel.ReadBool());
    SetHideStartWindow(parcel.ReadBool());
    windowLeftUsed_ = parcel.ReadBool();
    windowTopUsed_ = parcel.ReadBool();
    windowWidthUsed_ = parcel.ReadBool();
    windowHeightUsed_ = parcel.ReadBool();
    minWindowWidthUsed_ = parcel.ReadBool();
    minWindowHeightUsed_ = parcel.ReadBool();
    maxWindowWidthUsed_ = parcel.ReadBool();
    maxWindowHeightUsed_ = parcel.ReadBool();
    processOptions.reset(parcel.ReadParcelable<ProcessOptions>());
    startWindowOption.reset(parcel.ReadParcelable<StartWindowOption>());
    auto size = parcel.ReadInt32();
    if (size > MAX_SUPPOPRT_WINDOW_MODES_SIZE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "supportWindowModes size exceeds max");
        return false;
    }
    for (int i = 0; i < size; i++) {
        supportWindowModes_.emplace_back(AppExecFwk::SupportWindowMode(parcel.ReadInt32()));
    }
    requestId_ = parcel.ReadString();
    windowCreateParams_.reset(parcel.ReadParcelable<Rosen::WindowCreateParams>());
    SetSplitRatioPreference(parcel.ReadInt32());
    return true;
}

StartOptions *StartOptions::Unmarshalling(Parcel &parcel)
{
    StartOptions *option = new (std::nothrow) StartOptions();
    if (option == nullptr) {
        return nullptr;
    }

    if (!option->ReadFromParcel(parcel)) {
        delete option;
        option = nullptr;
    }

    return option;
}

bool StartOptions::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(GetWindowMode());
    parcel.WriteInt32(GetDisplayID());
    parcel.WriteBool(GetWithAnimation());
    parcel.WriteInt32(GetWindowLeft());
    parcel.WriteInt32(GetWindowTop());
    parcel.WriteInt32(GetWindowWidth());
    parcel.WriteInt32(GetWindowHeight());
    parcel.WriteInt32(GetMinWindowWidth());
    parcel.WriteInt32(GetMinWindowHeight());
    parcel.WriteInt32(GetMaxWindowWidth());
    parcel.WriteInt32(GetMaxWindowHeight());
    parcel.WriteBool(GetWindowFocused());
    parcel.WriteBool(GetHideStartWindow());
    parcel.WriteBool(windowLeftUsed_);
    parcel.WriteBool(windowTopUsed_);
    parcel.WriteBool(windowWidthUsed_);
    parcel.WriteBool(windowHeightUsed_);
    parcel.WriteBool(minWindowWidthUsed_);
    parcel.WriteBool(minWindowHeightUsed_);
    parcel.WriteBool(maxWindowWidthUsed_);
    parcel.WriteBool(maxWindowHeightUsed_);
    if (!parcel.WriteParcelable(processOptions.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write processOptions failed");
        return false;
    }
    if (!parcel.WriteParcelable(startWindowOption.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write startWindowOption failed");
        return false;
    }
    if (!parcel.WriteInt32(supportWindowModes_.size())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write supportWindowModes_ failed");
        return false;
    }
    for (auto windowMode : supportWindowModes_) {
        if (!parcel.WriteInt32(static_cast<int32_t>(windowMode))) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "write windowMode failed");
            return false;
        }
    }
    parcel.WriteString(requestId_);
    return MarshallingTwo(parcel);
}

bool StartOptions::MarshallingTwo(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(windowCreateParams_.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write windowCreateParams_ failed");
        return false;
    }
    if (!parcel.WriteInt32(GetSplitRatioPreference())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write splitRatioPreference_ failed");
        return false;
    }
    return true;
}

void StartOptions::SetWindowMode(int32_t windowMode)
{
    windowMode_ = windowMode;
}

int32_t StartOptions::GetWindowMode() const
{
    return windowMode_;
}

void StartOptions::SetDisplayID(int32_t id)
{
    displayId_ = id;
}

int32_t StartOptions::GetDisplayID() const
{
    return displayId_;
}

void StartOptions::SetWithAnimation(bool withAnimation)
{
    withAnimation_ = withAnimation;
}

bool StartOptions::GetWithAnimation() const
{
    return withAnimation_;
}

void StartOptions::SetWindowLeft(int32_t windowLeft)
{
    windowLeft_ = windowLeft;
}

int32_t StartOptions::GetWindowLeft() const
{
    return windowLeft_;
}

void StartOptions::SetWindowTop(int32_t windowTop)
{
    windowTop_ = windowTop;
}

int32_t StartOptions::GetWindowTop() const
{
    return windowTop_;
}

void StartOptions::SetWindowWidth(int32_t windowWidth)
{
    windowWidth_ = windowWidth;
}

int32_t StartOptions::GetWindowWidth() const
{
    return windowWidth_;
}

void StartOptions::SetWindowHeight(int32_t windowHeight)
{
    windowHeight_ = windowHeight;
}

int32_t StartOptions::GetWindowHeight() const
{
    return windowHeight_;
}

void StartOptions::SetMinWindowWidth(int32_t minWindowWidth)
{
    minWindowWidth_ = minWindowWidth;
}

int32_t StartOptions::GetMinWindowWidth() const
{
    return minWindowWidth_;
}

void StartOptions::SetMinWindowHeight(int32_t minWindowHeight)
{
    minWindowHeight_ = minWindowHeight;
}

int32_t StartOptions::GetMinWindowHeight() const
{
    return minWindowHeight_;
}

void StartOptions::SetMaxWindowWidth(int32_t maxWindowWidth)
{
    maxWindowWidth_ = maxWindowWidth;
}

int32_t StartOptions::GetMaxWindowWidth() const
{
    return maxWindowWidth_;
}

void StartOptions::SetMaxWindowHeight(int32_t maxWindowHeight)
{
    maxWindowHeight_ = maxWindowHeight;
}

int32_t StartOptions::GetMaxWindowHeight() const
{
    return maxWindowHeight_;
}

void StartOptions::SetWindowFocused(bool windowFocused)
{
    windowFocused_ = windowFocused;
}

int32_t StartOptions::GetWindowFocused() const
{
    return windowFocused_;
}

void StartOptions::SetHideStartWindow(bool hideStartWindow)
{
    hideStartWindow_ = hideStartWindow;
}

bool StartOptions::GetHideStartWindow() const
{
    return hideStartWindow_;
}

void StartOptions::SetCurrentProcessName(const std::string &processName)
{
    currentProcessName_ = processName;
}

std::string StartOptions::GetCurrentProcessName() const
{
    return currentProcessName_;
}

void StartOptions::SetSplitRatioPreference(int32_t splitRatioPreference)
{
    splitRatioPreference_ = splitRatioPreference;
}

int32_t StartOptions::GetSplitRatioPreference() const
{
    return splitRatioPreference_;
}
}  // namespace AAFwk
}  // namespace OHOS
