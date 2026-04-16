/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_SYS_EVENT_RECEIVER_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_SYS_EVENT_RECEIVER_H

#include "common_event_data.h"
#include "common_event_subscriber.h"
#include "common_event_subscribe_info.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AbilityRuntime {
class InsightIntentSysEventReceiver : public EventFwk::CommonEventSubscriber,
    public std::enable_shared_from_this<InsightIntentSysEventReceiver> {
public:
    InsightIntentSysEventReceiver(const EventFwk::CommonEventSubscribeInfo &subcribeInfo);
    ~InsightIntentSysEventReceiver() = default;

    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
private:
    void SaveInsightIntentInfos(const std::string &bundleName, const std::string &moduleName,
        uint32_t versionCode, int32_t userId);
    void DeleteInsightIntent(const std::string &bundleName, const std::string &moduleName, int32_t userId);
    void LoadInsightIntentInfos(int32_t userId = -1);
    void DeleteInsightIntentInfoByUserId(int32_t userId);
    void HandleBundleScanFinished();
    void HandleUserSwitched(const EventFwk::CommonEventData &data);
    void HandleUserRemove(const EventFwk::CommonEventData &data);

    int32_t lastUserId_ = 0;
    std::mutex userIdMutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_SYS_EVENT_RECEIVER_H
