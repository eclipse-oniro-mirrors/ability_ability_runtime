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

#include "ets_agent_extension_context.h"

#include "agent_card.h"
#include "ets_agent_manager_utils.h"
#include "ets_context_utils.h"
#include "ets_extension_context.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AgentManagerEts;

namespace OHOS {
namespace AgentRuntime {

namespace {
constexpr const char *CONTEXT_CLASS_NAME = "application.AgentExtensionContext.AgentExtensionContext";
}

void EtsAgentExtensionContext::Finalizer(ani_env *env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EtsAgentExtensionContext::Finalizer called");
    std::unique_ptr<EtsAgentExtensionContext>(static_cast<EtsAgentExtensionContext*>(data));
}

ani_object CreateEtsAgentExtensionContext(ani_env *env, std::shared_ptr<AgentExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CreateEtsAgentExtensionContext");
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env or context");
        return nullptr;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;

    if ((status = env->FindClass(CONTEXT_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to find class, status : %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to find constructor, status : %{public}d", status);
        return nullptr;
    }

    std::shared_ptr<AgentCard> agentCard = context->GetAgentCard();
    if (agentCard == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentCard");
        return nullptr;
    }

    std::unique_ptr<EtsAgentExtensionContext> etsContext =
        std::make_unique<EtsAgentExtensionContext>(context);

    ani_long nativeContext = reinterpret_cast<ani_long>(etsContext.get());
    if ((status = env->Object_New(cls, method, &contextObj,
        nativeContext)) != ANI_OK || contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create object, status : %{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(
        contextObj, "agentCard", AgentManagerEts::CreateEtsAgentCard(env, *agentCard));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "set agentCard failed:%{public}d", status);
        return nullptr;
    }

    auto workContext = new (std::nothrow)std::weak_ptr<AgentExtensionContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null workContext");
        return nullptr;
    }

    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)(workContext))) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to SetNativeContextLong");
        delete workContext;
        return nullptr;
    }
    etsContext.release();

    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());

    ani_ref *contextGlobalRef = new (std::nothrow) ani_ref;
    if (contextGlobalRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "new contextGlobalRef failed");
        return nullptr;
    }

    if ((status = env->GlobalReference_Create(contextObj, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create failed status: %{public}d", status);
        delete contextGlobalRef;
        return nullptr;
    }

    context->Bind(contextGlobalRef);
    return contextObj;
}
} // namespace AgentRuntime
} // namespace OHOS
