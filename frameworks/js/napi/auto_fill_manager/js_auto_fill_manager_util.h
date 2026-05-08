/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing perns and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_MANAGER_UTIL_H
#define OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_MANAGER_UTIL_H

#include "auto_fill_custom_config.h"
#include "napi/native_api.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value WrapAutoFillRect(napi_env env, const AbilityBase::Rect &rect);
napi_value WrapPageNodeInfo(napi_env env, const AbilityBase::PageNodeInfo &pageNodeInfo);
napi_value WrapViewData(napi_env env, const AbilityBase::ViewData &viewData);
napi_value WrapFillFailureResult(napi_env env, int32_t errCode);

bool UnwrapAutoFillRect(napi_env env, napi_value jsValue, AbilityBase::Rect &rect, std::string &errorMsg);
bool UnwrapPageNodeInfo(napi_env env, napi_value jsValue, AbilityBase::PageNodeInfo &pageNodeInfo,
    std::string &errorMsg);
bool UnwrapViewData(napi_env env, napi_value jsValue, AbilityBase::ViewData &viewData, std::string &errorMsg);
bool UnwrapSaveRequest(napi_env env, napi_value jsValue, AutoFill::AutoFillRequest &request, std::string &errorMsg);
bool UnwrapFillRequest(napi_env env, napi_value jsValue, AutoFill::AutoFillRequest &request, std::string &errorMsg);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_MANAGER_UTIL_H
