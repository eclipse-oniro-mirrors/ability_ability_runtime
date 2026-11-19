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

#ifndef LAUNCH_SERVICE_BUNDLE_MOCK_H
#define LAUNCH_SERVICE_BUNDLE_MOCK_H
#include "bundle_mgr_helper.h"
namespace OHOS::AppExecFwk {
struct LaunchServiceBundleMock {
    static LaunchServiceBundleMock &Instance();
    int32_t uid = 20000001;
    int32_t result = ERR_OK;
    int32_t provisionResult = ERR_OK;
    std::string callerName = "com.example.daemon";
    std::string developerId = "developer";
    LaunchServicesResult services;
    BundleInfo bundle;
    bool bundleAvailable = false;
    int32_t hspResult = ERR_OK;
    bool permissionGranted = false;
    uint32_t internetUid = 0;
    uint8_t internetAllowed = 0;
    int32_t networkResult = ERR_OK;
    void Reset();
};
} // namespace OHOS::AppExecFwk
#endif
