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

#include "agent_card.h"
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
static napi_status SetEnumItem(napi_env env, napi_value object, const char *name, int32_t value)
{
    napi_status status;
    napi_value itemName = nullptr;
    napi_value itemValue = nullptr;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);
    return napi_ok;
}

static napi_value InitAgentCardTypeObject(napi_env env)
{
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));
    NAPI_CALL(env, SetEnumItem(env, object, "APP", static_cast<int32_t>(AgentCardType::APP)));
    NAPI_CALL(env, SetEnumItem(env, object, "ATOMIC_SERVICE", static_cast<int32_t>(AgentCardType::ATOMIC_SERVICE)));
    NAPI_CALL(env, SetEnumItem(env, object, "LOW_CODE", static_cast<int32_t>(AgentCardType::LOW_CODE)));
    return object;
}
}

static napi_value AgentConstantInit(napi_env env, napi_value exports)
{
    napi_value agentCardType = InitAgentCardTypeObject(env);
    NAPI_ASSERT(env, agentCardType != nullptr, "failed to create AgentCardType object");

    napi_property_descriptor exportObjs[] = {
        DECLARE_NAPI_PROPERTY("AgentCardType", agentCardType),
    };

    napi_status status = napi_define_properties(env, exports, sizeof(exportObjs) / sizeof(exportObjs[0]), exportObjs);
    NAPI_ASSERT(env, status == napi_ok, "failed to define properties for exports");
    return exports;
}

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = AgentConstantInit,
    .nm_modname = "app.agent.agentConstant",
    .nm_priv = (static_cast<void *>(0)),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
} // namespace AgentRuntime
} // namespace OHOS
