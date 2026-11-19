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

#ifndef OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_MANAGER_UTIL_H
#define OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_MANAGER_UTIL_H

#include "ani.h"
#include "auto_fill_custom_config.h"
#include "view_data.h"

namespace OHOS {
namespace AutoFillManagerEts {
ani_object WrapAutoFillRect(ani_env *env, const AbilityBase::Rect &rect);
ani_object WrapPageNodeInfo(ani_env *env, const AbilityBase::PageNodeInfo &pageNodeInfo);
ani_object WrapViewData(ani_env *env, const AbilityBase::ViewData &viewData);
ani_object WrapFillFailureResult(ani_env *env, int32_t errCode);

bool UnwrapAutoFillRect(ani_env *env, ani_object object, AbilityBase::Rect &rect, std::string &errorMsg);
bool UnwrapPageNodeInfo(ani_env *env,
    ani_object object, AbilityBase::PageNodeInfo &pageNodeInfo, std::string &errorMsg);
bool UnwrapViewData(ani_env *env, ani_object object, AbilityBase::ViewData &viewData, std::string &errorMsg);
bool UnwrapSaveRequest(ani_env *env,
    ani_object object, AbilityRuntime::AutoFill::AutoFillRequest &request, std::string &errorMsg);
bool UnwrapFillRequest(ani_env *env,
    ani_object object, AbilityRuntime::AutoFill::AutoFillRequest &request, std::string &errorMsg);
}  // namespace AutoFillManagerEts
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_MANAGER_UTIL_H
