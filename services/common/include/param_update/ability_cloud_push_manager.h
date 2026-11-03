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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CLOUD_PUSH_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CLOUD_PUSH_MANAGER_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "ability_cloud_push_reader.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

class AbilityCloudPushManager {
public:
    static AbilityCloudPushManager &GetInstance();
    void InitParam();
    void SubscribeEvent();
    void OnReceiveEvent(const Want &want);

private:
    class CloudPushEventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        explicit CloudPushEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscriberInfo,
            AbilityCloudPushManager &registry)
            : CommonEventSubscriber(subscriberInfo), registry_(registry)
        {}
        ~CloudPushEventSubscriber() = default;

        void OnReceiveEvent(const EventFwk::CommonEventData &data) override
        {
            registry_.OnReceiveEvent(data.GetWant());
        }

    private:
        AbilityCloudPushManager &registry_;
    };

    AbilityCloudPushManager() = default;

    void ReloadParam();
    void CopyFileToLocal();
    bool CopyToTmp(const std::string &src, const std::string &tmpPath);
    void PurgeLocalParam();
    std::string LoadVersion();
    void HandleParamUpdate(const Want &want) const;

    std::shared_ptr<AbilityCloudPushReader> paramReader_ = nullptr;
    std::mutex initMutex_;
    std::mutex subscriberMutex_;
    using EventHandle = std::function<void(const Want &)>;
    std::map<std::string, EventHandle> eventHandles_;
    std::shared_ptr<CloudPushEventSubscriber> subscriber_ = nullptr;
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CLOUD_PUSH_MANAGER_H
