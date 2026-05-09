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

#include "napi_common_start_options.h"

#include "ability_info.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "int_wrapper.h"
#include "js_window_animation_utils.h"
#include "process_options.h"
#include "start_window_option.h"
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
#include "pixel_map_napi.h"
#endif
#include "wm_animation_common.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t ARGC_TWO = 2;
constexpr const char* FUNC_NAME_ON_REQUEST_SUCCESS = "onRequestSuccess";
constexpr const char* FUNC_NAME_ON_REQUEST_FAILURE = "onRequestFailure";
constexpr const char* COMPLETION_HANDLER = "completionHandler";
}
EXTERN_C_START

bool UnwrapProcessOptions(napi_env env, napi_value param, std::shared_ptr<AAFwk::ProcessOptions> &processOptions)
{
    if (!IsExistsByPropertyName(env, param, "processMode") &&
        !IsExistsByPropertyName(env, param, "startupVisibility")) {
        return true;
    }
    auto option = std::make_shared<AAFwk::ProcessOptions>();
    int32_t processMode = 0;
    if (!UnwrapInt32ByPropertyName(env, param, "processMode", processMode)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Unwrap processMode failed");
        return false;
    }
    option->processMode = AAFwk::ProcessOptions::ConvertInt32ToProcessMode(processMode);
    if (option->processMode == AAFwk::ProcessMode::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Convert processMode failed");
        return false;
    }
    int32_t startupVisibility = 0;
    if (!UnwrapInt32ByPropertyName(env, param, "startupVisibility", startupVisibility)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Unwrap startupVisibility failed");
        return false;
    }
    option->startupVisibility = AAFwk::ProcessOptions::ConvertInt32ToStartupVisibility(startupVisibility);
    if (option->startupVisibility == AAFwk::StartupVisibility::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Convert startupVisibility failed");
        return false;
    }
    processOptions = option;
    TAG_LOGI(AAFwkTag::JSNAPI, "processMode:%{public}d, startupVisibility:%{public}d",
        static_cast<int32_t>(processOptions->processMode),
        static_cast<int32_t>(processOptions->startupVisibility));
    return true;
}

#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
bool UnwrapPixelMapFromJS(napi_env env, napi_value param, std::shared_ptr<Media::PixelMap> &value)
{
    auto pixelMap = OHOS::Media::PixelMapNapi::GetPixelMap(env, param);
    if (!pixelMap) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Unwrap pixelMap failed");
        return false;
    }
    value = pixelMap;
    return true;
}

bool UnwrapPixelMapByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, std::shared_ptr<Media::PixelMap> &value)
{
    AbilityRuntime::HandleScope handleScope(env);
    napi_value jsValue = GetPropertyValueByPropertyName(env, jsObject, propertyName, napi_object);
    if (jsValue == nullptr) {
        return false;
    }

    return UnwrapPixelMapFromJS(env, jsValue, value);
}
#endif

bool UnwrapStartWindowOption(napi_env env, napi_value param,
    std::shared_ptr<AAFwk::StartWindowOption> &startWindowOption)
{
    auto option = std::make_shared<AAFwk::StartWindowOption>();
    std::string startWindowBackgroundColor;
    if (IsExistsByPropertyName(env, param, "startWindowBackgroundColor")) {
        if (!UnwrapStringByPropertyName(env, param, "startWindowBackgroundColor", startWindowBackgroundColor)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Unwrap startWindowBackgroundColor failed");
            return false;
        }
        option->startWindowBackgroundColor = startWindowBackgroundColor;
    }
    if (!startWindowBackgroundColor.empty()) {
        option->hasStartWindow = true;
    }

#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    std::shared_ptr<Media::PixelMap> startWindowIcon = nullptr;
    if (IsExistsByPropertyName(env, param, "startWindowIcon")) {
        if (!UnwrapPixelMapByPropertyName(env, param, "startWindowIcon", startWindowIcon)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Unwrap startWindowIcon failed");
            return false;
        }
        option->startWindowIcon = startWindowIcon;
    }
    if (startWindowIcon != nullptr) {
        option->hasStartWindow = true;
    }
#endif
    startWindowOption = option;
    return true;
}

bool UnwrapStartOptionsWithProcessOption(napi_env env, napi_value param, AAFwk::StartOptions &startOptions)
{
    UnwrapStartOptions(env, param, startOptions);
    if (!UnwrapProcessOptions(env, param, startOptions.processOptions)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Unwrap processOptions failed");
        return false;
    }
    if (!UnwrapStartWindowOption(env, param, startOptions.startWindowOption)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Unwrap startWindowOption failed");
        return false;
    }
    return true;
}

void UnwrapStartOptionsWindowOptions(napi_env env, napi_value param, AAFwk::StartOptions &startOptions)
{
    int32_t windowLeft = 0;
    if (UnwrapInt32ByPropertyName(env, param, "windowLeft", windowLeft)) {
        startOptions.SetWindowLeft(windowLeft);
        startOptions.windowLeftUsed_ = true;
    }

    int32_t windowTop = 0;
    if (UnwrapInt32ByPropertyName(env, param, "windowTop", windowTop)) {
        startOptions.SetWindowTop(windowTop);
        startOptions.windowTopUsed_ = true;
    }

    int32_t windowWidth = 0;
    if (UnwrapInt32ByPropertyName(env, param, "windowWidth", windowWidth)) {
        startOptions.SetWindowWidth(windowWidth);
        startOptions.windowWidthUsed_ = true;
    }

    int32_t windowHeight = 0;
    if (UnwrapInt32ByPropertyName(env, param, "windowHeight", windowHeight)) {
        startOptions.SetWindowHeight(windowHeight);
        startOptions.windowHeightUsed_ = true;
    }

    int32_t minWindowWidth = 0;
    if (UnwrapInt32ByPropertyName(env, param, "minWindowWidth", minWindowWidth)) {
        startOptions.SetMinWindowWidth(minWindowWidth);
        startOptions.minWindowWidthUsed_ = true;
    }

    int32_t minWindowHeight = 0;
    if (UnwrapInt32ByPropertyName(env, param, "minWindowHeight", minWindowHeight)) {
        startOptions.SetMinWindowHeight(minWindowHeight);
        startOptions.minWindowHeightUsed_ = true;
    }

    int32_t maxWindowWidth = 0;
    if (UnwrapInt32ByPropertyName(env, param, "maxWindowWidth", maxWindowWidth)) {
        startOptions.SetMaxWindowWidth(maxWindowWidth);
        startOptions.maxWindowWidthUsed_ = true;
    }

    int32_t maxWindowHeight = 0;
    if (UnwrapInt32ByPropertyName(env, param, "maxWindowHeight", maxWindowHeight)) {
        startOptions.SetMaxWindowHeight(maxWindowHeight);
        startOptions.maxWindowHeightUsed_ = true;
    }
}

bool UnwrapStartOptions(napi_env env, napi_value param, AAFwk::StartOptions &startOptions)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "not napi_object");
        return false;
    }

    int32_t windowMode = 0;
    if (UnwrapInt32ByPropertyName(env, param, "windowMode", windowMode)) {
        startOptions.SetWindowMode(windowMode);
    }

    int32_t displayId = 0;
    if (UnwrapInt32ByPropertyName(env, param, "displayId", displayId)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "get displayId %{public}d ok", displayId);
        startOptions.SetDisplayID(displayId);
    }

    bool withAnimation = true;
    if (UnwrapBooleanByPropertyName(env, param, "withAnimation", withAnimation)) {
        startOptions.SetWithAnimation(withAnimation);
    }

    UnwrapStartOptionsWindowOptions(env, param, startOptions);

    bool windowFocused = true;
    if (UnwrapBooleanByPropertyName(env, param, "windowFocused", windowFocused)) {
        startOptions.SetWindowFocused(windowFocused);
    }
    
    bool hideStartWindow = false;
    if (UnwrapBooleanByPropertyName(env, param, "hideStartWindow", hideStartWindow)) {
        startOptions.SetHideStartWindow(hideStartWindow);
    }

    std::vector<int32_t> supportWindowModes;
    if (UnwrapInt32ArrayByPropertyName(env, param, "supportWindowModes", supportWindowModes)) {
        for (int32_t mode : supportWindowModes) {
            startOptions.supportWindowModes_.emplace_back(AppExecFwk::SupportWindowMode(mode));
        }
    }

    bool hasWindowCreateParams = false;
    napi_has_named_property(env, param, "windowCreateParams", &hasWindowCreateParams);
    if (hasWindowCreateParams) {
        napi_value windowJsObject = nullptr;
        napi_get_named_property(env, param, "windowCreateParams", &windowJsObject);
        Rosen::WindowCreateParams windowCreateParams;
        if (Rosen::ConvertWindowCreateParamsFromJsValue(env, windowJsObject, windowCreateParams)) {
            startOptions.windowCreateParams_ = std::make_shared<Rosen::WindowCreateParams>(windowCreateParams);
        }
    }

    bool hasSplitRatio = false;
    napi_has_named_property(env, param, "splitRatio", &hasSplitRatio);
    if (hasSplitRatio) {
        napi_value splitRatioJsObject = nullptr;
        napi_get_named_property(env, param, "splitRatio", &splitRatioJsObject);
        Rosen::SplitRatioPreference splitRatioPreference;
        if (Rosen::ConvertSplitRatioPreferenceFromJsValue(env, splitRatioJsObject, splitRatioPreference)) {
            startOptions.SetSplitRatioPreference(static_cast<int32_t>(splitRatioPreference));
            TAG_LOGI(AAFwkTag::JSNAPI, "splitRatio value:%{public}d",
                static_cast<int32_t>(splitRatioPreference));
        } else {
            TAG_LOGW(AAFwkTag::JSNAPI, "ConvertSplitRatioPreferenceFromJsValue failed");
        }
    }

    return true;
}

bool UnwrapStartOptionsAndWant(napi_env env, napi_value param, AAFwk::StartOptions &startOptions, AAFwk::Want &want)
{
    AbilityRuntime::HandleScope handleScope(env);
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "not napi_object");
        return false;
    }
    napi_value jsValue = GetPropertyValueByPropertyName(env, param, "parameters", napi_object);
    if (jsValue != nullptr) {
        AAFwk::WantParams wantParams;
        if (UnwrapWantParams(env, jsValue, wantParams)) {
            want.SetParams(wantParams);
        }
    }
    int32_t flags = 0;
    if (UnwrapInt32ByPropertyName(env, param, "flags", flags)) {
        want.SetFlags(flags);
    }
    return UnwrapStartOptions(env, param, startOptions);
}
EXTERN_C_END

bool UnwrapCommonCompletionHandler(napi_env env, napi_value param,
    AbilityRuntime::OnRequestResult &onRequestSucc, AbilityRuntime::OnRequestResult &onRequestFail)
{
    AbilityRuntime::HandleScope handleScope(env);
    napi_value completionHandler = AppExecFwk::GetPropertyValueByPropertyName(env, param,
        COMPLETION_HANDLER, napi_object);
    if (completionHandler == nullptr) {
        TAG_LOGD(AAFwkTag::JSNAPI, "null completionHandler");
        return false;
    }
    napi_ref ref = nullptr;
    if (napi_create_reference(env, completionHandler, 1, &ref) != napi_ok) {
        TAG_LOGE(AAFwkTag::JSNAPI, "failed to create ref");
        return false;
    }
    auto completionHandlerRef = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
    onRequestSucc = UnwrapCompletionHandlerOnRequestResult(env, FUNC_NAME_ON_REQUEST_SUCCESS,
        completionHandlerRef);
    onRequestFail = UnwrapCompletionHandlerOnRequestResult(env, FUNC_NAME_ON_REQUEST_FAILURE,
        completionHandlerRef);
    return true;
}

AbilityRuntime::OnRequestResult UnwrapCompletionHandlerOnRequestResult(napi_env env,
    const char *funcName, std::shared_ptr<NativeReference> ref)
{
    if (ref == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ref");
        return nullptr;
    }
    return [env, ref, funcName](const AppExecFwk::ElementName &element, const std::string &message) {
        AbilityRuntime::HandleScope handleScope(env);
        napi_value completionHandler = ref->GetNapiValue();
        napi_value onRequestResultObj = AppExecFwk::GetPropertyValueByPropertyName(env, completionHandler,
            funcName, napi_function);
        size_t argc = ARGC_TWO;
        napi_value argv[ARGC_TWO] = {
            AppExecFwk::WrapElementName(env, element), AbilityRuntime::CreateJsValue(env, message)
        };
        napi_status status = napi_call_function(env, completionHandler, onRequestResultObj, argc, argv, nullptr);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::JSNAPI, "call %{public}s, failed: %{public}d", funcName, status);
        }
    };
}
}  // namespace AppExecFwk
}  // namespace OHOS
