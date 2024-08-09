/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_STUB_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_STUB_H

#include "event_handler.h"
#include "insight_intent_execute_callback_interface.h"
#include <iremote_stub.h>

namespace OHOS {
namespace AAFwk {
class InsightIntentExecuteCallbackStub : public IRemoteStub<IInsightIntentExecuteCallback> {
public:
    InsightIntentExecuteCallbackStub();
    ~InsightIntentExecuteCallbackStub();
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnExecuteDoneInner(MessageParcel &data, MessageParcel &reply);

    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_STUB_H