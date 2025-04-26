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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EVENT_MGR_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EVENT_MGR_H

#include <string>

#include "bundlemgr/bundle_mgr_interface.h"
#include "element_name.h"
#include "insight_intent_sys_event_receiver.h"

namespace OHOS {
namespace AbilityRuntime {
class InsightIntentEventMgr : public std::enable_shared_from_this<InsightIntentEventMgr> {
public:
    static void UpdateInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId);
    static void DeleteInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId);

    void SubscribeSysEventReceiver();
private:
    std::shared_ptr<InsightIntentSysEventReceiver> insightIntentSysEventReceiver_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EVENT_MGR_H
