/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "app_scheduler.h"
#include "app_scheduler_stub_probe.h"
#include "errors.h"

namespace OHOS {
namespace AAFwk {
namespace {
// Probe storage stays outside the class (mock header stays byte-identical).
AppSchedulerStubState g_appSchedulerStubState;
}  // namespace

AppSchedulerStubState &AppSchedulerStub()
{
    return g_appSchedulerStubState;
}

void AppSchedulerStubReset()
{
    g_appSchedulerStubState = AppSchedulerStubState();
}

/*
 * No AppMgrClient: appmgr-dependent methods fail fast on their null check
 * ("service unreachable" default). Layout stays real-class compatible.
 */
AppScheduler::AppScheduler() {}

AppScheduler::~AppScheduler() {}

int32_t AppScheduler::AttributeProcessForAbility(const std::string &taskId, int32_t callerUid, int32_t callerPid,
    int32_t targetPid, int64_t &attributionId)
{
    ++g_appSchedulerStubState.attributeCalls;
    if (!g_appSchedulerStubState.attributeSucceeds) {
        return INNER_ERR;
    }
    attributionId = g_appSchedulerStubState.nextAttributionId++;
    return ERR_OK;
}

int32_t AppScheduler::UnregisterTaskAttribution(const std::string &taskId, int64_t attributionId)
{
    ++g_appSchedulerStubState.unregisterCalls;
    if (!g_appSchedulerStubState.attributeSucceeds) {
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AppScheduler::RegisterManagedProcess(const std::string &taskId, int32_t pid, int32_t tool,
    int32_t callerUid, int64_t &attributionId)
{
    ++g_appSchedulerStubState.attributeCalls;
    if (!g_appSchedulerStubState.attributeSucceeds) {
        return INNER_ERR;
    }
    attributionId = g_appSchedulerStubState.nextAttributionId++;
    return ERR_OK;
}

int32_t AppScheduler::UnregisterManagedProcess(const std::string &taskId, int64_t attributionId, int32_t callerUid)
{
    ++g_appSchedulerStubState.unregisterCalls;
    if (!g_appSchedulerStubState.attributeSucceeds) {
        return INNER_ERR;
    }
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
