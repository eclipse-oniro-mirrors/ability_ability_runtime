/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "int_wrapper.h"
#include "process_options.h"

namespace OHOS {
namespace AppExecFwk {
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

bool UnwrapStartOptionsWithProcessOption(napi_env env, napi_value param, AAFwk::StartOptions &startOptions)
{
    UnwrapStartOptions(env, param, startOptions);
    if (!UnwrapProcessOptions(env, param, startOptions.processOptions)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Unwrap processOptions failed");
        return false;
    }
    return true;
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
        TAG_LOGI(AAFwkTag::JSNAPI, "get display id ok displayId %{public}d", displayId);
        startOptions.SetDisplayID(displayId);
    }

    bool withAnimation = true;
    if (UnwrapBooleanByPropertyName(env, param, "withAnimation", withAnimation)) {
        startOptions.SetWithAnimation(withAnimation);
    }

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

    bool windowFocused = true;
    if (UnwrapBooleanByPropertyName(env, param, "windowFocused", windowFocused)) {
        startOptions.SetWindowFocused(windowFocused);
    }

    return true;
}

bool UnwrapStartOptionsAndWant(napi_env env, napi_value param, AAFwk::StartOptions &startOptions, AAFwk::Want &want)
{
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
}  // namespace AppExecFwk
}  // namespace OHOS
