/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "extension_record.h"
#include "preload_uiext_state_observer.h"
#include "hilog_tag_wrapper.h"


namespace OHOS {
namespace AAFwk {
PreLoadUIExtStateObserver::PreLoadUIExtStateObserver(
    std::weak_ptr<OHOS::AbilityRuntime::ExtensionRecord> extensionRecord) : extensionRecord_(extensionRecord) {}

void PreLoadUIExtStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    auto diedProcessName = processData.processName;
    TAG_LOGD(AAFwkTag::UI_EXT, "pname:%{public}s", diedProcessName.c_str());
    auto extensionRecord = extensionRecord_.lock();
    if (extensionRecord != nullptr) {
        auto hostPid = extensionRecord->hostPid_;
        int32_t diedPid = processData.pid;
        TAG_LOGD(AAFwkTag::UI_EXT, "Host pid is %{public}d, died pid is %{public}d.", hostPid, diedPid);
        if (static_cast<int32_t>(hostPid) != diedPid) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Host pid is not equals to died pid.");
            return;
        }
        extensionRecord->UnloadUIExtensionAbility();
    } else {
        TAG_LOGW(AAFwkTag::UI_EXT, "null");
    }
}

void PreLoadUIExtStateObserver::OnAppCacheStateChanged(const AppExecFwk::AppStateData &appStateData)
{
    auto extensionRecord = extensionRecord_.lock();
    if (extensionRecord != nullptr) {
        auto hostPid = extensionRecord->hostPid_;
        int32_t cachePid = appStateData.pid;
        if (static_cast<int32_t>(hostPid) != cachePid ||
            appStateData.state != static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_CACHED)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "appStateData.state = %{public}d", appStateData.state);
            return;
        }
        extensionRecord->UnloadUIExtensionAbility();
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "extensionRecord null");
    }
}
} // namespace AAFwk
} // namespace OHOS
