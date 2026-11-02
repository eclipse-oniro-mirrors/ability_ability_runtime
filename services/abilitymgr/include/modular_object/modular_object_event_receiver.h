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

#ifndef OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_EVENT_RECEIVER_H
#define OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_EVENT_RECEIVER_H

#include <memory>
#include <string>
#include <vector>

#include "bundle_info.h"
#include "common_event_subscriber.h"
#include "modular_object_extension_info.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class ModularObjectEventReceiver
 * @brief Receives system common events related to bundle changes (scan, add, remove, update)
 *        and updates the database with ModularObjectExtensionInfo.
 */
class ModularObjectEventReceiver : public EventFwk::CommonEventSubscriber,
    public std::enable_shared_from_this<ModularObjectEventReceiver> {
public:
    explicit ModularObjectEventReceiver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo);
    ~ModularObjectEventReceiver() override = default;

    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

private:
    // Event handlers
    void HandleEventUserSwitched(const EventFwk::CommonEventData &data);
    void HandleBundleScanFinished(const EventFwk::CommonEventData &data);
    void HandleBundleInstall(const EventFwk::CommonEventData &data);
    void HandleBundleRemoved(const EventFwk::CommonEventData &data);
    void HandleBundleChanged(const EventFwk::CommonEventData &data);

    void LoadModularObjectExtensionInfos(int32_t userId = -1);
    void InsertModularObjectExtensionInfo(const std::string &bundleName, int32_t userId, int32_t appIndex = 0);
    void UpdateModularObjectExtensionInfos(const std::string &bundleName, int32_t userId, int32_t appIndex = 0);
    void RemoveModularObjectExtensionInfo(const std::string &bundleName, int32_t userId, int32_t appIndex = 0);
    void ProcessMetadata(const std::vector<AppExecFwk::Metadata> &metadata, AAFwk::ModularObjectExtensionInfo &info);
    void GetModularObjectExtensionInfos(const AppExecFwk::BundleInfo &bundleInfo,
        std::vector<AAFwk::ModularObjectExtensionInfo> &infos);
    std::string GenerateModularObjectKey(int32_t userId, const std::string &bundleName, int32_t appIndex);

    int32_t lastUserId_ = 0;
    std::mutex userIdMutex_;
    std::mutex loadMoeMutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_EVENT_RECEIVER_H