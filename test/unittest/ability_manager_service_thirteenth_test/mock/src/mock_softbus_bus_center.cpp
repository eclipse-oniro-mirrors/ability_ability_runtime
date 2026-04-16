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

#ifdef SUPPORT_RECORDER_DSOFTBUS
#include "softbus_bus_center.h"

#include "mock_my_status.h"

extern "C" int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info)
{
    if (info != nullptr) {
        info->networkId[0] = '0';
        info->networkId[1] = '\0';
    }
    return OHOS::AAFwk::MyStatus::GetInstance().softbusGetLocalNodeDeviceInfo_;
}
#endif // SUPPORT_RECORDER_DSOFTBUS
