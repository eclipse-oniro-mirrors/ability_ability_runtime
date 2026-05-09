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

#ifndef OHOS_ABILITY_RUNTIME_START_OPTIONS_H
#define OHOS_ABILITY_RUNTIME_START_OPTIONS_H

#include <string>

#include "ability_info.h"
#include "ability_window_configuration.h"
#include "iremote_object.h"
#include "parcel.h"

namespace OHOS {
namespace Rosen {
struct WindowCreateParams;
}

namespace AbilityRuntime {
constexpr int32_t USER_CANCEL = -7;
using OnRequestResult = std::function<void(const AppExecFwk::ElementName&, const std::string&)>;
using OnAtomicRequestSuccess = std::function<void(const std::string&)>;
using OnAtomicRequestFailure = std::function<void(const std::string&, int32_t, const std::string&)>;
struct OnRequestResultElement {
    std::string requestId_;
    OnRequestResult onRequestSuccess_;
    OnRequestResult onRequestFailure_;

    OnRequestResultElement(const std::string &requestId, OnRequestResult onRequestSucc,
        OnRequestResult onRequestFail) : requestId_(requestId), onRequestSuccess_(onRequestSucc),
        onRequestFailure_(onRequestFail)
    {}
};
struct OnAtomicRequestResult {
    std::string requestId_;
    std::string appId_;
    OnAtomicRequestSuccess onRequestSuccess_;
    OnAtomicRequestFailure onRequestFailure_;

    OnAtomicRequestResult(const std::string &requestId, std::string appId, OnAtomicRequestSuccess onRequestSucc,
        OnAtomicRequestFailure onRequestFail) : requestId_(requestId), appId_(appId), onRequestSuccess_(onRequestSucc),
        onRequestFailure_(onRequestFail)
    {}
};
enum class FailureCode {
    FAILURE_CODE_SYSTEM_MALFUNCTION = 0,
    FAILURE_CODE_USER_CANCEL = 1,
    FAILURE_CODE_USER_REFUSE = 2,
};
}

namespace AAFwk {
class ProcessOptions;
class StartWindowOption;

class StartOptions final : public Parcelable, public std::enable_shared_from_this<StartOptions> {
public:
    const int32_t DEFAULT_DISPLAY_ID {0};
    bool windowLeftUsed_ = false;
    bool windowTopUsed_ = false;
    bool windowWidthUsed_ = false;
    bool windowHeightUsed_ = false;
    bool minWindowWidthUsed_ = false;
    bool minWindowHeightUsed_ = false;
    bool maxWindowWidthUsed_ = false;
    bool maxWindowHeightUsed_ = false;
    std::shared_ptr<ProcessOptions> processOptions = nullptr;
    std::shared_ptr<StartWindowOption> startWindowOption = nullptr;
    std::vector<AppExecFwk::SupportWindowMode> supportWindowModes_;
    std::string requestId_;
    std::shared_ptr<Rosen::WindowCreateParams> windowCreateParams_ = nullptr;

    StartOptions() = default;
    ~StartOptions() = default;
    StartOptions(const StartOptions &other);
    StartOptions &operator=(const StartOptions &other);

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    bool MarshallingTwo(Parcel &parcel) const;
    static StartOptions *Unmarshalling(Parcel &parcel);

    void SetWindowMode(int32_t windowMode);
    int32_t GetWindowMode() const;

    void SetDisplayID(int32_t displayId);
    int32_t GetDisplayID() const;

    void SetWithAnimation(bool withAnimation);
    bool GetWithAnimation() const;

    void SetWindowFocused(bool windowFocused);
    int32_t GetWindowFocused() const;

    void SetHideStartWindow(bool hideStartWindow);
    bool GetHideStartWindow() const;

    void SetWindowLeft(int32_t windowLeft);
    int32_t GetWindowLeft() const;

    void SetWindowTop(int32_t windowTop);
    int32_t GetWindowTop() const;

    void SetWindowWidth(int32_t windowWidth);
    int32_t GetWindowWidth() const;

    void SetWindowHeight(int32_t windowHeight);
    int32_t GetWindowHeight() const;

    void SetMinWindowWidth(int32_t minWindowWidth);
    int32_t GetMinWindowWidth() const;

    void SetMinWindowHeight(int32_t minWindowHeight);
    int32_t GetMinWindowHeight() const;

    void SetMaxWindowWidth(int32_t maxWindowWidth);
    int32_t GetMaxWindowWidth() const;

    void SetMaxWindowHeight(int32_t maxWindowHeight);
    int32_t GetMaxWindowHeight() const;

    void SetCurrentProcessName(const std::string &processName);
    std::string GetCurrentProcessName() const;

    void SetSplitRatioPreference(int32_t splitRatioPreference);
    int32_t GetSplitRatioPreference() const;
private:
    bool withAnimation_ = true;
    bool windowFocused_ = true;
    bool hideStartWindow_ = false;
    int32_t windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    int32_t displayId_ = -1;
    int32_t windowLeft_ = 0;
    int32_t windowTop_ = 0;
    int32_t windowWidth_ = 0;
    int32_t windowHeight_ = 0;
    int32_t minWindowWidth_ = 0;
    int32_t minWindowHeight_ = 0;
    int32_t maxWindowWidth_ = 0;
    int32_t maxWindowHeight_ = 0;
    int32_t splitRatioPreference_ = 0;
    std::string currentProcessName_ = "";
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_START_OPTIONS_H
