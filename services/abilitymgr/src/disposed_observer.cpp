/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "disposed_observer.h"

#include "ability_record.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "want.h"
namespace OHOS {
namespace AAFwk {
DisposedObserver::DisposedObserver(const AppExecFwk::DisposedRule &disposedRule)
    : disposedRule_(disposedRule)
{}

DisposedObserver::~DisposedObserver()
{}

bool DisposedObserver::UnSubScribeAppState()
{
    HILOG_DEBUG("Call");
    sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppMgr();
    IN_PROCESS_CALL(appManager->UnregisterApplicationStateObserver(
        iface_cast<AppExecFwk::IApplicationStateObserver>(this)));
    return true;
}

sptr<OHOS::AppExecFwk::IAppMgr> DisposedObserver::GetAppMgr()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        HILOG_ERROR("get systemAbilityManager failed");
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> object = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    if (!object) {
        HILOG_ERROR("get systemAbilityManager failed");
        return nullptr;
    }
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = iface_cast<AppExecFwk::IAppMgr>(object);
    if (!appMgr || !appMgr->AsObject()) {
        return nullptr;
    }
    return appMgr;
}

void DisposedObserver::OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    HILOG_DEBUG("Call");
    std::lock_guard<ffrt::mutex> guard(observerLock_);
    if (abilityStateData.abilityState == static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND)) {
        token_ = abilityStateData.token;
    }
}

void DisposedObserver::OnPageShow(const AppExecFwk::PageStateData &pageStateData)
{
    HILOG_DEBUG("Call");
    if (disposedRule_.componentType == AppExecFwk::ComponentType::UI_ABILITY) {
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*disposedRule_.want));
        if (ret != ERR_OK) {
            UnSubScribeAppState();
            HILOG_ERROR("failed to start disposed ability");
            return;
        }
    }
    if (disposedRule_.componentType == AppExecFwk::ComponentType::UI_EXTENSION) {
        auto abilityRecord = Token::GetAbilityRecordByToken(token_);
        if (!abilityRecord) {
            UnSubScribeAppState();
            HILOG_ERROR("abilityRecord is nullptr");
            return;
        }
        int ret = abilityRecord->CreateModalUIExtension(*disposedRule_.want);
        if (ret != ERR_OK) {
            UnSubScribeAppState();
            HILOG_ERROR("failed to start disposed UIExtension");
            return;
        }
    }
    UnSubScribeAppState();
}
} // namespace AAFwk
} // namespace OHOS