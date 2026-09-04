/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "quick_fix_manager_service.h"

namespace OHOS {
namespace AAFwk {
namespace {
bool g_mockInitState = true;
bool g_mockGetInstanceState = true;
}

void MockInitState(bool state)
{
    g_mockInitState = state;
}

void MockGetInstanceState(bool state)
{
    g_mockGetInstanceState = state;
}

void ResetMockQuickFixManagerServiceState()
{
    g_mockInitState = true;
    g_mockGetInstanceState = true;
}

std::mutex QuickFixManagerService::mutex_;
sptr<QuickFixManagerService> QuickFixManagerService::instance_;

sptr<QuickFixManagerService> QuickFixManagerService::GetInstance()
{
    if (!g_mockGetInstanceState) {
        return nullptr;
    }

    if (instance_ == nullptr) {
        instance_ = new QuickFixManagerService();
    }
    return instance_;
}

bool QuickFixManagerService::Init()
{
    return g_mockInitState;
}

void QuickFixManagerService::Stop()
{
}

int32_t QuickFixManagerService::ApplyQuickFix(const std::vector<std::string>& quickFixFiles, bool isDebug,
    bool isReplace)
{
    return 0;
}

int32_t QuickFixManagerService::GetApplyedQuickFixInfo(const std::string& bundleName,
    ApplicationQuickFixInfo& quickFixInfo)
{
    return 0;
}

int32_t QuickFixManagerService::RevokeQuickFix(const std::string& bundleName)
{
    return 0;
}
}
} // namespace OHOS
