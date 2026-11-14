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

#include "utils/exit_reason_util.h"

#include <nlohmann/json.hpp>

#include "ability_manager_service.h"
#include "appspawn.h"
#include "app_exit_reason_data_manager.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t PIPE_MSG_READ_BUFFER = 1024;

void ExitReasonUtil::ProcessSignalData(void *token, uint32_t event)
{
    int32_t rFd = *(reinterpret_cast<int32_t*>(token));
    // read data from appspawn
    char buffer[PIPE_MSG_READ_BUFFER] = {0};
    std::string readResult = "";
    int32_t count = read(rFd, buffer, PIPE_MSG_READ_BUFFER - 1);
    if (count == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read pipe failed");
    } else if (count == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write end closed");
        fdsan_close_with_tag(rFd, static_cast<uint32_t>(AAFwkTag::ABILITYMGR));
    } else {
        int32_t pid = -1;
        int32_t signal = -1;
        int32_t uid = 0;
        std::string bundleName = "";
        std::string bufferStr = buffer;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "buffer read: %{public}s", bufferStr.c_str());
        nlohmann::json jsonObject = nlohmann::json::parse(bufferStr, nullptr, false);
        if (jsonObject.is_discarded()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "parse json string failed");
            return;
        }
        if (!jsonObject.contains("pid") || !jsonObject.contains("signal") || !jsonObject.contains("uid")
            || !jsonObject.contains("bundleName")) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "info lost!");
            return;
        }
        pid = jsonObject["pid"];
        signal = jsonObject["signal"];
        uid = jsonObject["uid"];
        bundleName = jsonObject["bundleName"];
        if (signal == 0) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "ignore signal 0, pid: %{public}d", pid);
            return;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "To update reason detail info because of SIGNAL");
        if (DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->RecordSignalReason(
            pid, uid, signal, bundleName) != 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "RecordSignalReason failed");
        }
    }
}

void ExitReasonUtil::AppSpawnStartCallback(const char *key, const char *value, void *context)
{
    auto weak = static_cast<std::weak_ptr<AbilityManagerService>*>(context);
    if (weak == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "context null");
        return;
    }
    auto abilityMgrService = weak->lock();
    if (abilityMgrService == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerService null");
        return;
    }
    int32_t rFd = 0;
    int32_t wFd = 0;
    abilityMgrService->GetAppSpawnFdsWithLock(rFd, wFd);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "rFd is: %{public}d, wFd is: %{public}d", rFd, wFd);
    if (rFd < 0 || wFd < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid appspawn msg pipe fd");
        return;
    }
    // send fd
    int32_t ret = SpawnListenFdSet(wFd);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "send fd to appspawn failed, ret: %{public}d", ret);
        abilityMgrService->CloseAppSpawnMsgPipe();
        return;
    }
    // set flag
    ret = SpawnListenCloseSet();
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SpawnListenCloseSet failed");
    }
}
}  // namespace AAFwk
}  // namespace OHOS
