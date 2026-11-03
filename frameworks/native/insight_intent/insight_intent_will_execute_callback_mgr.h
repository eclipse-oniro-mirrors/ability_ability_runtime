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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_WILL_EXECUTE_CALLBACK_MGR_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_WILL_EXECUTE_CALLBACK_MGR_H

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>

#include "insight_intent_will_execute_callback_stub.h"

namespace OHOS {
namespace AbilityRuntime {
using InsightIntentExecutionInfo = AppExecFwk::InsightIntentExecutionInfo;
using WillExecuteCallback = std::function<void(const InsightIntentExecutionInfo &)>;

class InsightIntentWillExecuteCallbackMgr : public AAFwk::InsightIntentWillExecuteCallbackStub,
    public std::enable_shared_from_this<InsightIntentWillExecuteCallbackMgr> {
public:
    InsightIntentWillExecuteCallbackMgr() = default;
    ~InsightIntentWillExecuteCallbackMgr() override = default;

    static sptr<InsightIntentWillExecuteCallbackMgr> GetInstance();

    int32_t Register(const WillExecuteCallback &callback, uint32_t &callbackId);
    void Unregister(uint32_t callbackId);
    void UnregisterAll();
    void OnWillInsightIntentExecute(const InsightIntentExecutionInfo &info) override;

private:
    static std::mutex instanceMutex_;
    static sptr<InsightIntentWillExecuteCallbackMgr> instance_;

    std::mutex callbackMutex_;
    std::map<uint32_t, WillExecuteCallback> callbacks_;
    uint32_t serialNumber_ = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_WILL_EXECUTE_CALLBACK_MGR_H