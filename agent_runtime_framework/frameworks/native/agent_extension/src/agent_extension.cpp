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

#include "agent_extension.h"

#include <dlfcn.h>
#include <mutex>

#include "agent_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "runtime.h"

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
using Runtime = OHOS::AbilityRuntime::Runtime;

namespace {
const char* JS_AGENT_EXTENSION_LIBNAME = "libjs_agent_extension.z.so";
const char* ETS_AGENT_EXTENSION_LIBNAME = "libets_agent_extension.z.so";
const char* CREATE_ETS_AGENT_EXTENSION_FUNC = "OHOS_CreateEtsAgentExtension";
const char* CREATE_JS_AGENT_EXTENSION_FUNC = "OHOS_CreateJsAgentExtension";

using CreateAgentExtensionFunc = AgentExtension*(*)(const std::unique_ptr<Runtime>&);
CreateAgentExtensionFunc g_etsAgentExtensionCreateFunc = nullptr;
std::mutex g_etsAgentExtensionCreateFuncMutex;
CreateAgentExtensionFunc g_jsAgentExtensionCreateFunc = nullptr;
std::mutex g_jsAgentExtensionCreateFuncMutex;
std::mutex g_handleMutex;
void *g_jsHandle = nullptr;
void *g_etsHandle = nullptr;

void *GetDlSymbol(const char * libName, const char *funcName, void **handle)
{
    if (libName == nullptr || funcName == nullptr || handle == nullptr) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(g_handleMutex);
    if (*handle == nullptr) {
        *handle = dlopen(libName, RTLD_LAZY);
        if (*handle == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "dlopen failed %{public}s, %{public}s", libName, dlerror());
            return nullptr;
        }
    }

    auto symbol = dlsym(*handle, funcName);
    if (symbol == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "dlsym failed %{public}s, %{public}s", funcName, dlerror());
        dlclose(*handle);
        *handle = nullptr;
        return nullptr;
    }
    return symbol;
}
}

AgentExtension* AgentExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    if (!runtime) {
        return new AgentExtension();
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "language: %{public}d", runtime->GetLanguage());
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return CreateJsAgentExtension(runtime);
        case Runtime::Language::ETS:
            return CreateEtsAgentExtension(runtime);
        default:
            return new AgentExtension();
    }
}

AgentExtension* AgentExtension::CreateEtsAgentExtension(const std::unique_ptr<Runtime> &runtime)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CreateEtsAgentExtension");
    {
        std::lock_guard<std::mutex> lock(g_etsAgentExtensionCreateFuncMutex);
        if (g_etsAgentExtensionCreateFunc != nullptr) {
            return g_etsAgentExtensionCreateFunc(runtime);
        }
    }

    auto symbol = GetDlSymbol(ETS_AGENT_EXTENSION_LIBNAME, CREATE_ETS_AGENT_EXTENSION_FUNC, &g_etsHandle);
    if (symbol == nullptr) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(g_etsAgentExtensionCreateFuncMutex);
    g_etsAgentExtensionCreateFunc = reinterpret_cast<CreateAgentExtensionFunc>(symbol);
    return g_etsAgentExtensionCreateFunc(runtime);
}

AgentExtension* AgentExtension::CreateJsAgentExtension(const std::unique_ptr<Runtime> &runtime)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CreateJsAgentExtension");
    {
        std::lock_guard<std::mutex> lock(g_jsAgentExtensionCreateFuncMutex);
        if (g_jsAgentExtensionCreateFunc != nullptr) {
            return g_jsAgentExtensionCreateFunc(runtime);
        }
    }

    auto symbol = GetDlSymbol(JS_AGENT_EXTENSION_LIBNAME, CREATE_JS_AGENT_EXTENSION_FUNC, &g_jsHandle);
    if (symbol == nullptr) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(g_jsAgentExtensionCreateFuncMutex);
    g_jsAgentExtensionCreateFunc = reinterpret_cast<CreateAgentExtensionFunc>(symbol);
    return g_jsAgentExtensionCreateFunc(runtime);
}

void AgentExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<AgentExtensionContext>::Init(record, application, handler, token);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "begin init context");
}

std::shared_ptr<AgentExtensionContext> AgentExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<AgentExtensionContext> context =
        ExtensionBase<AgentExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null context");
        return nullptr;
    }
    return context;
}
} // namespace AgentRuntime
} // namespace OHOS
