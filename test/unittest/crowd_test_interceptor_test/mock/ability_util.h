/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H

#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityUtil {
constexpr const char* MARKET_BUNDLE_NAME = "com.huawei.hmsapp.appgallery";

std::shared_ptr<AppExecFwk::BundleMgrHelper> GetBundleManagerHelper()
{
    if (AAFwk::MyStatus::GetInstance().isNullPtr) {
        return nullptr;
    }
    return DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
}
}  // namespace AbilityUtil
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
