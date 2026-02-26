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

#include "ets_agent_ui_extension.h"

#include "ets_ui_extension_base.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AgentRuntime {
EtsAgentUIExtension::EtsAgentUIExtension(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    std::shared_ptr<AbilityRuntime::UIExtensionBaseImpl> uiExtensionBaseImpl =
        std::make_shared<AbilityRuntime::EtsUIExtensionBase>(runtime);
    SetUIExtensionBaseImpl(uiExtensionBaseImpl);
}

EtsAgentUIExtension::~EtsAgentUIExtension()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "destructor");
}

ETS_EXPORT extern "C" AgentUIExtension *OHOS_ETS_AGENT_UI_EXTENSION_Create(
    const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    return new EtsAgentUIExtension(runtime);
}
} // namespace AgentRuntime
} // namespace OHOS
