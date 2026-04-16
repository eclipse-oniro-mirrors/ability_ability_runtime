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

#include "modular_object_event_mgr.h"

#include "common_event_manager.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void ModularObjectExtensionEventMgr::SubscribeSysEventReceiver()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);

    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    sysEventReceiver_ = std::make_shared<ModularObjectEventReceiver>(subscribeInfo);

    bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(sysEventReceiver_);
    if (!subResult) {
        TAG_LOGE(AAFwkTag::EXT, "Subscribe common event failed");
        return;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS