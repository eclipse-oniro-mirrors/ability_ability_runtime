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

#include "hidden_start_utils.h"

#include "ability_manager_errors.h"
#include "app_mgr_util.h"
#include "app_utils.h"
#include "process_options.h"
#include "permission_verification.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* APP_TURBO_SERVICE = "app_turbo_service";
}

bool HiddenStartUtils::IsHiddenStart(const Want &want, const StartOptions &options)
{
    if (!PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(APP_TURBO_SERVICE)) {
        return false;
    }

    if (options.processOptions == nullptr) {
        return false;
    }

    if (options.processOptions->startupVisibility != OHOS::AAFwk::StartupVisibility::STARTUP_HIDE) {
        return false;
    }

    return true;
}

int32_t HiddenStartUtils::CheckHiddenStartSupported(const Want &want, const StartOptions &options)
{
    if (!AppUtils::GetInstance().IsStartOptionsWithAnimation()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "start ability silently is not supported in this device");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    if (options.processOptions == nullptr ||
        options.processOptions->processMode != ProcessMode::NEW_HIDDEN_PROCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "this processMode is not supported in hidden start");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = want.GetElement().GetBundleName();
    bool isRunning = false;
    auto appMgr = AppMgrUtil::GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppMgrUtil::GetAppMgr failed");
        return ERR_INVALID_VALUE;
    }
    auto result = appMgr->IsAppRunning(bundleName, 0, isRunning);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Query for app running failed, error code: %{public}d", result);
        return result;
    }
    if (isRunning) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability is running now");
        return ERR_ABILITY_ALREADY_RUNNING;
    }

    return ERR_OK;
}
}
}