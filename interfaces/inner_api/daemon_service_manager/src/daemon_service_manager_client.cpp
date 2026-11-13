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

#include "daemon_service_manager_client.h"

#include "ability_manager_errors.h"
#include "app_mgr_proxy.h"
#include "app_service_manager.h"
#include "launch_service_operation_callback.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
sptr<AppExecFwk::IAppMgr> GetAppMgrProxy()
{
    auto remote = AppExecFwk::AppServiceManager().GetAppMgrService();
    if (remote == nullptr) {
        return nullptr;
    }
    return sptr<AppExecFwk::AppMgrProxy>::MakeSptr(remote);
}

template <typename Callable> int32_t InvokeAppMgr(Callable &&call)
{
    auto proxy = GetAppMgrProxy();
    if (proxy == nullptr) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    return call(proxy);
}

AAFwk::LaunchServiceDescriptor ConvertDescriptor(const LaunchServiceDescriptor &descriptor)
{
    AAFwk::LaunchServiceDescriptor result;
    result.bundleName = descriptor.bundleName;
    result.moduleName = descriptor.moduleName;
    result.serviceName = descriptor.serviceName;
    result.userId = descriptor.userId;
    result.appIndex = descriptor.appIndex;
    return result;
}
} // namespace

DaemonServiceManagerClient &DaemonServiceManagerClient::GetInstance()
{
    static DaemonServiceManagerClient instance;
    return instance;
}

int32_t DaemonServiceManagerClient::NotifyResidentStartupCompleted()
{
    return InvokeAppMgr([](const auto &proxy) { return proxy->NotifyResidentStartupCompleted(); });
}

int32_t DaemonServiceManagerClient::Register(const LaunchServiceDescriptor &descriptor)
{
    const auto appMgrDescriptor = ConvertDescriptor(descriptor);
    return InvokeAppMgr([&appMgrDescriptor](const auto &proxy) { return proxy->RegisterDaemon(appMgrDescriptor); });
}

int32_t DaemonServiceManagerClient::Unregister(const LaunchServiceDescriptor &descriptor)
{
    const auto appMgrDescriptor = ConvertDescriptor(descriptor);
    return InvokeAppMgr([&appMgrDescriptor](const auto &proxy) { return proxy->UnregisterDaemon(appMgrDescriptor); });
}

int32_t DaemonServiceManagerClient::Start(const LaunchServiceDescriptor &descriptor)
{
    const auto appMgrDescriptor = ConvertDescriptor(descriptor);
    return InvokeAppMgr([&appMgrDescriptor](const auto &proxy) { return proxy->StartDaemon(appMgrDescriptor); });
}

int32_t DaemonServiceManagerClient::Stop(const LaunchServiceDescriptor &descriptor)
{
    const auto appMgrDescriptor = ConvertDescriptor(descriptor);
    return InvokeAppMgr([&appMgrDescriptor](const auto &proxy) { return proxy->StopDaemon(appMgrDescriptor); });
}

int32_t DaemonServiceManagerClient::GetInformation(
    const LaunchServiceDescriptor &descriptor, LaunchServiceInformation &information)
{
    return InvokeAppMgr([&descriptor, &information](const auto &proxy) {
        return proxy->GetLaunchServiceInformation(descriptor, information);
    });
}

int32_t DaemonServiceManagerClient::GetAllRegisteredInformation(
    int32_t userId, std::vector<LaunchServiceInformation> &informations)
{
    return InvokeAppMgr([userId, &informations](const auto &proxy) {
        return proxy->GetAllRegisteredInformation(userId, informations);
    });
}

void DaemonServiceManagerClient::SetLaunchServicesEnabled(const std::string &bundleName, int32_t userId,
    int32_t appIndex, const std::vector<uint8_t> &token, bool enable, std::function<void(int32_t)> completion)
{
    if (!completion) {
        return;
    }
    sptr<LaunchServiceOperationCallback> callback =
        new LaunchServiceOperationCallback(std::move(completion));
    auto proxy = GetAppMgrProxy();
    if (proxy == nullptr || !callback->WatchRemote(proxy->AsObject())) {
        callback->OnResult(AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
        return;
    }
    auto ret = proxy->SetLaunchServicesEnabled(bundleName, userId, appIndex, token, enable, callback);
    if (ret != ERR_OK) {
        callback->OnResult(ret);
    }
}

int32_t DaemonServiceManagerClient::RegisterObserver(const sptr<ILaunchServiceObserver> &observer)
{
    return InvokeAppMgr([&observer](const auto &proxy) { return proxy->RegisterLaunchServiceObserver(observer); });
}

int32_t DaemonServiceManagerClient::UnregisterObserver(const sptr<ILaunchServiceObserver> &observer)
{
    return InvokeAppMgr([&observer](const auto &proxy) { return proxy->UnregisterLaunchServiceObserver(observer); });
}
} // namespace AbilityRuntime
} // namespace OHOS
