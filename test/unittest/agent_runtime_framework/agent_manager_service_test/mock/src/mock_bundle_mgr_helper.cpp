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

#include "mock_my_flag.h"

namespace OHOS {
bool AgentRuntime::MyFlag::retRegisterBundleEventCallback = false;
bool AgentRuntime::MyFlag::retGetApplicationInfo = false;
bool AgentRuntime::MyFlag::isRegisterBundleEventCallbackCalled = false;

namespace AppExecFwk {
BundleMgrHelper::BundleMgrHelper()
{}

BundleMgrHelper::~BundleMgrHelper()
{}

bool BundleMgrHelper::RegisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback)
{
    AgentRuntime::MyFlag::isRegisterBundleEventCallbackCalled = true;
    return AgentRuntime::MyFlag::retRegisterBundleEventCallback;
}

bool BundleMgrHelper::GetApplicationInfo(const std::string &appName, const ApplicationFlag flag, const int32_t userId,
    ApplicationInfo &appInfo)
{
    return AgentRuntime::MyFlag::retGetApplicationInfo;
}
}  // namespace AppExecFwk
}  // namespace OHOS