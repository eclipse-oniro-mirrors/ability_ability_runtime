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

#include "ams_want_param_filter.h"

#include <map>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_param.h"
#include "ipc_skeleton.h"
#include "permission_verification.h"
#include "skill_execute_param.h"

namespace OHOS {
namespace AAFwk {
namespace {
// System-internal parameter keys that are stripped from a Want coming from a
// third-party app (a third-party setting them is ineffective). key is the
// parameter name, value is a short description (used for logs and
// self-documentation). Add one more { key, description } entry to protect
// another key.
const std::map<std::string, std::string> THIRD_PARTY_PARAMS_TO_STRIP = {
    // Skill params, mirroring SkillExecuteParam::RemoveSkillParam.
    { AppExecFwk::SKILL_EXECUTE_PARAM_BUNDLE_NAME, "skill execution bundle name" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_MODULE_NAME, "skill execution module name" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME, "skill execution skill name" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_SCRIPT_PATH, "skill execution script path" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_FUNCTION_NAME, "skill execution function name" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_ARGS_KEYS, "skill execution args keys" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT, "skill execution src entries count" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_HAP_PATH, "skill execution hap path" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_REQUEST_CODE, "skill execution request code" },

    // Insight intent params, mirroring InsightIntentExecuteParam::RemoveInsightIntent.
    { AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, "insight intent execute name" },
    { AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, "insight intent execute id" },
    { AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_MODE, "insight intent execute mode" },
    { AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, "insight intent execute param" },
    { AppExecFwk::INSIGHT_INTENT_SRC_ENTRY, "insight intent src entry" },
    { AppExecFwk::INSIGHT_INTENT_ARKTS_MODE, "insight intent arkTS mode" },
    { AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_URI, "insight intent execute uris" },
    { AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_FLAGS, "insight intent execute flags" },
    { AppExecFwk::INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG, "insight intent openlink flag" },
    { AppExecFwk::INSIGHT_INTENT_DECORATOR_TYPE, "insight intent decorator type" },
    { AppExecFwk::INSIGHT_INTENT_SRC_ENTRANCE, "insight intent src entrance" },
    { AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_CLASSNAME, "insight intent func class name" },
    { AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_METHODNAME, "insight intent func method name" },
    { AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS, "insight intent func method params" },
    { AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE, "insight intent func return type" },
    { AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_PAGEPATH, "insight intent page path" },
    { AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID, "insight intent navigation id" },
    { AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME, "insight intent nav destination name" },
    { AppExecFwk::INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME, "insight intent query entity class name" },
};

// Dynamic system-internal params (e.g. skill args / src entries) are constructed
// at runtime as "<prefix><name>", so they cannot be listed as exact keys above
// and are stripped by prefix instead; a third-party setting them is ineffective.
const std::vector<std::string> THIRD_PARTY_PARAM_PREFIXES_TO_STRIP = {
    AppExecFwk::SKILL_EXECUTE_PARAM_ARGS_PREFIX,
    AppExecFwk::SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX,
};
}

std::shared_ptr<AMSWantParamFilter> AMSWantParamFilter::GetInstance()
{
    static std::shared_ptr<AMSWantParamFilter> instance = std::make_shared<AMSWantParamFilter>();
    return instance;
}

void AMSWantParamFilter::InstallFilter()
{
    WantParams::RegisterDeserializationObserver(GetInstance());
}

void AMSWantParamFilter::OnDeserialized(WantParams &params)
{
    // Skip filtering when there is no cross-process IPC caller (e.g. local
    // deserialization from persistent storage or file recovery): GetCallingPid()
    // then equals the current pid, and the caller-identity check below would
    // misjudge the caller and wrongly strip system-internal params kept by AMS.
    if (IPCSkeleton::GetCallingPid() == getpid()) {
        return;
    }
    // System callers pass through untouched; only third-party callers are stripped.
    if (PermissionVerification::GetInstance()->IsSACall() ||
        PermissionVerification::GetInstance()->IsSystemAppCall()) {
        return;
    }
    StripExactParams(params);
    StripPrefixParams(params);
}

void AMSWantParamFilter::StripExactParams(WantParams &params)
{
    for (const auto &entry : THIRD_PARTY_PARAMS_TO_STRIP) {
        if (params.HasParam(entry.first)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR,
                "strip system-internal param %{public}s from non-system caller: %{public}s",
                entry.first.c_str(), entry.second.c_str());
            params.Remove(entry.first);
        }
    }
}

void AMSWantParamFilter::StripPrefixParams(WantParams &params)
{
    auto keySet = params.KeySet();
    for (const auto &key : keySet) {
        for (const auto &prefix : THIRD_PARTY_PARAM_PREFIXES_TO_STRIP) {
            if (key.rfind(prefix, 0) == 0) {
                // Log only the fixed system prefix, not the full key, so that the
                // third-party-controlled dynamic suffix is not exposed in logs.
                TAG_LOGD(AAFwkTag::ABILITYMGR,
                    "strip system-internal param %{public}s from non-system caller", prefix.c_str());
                params.Remove(key);
                break;
            }
        }
    }
}

} // namespace AAFwk
} // namespace OHOS
