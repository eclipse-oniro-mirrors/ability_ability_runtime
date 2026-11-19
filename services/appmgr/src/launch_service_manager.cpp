/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "launch_service_manager.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <csignal>
#include <future>
#include <map>
#include <mutex>
#include <optional>
#include <sys/syscall.h>
#include <unistd.h>
#include <tuple>
#include <utility>

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_mgr_service_inner.h"
#include "app_pidfd_manager.h"
#include "app_spawn_client.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iam_common_defines.h"
#include "in_process_call_wrapper.h"
#include "launch_service_observer_manager.h"
#include "parameter.h"
#include "parameters.h"
#include "user_launch_service_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Security;

namespace {
constexpr const char *ENTERPRISE_SPACE_ENABLE = "persist.space_mgr_service.enterprise_space_enable";
constexpr const char *PERMISSION_PROTECT_SCREEN_LOCK_DATA = "ohos.permission.PROTECT_SCREEN_LOCK_DATA";
constexpr const char *DAEMON_BUNDLE_PATH_PREFIX = "/data/storage/el1/bundle/libs/";
const std::map<std::string, std::string> DAEMON_ABI_ALIASES = {
    {"armeabi", "arm"},
    {"armeabi-v7a", "arm"},
    {"arm64-v8a", "arm64"},
    {"x86", "x86"},
    {"x86_64", "x86_64"},
};
constexpr int32_t SIGNAL_KILL = 9;
constexpr int64_t LAUNCH_SERVICE_RESTART_INTERVAL = 120000;
constexpr uint32_t LAUNCH_SERVICE_MAX_IMMEDIATE_RESTARTS = 3;

std::atomic_bool g_enterpriseSpaceEnabled = false;

void EnterpriseSpaceEnableCallback(const char *, const char *, void *)
{
    g_enterpriseSpaceEnabled.store(system::GetBoolParameter(ENTERPRISE_SPACE_ENABLE, false));
}

bool IsEnterpriseSpaceEnabled()
{
    static std::once_flag enterpriseSpaceInitFlag;
    std::call_once(enterpriseSpaceInitFlag, []() {
        g_enterpriseSpaceEnabled.store(system::GetBoolParameter(ENTERPRISE_SPACE_ENABLE, false));
        if (WatchParameter(ENTERPRISE_SPACE_ENABLE, EnterpriseSpaceEnableCallback, nullptr) != 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "watch enterprise space parameter failed");
        }
    });
    return g_enterpriseSpaceEnabled.load();
}

bool IsEnterpriseSpaceCrossUser(int32_t callerUserId, int32_t targetUserId)
{
    return IsEnterpriseSpaceEnabled() && callerUserId != 0 && callerUserId != targetUserId;
}

int64_t GetLaunchServiceRestartDelay(uint32_t restartCount, std::chrono::steady_clock::time_point startTime)
{
    auto runningTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
    if (restartCount < LAUNCH_SERVICE_MAX_IMMEDIATE_RESTARTS || runningTime >= LAUNCH_SERVICE_RESTART_INTERVAL) {
        return 0;
    }
    return LAUNCH_SERVICE_RESTART_INTERVAL;
}

void SetLaunchServiceDescriptor(AbilityRuntime::LaunchServiceDescriptor &descriptor, const std::string &bundleName,
    const std::string &moduleName, const std::string &serviceName, int32_t userId, int32_t appIndex)
{
    descriptor.bundleName = bundleName;
    descriptor.moduleName = moduleName;
    descriptor.serviceName = serviceName;
    descriptor.userId = userId;
    descriptor.appIndex = appIndex;
}

AbilityRuntime::LaunchServiceState ConvertLaunchServiceState(UserIam::UserAuth::LaunchServiceState state)
{
    switch (state) {
        case UserIam::UserAuth::LaunchServiceState::ACTIVATE:
            return AbilityRuntime::LaunchServiceState::ENABLED;
        case UserIam::UserAuth::LaunchServiceState::DEACTIVATE:
            return AbilityRuntime::LaunchServiceState::DISABLED;
        default:
            return AbilityRuntime::LaunchServiceState::NOT_REGISTERED;
    }
}

enum class DeveloperCheckResult {
    MATCH,
    QUERY_FAILED,
    MISMATCH,
};

DeveloperCheckResult CheckCallerDeveloper(const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper, int32_t callerUid,
    const std::string &targetDeveloperId, bool rejectEmptyBundleName)
{
    std::string callerBundleName;
    auto result = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerUid, callerBundleName));
    if (result != ERR_OK || (rejectEmptyBundleName && callerBundleName.empty())) {
        return DeveloperCheckResult::QUERY_FAILED;
    }
    AppProvisionInfo callerProvisionInfo;
    result = IN_PROCESS_CALL(
        bundleMgrHelper->GetAppProvisionInfo(callerBundleName, callerUid / BASE_USER_RANGE, callerProvisionInfo));
    if (result != ERR_OK) {
        return DeveloperCheckResult::QUERY_FAILED;
    }
    return !targetDeveloperId.empty() && !callerProvisionInfo.developerId.empty() &&
            targetDeveloperId == callerProvisionInfo.developerId ?
        DeveloperCheckResult::MATCH :
        DeveloperCheckResult::MISMATCH;
}

int32_t CheckLaunchServiceQuery(const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper,
    const AbilityRuntime::LaunchServiceDescriptor &targetDescriptor, int32_t callerUid)
{
    LaunchServicesResult launchServicesResult;
    auto result = IN_PROCESS_CALL(bundleMgrHelper->GetLaunchServices(
        targetDescriptor.bundleName, targetDescriptor.moduleName, targetDescriptor.userId, launchServicesResult));
    if (result != ERR_OK) {
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    auto developerResult = CheckCallerDeveloper(bundleMgrHelper, callerUid, launchServicesResult.developerId, false);
    if (developerResult == DeveloperCheckResult::QUERY_FAILED) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    if (developerResult == DeveloperCheckResult::MISMATCH) {
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    if (std::none_of(launchServicesResult.launchServices.begin(), launchServicesResult.launchServices.end(),
        [&targetDescriptor](const auto &service) { return service.name == targetDescriptor.serviceName; })) {
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    return ERR_OK;
}

bool IsDaemonProcessAlive(pid_t pid)
{
    auto signalError = AppPidFdManager::GetInstance().IsWatchedProcessAlive(pid);
    return signalError != ESRCH;
}

constexpr int32_t IAM_LAUNCH_SERVICE_CALL_TIMEOUT_MS = 1000;

class IAMLaunchServiceOperateCallback final : public UserIam::UserAuth::LaunchServiceCallback {
public:
    explicit IAMLaunchServiceOperateCallback(std::shared_ptr<std::promise<int32_t>> promise)
        : promise_(std::move(promise))
    {
    }
    ~IAMLaunchServiceOperateCallback() override = default;

    void OnResult(int32_t result) override
    {
        if (promise_ != nullptr) {
            promise_->set_value(result);
        }
    }

private:
    std::shared_ptr<std::promise<int32_t>> promise_;
};

class IAMLaunchServiceListCallback final : public UserIam::UserAuth::LaunchServiceListCallback {
public:
    using Result = std::pair<int32_t, std::vector<UserIam::UserAuth::LaunchServiceFlatInfo>>;

    explicit IAMLaunchServiceListCallback(std::shared_ptr<std::promise<Result>> promise) : promise_(std::move(promise))
    {
    }
    ~IAMLaunchServiceListCallback() override = default;

    void OnResult(int32_t result, const std::vector<UserIam::UserAuth::LaunchServiceFlatInfo> &serviceList) override
    {
        if (promise_ != nullptr) {
            promise_->set_value({result, serviceList});
        }
    }

private:
    std::shared_ptr<std::promise<Result>> promise_;
};

std::optional<int32_t> GetIAMLaunchServiceInfo(
    int32_t uid, const UserIam::UserAuth::LaunchServiceKey &key, UserIam::UserAuth::LaunchServiceFlatInfo &record)
{
    auto promise = std::make_shared<std::promise<IAMLaunchServiceListCallback::Result>>();
    auto future = promise->get_future();
    auto callback = std::make_shared<IAMLaunchServiceListCallback>(promise);
    UserIam::UserAuth::UserLaunchServiceClient::GetInstance().GetLaunchServiceInfo(
        uid, std::make_optional(key), callback);
    if (future.wait_for(std::chrono::milliseconds(IAM_LAUNCH_SERVICE_CALL_TIMEOUT_MS)) != std::future_status::ready) {
        TAG_LOGE(AAFwkTag::APPMGR, "get IAM launch service info timeout");
        return std::nullopt;
    }
    auto result = future.get();
    if (result.first != UserIam::UserAuth::SUCCESS) {
        return result.first;
    }
    auto iter = std::find_if(result.second.begin(), result.second.end(), [&key, uid](const auto &item) {
        return item.uid == uid && item.serviceKey.moduleName == key.moduleName &&
            item.serviceKey.serviceName == key.serviceName;
    });
    if (iter == result.second.end()) {
        return UserIam::UserAuth::NOT_ENROLLED;
    }
    record = std::move(*iter);
    return UserIam::UserAuth::SUCCESS;
}

std::optional<int32_t> GetIAMLaunchServiceList(
    const std::optional<int32_t> &userId, std::vector<UserIam::UserAuth::LaunchServiceFlatInfo> &serviceList)
{
    auto promise = std::make_shared<std::promise<IAMLaunchServiceListCallback::Result>>();
    auto future = promise->get_future();
    auto callback = std::make_shared<IAMLaunchServiceListCallback>(promise);
    UserIam::UserAuth::UserLaunchServiceClient::GetInstance().GetLaunchServiceList(userId, callback);
    if (future.wait_for(std::chrono::milliseconds(IAM_LAUNCH_SERVICE_CALL_TIMEOUT_MS)) != std::future_status::ready) {
        TAG_LOGE(AAFwkTag::APPMGR, "get IAM launch service list timeout");
        return std::nullopt;
    }
    auto result = future.get();
    serviceList = std::move(result.second);
    return result.first;
}

class IAMLaunchServiceEnableCallback final : public UserIam::UserAuth::LaunchServiceCallback {
public:
    explicit IAMLaunchServiceEnableCallback(std::function<void(int32_t)> completion)
        : completion_(std::move(completion))
    {
    }

    ~IAMLaunchServiceEnableCallback() override
    {
        OnResult(UserIam::UserAuth::GENERAL_ERROR);
    }

    void OnResult(int32_t result) override
    {
        if (!completed_.exchange(true)) {
            completion_(result);
        }
    }

private:
    std::function<void(int32_t)> completion_;
    std::atomic_bool completed_ = false;
};

int32_t GetDaemonUid(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t &uid)
{
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "daemon get bundle manager helper failed");
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    uid = IN_PROCESS_CALL(
        bundleMgrHelper->GetUidByBundleName(descriptor.bundleName, descriptor.userId, descriptor.appIndex));
    if (uid < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "daemon get uid failed, bundle:%{public}s, result:%{public}d",
            descriptor.bundleName.c_str(), uid);
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    return ERR_OK;
}

UserIam::UserAuth::LaunchServiceType ToIAMLaunchServiceType(LaunchServiceType type)
{
    if (type == LaunchServiceType::DAEMON) {
        return UserIam::UserAuth::LaunchServiceType::PRIVILEGED_DAEMON;
    }
    return UserIam::UserAuth::LaunchServiceType::INVALID;
}

int32_t MapIAMLaunchServiceOperateResult(int32_t result)
{
    switch (result) {
        case UserIam::UserAuth::SUCCESS:
            return ERR_OK;
        case UserIam::UserAuth::CAPABILITY_NOT_SUPPORTED:
            return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
        case UserIam::UserAuth::INVALID_PARAMETERS:
            return ERR_INVALID_VALUE;
        case UserIam::UserAuth::NOT_ENROLLED:
            return AAFwk::ERR_LAUNCH_SERVICE_NOT_REGISTERED;
        case UserIam::UserAuth::REPLICATE_REGISTER:
            return AAFwk::ERR_LAUNCH_SERVICE_ALREADY_REGISTERED;
        default:
            TAG_LOGE(AAFwkTag::APPMGR, "IAM launch service operate failed, result:%{public}d", result);
            return AAFwk::INNER_ERR;
    }
}

int32_t QueryLaunchServiceRecord(
    const AAFwk::LaunchServiceDescriptor &descriptor, UserIam::UserAuth::LaunchServiceFlatInfo &record)
{
    int32_t uid = -1;
    auto uidResult = GetDaemonUid(descriptor, uid);
    if (uidResult != ERR_OK) {
        return uidResult;
    }
    UserIam::UserAuth::LaunchServiceKey key{descriptor.serviceName, descriptor.moduleName};
    auto result = GetIAMLaunchServiceInfo(uid, key, record);
    if (!result.has_value()) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    if (*result == UserIam::UserAuth::SUCCESS) {
        return ERR_OK;
    }
    if (*result == UserIam::UserAuth::CAPABILITY_NOT_SUPPORTED) {
        return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
    }
    TAG_LOGW(AAFwkTag::APPMGR, "launch service not registered, service:%{public}s, result:%{public}d",
        descriptor.serviceName.c_str(), *result);
    return AAFwk::ERR_LAUNCH_SERVICE_NOT_REGISTERED;
}

int32_t RegisterLaunchServiceToIAM(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t uid,
    const LaunchServicesResult &launchServicesResult, const LaunchService &launchService,
    std::optional<UserIam::UserAuth::LaunchServiceFlatInfo> &record)
{
    record.reset();
    UserIam::UserAuth::LaunchServiceRegisterInfo registerInfo;
    registerInfo.uid = uid;
    registerInfo.serviceKey = UserIam::UserAuth::LaunchServiceKey{descriptor.serviceName, descriptor.moduleName};
    registerInfo.bundleInfo.ownerBundleName = descriptor.bundleName;
    registerInfo.bundleInfo.appIndex = descriptor.appIndex;
    registerInfo.bundleInfo.appIdentifier = launchServicesResult.appIdentifier;
    registerInfo.serviceInfo.serviceType = ToIAMLaunchServiceType(launchService.type);

    auto promise = std::make_shared<std::promise<IAMLaunchServiceListCallback::Result>>();
    auto future = promise->get_future();
    auto callback = std::make_shared<IAMLaunchServiceListCallback>(promise);
    UserIam::UserAuth::UserLaunchServiceClient::GetInstance().RegisterLaunchService({registerInfo}, callback);
    if (future.wait_for(std::chrono::milliseconds(IAM_LAUNCH_SERVICE_CALL_TIMEOUT_MS)) != std::future_status::ready) {
        TAG_LOGE(
            AAFwkTag::APPMGR, "register launch service timeout, service:%{public}s", descriptor.serviceName.c_str());
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto result = future.get();
    if (result.first != UserIam::UserAuth::SUCCESS) {
        return MapIAMLaunchServiceOperateResult(result.first);
    }
    auto iter = std::find_if(result.second.begin(), result.second.end(), [&descriptor, uid](const auto &item) {
        return item.uid == uid && item.serviceKey.moduleName == descriptor.moduleName &&
            item.serviceKey.serviceName == descriptor.serviceName;
    });
    if (iter == result.second.end()) {
        TAG_LOGW(AAFwkTag::APPMGR, "IAM registration succeeded without matching service info");
        return ERR_OK;
    }
    record = std::move(*iter);
    return ERR_OK;
}

int32_t UnregisterLaunchServiceFromIAM(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t uid)
{
    auto promise = std::make_shared<std::promise<int32_t>>();
    auto future = promise->get_future();
    auto callback = std::make_shared<IAMLaunchServiceOperateCallback>(promise);
    UserIam::UserAuth::LaunchServiceKey key{descriptor.serviceName, descriptor.moduleName};
    UserIam::UserAuth::UserLaunchServiceClient::GetInstance().UnRegisterLaunchService(
        uid, std::make_optional(key), callback);
    if (future.wait_for(std::chrono::milliseconds(IAM_LAUNCH_SERVICE_CALL_TIMEOUT_MS)) != std::future_status::ready) {
        TAG_LOGE(
            AAFwkTag::APPMGR, "unregister launch service timeout, service:%{public}s", descriptor.serviceName.c_str());
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    return MapIAMLaunchServiceOperateResult(future.get());
}

int32_t UnregisterLaunchServicesByUidFromIAM(int32_t uid)
{
    auto promise = std::make_shared<std::promise<int32_t>>();
    auto future = promise->get_future();
    auto callback = std::make_shared<IAMLaunchServiceOperateCallback>(promise);
    UserIam::UserAuth::UserLaunchServiceClient::GetInstance().UnRegisterLaunchService(uid, std::nullopt, callback);
    if (future.wait_for(std::chrono::milliseconds(IAM_LAUNCH_SERVICE_CALL_TIMEOUT_MS)) != std::future_status::ready) {
        TAG_LOGE(AAFwkTag::APPMGR, "unregister launch services by uid timeout, uid:%{public}d", uid);
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    return MapIAMLaunchServiceOperateResult(future.get());
}

template <typename Identity> bool IsValidDaemonIdentity(const Identity &descriptor, int32_t minUserId)
{
    if (descriptor.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleName must not be empty");
        return false;
    }
    if (descriptor.moduleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "moduleName must not be empty");
        return false;
    }
    if (descriptor.serviceName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "serviceName must not be empty");
        return false;
    }
    if (descriptor.userId < minUserId) {
        TAG_LOGE(AAFwkTag::APPMGR, "userId is below the allowed minimum");
        return false;
    }
    if (descriptor.appIndex < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "appIndex must not be negative");
        return false;
    }
    return true;
}

AAFwk::LaunchServiceDescriptor ResolveDaemonDescriptor(
    const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid)
{
    AAFwk::LaunchServiceDescriptor resolved = descriptor;
    if (resolved.userId < 0) {
        resolved.userId = callerUid / BASE_USER_RANGE;
    }
    return resolved;
}

int32_t CheckDaemonHostUser(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid)
{
    const int32_t callerUserId = callerUid / BASE_USER_RANGE;
    const int32_t hostUserId = descriptor.userId < 0 ? callerUserId : descriptor.userId;
    if (!IsEnterpriseSpaceCrossUser(callerUserId, hostUserId)) {
        return ERR_OK;
    }
    TAG_LOGE(
        AAFwkTag::APPMGR, "daemon host user mismatch, host:%{public}d, caller:%{public}d", hostUserId, callerUserId);
    return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
}

const LaunchService *FindDaemonLaunchService(const LaunchServicesResult &result, const std::string &serviceName)
{
    auto iter =
        std::find_if(result.launchServices.begin(), result.launchServices.end(), [&serviceName](const auto &service) {
            return service.name == serviceName && service.type == LaunchServiceType::DAEMON;
        });
    return iter == result.launchServices.end() ? nullptr : &(*iter);
}

int32_t GetDaemonLaunchService(
    const AAFwk::LaunchServiceDescriptor &descriptor, LaunchServicesResult &result, LaunchService &service)
{
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "daemon declaration get bundle manager helper failed");
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto ret = IN_PROCESS_CALL(
        bundleMgrHelper->GetLaunchServices(descriptor.bundleName, descriptor.moduleName, descriptor.userId, result));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "daemon declaration GetLaunchServices failed: %{public}d", ret);
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    auto launchService = FindDaemonLaunchService(result, descriptor.serviceName);
    if (launchService != nullptr) {
        service = *launchService;
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "daemon declaration not found, service:%{public}s", descriptor.serviceName.c_str());
    return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
}

int32_t CheckDaemonSameDeveloper(
    const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid, const std::string &targetDeveloperId)
{
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto developerResult = CheckCallerDeveloper(bundleMgrHelper, callerUid, targetDeveloperId, true);
    if (developerResult == DeveloperCheckResult::QUERY_FAILED) {
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    if (developerResult == DeveloperCheckResult::MISMATCH) {
        TAG_LOGE(AAFwkTag::APPMGR, "daemon developer mismatch, bundle:%{public}s", descriptor.bundleName.c_str());
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    return ERR_OK;
}

int32_t CheckDaemonSameDeveloper(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid)
{
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    AppProvisionInfo targetProvisionInfo;
    auto ret = IN_PROCESS_CALL(
        bundleMgrHelper->GetAppProvisionInfo(descriptor.bundleName, descriptor.userId, targetProvisionInfo));
    if (ret != ERR_OK) {
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    return CheckDaemonSameDeveloper(descriptor, callerUid, targetProvisionInfo.developerId);
}

int32_t CheckDaemonRegistrationCaller(
    const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid, int32_t &daemonUid)
{
    auto result = GetDaemonUid(descriptor, daemonUid);
    if (result != ERR_OK) {
        return result;
    }
    if (daemonUid != callerUid) {
        TAG_LOGE(AAFwkTag::APPMGR, "daemon registration caller does not own target service");
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    return ERR_OK;
}

int32_t GetDaemonBundleInfo(const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper,
    const AAFwk::LaunchServiceDescriptor &descriptor, BundleInfo &bundleInfo)
{
    auto bundleFlags = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
    bool getBundleInfoResult = false;
    if (descriptor.appIndex == 0) {
        getBundleInfoResult = IN_PROCESS_CALL(
            bundleMgrHelper->GetBundleInfo(descriptor.bundleName, bundleFlags, bundleInfo, descriptor.userId));
    } else {
        auto cloneRet = IN_PROCESS_CALL(bundleMgrHelper->GetCloneBundleInfo(
            descriptor.bundleName, bundleFlags, descriptor.appIndex, bundleInfo, descriptor.userId));
        getBundleInfoResult = (cloneRet == ERR_OK);
    }
    if (!getBundleInfoResult) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn daemon get bundle info failed, appIndex:%{public}d", descriptor.appIndex);
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    return ERR_OK;
}

bool IsLaunchServiceDeclared(const BundleInfo &bundleInfo, const UserIam::UserAuth::LaunchServiceKey &key)
{
    for (const auto &module : bundleInfo.hapModuleInfos) {
        if (module.moduleName != key.moduleName) {
            continue;
        }
        return std::any_of(module.launchServices.begin(), module.launchServices.end(), [&key](const auto &service) {
            return service.name == key.serviceName && service.type == LaunchServiceType::DAEMON;
        });
    }
    return false;
}

int32_t GetDaemonBinaryPath(
    const BundleInfo &bundleInfo, const AAFwk::LaunchServiceDescriptor &descriptor, std::string &binPath)
{
    for (const auto &hapModuleInfo : bundleInfo.hapModuleInfos) {
        if (hapModuleInfo.moduleName != descriptor.moduleName) {
            continue;
        }
        auto service = std::find_if(hapModuleInfo.launchServices.begin(), hapModuleInfo.launchServices.end(),
            [&descriptor](const auto &entry) {
                return entry.name == descriptor.serviceName && !entry.binaryPath.empty();
            });
        if (service != hapModuleInfo.launchServices.end()) {
            const auto &cpuAbi =
                hapModuleInfo.cpuAbi.empty() ? bundleInfo.applicationInfo.cpuAbi : hapModuleInfo.cpuAbi;
            auto alias = DAEMON_ABI_ALIASES.find(cpuAbi);
            if (alias == DAEMON_ABI_ALIASES.end()) {
                TAG_LOGE(AAFwkTag::APPMGR, "unsupported daemon ABI: %{public}s", cpuAbi.c_str());
                return ERR_INVALID_VALUE;
            }
            binPath = alias->second + '/' + service->binaryPath;
        }
        if (!binPath.empty()) {
            break;
        }
    }
    if (binPath.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn daemon launch service not declared");
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    return ERR_OK;
}

int32_t PrepareDaemonAccessToken(const AAFwk::LaunchServiceDescriptor &descriptor, BundleInfo &bundleInfo)
{
    if (bundleInfo.uid < 0 || bundleInfo.gid < 0 || bundleInfo.applicationInfo.appPrivilegeLevel.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn daemon invalid bundle info");
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    uint64_t accessTokenIdEx = bundleInfo.applicationInfo.accessTokenIdEx;
    if (accessTokenIdEx == 0) {
        auto tokenInfo =
            AccessToken::AccessTokenKit::GetHapTokenIDEx(descriptor.userId, descriptor.bundleName, descriptor.appIndex);
        accessTokenIdEx = tokenInfo.tokenIDEx;
    }
    if (accessTokenIdEx == 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn daemon invalid access token");
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    if (bundleInfo.applicationInfo.accessTokenId == 0) {
        bundleInfo.applicationInfo.accessTokenId = static_cast<uint32_t>(accessTokenIdEx & 0xFFFFFFFF);
    }
    bundleInfo.applicationInfo.accessTokenIdEx = accessTokenIdEx;

    return ERR_OK;
}

void ConvertLaunchServiceInformations(const std::vector<UserIam::UserAuth::LaunchServiceFlatInfo> &records,
    const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper,
    std::vector<AbilityRuntime::LaunchServiceInformation> &informations)
{
    for (const auto &record : records) {
        if (record.uid < 0) {
            continue;
        }
        const int32_t recordUserId = record.uid / BASE_USER_RANGE;
        LaunchServicesResult launchServicesResult;
        auto result = IN_PROCESS_CALL(bundleMgrHelper->GetLaunchServices(
            record.bundleInfo.ownerBundleName, record.serviceKey.moduleName, recordUserId, launchServicesResult));
        if (result != ERR_OK) {
            TAG_LOGW(AAFwkTag::APPMGR, "get registered launch service configuration failed: %{public}d", result);
            continue;
        }
        if (FindDaemonLaunchService(launchServicesResult, record.serviceKey.serviceName) == nullptr) {
            continue;
        }
        AbilityRuntime::LaunchServiceInformation information;
        SetLaunchServiceDescriptor(information.descriptor, record.bundleInfo.ownerBundleName,
            record.serviceKey.moduleName, record.serviceKey.serviceName, recordUserId, record.bundleInfo.appIndex);
        information.state = ConvertLaunchServiceState(record.activeState);
        informations.emplace_back(std::move(information));
    }
}

int32_t SendLaunchServiceKillSignal(int32_t pid)
{
    if (AppPidFdManager::GetInstance().KillWatchedProcess(pid)) {
        return ERR_OK;
    }
    return AAFwk::INNER_ERR;
}

} // namespace

LaunchServiceManager::LaunchServiceManager(const std::weak_ptr<AppMgrServiceInner> &appMgrServiceInner)
    : appMgrServiceInner_(appMgrServiceInner)
{
}

std::shared_ptr<AAFwk::TaskHandlerWrap> LaunchServiceManager::GetTaskHandler() const
{
    auto appMgrServiceInner = appMgrServiceInner_.lock();
    return appMgrServiceInner == nullptr ? nullptr : appMgrServiceInner->GetLaunchServiceTaskHandler();
}

int32_t LaunchServiceManager::CheckLaunchServiceStartLocked(
    const LaunchServiceKey &key, AAFwk::LaunchServiceDescriptor &descriptor)
{
    descriptor.bundleName = key.bundleName;
    descriptor.moduleName = key.moduleName;
    descriptor.serviceName = key.serviceName;
    descriptor.userId = key.userId;
    descriptor.appIndex = key.appIndex;
    UserIam::UserAuth::LaunchServiceFlatInfo record;
    auto queryResult = QueryLaunchServiceRecord(descriptor, record);
    if (queryResult != ERR_OK) {
        return queryResult;
    }
    auto appMgrServiceInner = appMgrServiceInner_.lock();
    if (appMgrServiceInner == nullptr) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    if (appMgrServiceInner->IsUninstallingOrUpgrading(record.uid) ||
        upgradingLaunchServiceKeys_.find(record.uid) != upgradingLaunchServiceKeys_.end()) {
        return AAFwk::ERR_UNINSTALLING_OR_UPGRADING_APP;
    }
    if (record.activeState != UserIam::UserAuth::LaunchServiceState::ACTIVATE || record.isForbidden) {
        TAG_LOGW(
            AAFwkTag::APPMGR, "daemon is disabled or forbidden, service:%{public}s", descriptor.serviceName.c_str());
        return AAFwk::ERR_LAUNCH_SERVICE_DISABLED;
    }
    return ERR_OK;
}

int32_t LaunchServiceManager::StartLaunchService(
    const LaunchServiceKey &key, const LaunchService &service, uint32_t restartCount, uint64_t expectedGeneration)
{
    auto appMgrServiceInner = appMgrServiceInner_.lock();
    if (appMgrServiceInner == nullptr) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    // Keep the same lock order as uninstall/upgrade: process lock, then launch service lock.
    std::shared_lock lock(appMgrServiceInner->startProcessLock_);
    return StartLaunchServiceLocked(key, service, restartCount, expectedGeneration);
}

int32_t LaunchServiceManager::StartLaunchServiceLocked(
    const LaunchServiceKey &key, const LaunchService &service, uint32_t restartCount, uint64_t expectedGeneration)
{
    if (!IsValidDaemonIdentity(key, 0)) {
        return ERR_INVALID_VALUE;
    }
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t pid = 0;
    {
        std::lock_guard<std::shared_mutex> lock(launchServiceMutex_);
        auto iter = launchServiceRuntimeRecords_.find(key);
        if (expectedGeneration != 0 &&
            (iter == launchServiceRuntimeRecords_.end() || iter->second.generation != expectedGeneration ||
                iter->second.pid > 0 || !iter->second.service.keepAlive)) {
            return ERR_OK;
        }
        AAFwk::LaunchServiceDescriptor descriptor;
        auto queryResult = CheckLaunchServiceStartLocked(key, descriptor);
        if (queryResult != ERR_OK) {
            return queryResult;
        }
        if (iter != launchServiceRuntimeRecords_.end()) {
            if (iter->second.pid > 0 && IsDaemonProcessAlive(iter->second.pid)) {
                TAG_LOGI(AAFwkTag::APPMGR, "launch service already running, pid=%{public}d", iter->second.pid);
                return ERR_OK;
            }
            launchServiceRuntimeRecords_.erase(iter);
        }

        int32_t daemonUid = -1;
        auto ret = SpawnDaemonProcess(descriptor, pid, daemonUid);
        if (ret != ERR_OK || pid <= 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "spawn launch service failed, result=%{public}d", ret);
            return ret != ERR_OK ? ret : AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
        }

        if (++launchServiceGeneration_ == 0) {
            ++launchServiceGeneration_;
        }
        launchServiceRuntimeRecords_[key] = {service, daemonUid, pid, restartCount, launchServiceGeneration_};
        TAG_LOGI(AAFwkTag::APPMGR,
            "start launch service success, bundle=%{public}s, service=%{public}s, pid=%{public}d",
            key.bundleName.c_str(), key.serviceName.c_str(), pid);
        NotifyLaunchServiceProcessStateChanged(key, pid, AbilityRuntime::LaunchServiceProcessState::CREATED);
        if (!AppPidFdManager::GetInstance().AddWatcher(pid, PidFdType::LAUNCH_SERVICE)) {
            TAG_LOGE(AAFwkTag::APPMGR, "watch launch service failed, pid:%{public}d", pid);
        }
    }
    return ERR_OK;
}

int32_t LaunchServiceManager::StopLaunchService(const LaunchServiceKey &key, int32_t expectedPid)
{
    auto taskHandler = GetTaskHandler();
    if (!IsValidDaemonIdentity(key, 0)) {
        return ERR_INVALID_VALUE;
    }
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t pid = 0;
    {
        std::lock_guard<std::shared_mutex> lock(launchServiceMutex_);
        auto iter = launchServiceRuntimeRecords_.find(key);
        if (iter == launchServiceRuntimeRecords_.end()) {
            TAG_LOGE(AAFwkTag::APPMGR, "launch service not running, bundle=%{public}s, service=%{public}s",
                key.bundleName.c_str(), key.serviceName.c_str());
            return AAFwk::ERR_LAUNCH_SERVICE_NOT_RUNNING;
        }
        if (expectedPid > 0 && iter->second.pid != expectedPid) {
            return AAFwk::ERR_LAUNCH_SERVICE_NOT_RUNNING;
        }
        if (taskHandler != nullptr) {
            taskHandler->CancelTask(GetLaunchServiceRestartTaskName(key, iter->second.generation));
        }
        if (iter->second.pid <= 0) {
            launchServiceRuntimeRecords_.erase(iter);
            TAG_LOGE(AAFwkTag::APPMGR, "launch service not running, bundle=%{public}s, service=%{public}s",
                key.bundleName.c_str(), key.serviceName.c_str());
            return AAFwk::ERR_LAUNCH_SERVICE_NOT_RUNNING;
        }
        pid = iter->second.pid;
        auto ret = SendLaunchServiceKillSignal(pid);
        if (ret != ERR_OK) {
            return ret;
        }
        launchServiceRuntimeRecords_.erase(iter);
        TAG_LOGI(AAFwkTag::APPMGR, "stop launch service success, bundle=%{public}s, service=%{public}s, pid=%{public}d",
            key.bundleName.c_str(), key.serviceName.c_str(), pid);
        NotifyLaunchServiceProcessStateChanged(key, pid, AbilityRuntime::LaunchServiceProcessState::TERMINATED);
    }
    return ERR_OK;
}

int32_t LaunchServiceManager::RegisterDaemon(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid)
{
    if (!IsValidDaemonIdentity(descriptor, -1)) {
        return ERR_INVALID_VALUE;
    }
    auto resolvedDescriptor = ResolveDaemonDescriptor(descriptor, callerUid);
    int32_t daemonUid = -1;
    auto callerResult = CheckDaemonRegistrationCaller(resolvedDescriptor, callerUid, daemonUid);
    if (callerResult != ERR_OK) {
        return callerResult;
    }
    LaunchServicesResult launchServicesResult;
    LaunchService launchService;
    auto declarationResult = GetDaemonLaunchService(resolvedDescriptor, launchServicesResult, launchService);
    if (declarationResult != ERR_OK) {
        return declarationResult;
    }

    std::optional<UserIam::UserAuth::LaunchServiceFlatInfo> registeredRecord;
    auto registerResult = RegisterLaunchServiceToIAM(
        resolvedDescriptor, daemonUid, launchServicesResult, launchService, registeredRecord);
    if (registerResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "register daemon to IAM failed: %{public}d", registerResult);
        if (registerResult == AAFwk::ERR_LAUNCH_SERVICE_ALREADY_REGISTERED) {
            return registerResult;
        }
        return AAFwk::INNER_ERR;
    }
    if (!registeredRecord.has_value()) {
        return ERR_OK;
    }
    NotifyLaunchServiceStateChanged(resolvedDescriptor, ConvertLaunchServiceState(registeredRecord->activeState));

    if (launchService.runAtLoad && registeredRecord->activeState == UserIam::UserAuth::LaunchServiceState::ACTIVATE &&
        !registeredRecord->isForbidden) {
        LaunchServiceKey key = {resolvedDescriptor.bundleName, resolvedDescriptor.moduleName,
            resolvedDescriptor.serviceName, resolvedDescriptor.userId, resolvedDescriptor.appIndex};
        auto spawnResult = StartLaunchService(key, launchService);
        if (spawnResult != ERR_OK) {
            TAG_LOGW(AAFwkTag::APPMGR, "register daemon runAtLoad spawn failed: %{public}d", spawnResult);
        }
    }
    return ERR_OK;
}

int32_t LaunchServiceManager::UnregisterDaemon(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid)
{
    if (!IsValidDaemonIdentity(descriptor, -1)) {
        return ERR_INVALID_VALUE;
    }
    auto resolvedDescriptor = ResolveDaemonDescriptor(descriptor, callerUid);
    int32_t daemonUid = -1;
    auto callerResult = CheckDaemonRegistrationCaller(resolvedDescriptor, callerUid, daemonUid);
    if (callerResult != ERR_OK) {
        return callerResult;
    }
    auto unregisterResult = UnregisterLaunchServiceFromIAM(resolvedDescriptor, daemonUid);
    if (unregisterResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "unregister daemon from IAM failed: %{public}d", unregisterResult);
        return unregisterResult;
    }
    LaunchServiceKey key = {resolvedDescriptor.bundleName, resolvedDescriptor.moduleName,
        resolvedDescriptor.serviceName, resolvedDescriptor.userId, resolvedDescriptor.appIndex};
    (void)StopLaunchService(key);
    NotifyLaunchServiceStateChanged(resolvedDescriptor, AbilityRuntime::LaunchServiceState::NOT_REGISTERED);
    return ERR_OK;
}

int32_t LaunchServiceManager::StartDaemon(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid)
{
    if (!IsValidDaemonIdentity(descriptor, -1)) {
        return ERR_INVALID_VALUE;
    }
    auto resolvedDescriptor = ResolveDaemonDescriptor(descriptor, callerUid);
    TAG_LOGI(AAFwkTag::APPMGR,
        "start daemon bundle:%{public}s module:%{public}s service:%{public}s "
        "userId:%{public}d appIndex:%{public}d callerUid:%{public}d",
        resolvedDescriptor.bundleName.c_str(), resolvedDescriptor.moduleName.c_str(),
        resolvedDescriptor.serviceName.c_str(), resolvedDescriptor.userId, resolvedDescriptor.appIndex, callerUid);
    auto userResult = CheckDaemonHostUser(resolvedDescriptor, callerUid);
    if (userResult != ERR_OK) {
        return userResult;
    }
    LaunchServicesResult launchServicesResult;
    LaunchService launchService;
    auto declarationResult = GetDaemonLaunchService(resolvedDescriptor, launchServicesResult, launchService);
    if (declarationResult != ERR_OK) {
        return declarationResult;
    }
    auto developerResult = CheckDaemonSameDeveloper(resolvedDescriptor, callerUid, launchServicesResult.developerId);
    if (developerResult != ERR_OK) {
        return developerResult;
    }

    LaunchServiceKey key = {resolvedDescriptor.bundleName, resolvedDescriptor.moduleName,
        resolvedDescriptor.serviceName, resolvedDescriptor.userId, resolvedDescriptor.appIndex};
    return StartLaunchService(key, launchService);
}

int32_t LaunchServiceManager::StopDaemon(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t callerUid)
{
    if (!IsValidDaemonIdentity(descriptor, -1)) {
        return ERR_INVALID_VALUE;
    }
    auto resolvedDescriptor = ResolveDaemonDescriptor(descriptor, callerUid);
    auto userResult = CheckDaemonHostUser(resolvedDescriptor, callerUid);
    if (userResult != ERR_OK) {
        return userResult;
    }
    auto developerResult = CheckDaemonSameDeveloper(resolvedDescriptor, callerUid);
    if (developerResult != ERR_OK) {
        return developerResult;
    }
    LaunchServiceKey key = {resolvedDescriptor.bundleName, resolvedDescriptor.moduleName,
        resolvedDescriptor.serviceName, resolvedDescriptor.userId, resolvedDescriptor.appIndex};
    return StopLaunchService(key);
}

void LaunchServiceManager::NotifyLaunchServiceStateChanged(
    const AAFwk::LaunchServiceDescriptor &descriptor, AbilityRuntime::LaunchServiceState state)
{
    AbilityRuntime::LaunchServiceStateData stateData;
    SetLaunchServiceDescriptor(stateData.descriptor, descriptor.bundleName, descriptor.moduleName,
        descriptor.serviceName, descriptor.userId, descriptor.appIndex);
    stateData.state = state;
    LaunchServiceObserverManager::GetInstance().NotifyStatusChanged(stateData);
}

void LaunchServiceManager::NotifyLaunchServiceProcessStateChanged(
    const LaunchServiceKey &key, int32_t pid, AbilityRuntime::LaunchServiceProcessState state)
{
    AbilityRuntime::LaunchServiceProcessStateData processStateData;
    SetLaunchServiceDescriptor(
        processStateData.descriptor, key.bundleName, key.moduleName, key.serviceName, key.userId, key.appIndex);
    processStateData.pid = pid;
    processStateData.state = state;
    LaunchServiceObserverManager::GetInstance().NotifyProcessStateChanged(processStateData);
}

void LaunchServiceManager::PrepareDaemonStartMsg(const std::shared_ptr<AppMgrServiceInner> &appMgrServiceInner,
    const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper, const BundleInfo &bundleInfo, AppSpawnStartMsg &startMsg)
{
    appMgrServiceInner->SetAppInfo(bundleInfo, startMsg);
    startMsg.appSignType = bundleInfo.applicationInfo.appSignType;

    auto userId = GetUserIdByUid(bundleInfo.uid);
    HspList hspList;
    auto hspRet = IN_PROCESS_CALL(bundleMgrHelper->GetBaseSharedBundleInfos(
        bundleInfo.name, hspList, AppExecFwk::GetDependentBundleInfoFlag::GET_ALL_DEPENDENT_BUNDLE_INFO));
    if (hspRet == ERR_OK) {
        startMsg.hspList = std::move(hspList);
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "spawn daemon get hsp list failed, ret:%{public}d", hspRet);
    }

    DataGroupInfoList dataGroupInfoList;
    (void)IN_PROCESS_CALL(bundleMgrHelper->QueryDataGroupInfos(bundleInfo.name, userId, dataGroupInfoList));
    startMsg.dataGroupInfoList = std::move(dataGroupInfoList);
    startMsg.isScreenLockDataProtect = std::any_of(bundleInfo.reqPermissions.begin(), bundleInfo.reqPermissions.end(),
        [](const std::string &permission) { return permission == PERMISSION_PROTECT_SCREEN_LOCK_DATA; });

    appMgrServiceInner->SetOverlayInfo(bundleInfo.name, userId, startMsg);
    appMgrServiceInner->AddMountPermission(bundleInfo.applicationInfo.accessTokenId, startMsg.permissions);
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, bundleInfo.applicationInfo.accessTokenId);
    appMgrServiceInner->GetKernelPermissions(bundleInfo.applicationInfo.accessTokenId, startMsg.jitPermissionsMap);
}

int32_t LaunchServiceManager::StartDaemonProcess(
    const std::shared_ptr<AppMgrServiceInner> &appMgrServiceInner, AppSpawnStartMsg &startMsg, int32_t &pid)
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "spawn daemon call appspawn process:%{public}s binPath:%{public}s uid:%{public}d "
        "gid:%{public}d token:0x%{public}" PRIx64,
        startMsg.procName.c_str(), startMsg.daemonBinPath.c_str(), startMsg.uid, startMsg.gid,
        startMsg.accessTokenIdEx);
    auto errCode = appMgrServiceInner->remoteClientManager_->GetSpawnClient()->StartProcess(startMsg, pid);
    if (errCode != ERR_OK || pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn daemon failed, process:%{public}s result=%{public}d, pid=%{public}d",
            startMsg.procName.c_str(), errCode, pid);
        pid = 0;
        return errCode != ERR_OK ? errCode : AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    TAG_LOGI(
        AAFwkTag::APPMGR, "spawn daemon success process:%{public}s pid:%{public}d", startMsg.procName.c_str(), pid);
    return ERR_OK;
}

int32_t LaunchServiceManager::SpawnDaemonProcess(
    const AAFwk::LaunchServiceDescriptor &descriptor, int32_t &pid, int32_t &uid)
{
    pid = 0;
    uid = -1;
    auto appMgrServiceInner = appMgrServiceInner_.lock();
    if (appMgrServiceInner == nullptr) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn daemon get bundle manager helper failed");
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    BundleInfo bundleInfo;
    auto result = GetDaemonBundleInfo(bundleMgrHelper, descriptor, bundleInfo);
    if (result != ERR_OK) {
        return result;
    }
    uid = bundleInfo.uid;
    std::string binPath;
    result = GetDaemonBinaryPath(bundleInfo, descriptor, binPath);
    if (result != ERR_OK) {
        return result;
    }
    result = PrepareDaemonAccessToken(descriptor, bundleInfo);
    if (result != ERR_OK) {
        return result;
    }
    if (!appMgrServiceInner->remoteClientManager_ || !appMgrServiceInner->remoteClientManager_->GetSpawnClient()) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn daemon get spawn client failed");
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }

    AppSpawnStartMsg startMsg{};
    startMsg.code = MSG_SPAWN_SERVICE_BIN;
    startMsg.bundleName = descriptor.bundleName;
    startMsg.procName = descriptor.bundleName + ":" + descriptor.serviceName;
    startMsg.bundleIndex = descriptor.appIndex;
    startMsg.daemonBinPath = DAEMON_BUNDLE_PATH_PREFIX + binPath;
    PrepareDaemonStartMsg(appMgrServiceInner, bundleMgrHelper, bundleInfo, startMsg);
    return StartDaemonProcess(appMgrServiceInner, startMsg, pid);
}

void LaunchServiceManager::UnregisterLaunchServicesByUid(int32_t uid)
{
    if (uid < 0) {
        return;
    }

    std::vector<AAFwk::LaunchServiceDescriptor> descriptors;
    std::vector<UserIam::UserAuth::LaunchServiceFlatInfo> records;
    auto listResult = GetIAMLaunchServiceList(std::make_optional(uid / BASE_USER_RANGE), records);
    if (!listResult.has_value() || *listResult != UserIam::UserAuth::SUCCESS) {
        TAG_LOGW(AAFwkTag::APPMGR, "get launch services before unregister failed, uid:%{public}d, result:%{public}d",
            uid, listResult.value_or(AAFwk::ABILITY_SERVICE_NOT_CONNECTED));
    } else {
        for (const auto &record : records) {
            if (record.uid != uid ||
                record.serviceInfo.serviceType != UserIam::UserAuth::LaunchServiceType::PRIVILEGED_DAEMON) {
                continue;
            }
            descriptors.emplace_back(
                AAFwk::LaunchServiceDescriptor{record.bundleInfo.ownerBundleName, record.serviceKey.moduleName,
                    record.serviceKey.serviceName, record.uid / BASE_USER_RANGE, record.bundleInfo.appIndex});
        }
    }

    auto IAMResult = UnregisterLaunchServicesByUidFromIAM(uid);
    if (IAMResult != ERR_OK && IAMResult != AAFwk::ERR_LAUNCH_SERVICE_NOT_REGISTERED) {
        TAG_LOGW(AAFwkTag::APPMGR, "unregister launch services from IAM failed, uid:%{public}d, result:%{public}d", uid,
            IAMResult);
    }
    CleanupLaunchServicesByUid(uid);
    if (IAMResult == ERR_OK) {
        for (const auto &descriptor : descriptors) {
            NotifyLaunchServiceStateChanged(descriptor, AbilityRuntime::LaunchServiceState::NOT_REGISTERED);
        }
    }
}

void LaunchServiceManager::CleanupLaunchServicesByUid(int32_t uid)
{
    if (uid < 0) {
        return;
    }
    std::vector<LaunchServiceKey> serviceKeys;
    {
        std::shared_lock<std::shared_mutex> lock(launchServiceMutex_);
        for (const auto &[key, record] : launchServiceRuntimeRecords_) {
            if (record.uid == uid) {
                serviceKeys.emplace_back(key);
            }
        }
    }
    for (const auto &key : serviceKeys) {
        (void)StopLaunchService(key);
    }
}

void LaunchServiceManager::HandleLaunchServiceUninstallOrUpgrade(
    const std::string &bundleName, int32_t uid, bool isUpgrade)
{
    if (!isUpgrade) {
        UnregisterLaunchServicesByUid(uid);
    }
    {
        std::lock_guard<std::shared_mutex> lock(launchServiceMutex_);
        upgradingLaunchServiceKeys_.erase(uid);
        if (isUpgrade) {
            upgradingLaunchServiceKeys_.try_emplace(uid);
        }
    }
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    int32_t appIndex = 0;
    std::string resolvedBundleName;
    if (bundleMgrHelper == nullptr ||
        IN_PROCESS_CALL(bundleMgrHelper->GetNameAndIndexForUid(uid, resolvedBundleName, appIndex)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app index for launch service failed");
        return;
    }
    const int32_t userId = uid / BASE_USER_RANGE;
    if (isUpgrade) {
        std::lock_guard<std::shared_mutex> lock(launchServiceMutex_);
        auto &serviceKeys = upgradingLaunchServiceKeys_[uid];
        for (const auto &[key, record] : launchServiceRuntimeRecords_) {
            if (key.bundleName == bundleName && key.userId == userId && key.appIndex == appIndex && record.pid > 0 &&
                record.service.keepAlive) {
                serviceKeys.emplace_back(key);
            }
        }
    }
    StopLaunchServices(bundleName, userId, appIndex);
}

void LaunchServiceManager::UnregisterRemovedLaunchService(const AAFwk::LaunchServiceDescriptor &descriptor, int32_t uid)
{
    auto result = UnregisterLaunchServiceFromIAM(descriptor, uid);
    if (result != ERR_OK && result != AAFwk::ERR_LAUNCH_SERVICE_NOT_REGISTERED) {
        TAG_LOGE(AAFwkTag::APPMGR, "unregister removed launch service failed: %{public}d", result);
        return;
    }
    LaunchServiceKey key = {
        descriptor.bundleName, descriptor.moduleName, descriptor.serviceName, descriptor.userId, descriptor.appIndex};
    (void)StopLaunchService(key);
    NotifyLaunchServiceStateChanged(descriptor, AbilityRuntime::LaunchServiceState::NOT_REGISTERED);
}

void LaunchServiceManager::CleanupRemovedLaunchServices(int32_t uid)
{
    if (uid < 0) {
        return;
    }
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        return;
    }
    AAFwk::LaunchServiceDescriptor descriptor;
    descriptor.userId = uid / BASE_USER_RANGE;
    auto result =
        IN_PROCESS_CALL(bundleMgrHelper->GetNameAndIndexForUid(uid, descriptor.bundleName, descriptor.appIndex));
    BundleInfo bundleInfo;
    if (result != ERR_OK || GetDaemonBundleInfo(bundleMgrHelper, descriptor, bundleInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "get updated bundle failed, uid:%{public}d", uid);
        return;
    }
    std::vector<UserIam::UserAuth::LaunchServiceFlatInfo> records;
    auto IAMResult = GetIAMLaunchServiceList(std::make_optional(descriptor.userId), records);
    if (!IAMResult.has_value() || *IAMResult != UserIam::UserAuth::SUCCESS) {
        TAG_LOGE(AAFwkTag::APPMGR, "get registered launch services after update failed, uid:%{public}d", uid);
        return;
    }
    for (const auto &record : records) {
        if (record.uid != uid ||
            record.serviceInfo.serviceType != UserIam::UserAuth::LaunchServiceType::PRIVILEGED_DAEMON ||
            IsLaunchServiceDeclared(bundleInfo, record.serviceKey)) {
            continue;
        }
        descriptor.moduleName = record.serviceKey.moduleName;
        descriptor.serviceName = record.serviceKey.serviceName;
        UnregisterRemovedLaunchService(descriptor, uid);
    }
}

void LaunchServiceManager::HandleLaunchServiceUpgradeEnd(int32_t uid)
{
    CleanupRemovedLaunchServices(uid);
    std::vector<LaunchServiceKey> serviceKeys;
    {
        std::lock_guard<std::shared_mutex> lock(launchServiceMutex_);
        auto iter = upgradingLaunchServiceKeys_.find(uid);
        if (iter == upgradingLaunchServiceKeys_.end()) {
            return;
        }
        serviceKeys = std::move(iter->second);
        upgradingLaunchServiceKeys_.erase(iter);
    }
    if (serviceKeys.empty()) {
        return;
    }

    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        return;
    }
    for (const auto &key : serviceKeys) {
        LaunchServicesResult launchServicesResult;
        auto result = IN_PROCESS_CALL(
            bundleMgrHelper->GetLaunchServices(key.bundleName, key.moduleName, key.userId, launchServicesResult));
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "get updated launch services failed: %{public}d", result);
            continue;
        }
        auto launchService = FindDaemonLaunchService(launchServicesResult, key.serviceName);
        if (launchService != nullptr) {
            auto startResult = StartLaunchService(key, *launchService);
            if (startResult != ERR_OK) {
                TAG_LOGW(AAFwkTag::APPMGR, "restart updated launch service failed: %{public}d", startResult);
            }
        }
    }
}

bool LaunchServiceManager::LaunchServiceKey::operator==(const LaunchServiceKey &other) const
{
    return std::tie(bundleName, moduleName, serviceName, userId, appIndex) ==
        std::tie(other.bundleName, other.moduleName, other.serviceName, other.userId, other.appIndex);
}

size_t LaunchServiceManager::LaunchServiceKey::Hash::operator()(const LaunchServiceKey &key) const
{
    size_t seed = 0;
    auto combine = [&seed](size_t value) { seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2); };
    combine(std::hash<std::string>{}(key.bundleName));
    combine(std::hash<std::string>{}(key.moduleName));
    combine(std::hash<std::string>{}(key.serviceName));
    combine(std::hash<int32_t>{}(key.userId));
    combine(std::hash<int32_t>{}(key.appIndex));
    return seed;
}

std::string LaunchServiceManager::GetLaunchServiceRestartTaskName(
    const LaunchServiceKey &key, uint64_t generation) const
{
    return "RestartLaunchService_" + key.bundleName + "_" + key.moduleName + "_" + key.serviceName + "_" +
        std::to_string(key.userId) + "_" + std::to_string(key.appIndex) + "_" + std::to_string(generation);
}

void LaunchServiceManager::StartLaunchServices(
    const std::string &bundleName, int32_t userId, int32_t appIndex, bool runAtLoadOnly)
{
    std::optional<int32_t> IAMUserId = userId == Constants::ALL_USERID ? std::nullopt : std::make_optional(userId);
    std::vector<UserIam::UserAuth::LaunchServiceFlatInfo> records;
    auto IAMResult = GetIAMLaunchServiceList(IAMUserId, records);
    if (!IAMResult.has_value() || *IAMResult != UserIam::UserAuth::SUCCESS) {
        TAG_LOGE(AAFwkTag::APPMGR, "get IAM launch service list failed: %{public}d",
            IAMResult.value_or(AAFwk::ABILITY_SERVICE_NOT_CONNECTED));
        return;
    }
    if (records.empty()) {
        return;
    }
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundle manager helper is nullptr");
        return;
    }
    const bool startAll = bundleName.empty();
    for (const auto &record : records) {
        if (record.uid < 0 ||
            (!startAll &&
                (record.bundleInfo.ownerBundleName != bundleName || record.bundleInfo.appIndex != appIndex)) ||
            record.activeState != UserIam::UserAuth::LaunchServiceState::ACTIVATE || record.isForbidden) {
            continue;
        }
        const int32_t recordUserId = record.uid / BASE_USER_RANGE;
        LaunchServicesResult launchServicesResult;
        auto result = IN_PROCESS_CALL(bundleMgrHelper->GetLaunchServices(
            record.bundleInfo.ownerBundleName, record.serviceKey.moduleName, recordUserId, launchServicesResult));
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "get launch services failed: %{public}d", result);
            continue;
        }
        auto launchService = FindDaemonLaunchService(launchServicesResult, record.serviceKey.serviceName);
        if (launchService == nullptr || (runAtLoadOnly && !launchService->runAtLoad)) {
            continue;
        }
        LaunchServiceKey key = {record.bundleInfo.ownerBundleName, record.serviceKey.moduleName, launchService->name,
            recordUserId, record.bundleInfo.appIndex};
        auto startResult = StartLaunchService(key, *launchService);
        if (startResult != ERR_OK) {
            TAG_LOGW(AAFwkTag::APPMGR, "start launch service failed: %{public}d", startResult);
        }
    }
}

void LaunchServiceManager::RestartLaunchService(const LaunchServiceKey &key, uint64_t generation)
{
    LaunchService service;
    uint32_t restartCount = 0;
    {
        std::shared_lock<std::shared_mutex> lock(launchServiceMutex_);
        auto iter = launchServiceRuntimeRecords_.find(key);
        if (iter == launchServiceRuntimeRecords_.end() || generation == 0 || iter->second.generation != generation ||
            iter->second.pid > 0 || !iter->second.service.keepAlive) {
            return;
        }
        service = iter->second.service;
        restartCount = iter->second.restartCount;
    }
    auto result = StartLaunchService(key, service, restartCount, generation);
    if (result != ERR_OK) {
        TAG_LOGW(AAFwkTag::APPMGR, "restart launch service failed: %{public}d", result);
    }
}

void LaunchServiceManager::HandleLaunchServiceProcessDied(int32_t pid)
{
    auto taskHandler = GetTaskHandler();
    LaunchServiceKey key;
    int64_t delay = 0;
    uint64_t generation = 0;
    bool needRestart = false;
    {
        std::lock_guard<std::shared_mutex> lock(launchServiceMutex_);
        auto iter = std::find_if(launchServiceRuntimeRecords_.begin(), launchServiceRuntimeRecords_.end(),
            [pid](const auto &item) { return item.second.pid == pid; });
        if (iter == launchServiceRuntimeRecords_.end()) {
            return;
        }
        key = iter->first;
        iter->second.pid = 0;
        generation = iter->second.generation;
        needRestart = generation != 0 && iter->second.service.keepAlive;
        if (needRestart) {
            delay = GetLaunchServiceRestartDelay(iter->second.restartCount, iter->second.startTime);
            if (iter->second.restartCount < LAUNCH_SERVICE_MAX_IMMEDIATE_RESTARTS) {
                iter->second.restartCount++;
            }
        }
        NotifyLaunchServiceProcessStateChanged(key, pid, AbilityRuntime::LaunchServiceProcessState::TERMINATED);
    }
    if (!needRestart || taskHandler == nullptr) {
        return;
    }

    auto restartTask = [managerWeak = weak_from_this(), key, generation]() {
        auto manager = managerWeak.lock();
        if (manager == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "get launch service manager failed");
            return;
        }
        manager->RestartLaunchService(key, generation);
    };
    taskHandler->SubmitTask(restartTask, GetLaunchServiceRestartTaskName(key, generation), delay, false);
}

void LaunchServiceManager::StopLaunchServices(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    auto taskHandler = GetTaskHandler();
    std::vector<LaunchServiceKey> runningServices;
    {
        std::lock_guard<std::shared_mutex> lock(launchServiceMutex_);
        for (auto iter = launchServiceRuntimeRecords_.begin(); iter != launchServiceRuntimeRecords_.end();) {
            const auto &key = iter->first;
            if (key.bundleName != bundleName || key.userId != userId || key.appIndex != appIndex) {
                ++iter;
                continue;
            }
            if (taskHandler != nullptr) {
                taskHandler->CancelTask(GetLaunchServiceRestartTaskName(key, iter->second.generation));
            }
            iter->second.generation = 0;
            if (iter->second.pid > 0) {
                runningServices.emplace_back(key);
                ++iter;
            } else {
                iter = launchServiceRuntimeRecords_.erase(iter);
            }
        }
    }
    for (const auto &key : runningServices) {
        auto result = StopLaunchService(key);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "stop launch service process failed: %{public}d", result);
        }
    }
}

int32_t LaunchServiceManager::GetLaunchServiceInformation(
    const AbilityRuntime::LaunchServiceDescriptor &targetDescriptor,
    AbilityRuntime::LaunchServiceInformation &information, int32_t callerUid)
{
    const int32_t callerUserId = callerUid / BASE_USER_RANGE;
    if (IsEnterpriseSpaceCrossUser(callerUserId, targetDescriptor.userId)) {
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto result = CheckLaunchServiceQuery(bundleMgrHelper, targetDescriptor, callerUid);
    if (result != ERR_OK) {
        return result;
    }
    int32_t uid = IN_PROCESS_CALL(bundleMgrHelper->GetUidByBundleName(
        targetDescriptor.bundleName, targetDescriptor.userId, targetDescriptor.appIndex));
    if (uid == Constants::INVALID_UID) {
        return AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND;
    }

    information.descriptor = targetDescriptor;
    information.state = AbilityRuntime::LaunchServiceState::NOT_REGISTERED;
    LaunchServiceKey runtimeKey = {targetDescriptor.bundleName, targetDescriptor.moduleName,
        targetDescriptor.serviceName, targetDescriptor.userId, targetDescriptor.appIndex};
    {
        std::shared_lock<std::shared_mutex> lock(launchServiceMutex_);
        auto iter = launchServiceRuntimeRecords_.find(runtimeKey);
        information.pid = iter == launchServiceRuntimeRecords_.end() ? 0 : iter->second.pid;
    }

    UserIam::UserAuth::LaunchServiceKey IAMKey = {targetDescriptor.serviceName, targetDescriptor.moduleName};
    UserIam::UserAuth::LaunchServiceFlatInfo record;
    const auto IAMResult = GetIAMLaunchServiceInfo(uid, IAMKey, record);
    if (IAMResult.has_value() && *IAMResult == UserIam::UserAuth::ResultCode::NOT_ENROLLED) {
        return ERR_OK;
    }
    if (!IAMResult.has_value() || *IAMResult != UserIam::UserAuth::SUCCESS) {
        TAG_LOGW(AAFwkTag::APPMGR, "get IAM launch service info failed: %{public}d",
            IAMResult.value_or(AAFwk::ABILITY_SERVICE_NOT_CONNECTED));
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    information.state = ConvertLaunchServiceState(record.activeState);
    return ERR_OK;
}

int32_t LaunchServiceManager::GetAllRegisteredInformation(
    std::vector<AbilityRuntime::LaunchServiceInformation> &informations, int32_t userId, int32_t callerUserId)
{
    if (userId != Constants::ALL_USERID && IsEnterpriseSpaceCrossUser(callerUserId, userId)) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    const int32_t queryUserId =
        userId == Constants::ALL_USERID && IsEnterpriseSpaceEnabled() && callerUserId != 0 ? callerUserId : userId;
    std::optional<int32_t> IAMUserId =
        queryUserId == Constants::ALL_USERID ? std::nullopt : std::make_optional(queryUserId);
    std::vector<UserIam::UserAuth::LaunchServiceFlatInfo> records;
    auto IAMResult = GetIAMLaunchServiceList(IAMUserId, records);
    if (!IAMResult.has_value() || *IAMResult != UserIam::UserAuth::SUCCESS) {
        TAG_LOGW(AAFwkTag::APPMGR, "get IAM launch service list failed: %{public}d",
            IAMResult.value_or(AAFwk::ABILITY_SERVICE_NOT_CONNECTED));
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    if (records.empty()) {
        informations.clear();
        return ERR_OK;
    }
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        return AAFwk::ABILITY_SERVICE_NOT_CONNECTED;
    }
    informations.clear();
    informations.reserve(records.size());
    ConvertLaunchServiceInformations(records, bundleMgrHelper, informations);

    std::shared_lock<std::shared_mutex> lock(launchServiceMutex_);
    for (auto &information : informations) {
        const auto &descriptor = information.descriptor;
        LaunchServiceKey key = {descriptor.bundleName, descriptor.moduleName, descriptor.serviceName, descriptor.userId,
            descriptor.appIndex};
        auto runtimeIter = launchServiceRuntimeRecords_.find(key);
        if (runtimeIter != launchServiceRuntimeRecords_.end()) {
            information.pid = runtimeIter->second.pid;
        }
    }
    return ERR_OK;
}

std::function<void(int32_t)> LaunchServiceManager::CreateLaunchServicesEnabledCompletion(const std::string &bundleName,
    int32_t userId, int32_t appIndex, bool enable, std::function<void(int32_t)> completion)
{
    return [managerWeak = weak_from_this(), bundleName, userId, appIndex, enable, completion](int32_t result) {
        if (result != UserIam::UserAuth::SUCCESS) {
            TAG_LOGE(AAFwkTag::APPMGR, "set IAM launch service state failed: %{public}d", result);
            completion(result == UserIam::UserAuth::AUTH_TOKEN_CHECK_FAILED || result == UserIam::UserAuth::CANCELED ?
                    AAFwk::ERR_LAUNCH_SERVICE_APPROVAL_REJECTED :
                    AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
            return;
        }
        auto manager = managerWeak.lock();
        auto handler = manager == nullptr ? nullptr : manager->GetTaskHandler();
        if (handler == nullptr) {
            completion(AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
            return;
        }
        auto handle = handler->SubmitTask([managerWeak, bundleName, userId, appIndex, enable, completion]() {
            auto manager = managerWeak.lock();
            if (manager == nullptr) {
                completion(AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
                return;
            }
            if (enable) {
                manager->StartLaunchServices(bundleName, userId, appIndex, true);
            } else {
                manager->StopLaunchServices(bundleName, userId, appIndex);
            }
            completion(ERR_OK);
        });
        if (!handle) {
            completion(AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
        }
    };
}

void LaunchServiceManager::SetLaunchServicesEnabled(const std::string &bundleName, int32_t userId, int32_t appIndex,
    const std::vector<uint8_t> &token, bool enable, int32_t callerUid, std::function<void(int32_t)> completion)
{
    if (IsEnterpriseSpaceCrossUser(callerUid / BASE_USER_RANGE, userId)) {
        completion(AAFwk::ERR_TARGET_BUNDLE_NOT_EXIST);
        return;
    }
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        completion(AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
        return;
    }
    int32_t uid = IN_PROCESS_CALL(bundleMgrHelper->GetUidByBundleName(bundleName, userId, appIndex));
    if (uid == Constants::INVALID_UID) {
        completion(AAFwk::ERR_TARGET_BUNDLE_NOT_EXIST);
        return;
    }

    auto callback = std::make_shared<IAMLaunchServiceEnableCallback>(
        CreateLaunchServicesEnabledCompletion(bundleName, userId, appIndex, enable, completion));
    auto activeState =
        enable ? UserIam::UserAuth::LaunchServiceState::ACTIVATE : UserIam::UserAuth::LaunchServiceState::DEACTIVATE;
    UserIam::UserAuth::UserLaunchServiceClient::GetInstance().SetLaunchServiceState(uid, token, activeState, callback);
}

int32_t LaunchServiceManager::RegisterLaunchServiceObserver(
    const sptr<AbilityRuntime::ILaunchServiceObserver> &observer, int32_t callerUid)
{
    return LaunchServiceObserverManager::GetInstance().RegisterObserver(
        observer, callerUid / BASE_USER_RANGE, callerUid);
}

int32_t LaunchServiceManager::UnregisterLaunchServiceObserver(
    const sptr<AbilityRuntime::ILaunchServiceObserver> &observer)
{
    return LaunchServiceObserverManager::GetInstance().UnregisterObserver(observer);
}
} // namespace AppExecFwk
} // namespace OHOS
