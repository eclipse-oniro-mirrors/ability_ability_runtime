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

#include "agent_ui_extension.h"

#include <dlfcn.h>
#include <mutex>

#include "hilog_tag_wrapper.h"
#include "js_agent_ui_extension.h"
#include "ets_agent_ui_extension.h"
#include "runtime.h"
#include "ui_extension_context.h"


namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
using Runtime = OHOS::AbilityRuntime::Runtime;

namespace {
const char* JS_AGENT_UI_EXTENSION_LIBNAME = "libjs_agent_ui_extension.z.so";
const char* CREATE_JS_AGENT_UI_EXTENSION_FUNC = "OHOS_CreateJsAgentUIExtension";
const char* ETS_AGENT_UI_EXTENSION_LIBNAME = "libagent_ui_extension_ani.z.so";
const char* CREATE_ETS_AGENT_UI_EXTENSION_FUNC = "OHOS_ETS_AGENT_UI_EXTENSION_Create";

using CreateAgentUIExtensionFunc = AgentUIExtension*(*)(const std::unique_ptr<Runtime>&);
CreateAgentUIExtensionFunc g_jsAgentUIExtensionCreateFunc = nullptr;
CreateAgentUIExtensionFunc g_etsAgentUIExtensionCreateFunc = nullptr;
std::mutex g_jsAgentUIExtensionCreateFuncMutex;
std::mutex g_etsAgentUIExtensionCreateFuncMutex;
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

AgentUIExtension *AgentUIExtension::Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "called");
    if (!runtime) {
        return new AgentUIExtension();
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "language: %{public}d", runtime->GetLanguage());
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return CreateJsAgentUIExtension(runtime);
        case Runtime::Language::ETS:
            return CreateETSAgentUIExtension(runtime);
        default:
            return new AgentUIExtension();
    }
}

AgentUIExtension* AgentUIExtension::CreateJsAgentUIExtension(const std::unique_ptr<Runtime> &runtime)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CreateJsAgentUIExtension");
    {
        std::lock_guard<std::mutex> lock(g_jsAgentUIExtensionCreateFuncMutex);
        if (g_jsAgentUIExtensionCreateFunc != nullptr) {
            return g_jsAgentUIExtensionCreateFunc(runtime);
        }
    }

    auto symbol = GetDlSymbol(JS_AGENT_UI_EXTENSION_LIBNAME, CREATE_JS_AGENT_UI_EXTENSION_FUNC, &g_jsHandle);
    if (symbol == nullptr) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(g_jsAgentUIExtensionCreateFuncMutex);
    g_jsAgentUIExtensionCreateFunc = reinterpret_cast<CreateAgentUIExtensionFunc>(symbol);
    return g_jsAgentUIExtensionCreateFunc(runtime);
}

AgentUIExtension* AgentUIExtension::CreateETSAgentUIExtension(const std::unique_ptr<Runtime> &runtime)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CreateETSAgentUIExtension");
    {
        std::lock_guard<std::mutex> lock(g_etsAgentUIExtensionCreateFuncMutex);
        if (g_etsAgentUIExtensionCreateFunc != nullptr) {
            return g_etsAgentUIExtensionCreateFunc(runtime);
        }
    }

    auto symbol = GetDlSymbol(ETS_AGENT_UI_EXTENSION_LIBNAME, CREATE_ETS_AGENT_UI_EXTENSION_FUNC, &g_etsHandle);
    if (symbol == nullptr) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(g_etsAgentUIExtensionCreateFuncMutex);
    g_etsAgentUIExtensionCreateFunc = reinterpret_cast<CreateAgentUIExtensionFunc>(symbol);
    return g_etsAgentUIExtensionCreateFunc(runtime);
}

} // namespace AgentRuntime
} // namespace OHOS
