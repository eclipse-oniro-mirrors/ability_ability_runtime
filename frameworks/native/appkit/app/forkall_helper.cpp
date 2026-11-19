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

#include "forkall_helper.h"

#include <unistd.h>

#include "app_mgr_constants.h"
#include "constants.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityBase::Constants;
namespace {
constexpr int32_t ONE_MILLISECOND = 1000;
constexpr const char* SNAPSHOT_LAUNCH_DELAY_MS = "persist.resourceschedule.snapshot_launch_delay_ms";
} // namespace

void ForkAllHelper::HandleDebugAppLaunchDelay(const AppLaunchData &appLaunchData, bool isDeveloperMode)
{
    auto appInfo = appLaunchData.GetApplicationInfo();
    if (!isDeveloperMode || appInfo.appProvisionType != Constants::APP_PROVISION_TYPE_DEBUG ||
        appLaunchData.GetImageProcessType() != static_cast<int32_t>(ImageProcessType::TEMPLATE)) {
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_APP, "Debug app sleep");
    int32_t snapshotLaunchDelayMs = system::GetIntParameter<int32_t>(SNAPSHOT_LAUNCH_DELAY_MS, 0);
    TAG_LOGI(AAFwkTag::APPKIT, "Debug app goes to sleep %{public}ds", snapshotLaunchDelayMs / ONE_MILLISECOND);
    if (snapshotLaunchDelayMs > 0) {
        usleep(snapshotLaunchDelayMs * ONE_MILLISECOND);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
