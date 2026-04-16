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

#ifndef OHOS_ABILITY_RUNTIME_FORK_IMAGE_INFO_H
#define OHOS_ABILITY_RUNTIME_FORK_IMAGE_INFO_H

#include "ability_info.h"
#include "app_refresh_recipient.h"
#include "app_running_record.h"
#include "app_scheduler_interface.h"
#include "image_error_handler_interface.h"

namespace OHOS {
namespace AppExecFwk {
struct ForkImageInfo {
    ForkImageInfo();
    ~ForkImageInfo() = default;
    static int32_t CreateId();

    int32_t imageInfoId = 0;
    int32_t imagePid = -1;
    uint64_t checkpointId = 0;
    int32_t templatePid = -1;
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    sptr<IImageErrorHandler> errorHandler;
    sptr<IAppScheduler> appScheduler;
    sptr<AppRefreshRecipient> appRefreshRecipient;
    std::shared_ptr<AppRunningRecord> baseAppRecord;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FORK_IMAGE_INFO_H