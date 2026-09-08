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

#include "multi_app_utils.h"

#include "ability_util.h"
#include "app_mgr_util.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "running_multi_info.h"

namespace OHOS {
namespace AAFwk {
void MultiAppUtils::GetRunningMultiAppIndex(const std::string &bundleName, int32_t uid, int32_t &appIndex)
{
    AppExecFwk::RunningMultiAppInfo runningMultiAppInfo;
    auto appMgr = AppMgrUtil::GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "AppMgrUtil::GetAppMgr failed");
        return;
    }
    auto ret = IN_PROCESS_CALL(appMgr->GetRunningMultiAppInfoByBundleName(bundleName, runningMultiAppInfo));
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "getAppInfo failed, bundle:%{public}s",
            bundleName.c_str());
    }
    for (auto &item : runningMultiAppInfo.runningAppClones) {
        if (item.uid == uid) {
            appIndex = item.appCloneIndex;
            break;
        }
    }
}

bool MultiAppUtils::GetPreferredAppCloneIndex(const std::string &bundleName, int32_t userId, int32_t &appIndex)
{
    if (bundleName.empty()) {
        return false;
    }

    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleMgrHelper is nullptr");
        return false;
    }

    AppExecFwk::AppClonePreference preference;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetAppClonePreference(bundleName, userId, preference));
    if (ret != ERR_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "GetAppClonePreference failed, ret:%{public}d", ret);
        return false;
    }
    if (preference.mode == AppExecFwk::AppClonePreferenceMode::MAIN_APP) {
        appIndex = 0;
        return true;
    }
    if (preference.mode != AppExecFwk::AppClonePreferenceMode::CLONE_APP) {
        return false;
    }
    if (preference.appIndex <= 0 || preference.appIndex > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid default clone index:%{public}d", preference.appIndex);
        return false;
    }
    appIndex = preference.appIndex;
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
