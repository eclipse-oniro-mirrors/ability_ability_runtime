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

#include "js_agent_extension_context.h"

#include "agent_card.h"
#include "agent_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "js_agent_manager_utils.h"
#include "js_error_utils.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AbilityRuntime;
namespace {
class JsAgentExtensionContext final {
public:
    explicit JsAgentExtensionContext(
        const std::shared_ptr<AgentExtensionContext>& context) : context_(context) {}
    ~JsAgentExtensionContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "called");
        std::unique_ptr<JsAgentExtensionContext>(static_cast<JsAgentExtensionContext*>(data));
    }

    std::shared_ptr<AgentExtensionContext> GetContext() const
    {
        return context_.lock();
    }

private:
    std::weak_ptr<AgentExtensionContext> context_;
};
}

napi_value CreateJsAgentExtensionContext(napi_env env, std::shared_ptr<AgentExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "called");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<AgentCard> agentCard;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
        agentCard = context->GetAgentCard();
    }
    napi_value object = CreateJsExtensionContext(env, context, abilityInfo);
    std::unique_ptr<JsAgentExtensionContext> jsContext = std::make_unique<JsAgentExtensionContext>(context);
    napi_wrap(env, object, jsContext.release(), JsAgentExtensionContext::Finalizer, nullptr, nullptr);
    if (agentCard == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentCard");
        return object;
    }
    napi_set_named_property(env, object, "agentCard", CreateJsAgentCard(env, *agentCard));
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return object;
}

bool UnwrapJsAgentExtensionContext(napi_env env, napi_value value, std::shared_ptr<AgentExtensionContext> &context)
{
    context = nullptr;
    if (env == nullptr || value == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env or value");
        return false;
    }

    JsAgentExtensionContext *jsContext = nullptr;
    napi_status status = napi_unwrap(env, value, reinterpret_cast<void **>(&jsContext));
    if (status != napi_ok || jsContext == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "unwrap JsAgentExtensionContext failed");
        return false;
    }
    context = jsContext->GetContext();
    return true;
}
}
}
