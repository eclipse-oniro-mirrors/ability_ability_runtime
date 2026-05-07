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

#ifndef MOCK_BUNDLE_MGR_HELPER_H
#define MOCK_BUNDLE_MGR_HELPER_H

#include <string>
#include <memory>
#include "mock_flag.h"
#include "ability_record/ability_request.h"

namespace OHOS {
namespace AppExecFwk {

class BundleMgrHelper {
    DECLARE_DELAYED_SINGLETON(BundleMgrHelper);
public:
    int32_t GetNameAndIndexForUid(int32_t uid, std::string &bundleName, int32_t &appIndex)
    {
        if (MockFlag::getNameAndIndexRet != 0) {
            return MockFlag::getNameAndIndexRet;
        }
        bundleName = "com.caller.bundle";
        appIndex = 0;
        return 0;
    }
    bool GetApplicationInfoWithAppIndex(const std::string &appName, int32_t appIndex,
        int32_t userId, ApplicationInfo &appInfo)
    {
        if (!MockFlag::getApplicationInfoRet) {
            return false;
        }
        appInfo.appDistributionType = "debug";
        appInfo.uid = 1000;
        return true;
    }
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_BUNDLE_MGR_HELPER_H
