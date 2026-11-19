/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_CLIENT_H

#include <condition_variable>
#include <functional>
#include <vector>

#include "iquick_fix_manager.h"
#include "quick_fix_info.h"

namespace OHOS {
namespace AAFwk {
using ClearProxyCallback = std::function<void(const wptr<IRemoteObject>&)>;

class QuickFixManagerClient : public std::enable_shared_from_this<QuickFixManagerClient> {
public:
    QuickFixManagerClient() = default;
    virtual ~QuickFixManagerClient() = default;
    static std::shared_ptr<QuickFixManagerClient> GetInstance();

    /**
     * @brief Apply quick fix.
     *
     * @param quickFixFiles quick fix files need to apply, this value should include file path and file name.
     * @param isDebug this value is for the quick fix debug mode selection.
     * @param isReplace this value is for the quick fix replace mode selection.
     * @return returns 0 on success, error code on failure.
     */
    int32_t ApplyQuickFix(const std::vector<std::string> &quickFixFiles, bool isDebug = false, bool isReplace = false);

    /**
     * @brief Get applyed quick fix info.
     *
     * @param bundleName bundle name of quick fix info.
     * @param quickFixInfo quick fix info, including bundleName, bundleVersion and so on.
     * @return int32_t returns 0 on success, error code on failure.
     */
    int32_t GetApplyedQuickFixInfo(const std::string &bundleName, ApplicationQuickFixInfo &quickFixInfo);

    /**
     * @brief Revoke quick fix by bundle name.
     *
     * @param bundleName quick fix files need to revoke.
     * @return returns QUICK_FIX_OK on success, error code on failure.
     */
    int32_t RevokeQuickFix(const std::string &bundleName);

    void OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void OnLoadSystemAbilityFail();

private:
    sptr<IQuickFixManager> GetQuickFixMgrProxy();
    void ClearProxy();
    void ClearProxy(const wptr<IRemoteObject> &remote);
    bool LoadQuickFixMgrService();
    void SetQuickFixMgr(const sptr<IRemoteObject> &remoteObject);
    sptr<IQuickFixManager> GetQuickFixMgr();

    class QfmsDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit QfmsDeathRecipient(const ClearProxyCallback &proxy) : proxy_(proxy) {}
        virtual ~QfmsDeathRecipient() = default;
        void OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject> &remote) override;

    private:
        ClearProxyCallback proxy_;
    };

private:
    std::condition_variable loadSaCondation_;
    std::mutex loadSaMutex_;
    bool loadSaFinished_;
    std::mutex mutex_;
    sptr<IQuickFixManager> quickFixMgr_ = nullptr;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_CLIENT_H
