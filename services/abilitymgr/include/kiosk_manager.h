/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_KIOSK_MANAGER_H
#define OHOS_ABILITY_RUNTIME_KIOSK_MANAGER_H

#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "app_scheduler.h"
#include "interceptor/ability_interceptor_executer.h"
#include "kiosk_status.h"
#include "nocopyable.h"
#include "system_dialog_scheduler.h"

namespace OHOS {
namespace AAFwk {
class KioskManager {
public:
    static KioskManager &GetInstance();
    int32_t UpdateKioskApplicationList(const std::vector<std::string> &appList);
    int32_t EnterKioskMode(sptr<IRemoteObject> callerToken);
    int32_t ExitKioskMode(sptr<IRemoteObject> callerToken, bool isFoundation);
    int32_t GetKioskStatus(KioskStatus &kioskStatus);
    bool IsInKioskMode();
    bool IsInWhiteList(const std::string &bundleName);
    void OnAppStop(const AppInfo &info);
    void FilterDialogAppInfos(std::vector<DialogAppInfo> &dialogAppInfos);
    void FilterAbilityInfos(std::vector<AppExecFwk::AbilityInfo> &abilityInfos);
    bool IsKioskBundleUid(int32_t uid);

private:
    KioskManager() = default;
    DISALLOW_COPY_AND_MOVE(KioskManager);
    int32_t ExitKioskModeInner(const std::string &bundleName, sptr<IRemoteObject> callerToken,
        bool isFoundation);
    bool IsInKioskModeInner();
    void NotifyKioskModeChanged(bool isInKioskMode, const std::string &bundleName, int32_t kioskBundleUid);
    bool IsInWhiteListInner(const std::string &bundleName);
    std::function<void()> GetEnterKioskModeCallback();
    std::function<void()> GetExitKioskModeCallback();
    void AddKioskInterceptor();
    void RemoveKioskInterceptor();
    bool CheckCallerIsForeground(sptr<IRemoteObject> callerToken);

    std::unordered_set<std::string> whitelist_;
    KioskStatus kioskStatus_;
    std::mutex kioskManagerMutex_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_KIOSK_MANAGER_H
