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

#include "quick_fix_manager_client.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "quick_fix_error_utils.h"
#include "quick_fix_load_callback.h"
#include "quick_fix_manager_proxy.h"
#include "quick_fix_utils.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int LOAD_SA_TIMEOUT_MS = 4 * 1000;
} // namespace
std::shared_ptr<QuickFixManagerClient> QuickFixManagerClient::GetInstance()
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "GetInstance called");
    static std::shared_ptr<QuickFixManagerClient> instance = std::make_shared<QuickFixManagerClient>();
    return instance;
}

int32_t QuickFixManagerClient::ApplyQuickFix(const std::vector<std::string> &quickFixFiles, bool isDebug,
    bool isReplace)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");

    auto quickFixMgr = GetQuickFixMgrProxy();
    if (quickFixMgr == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Get quick fix manager service failed");
        return QUICK_FIX_CONNECT_FAILED;
    }

    auto bundleQuickFixMgr = QuickFixUtil::GetBundleQuickFixMgrProxy();
    if (bundleQuickFixMgr == nullptr) {
        return QUICK_FIX_CONNECT_FAILED;
    }

    TAG_LOGD(AAFwkTag::QUICKFIX, "hqf file number need to apply: %{public}zu", quickFixFiles.size());
    std::vector<std::string> destFiles;
    auto copyRet = bundleQuickFixMgr->CopyFiles(quickFixFiles, destFiles);
    if (copyRet != 0) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Copy files failed.");
        return (copyRet == ERR_BUNDLEMANAGER_QUICK_FIX_PERMISSION_DENIED) ? QUICK_FIX_VERIFY_PERMISSION_FAILED :
            QUICK_FIX_COPY_FILES_FAILED;
    }

    return quickFixMgr->ApplyQuickFix(destFiles, isDebug, isReplace);
}

int32_t QuickFixManagerClient::GetApplyedQuickFixInfo(const std::string &bundleName,
    ApplicationQuickFixInfo &quickFixInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");

    auto quickFixMgr = GetQuickFixMgrProxy();
    if (quickFixMgr == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Get quick fix manager service failed");
        return QUICK_FIX_CONNECT_FAILED;
    }

    return quickFixMgr->GetApplyedQuickFixInfo(bundleName, quickFixInfo);
}

sptr<IQuickFixManager> QuickFixManagerClient::GetQuickFixMgrProxy()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");
    auto quickFixMgr = GetQuickFixMgr();
    if (quickFixMgr != nullptr) {
        TAG_LOGD(AAFwkTag::QUICKFIX, "Quick fix manager has been started");
        return quickFixMgr;
    }

    if (!LoadQuickFixMgrService()) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Load quick fix manager service failed");
        return nullptr;
    }

    quickFixMgr = GetQuickFixMgr();
    if (quickFixMgr == nullptr || quickFixMgr->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Failed to get quick fix manager");
        return nullptr;
    }

    auto self = weak_from_this();
    const auto &onClearProxyCallback = [self](const wptr<IRemoteObject> &remote) {
        // Do not touch quickFixMgr_ without mutex_ here; the identity check and reset
        // are both done under the lock inside ClearProxy(remote) to avoid racing with
        // SetQuickFixMgr/ClearProxy from samgr callback or other threads.
        auto impl = self.lock();
        if (impl != nullptr) {
            impl->ClearProxy(remote);
        }
    };

    sptr<QfmsDeathRecipient> recipient(new (std::nothrow) QfmsDeathRecipient(onClearProxyCallback));
    quickFixMgr->AsObject()->AddDeathRecipient(recipient);

    return quickFixMgr;
}

int32_t QuickFixManagerClient::RevokeQuickFix(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");

    auto quickFixMgr = GetQuickFixMgrProxy();
    if (quickFixMgr == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Get quick fix manager service failed");
        return QUICK_FIX_CONNECT_FAILED;
    }

    auto retval = quickFixMgr->RevokeQuickFix(bundleName);
    TAG_LOGD(AAFwkTag::QUICKFIX, "Function call end, retval is %{public}d", retval);
    return retval;
}

void QuickFixManagerClient::ClearProxy()
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    quickFixMgr_ = nullptr;
}

void QuickFixManagerClient::ClearProxy(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::QUICKFIX, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (quickFixMgr_ == nullptr) {
        TAG_LOGI(AAFwkTag::QUICKFIX, "null proxy_, no need clear");
        return;
    }

    // Compare the remote binder object with the one wrapped by current proxy, instead of
    // the proxy object itself: quickFixMgr_ points to a QuickFixManagerProxy instance whose
    // address never equals the binder object's. Check and reset under the same lock so that
    // a proxy reconnected by SetQuickFixMgr right after the SA died will not be cleared.
    auto serviceRemote = quickFixMgr_->AsObject();
    if (serviceRemote != nullptr && serviceRemote == remote.promote()) {
        TAG_LOGI(AAFwkTag::QUICKFIX, "quick fix manager service died, clear proxy");
        quickFixMgr_ = nullptr;
    }
}

void QuickFixManagerClient::QfmsDeathRecipient::OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject> &remote)
{
    if (proxy_ != nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "quick fix manager service died");
        proxy_(remote);
    }
}

bool QuickFixManagerClient::LoadQuickFixMgrService()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    {
        std::unique_lock<std::mutex> lock(loadSaMutex_);
        loadSaFinished_ = false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "GetSystemAbilityManager");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Failed to get SystemAbilityManager");
        return false;
    }

    sptr<QuickFixLoadCallback> loadCallback = new (std::nothrow) QuickFixLoadCallback();
    if (loadCallback == nullptr) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Create load callback failed");
        return false;
    }

    auto ret = systemAbilityMgr->LoadSystemAbility(QUICK_FIX_MGR_SERVICE_ID, loadCallback);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::QUICKFIX, "Load system ability %{public}d failed with %{public}d", QUICK_FIX_MGR_SERVICE_ID,
            ret);
        return false;
    }

    {
        std::unique_lock<std::mutex> lock(loadSaMutex_);
        auto waitStatus = loadSaCondation_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
            [this]() {
                return loadSaFinished_;
            });
        if (!waitStatus) {
            TAG_LOGE(AAFwkTag::QUICKFIX, "Wait for load sa timeout");
            return false;
        }
    }

    return true;
}

void QuickFixManagerClient::SetQuickFixMgr(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(mutex_);
    quickFixMgr_ = iface_cast<IQuickFixManager>(remoteObject);
}

sptr<IQuickFixManager> QuickFixManagerClient::GetQuickFixMgr()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return quickFixMgr_;
}

void QuickFixManagerClient::OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    SetQuickFixMgr(remoteObject);
    std::unique_lock<std::mutex> lock(loadSaMutex_);
    loadSaFinished_ = true;
    loadSaCondation_.notify_one();
}

void QuickFixManagerClient::OnLoadSystemAbilityFail()
{
    SetQuickFixMgr(nullptr);
    std::unique_lock<std::mutex> lock(loadSaMutex_);
    loadSaFinished_ = true;
    loadSaCondation_.notify_one();
}
}  // namespace AAFwk
}  // namespace OHOS
