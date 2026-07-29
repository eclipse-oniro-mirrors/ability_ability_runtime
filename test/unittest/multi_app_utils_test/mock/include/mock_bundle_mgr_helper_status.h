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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MGR_HELPER_STATUS_H
#define OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MGR_HELPER_STATUS_H

#include <string>

#include "app_clone_preference.h"
#include "errors.h"

namespace OHOS {
namespace AAFwk {
struct MockBundleMgrHelperStatus {
    static void Reset();

    static ErrCode getAppClonePreferenceRet_;
    static AppExecFwk::AppClonePreference appClonePreference_;
    static std::string lastClonePreferenceBundleName_;
    static int32_t lastClonePreferenceUserId_;
    static bool returnNullHelper_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MGR_HELPER_STATUS_H
