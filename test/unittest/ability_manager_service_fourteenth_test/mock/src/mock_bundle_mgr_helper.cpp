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

#include "bundle_mgr_helper.h"

#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr char MOCK_INSIGHT_INTENT_PROFILE[] = "{"
    "\"insightIntents\":[{"
        "\"intentName\":\"test1\","
        "\"domain\":\"domain1\","
        "\"intentVersion\":\"1.0\","
        "\"srcEntry\":\"entry1\","
        "\"uiAbility\":{"
            "\"ability\":\"MainAbility\","
            "\"executeMode\":[\"foreground\"]"
        "}"
    "}]"
    "}";
}

ErrCode BundleMgrHelper::GetJsonProfile(ProfileType profileType, const std::string &bundleName,
    const std::string &moduleName, std::string &profile, int32_t userId)
{
    if (!AAFwk::MyStatus::GetInstance().mockGetJsonProfileSuccess_) {
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }
    profile = MOCK_INSIGHT_INTENT_PROFILE;
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetNameForUid(const int32_t uid, std::string &name)
{
    name = AAFwk::MyStatus::GetInstance().bundleNameForUid_;
    return AAFwk::MyStatus::GetInstance().getNameForUidRet_;
}
}  // namespace AppExecFwk
}  // namespace OHOS