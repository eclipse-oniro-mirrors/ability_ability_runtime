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

#include "security_audit_report.h"

#include <chrono>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "interceptor/ability_interceptor_interface.h"
#include "want.h"
#ifdef ABILITYMS_SECURITY_GUARD_ENABLE
#include "event_info.h"
#include "nlohmann/json.hpp"
#include "sg_collect_client.h"
#endif

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef ABILITYMS_SECURITY_GUARD_ENABLE
constexpr int64_t EDM_BLOCKED_START_EVENT_ID = 0x18000104;
constexpr const char *EVENT_VERSION = "1.0";
#endif
}

void SecurityAuditReport::ReportEdmBlockedStart(const AbilityInterceptorParam &param)
{
#ifdef ABILITYMS_SECURITY_GUARD_ENABLE
    using namespace Security::SecurityGuard;
    auto timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    nlohmann::json content = nlohmann::json {
        { "bundleName", param.want.GetBundleNameRef() },
        { "isPreload", param.IsPreloadStartAbility() },
        { "timestamp_utc", timeStamp },
    };
    nlohmann::json metadata = nlohmann::json {
        { "version", "" },
        { "date", "" },
        { "deviceId", "" },
        { "userId", 0 },
        { "eventType", 0 },
    };
    nlohmann::json contentJson = nlohmann::json {
        { "content", content },
        { "eventId", EDM_BLOCKED_START_EVENT_ID },
        { "metadata", metadata },
    };
    auto eventInfo = std::make_shared<EventInfo>(EDM_BLOCKED_START_EVENT_ID, EVENT_VERSION, contentJson.dump());
    auto ret = NativeDataCollectKit::ReportSecurityInfoAsync(eventInfo);
    TAG_LOGW(AAFwkTag::ABILITYMGR,
        "ReportEdmBlockedStart bundle:%{public}s, isPreload:%{public}d, ret:%{public}d",
        param.want.GetBundleNameRef().c_str(), param.IsPreloadStartAbility(), ret);
#else
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ReportEdmBlockedStart sg not enable");
#endif
}
} // namespace AAFwk
} // namespace OHOS
