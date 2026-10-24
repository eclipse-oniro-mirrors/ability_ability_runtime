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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_WILL_EXECUTE_CALLBACK_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_WILL_EXECUTE_CALLBACK_INTERFACE_H

#include "insight_intent_execution_info.h"
#include "iremote_broker.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class IInsightIntentWillExecuteCallback
 * Used to notify the target ability process that an insight intent is about to be
 * executed and the caller identity.
 */
class IInsightIntentWillExecuteCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.AAFwk.InsightIntentWillExecuteCallback");

    virtual void OnWillInsightIntentExecute(const AppExecFwk::InsightIntentExecutionInfo &info) = 0;

    enum {
        // ipc id for OnWillInsightIntentExecute
        ON_WILL_INSIGHT_INTENT_EXECUTE = 1,
    };
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_WILL_EXECUTE_CALLBACK_INTERFACE_H