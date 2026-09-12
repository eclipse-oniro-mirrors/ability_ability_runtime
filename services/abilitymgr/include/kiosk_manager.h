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
#include <unordered_map>
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

    // Legacy EDM path, global whitelist, no data isolation. Kept unchanged.
    int32_t UpdateKioskApplicationList(const std::vector<std::string> &appList);

    // append apps to the caller's own kiosk whitelist (no clear). Empty appList is a no-op.
    int32_t AddKioskApplicationList(const std::vector<std::string> &appList);

    // delete caller's own kiosk application list entries.
    int32_t DeleteKioskApplicationList(const std::vector<std::string> &appList);

    // Enter kiosk mode. The target app is identified by callerToken (self or proxied app).
    // kioskType is transparently passed to WMS and common events.
    int32_t EnterKioskMode(sptr<IRemoteObject> callerToken, int32_t kioskType = 0);

    // Exit kiosk mode. The target app is identified by callerToken. isFoundation=true bypasses
    // permission/uid checks, used by system auto-exit on app death.
    int32_t ExitKioskMode(sptr<IRemoteObject> callerToken, bool isFoundation = false);

    int32_t GetKioskStatus(KioskStatus &kioskStatus);
    bool IsInKioskMode();
    bool IsInWhiteList(const std::string &bundleName);
    void OnAppStop(const AppInfo &info);
    void FilterDialogAppInfos(std::vector<DialogAppInfo> &dialogAppInfos);
    void FilterAbilityInfos(std::vector<AppExecFwk::AbilityInfo> &abilityInfos);
    bool IsKioskBundleUid(int32_t uid);
    bool ShouldIntercept(const std::string &bundleName);

private:
    KioskManager() = default;
    DISALLOW_COPY_AND_MOVE(KioskManager);
    int32_t ExitKioskModeInner(const std::string &bundleName, sptr<IRemoteObject> callerToken, bool isFoundation);
    int32_t VerifyUpdatePermissions();
    int32_t VerifyKioskPermissions();
    bool IsInKioskModeInner();
    bool IsInWhiteListInner(const std::string &bundleName);
    bool IsSAProxyInKioskWhitelist(int32_t callerUid, const std::string &bundleName);
    std::function<void()> GetEnterKioskModeCallback();
    std::function<void()> GetExitKioskModeCallback();
    void NotifyKioskModeChanged(bool isInKioskMode, const std::string &bundleName,
        int32_t kioskBundleUid, int32_t kioskType);
    void AddKioskInterceptor();
    void RemoveKioskInterceptor();
    int32_t CheckAndExitIfOutOfList(const std::vector<std::string> &appList);
    int32_t CheckAndExitIfOutOfActiveList(int32_t callerUid, const std::vector<std::string> &appList);
    int32_t NotifyWmsUpdateAppList(const std::vector<std::string> &appList);

    std::unordered_set<std::string> whitelist_;
    std::unordered_map<int32_t, std::unordered_set<std::string>> kioskWhitelistMap_;
    KioskStatus kioskStatus_;
    std::mutex kioskManagerMutex_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_KIOSK_MANAGER_H
