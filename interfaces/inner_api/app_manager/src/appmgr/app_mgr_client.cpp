/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "app_mgr_client.h"

#include <cstdio>
#include <string>
#include <unistd.h>

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"

#include "app_mem_info.h"
#include "app_mgr_interface.h"
#include "app_service_manager.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "param.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrRemoteHolder : public std::enable_shared_from_this<AppMgrRemoteHolder> {
public:
    AppMgrRemoteHolder() = default;

    ~AppMgrRemoteHolder()
    {
        RemoveDeathRecipient();
    }

    void SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        serviceManager_ = std::move(serviceMgr);
    }

    AppMgrResultCode ConnectAppMgrService()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return ConnectAppMgrServiceInner();
    }

    sptr<IRemoteObject> GetRemoteObject()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!remote_) {
            (void) ConnectAppMgrServiceInner();
        }
        return remote_;
    }

private:
    void HandleRemoteDied(const wptr<IRemoteObject>& remote)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!remote_) {
            return;
        }

        if (remote_ == remote.promote()) {
            remote_->RemoveDeathRecipient(deathRecipient_);
            remote_ = nullptr;
            deathRecipient_ = nullptr;
        }
    }

    AppMgrResultCode ConnectAppMgrServiceInner()
    {
        if (!serviceManager_) {
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
        if (remote_) {
            return AppMgrResultCode::RESULT_OK;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "get AppMgrRemote object");
        remote_ = serviceManager_->GetAppMgrService();
        if (!remote_) {
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }

        auto me = shared_from_this();
        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new AppMgrDeathRecipient(me));
        if (deathRecipient_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "create AppMgrDeathRecipient failed");
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
        if ((remote_->IsProxyObject()) && (!remote_->AddDeathRecipient(deathRecipient_))) {
            TAG_LOGE(AAFwkTag::APPMGR, "AddDeathRecipient to AppMs failed");
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }

        return AppMgrResultCode::RESULT_OK;
    }

    void RemoveDeathRecipient()
    {
        TAG_LOGD(AAFwkTag::APPMGR, "RemoveDeathRecipient");
        std::lock_guard<std::mutex> lock(mutex_);
        if (remote_ == nullptr) {
            TAG_LOGI(AAFwkTag::APPMGR, "null remote_");
            return;
        }
        if (deathRecipient_ == nullptr) {
            TAG_LOGI(AAFwkTag::APPMGR, "null deathRecipient_");
            return;
        }
        bool ret = remote_->RemoveDeathRecipient(deathRecipient_);
        if (!ret) {
            TAG_LOGW(AAFwkTag::APPMGR, "RemoveDeathRecipient fail");
            return;
        }
        remote_ = nullptr;
        deathRecipient_ = nullptr;
        TAG_LOGD(AAFwkTag::APPMGR, "RemoveDeathRecipient success");
    }

    class AppMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AppMgrDeathRecipient(const std::shared_ptr<AppMgrRemoteHolder>& holder) : owner_(holder) {}

        virtual ~AppMgrDeathRecipient() = default;

        void OnRemoteDied(const wptr<IRemoteObject>& remote) override
        {
            std::shared_ptr<AppMgrRemoteHolder> holder = owner_.lock();
            if (holder) {
                holder->HandleRemoteDied(remote);
            }
        }

    private:
        std::weak_ptr<AppMgrRemoteHolder> owner_;
    };

private:
    std::unique_ptr<AppServiceManager> serviceManager_;
    sptr<IRemoteObject> remote_;
    std::mutex mutex_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};

AppMgrClient::AppMgrClient()
{
    SetServiceManager(std::make_unique<AppServiceManager>());
}

AppMgrClient::~AppMgrClient()
{}

AppMgrResultCode AppMgrClient::LoadAbility(const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo,
    const AAFwk::Want &want, AbilityRuntime::LoadParam loadParam)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            // From here, separate AbilityInfo and ApplicationInfo from AA.
            std::shared_ptr<AbilityInfo> abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
            std::shared_ptr<ApplicationInfo> appInfoPtr = std::make_shared<ApplicationInfo>(appInfo);
            std::shared_ptr<AAFwk::Want> wantPtr = std::make_shared<AAFwk::Want>(want);
            auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
            amsService->LoadAbility(abilityInfoPtr, appInfoPtr, wantPtr, loadParamPtr);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

void AppMgrClient::NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->NotifyLoadAbilityFinished(callingPid, targetPid, callbackId);
        }
    }
}

AppMgrResultCode AppMgrClient::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->TerminateAbility(token, clearMissionFlag);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->UpdateAbilityState(token, state);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->UpdateExtensionState(token, state);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->RegisterAppStateCallback(callback);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->KillProcessByAbilityToken(token);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
    sptr<AAFwk::IUserCallback> callback)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->KillProcessesByUserId(userId, isNeedSendAppSpawnMsg, callback);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    if (callback) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        callback->OnLogoutUserDone(userId, AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED);
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillProcessesByPids(const std::vector<int32_t> &pids, const std::string &reason,
    bool subProcess, bool isKillPrecedeStart)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t ret = amsService->KillProcessesByPids(pids, reason, subProcess, isKillPrecedeStart);
            if (ret == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_KILL_PROCESSES_BY_PIDS;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::AttachPidToParent(const sptr<IRemoteObject> &token,
    const sptr<IRemoteObject> &callerToken)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->AttachPidToParent(token, callerToken);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid,
    const std::string &moduleName, bool isPlugin)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, isPlugin);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillApplication(const std::string &bundleName, bool clearPageStack, int32_t appIndex,
    const std::string &reason)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->KillApplication(bundleName, clearPageStack, appIndex, reason);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::ForceKillApplication(const std::string &bundleName,
    const int userId, const int appIndex)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->ForceKillApplication(bundleName, userId, appIndex);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->KillProcessesByAccessTokenId(accessTokenId);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillApplicationByUid(const std::string &bundleName, const int uid,
    const std::string& reason)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->KillApplicationByUid(bundleName, uid, reason);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid, bool isUpgrade)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->NotifyUninstallOrUpgradeApp(bundleName, uid, isUpgrade);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

void AppMgrClient::NotifyUninstallOrUpgradeAppEnd(int32_t uid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr");
        return;
    }
    amsService->NotifyUninstallOrUpgradeAppEnd(uid);
}

AppMgrResultCode AppMgrClient::KillApplicationSelf(const bool clearPageStack, const std::string& reason)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->KillApplicationSelf(clearPageStack, reason);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_KILL_APPLICATION;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

int32_t AppMgrClient::UpdateProcessMemoryState(const std::vector<AppExecFwk::ProcessMemoryState> &procMemState)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UpdateProcessMemoryState(procMemState);
}

AppMgrResultCode AppMgrClient::ClearUpApplicationData(const std::string &bundleName, int32_t appCloneIndex,
    int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->ClearUpApplicationData(bundleName, appCloneIndex, userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::ClearUpApplicationDataBySelf(int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->ClearUpApplicationDataBySelf(userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetAllRunningProcesses(info);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetProcessRunningInfosByUserId(info, userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetProcessRunningInformation(AppExecFwk::RunningProcessInfo &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetProcessRunningInformation(info);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetAllRunningInstanceKeysBySelf(instanceKeys);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        TAG_LOGE(AAFwkTag::APPMGR, "GetAllRunningInstanceKeysBySelf returns result=%{public}d", result);
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
    std::vector<std::string> &instanceKeys, int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        TAG_LOGE(AAFwkTag::APPMGR, "GetAllRunningInstanceKeysByBundleName returns result=%{public}d", result);
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetAllRenderProcesses(std::vector<RenderProcessInfo> &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetAllRenderProcesses(info);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info)
{
#ifdef SUPPORT_CHILD_PROCESS
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    int32_t result = service->GetAllChildrenProcesses(info);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "service->GetAllChildrenProcesses failed,result=%{public}d", result);
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
#endif // SUPPORT_CHILD_PROCESS
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::NotifyMemoryLevel(MemoryLevel level)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());

    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return AppMgrResultCode(service->NotifyMemoryLevel(level));
}

AppMgrResultCode AppMgrClient::NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());

    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return AppMgrResultCode(service->NotifyProcMemoryLevel(procLevelMap));
}

AppMgrResultCode AppMgrClient::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "DumpHeapMemory: service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return AppMgrResultCode(service->DumpHeapMemory(pid, mallocInfo));
}

AppMgrResultCode AppMgrClient::DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "DumpJsHeapMemory: service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return AppMgrResultCode(service->DumpJsHeapMemory(info));
}

AppMgrResultCode AppMgrClient::DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "DumpCjHeapMemory: service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    int32_t result = service->DumpCjHeapMemory(info);
    switch (result) {
        case static_cast<int32_t>(AppMgrResultCode::RESULT_OK):
            return AppMgrResultCode::RESULT_OK;
        case static_cast<int32_t>(AppMgrResultCode::ERROR_SERVICE_NOT_READY):
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        case static_cast<int32_t>(AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED):
            return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
        case static_cast<int32_t>(AppMgrResultCode::ERROR_KILL_APPLICATION):
            return AppMgrResultCode::ERROR_KILL_APPLICATION;
        case static_cast<int32_t>(AppMgrResultCode::ERROR_KILL_PROCESSES_BY_PIDS):
            return AppMgrResultCode::ERROR_KILL_PROCESSES_BY_PIDS;
        default:
            return static_cast<AppMgrResultCode>(result);
    }
}

AppMgrResultCode AppMgrClient::GetConfiguration(Configuration& config)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetConfiguration(config);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetConfiguration(Configuration& config, int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetConfiguration(config, userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::ConnectAppMgrService()
{
    if (mgrHolder_) {
        return mgrHolder_->ConnectAppMgrService();
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
}

bool AppMgrClient::IsProcessContainsOnlyUIAbility(const pid_t pid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            return amsService->IsProcessContainsOnlyUIAbility(pid);
        }
    }
    return false;
}

void AppMgrClient::SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr)
{
    if (!mgrHolder_) {
        mgrHolder_ = std::make_shared<AppMgrRemoteHolder>();
    }
    mgrHolder_->SetServiceManager(std::move(serviceMgr));
}

void AppMgrClient::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->AbilityAttachTimeOut(token);
}

void AppMgrClient::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->PrepareTerminate(token, clearMissionFlag);
}

void AppMgrClient::GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->GetRunningProcessInfoByToken(token, info);
        }
    }
}

int32_t AppMgrClient::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->GetRunningProcessInfoByPid(pid, info);
}

int32_t AppMgrClient::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->GetRunningProcessInfoByChildProcessPid(childPid, info);
}

void AppMgrClient::SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->SetAbilityForegroundingFlagToAppRecord(pid);
        }
    }
}

void AppMgrClient::AddAbilityStageDone(const int32_t recordId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return;
    }

    service->AddAbilityStageDone(recordId);
}

void AppMgrClient::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return;
    }

    service->StartupResidentProcess(bundleInfos);
}

int AppMgrClient::StartUserTestProcess(
    const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return service->StartUserTestProcess(want, observer, bundleInfo, userId);
}

int AppMgrClient::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return service->FinishUserTest(msg, resultCode, bundleName);
}

void AppMgrClient::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->StartSpecifiedAbility(want, abilityInfo, requestId, customProcess);
}

void AppMgrClient::PrepareTerminateApp(const pid_t pid, const std::string &moduleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr");
        return;
    }
    amsService->PrepareTerminateApp(pid, moduleName);
}

void AppMgrClient::SetKeepAliveEnableState(const std::string &bundleName, bool enable, int32_t uid)
{
    if (!IsAmsServiceReady()) {
        return;
    }
    amsService_->SetKeepAliveEnableState(bundleName, enable, uid);
}

void AppMgrClient::SetKeepAliveDkv(const std::string &bundleName, bool enable, int32_t uid)
{
    if (!IsAmsServiceReady()) {
        return;
    }
    amsService_->SetKeepAliveDkv(bundleName, enable, uid);
}

void AppMgrClient::SetKeepAliveAppService(const std::string &bundleName, bool enable, int32_t uid)
{
    if (!IsAmsServiceReady()) {
        return;
    }
    amsService_->SetKeepAliveAppService(bundleName, enable, uid);
}

void AppMgrClient::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->StartSpecifiedProcess(want, abilityInfo, requestId, customProcess);
}

void AppMgrClient::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->RegisterStartSpecifiedAbilityResponse(response);
}

void AppMgrClient::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return;
    }

    service->ScheduleAcceptWantDone(recordId, want, flag);
}

AppMgrResultCode AppMgrClient::UpdateConfiguration(const Configuration &config, const int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    service->UpdateConfiguration(config, userId);
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::UpdateConfigurationByUserIds(
    const Configuration &config, const std::vector<int32_t> userIds)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    service->UpdateConfigurationByUserIds(config, userIds);
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::UpdateConfigurationByBundleName(const Configuration &config, const std::string &name,
    int32_t appIndex)
{
    if (!mgrHolder_) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    service->UpdateConfigurationByBundleName(config, name, appIndex);
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer,
    const int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->RegisterConfigurationObserver(observer, userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->UnregisterConfigurationObserver(observer);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

int AppMgrClient::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }

    return service->GetAbilityRecordsByProcessID(pid, tokens);
}

int AppMgrClient::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService->GetApplicationInfoByProcessID(pid, application, debug);
}

int32_t AppMgrClient::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService->NotifyAppMgrRecordExitReason(pid, reason, exitMsg);
}

int32_t AppMgrClient::StartNativeProcessForDebugger(const AAFwk::Want &want)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->StartNativeProcessForDebugger(want);
}

int AppMgrClient::PreStartNWebSpawnProcess()
{
    TAG_LOGI(AAFwkTag::APPMGR, "PreStartNWebSpawnProcess");

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        return service->PreStartNWebSpawnProcess();
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

int AppMgrClient::StartRenderProcess(const std::string &renderParam,
                                     int32_t ipcFd, int32_t sharedFd,
                                     int32_t crashFd, pid_t &renderPid, bool isGPU)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        return service->StartRenderProcess(renderParam, ipcFd, sharedFd, crashFd,
                                           renderPid, isGPU);
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

void AppMgrClient::AttachRenderProcess(const sptr<IRenderScheduler> &renderScheduler)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!renderScheduler) {
        TAG_LOGI(AAFwkTag::APPMGR, "renderScheduler is nullptr");
        return;
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "AttachRenderProcess");
        service->AttachRenderProcess(renderScheduler->AsObject());
    }
}

int AppMgrClient::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        return service->GetRenderProcessTerminationStatus(renderPid, status);
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

sptr<IRemoteObject> AppMgrClient::GetRemoteObject()
{
    return mgrHolder_->GetRemoteObject();
}

void AppMgrClient::SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->SetEnableStartProcessFlagByUserId(userId, enableStartProcess);
}

int32_t AppMgrClient::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }

    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService != nullptr) {
        return amsService->GetBundleNameByPid(pid, bundleName, uid);
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

int32_t AppMgrClient::NotifyAppFault(const FaultData &faultData)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->NotifyAppFault(faultData);
}

int32_t AppMgrClient::NotifyAppFaultBySA(const AppFaultDataBySA &faultData)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->NotifyAppFaultBySA(faultData);
}

bool AppMgrClient::SetAppFreezeFilter(int32_t pid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return false;
    }
    return service->SetAppFreezeFilter(pid);
}

int32_t AppMgrClient::ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "tid is %{private}" PRIu64, tid);
    if (mgrHolder_ == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->ChangeAppGcState(pid, state, tid);
}

int32_t AppMgrClient::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->RegisterAppDebugListener(listener);
}

int32_t AppMgrClient::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->UnregisterAppDebugListener(listener);
}

int32_t AppMgrClient::AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->AttachAppDebug(bundleName, isDebugFromLocal);
}

int32_t AppMgrClient::DetachAppDebug(const std::string &bundleName)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->DetachAppDebug(bundleName);
}

int32_t AppMgrClient::SetAppWaitingDebug(const std::string &bundleName, bool isPersist)
{
    if (!IsAmsServiceReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "App manager service is not ready.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->SetAppWaitingDebug(bundleName, isPersist);
}

int32_t AppMgrClient::CancelAppWaitingDebug()
{
    if (!IsAmsServiceReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "App manager service is not ready.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->CancelAppWaitingDebug();
}

int32_t AppMgrClient::GetWaitingDebugApp(std::vector<std::string> &debugInfoList)
{
    if (!IsAmsServiceReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "App manager service is not ready.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->GetWaitingDebugApp(debugInfoList);
}

bool AppMgrClient::IsWaitingDebugApp(const std::string &bundleName)
{
    if (!IsAmsServiceReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "App manager service is not ready.");
        return false;
    }
    return amsService_->IsWaitingDebugApp(bundleName);
}

void AppMgrClient::ClearNonPersistWaitingDebugFlag()
{
    if (!IsAmsServiceReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "App manager service is not ready.");
        return;
    }
    amsService_->ClearNonPersistWaitingDebugFlag();
}

int32_t AppMgrClient::RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->RegisterAbilityDebugResponse(response);
}

bool AppMgrClient::IsAttachDebug(const std::string &bundleName)
{
    if (!IsAmsServiceReady()) {
        return false;
    }
    return amsService_->IsAttachDebug(bundleName);
}

bool AppMgrClient::IsAmsServiceReady()
{
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder_ is nullptr.");
        return false;
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App manager service is nullptr.");
        return false;
    }

    amsService_ = service->GetAmsMgr();
    if (amsService_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService_ is nullptr.");
        return false;
    }
    return true;
}

int32_t AppMgrClient::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
    const std::vector<std::string> &bundleNameList)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->RegisterApplicationStateObserver(observer, bundleNameList);
}

int32_t AppMgrClient::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UnregisterApplicationStateObserver(observer);
}

int32_t AppMgrClient::RegisterNativeChildExitNotify(sptr<INativeChildNotify> notify)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->RegisterNativeChildExitNotify(notify);
}

int32_t AppMgrClient::UnregisterNativeChildExitNotify(sptr<INativeChildNotify> notify)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UnregisterNativeChildExitNotify(notify);
}

int32_t AppMgrClient::NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->NotifyPageShow(token, pageStateData);
}

int32_t AppMgrClient::NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->NotifyPageHide(token, pageStateData);
}

int32_t AppMgrClient::RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Listener is nullptr.");
        return ERR_INVALID_DATA;
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->RegisterAppRunningStatusListener(listener);
}

int32_t AppMgrClient::UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Listener is nullptr.");
        return ERR_INVALID_DATA;
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UnregisterAppRunningStatusListener(listener);
}

void AppMgrClient::ClearProcessByToken(sptr<IRemoteObject> token) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr.");
        return;
    }
    amsService->ClearProcessByToken(token);
}

bool AppMgrClient::IsFinalAppProcess()
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return false;
    }
    return service->IsFinalAppProcess();
}

int32_t AppMgrClient::RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->RegisterRenderStateObserver(observer);
}

int32_t AppMgrClient::UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UnregisterRenderStateObserver(observer);
}

int32_t AppMgrClient::UpdateRenderState(pid_t renderPid, int32_t state)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UpdateRenderState(renderPid, state);
}

int32_t AppMgrClient::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
}

int32_t AppMgrClient::GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->GetAllUIExtensionRootHostPid(pid, hostPids);
}

int32_t AppMgrClient::GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->GetAllUIExtensionProviderPid(hostPid, providerPids);
}

int32_t AppMgrClient::NotifyMemorySizeStateChanged(int32_t memorySizeState)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->NotifyMemorySizeStateChanged(memorySizeState);
}

bool AppMgrClient::IsMemorySizeSufficient() const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return true;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr.");
        return true;
    }
    return amsService->IsMemorySizeSufficient();
}

bool AppMgrClient::IsNoRequireBigMemory() const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return true;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr.");
        return true;
    }
    return amsService->IsNoRequireBigMemory();
}

int32_t AppMgrClient::PreloadApplication(const std::string &bundleName, int32_t userId,
    AppExecFwk::PreloadMode preloadMode, int32_t appIndex)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->PreloadApplication(bundleName, userId, preloadMode, appIndex);
}

int32_t AppMgrClient::SetSupportedProcessCacheSelf(bool isSupport)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->SetSupportedProcessCacheSelf(isSupport);
}

int32_t AppMgrClient::SetSupportedProcessCache(int32_t pid, bool isSupport)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->SetSupportedProcessCache(pid, isSupport);
}

int32_t AppMgrClient::IsProcessCacheSupported(int32_t pid, bool &isSupported)
{
    TAG_LOGD(AAFwkTag::APPMGR, "IsProcessCacheSupported called");
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder_ is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->IsProcessCacheSupported(pid, isSupported);
}

int32_t AppMgrClient::SetProcessCacheEnable(int32_t pid, bool enable)
{
    TAG_LOGD(AAFwkTag::APPMGR, "SetProcessCacheEnable called");
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder_ is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->SetProcessCacheEnable(pid, enable);
}

int32_t AppMgrClient::LockProcessCache(int32_t pid, bool isLock)
{
    TAG_LOGD(AAFwkTag::APPMGR, "LockProcessCache called");
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder_ is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->LockProcessCache(pid, isLock);
}

void AppMgrClient::SaveBrowserChannel(sptr<IRemoteObject> browser)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return;
    }
    service->SaveBrowserChannel(browser);
}

int32_t AppMgrClient::CheckCallingIsUserTestMode(const pid_t pid, bool &isUserTest)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        return service->CheckCallingIsUserTestMode(pid, isUserTest);
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::AttachedToStatusBar(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->AttachedToStatusBar(token);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    TAG_LOGE(AAFwkTag::APPMGR, "Service is not connected.");
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

int32_t AppMgrClient::NotifyProcessDependedOnWeb()
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    return service->NotifyProcessDependedOnWeb();
}

void AppMgrClient::KillProcessDependedOnWeb()
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    service->KillProcessDependedOnWeb();
}

AppMgrResultCode AppMgrClient::BlockProcessCacheByPids(const std::vector<int32_t> &pids)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->BlockProcessCacheByPids(pids);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

bool AppMgrClient::IsKilledForUpgradeWeb(const std::string &bundleName)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return false;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr.");
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    return amsService->IsKilledForUpgradeWeb(bundleName);
}

bool AppMgrClient::CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "get appmgrservice is nullptr.");
        return false;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "get abilityms service is nullptr.");
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    return amsService->CleanAbilityByUserRequest(token);
}

bool AppMgrClient::IsProcessAttached(sptr<IRemoteObject> token) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return false;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return false;
    }
    return amsService->IsProcessAttached(token);
}

bool AppMgrClient::IsCallerKilling(const std::string& callerKey) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return false;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return false;
    }
    return amsService->IsCallerKilling(callerKey);
}

int32_t AppMgrClient::PreloadApplicationByPhase(const std::string &bundleName, int32_t userId, int32_t appIndex,
    AppExecFwk::PreloadPhase preloadPhase)
{
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService->PreloadApplicationByPhase(bundleName, userId, appIndex, preloadPhase);
}

int32_t AppMgrClient::NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token, bool isPreForeground)
{
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService->NotifyPreloadAbilityStateChanged(token, isPreForeground);
}

int32_t AppMgrClient::CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId, int32_t appIndex,
    bool &isExist)
{
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService->CheckPreloadAppRecordExist(bundleName, userId, appIndex, isExist);
}

AppMgrResultCode AppMgrClient::IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
    bool &isRunning)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        return AppMgrResultCode(service->IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning));
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

int32_t AppMgrClient::PromoteCurrentToCandidateMasterProcess(bool isInsertToHead)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->PromoteCurrentToCandidateMasterProcess(isInsertToHead);
}

int32_t AppMgrClient::DemoteCurrentFromCandidateMasterProcess()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->DemoteCurrentFromCandidateMasterProcess();
}

int32_t AppMgrClient::ExitMasterProcessRole()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->ExitMasterProcessRole();
}

void AppMgrClient::SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return;
    }
    service->SetSpecifiedProcessRequestId(recordId, requestId);
}

void AppMgrClient::AllowScbProcessMoveToBackground()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return;
    }
    service->AllowScbProcessMoveToBackground();
}

int32_t AppMgrClient::QueryRunningSharedBundles(pid_t pid, std::map<std::string, uint32_t> &sharedBundles)
{
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->QueryRunningSharedBundles(pid, sharedBundles);
}

int32_t AppMgrClient::VerifyKillProcessPermission(const std::string &bundleName) const
{
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService->VerifyKillProcessPermission(bundleName);
}

int32_t AppMgrClient::RegisterApplicationStateObserverWithFilter(sptr<IApplicationStateObserver> observer,
    const std::vector<std::string> &bundleNameList, const AppStateFilter &appStateFilter, bool isUsingFilter)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->RegisterApplicationStateObserverWithFilter(observer,
        bundleNameList, appStateFilter, isUsingFilter);
}

int32_t AppMgrClient::KillChildProcessByPid(int32_t pid)
{
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder is nullptr.");
        return ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->KillChildProcessByPid(pid);
}

int32_t AppMgrClient::PreloadExtension(const AAFwk::Want &want, int32_t appIndex, int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->PreloadExtension(want, appIndex, userId);
}

}  // namespace AppExecFwk
}  // namespace OHOS
