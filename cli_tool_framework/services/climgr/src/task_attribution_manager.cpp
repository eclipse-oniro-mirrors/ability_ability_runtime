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

#include "task_attribution_manager.h"

#include "ability_manager_errors.h"
#include "app_mgr_client.h"
#include "errors.h"
#include "exec_options.h"
#include "hilog_tag_wrapper.h"

#include <cinttypes>

namespace OHOS {
namespace CliTool {

TaskAttributionManager &TaskAttributionManager::GetInstance()
{
    static TaskAttributionManager instance;
    return instance;
}

void TaskAttributionManager::ResetConnection()
{
    appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
}

int32_t TaskAttributionManager::AttachProcess(const std::string &taskId, pid_t pid, int32_t callerUid,
    int64_t &attributionId)
{
    attributionId = 0;
    if (!IsValidTraceId(taskId) || pid <= 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "attach skipped: invalid taskId or pid: %{public}d", pid);
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    AppExecFwk::AppMgrClient *appMgrClient = appMgrClient_.get();
    if (appMgrClient->ConnectAppMgrService() != AppExecFwk::AppMgrResultCode::RESULT_OK) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "attach failed: appmgr unreachable, taskId: %{public}s",
            taskId.c_str());
        return AAFwk::INNER_ERR;
    }
    int32_t result = appMgrClient->RegisterManagedProcess(taskId, static_cast<int32_t>(pid),
        ATTRIBUTION_TOOL_CLI, callerUid, attributionId);
    if (result != ERR_OK) {
        attributionId = 0;
        TAG_LOGW(AAFwkTag::CLI_TOOL, "attach rejected by appmgr, taskId: %{public}s, pid: %{public}d, "
            "ret: %{public}d", taskId.c_str(), pid, result);
    }
    return result;
}

int32_t TaskAttributionManager::DetachProcess(const std::string &taskId, int64_t attributionId,
    int32_t callerUid)
{
    if (taskId.empty() || attributionId <= 0) {
        return ERR_OK;
    }
    AppExecFwk::AppMgrClient *appMgrClient = appMgrClient_.get();
    if (appMgrClient->ConnectAppMgrService() != AppExecFwk::AppMgrResultCode::RESULT_OK) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "detach failed: appmgr unreachable, taskId: %{public}s",
            taskId.c_str());
        return AAFwk::INNER_ERR;
    }
    int32_t result = appMgrClient->UnregisterManagedProcess(taskId, attributionId, callerUid);
    if (result != ERR_OK) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "detach rejected by appmgr, taskId: %{public}s, "
            "attributionId: %{public}" PRId64 ", ret: %{public}d", taskId.c_str(), attributionId, result);
    }
    return result;
}

int32_t TaskAttributionManager::AttachInAppSkillProcess(const std::string &taskId,
    const std::string &bundleName, pid_t &outPid, int64_t &attributionId, int32_t callerUid)
{
    outPid = -1;
    attributionId = 0;
    if (!IsValidTraceId(taskId) || bundleName.empty()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "in-app skill attach skipped: invalid taskId or empty bundleName");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    AppExecFwk::AppMgrClient *appMgrClient = appMgrClient_.get();
    if (appMgrClient->ConnectAppMgrService() != AppExecFwk::AppMgrResultCode::RESULT_OK) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "in-app skill attach failed: appmgr unreachable, taskId: %{public}s",
            taskId.c_str());
        return AAFwk::INNER_ERR;
    }
    int32_t resolvedPid = -1;
    int32_t result = appMgrClient->RegisterInAppSkillProcess(taskId, bundleName, callerUid, resolvedPid,
        attributionId);
    if (result != ERR_OK || resolvedPid <= 0) {
        attributionId = 0;
        TAG_LOGW(AAFwkTag::CLI_TOOL, "in-app skill attach rejected, taskId: %{public}s, "
            "bundle: %{public}s, ret: %{public}d", taskId.c_str(), bundleName.c_str(), result);
        return result == ERR_OK ? AAFwk::INNER_ERR : result;
    }
    outPid = static_cast<pid_t>(resolvedPid);
    return ERR_OK;
}
} // namespace CliTool
} // namespace OHOS
