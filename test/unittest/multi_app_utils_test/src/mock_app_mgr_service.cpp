/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "mock_app_mgr_service.h"

namespace OHOS {
namespace AppExecFwk {
int32_t MockAppMgrService::retCode_ = 0;
RunningMultiAppInfo MockAppMgrService::retInfo_;

int32_t MockAppMgrService::GetRunningMultiAppInfoByBundleName(const std::string &bundleName,
    RunningMultiAppInfo &info)
{
    if (retCode_ != 0) {
        return retCode_;
    }

    info = retInfo_;
    return 0;
}
}  // namespace AAFwk
}  // namespace OHOS
