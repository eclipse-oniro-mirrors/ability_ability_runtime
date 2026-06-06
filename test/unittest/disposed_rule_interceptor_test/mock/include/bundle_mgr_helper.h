/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include <memory>

#include "ability_info.h"
#include "application_info.h"
#include "app_control_interface.h"

namespace OHOS {

namespace AppExecFwk {
class BundleMgrHelper {
public:
    BundleMgrHelper();

    ~BundleMgrHelper();

    static std::shared_ptr<BundleMgrHelper> GetInstance();

    sptr<IAppControlMgr> GetAppControlProxy();

    bool GetApplicationInfo(const std::string &appName, const ApplicationFlag flag, const int32_t userId,
        ApplicationInfo &applicationInfo);

    bool GetApplicationInfo(const std::string& appName, const ApplicationFlag flag,
        const int32_t userId, const int32_t appIndex, ApplicationInfo &applicationInfo);

    bool GetApplicationInfoWithAppIndex(const std::string& appName, const ApplicationFlag flag,
        const int32_t userId, const int32_t appIndex, ApplicationInfo &applicationInfo);

    ErrCode GetNameForUid(int32_t uid, std::string &name);

    bool QueryAppGalleryBundleName(std::string &appGalleryBundleName);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H