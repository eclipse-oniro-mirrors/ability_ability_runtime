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

#include "task_mgr/system_load_provider.h"

#include <mutex>

#include "refbase.h"
#include "res_sched_client.h"
#include "res_sched_systemload_notifier_client.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

using ResourceSchedule::ResSchedClient;
using ResourceSchedule::ResSchedSystemloadNotifierClient;

namespace {
constexpr int32_t MIN_FUSION_LEVEL = 0;
constexpr int32_t MAX_FUSION_LEVEL = 7;
}

// Bridges the resource-schedule notifier (RefBase based) to the std::function callback the
// provider works with; the client must stay alive until UnregisterSystemloadNotifier.
class SystemloadNotifierClientImpl : public ResSchedSystemloadNotifierClient {
public:
    explicit SystemloadNotifierClientImpl(std::function<void(int32_t)> callback)
        : callback_(std::move(callback))
    {
    }

    void OnSystemloadLevel(int32_t level) override
    {
        if (callback_ != nullptr) {
            callback_(level);
        }
    }

private:
    std::function<void(int32_t)> callback_;
};

class ResschedSystemLoadProvider : public ISystemLoadProvider {
public:
    ResschedSystemLoadProvider() = default;
    ~ResschedSystemLoadProvider() override
    {
        UnregisterListener();
    }

    bool RegisterListener(const std::function<void(int32_t)> &callback) override
    {
        sptr<SystemloadNotifierClientImpl> client;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (registered_) {
                return true;
            }
            client = sptr(new (std::nothrow) SystemloadNotifierClientImpl(callback));
            if (client == nullptr) {
                TAG_LOGE(AAFwkTag::APPMGR, "create systemload notifier client failed");
                return false;
            }
        }
        // Register and seed outside the provider mutex: RegisterSystemloadNotifier goes over
        // IPC and the seed callback drives a full task re-judgment (judge/task locks); none
        // of that may happen while holding this leaf lock.
        ResSchedClient::GetInstance().RegisterSystemloadNotifier(client);
        int32_t level = ResSchedClient::GetInstance().GetSystemloadLevel();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            listener_ = client;
            registered_ = true;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "systemload notifier registered");
        if (callback != nullptr && level >= MIN_FUSION_LEVEL && level <= MAX_FUSION_LEVEL) {
            callback(level);
        }
        return true;
    }

    void UnregisterListener() override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!registered_) {
            return;
        }
        ResSchedClient::GetInstance().UnRegisterSystemloadNotifier(listener_);
        listener_ = nullptr;
        registered_ = false;
        TAG_LOGI(AAFwkTag::APPMGR, "systemload notifier unregistered");
    }

    int32_t GetCurrentLevel() const override
    {
        int32_t level = ResSchedClient::GetInstance().GetSystemloadLevel();
        if (level < MIN_FUSION_LEVEL || level > MAX_FUSION_LEVEL) {
            return -1;
        }
        return level;
    }

private:
    mutable std::mutex mutex_;
    sptr<SystemloadNotifierClientImpl> listener_;
    bool registered_ = false;
};

std::shared_ptr<ISystemLoadProvider> CreateDefaultSystemLoadProvider()
{
    return std::make_shared<ResschedSystemLoadProvider>();
}

}  // namespace AppExecFwk
}  // namespace OHOS
