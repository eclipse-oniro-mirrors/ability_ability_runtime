/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <unistd.h>
#include "cpp/condition_variable.h"

#include "application_context.h"

#include "ability_business_error_utils.h"
#include "ability_manager_client.h"
#include "app_mgr_client.h"
#include "app_mgr_interface.h"
#include "child_process_info_internal.h"
#include "context.h"
#include "context/application_context.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "load_ability_callback_impl.h"
#include "load_ability_callback_manager.h"
#include "page_config_manager.h"
#include "start_options_impl.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "want_manager.h"
#include "want_utils.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace {
constexpr int32_t ATTACH_ABILITY_THREAD_TIMEOUT_TIME = 100 * 1000; // attach ability thread timeout, 100s
std::mutex g_appMgrMutex;
sptr<AppExecFwk::IAppMgr> g_appMgr = nullptr;

AbilityRuntime_ErrorCode WriteStringToBuffer(
    const std::string &src, char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    const auto srcLength = static_cast<int32_t>(src.length());
    if (bufferSize - 1 < srcLength) {
        TAG_LOGE(AAFwkTag::APPKIT, "the buffer size is less than the minimum buffer size");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    src.copy(buffer, srcLength);
    buffer[srcLength] = '\0';
    *writeLength = srcLength;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode CheckParameters(char* buffer, int32_t* writeLength)
{
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "buffer is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (writeLength == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "writeLength is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

sptr<AppExecFwk::IAppMgr> GetAppMgr()
{
    std::unique_lock<std::mutex> lock(g_appMgrMutex);
    if (g_appMgr) {
        return g_appMgr;
    }

    auto sysMgrClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMgrClient == nullptr) {
        return nullptr;
    }
    auto object = sysMgrClient->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        return nullptr;
    }
    g_appMgr = OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(object);
    return g_appMgr;
}

AbilityRuntime_ErrorCode CheckAppMainThread()
{
    auto appMgrClient = GetAppMgr();
    if (appMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null appMgr");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    AppExecFwk::ChildProcessInfo childProcessInfo;
    auto ret = appMgrClient->GetChildProcessInfoForSelf(childProcessInfo);
    if (ret != ERR_OK && getpid() == gettid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "calling in app's main thread is not supported");
        return ABILITY_RUNTIME_ERROR_CODE_MAIN_THREAD_NOT_SUPPORTED;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

uint64_t GenerateCallbackId()
{
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count());
}
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetCacheDir(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "buffer is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (writeLength == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "writeLength is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string cacheDir = appContext->GetCacheDir();
    if (cacheDir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "cacheDir is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(cacheDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetAreaMode(AbilityRuntime_AreaMode* areaMode)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (areaMode == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "areaMode is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    int32_t area = appContext->GetArea();
    *areaMode = static_cast<AbilityRuntime_AreaMode>(area);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetBundleName(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "buffer is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (writeLength == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "writeLength is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string bundleName = appContext->GetBundleName();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleName is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(bundleName, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetTempDir(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getTempDir called");
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string tempDir = appContext->GetTempDir();
    if (tempDir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "tempDir is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(tempDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetFilesDir(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getFilesDir called");
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string filesDir = appContext->GetFilesDir();
    if (filesDir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "filesDir is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(filesDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetDatabaseDir(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getDatabaseDir called");
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string databaseDir = appContext->GetDatabaseDir();
    if (databaseDir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "databaseDir is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(databaseDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetPreferencesDir(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getPreferencesDir called");
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string preferencesDir = appContext->GetPreferencesDir();
    if (preferencesDir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "preferencesDir is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(preferencesDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetBundleCodeDir(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getBundleCodeDir called");
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string bundleCodeDir = appContext->GetBundleCodeDir();
    if (bundleCodeDir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleCodeDir is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(bundleCodeDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetDistributedFilesDir(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getDistributedFilesDir called");
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string distributedFilesDir = appContext->GetDistributedFilesDir();
    if (distributedFilesDir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "distributedFilesDir is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(distributedFilesDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetCloudFileDir(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getCloudFileDir called");
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string cloudFileDir = appContext->GetCloudFileDir();
    if (cloudFileDir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "cloudFileDir is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(cloudFileDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetResourceDir(const char* moduleName,
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getResourceDir called");
    if (moduleName == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "moduleName is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (std::strlen(moduleName) == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "moduleName is empty");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    if (!appContext->IsModuleExist(moduleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "moduleName is not exist");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const std::string resourceDir = appContext->GetResourceDir(moduleName);
    return WriteStringToBuffer(resourceDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetLaunchParameter(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s called", __func__);
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }

    std::string launchParameter = appContext->GetLaunchParameter();
    return WriteStringToBuffer(launchParameter, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetLatestParameter(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s called", __func__);
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }

    std::string latestParameter = appContext->GetLatestParameter();
    return WriteStringToBuffer(latestParameter, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_StartSelfUIAbility(AbilityBase_Want *want)
{
    TAG_LOGD(AAFwkTag::APPKIT, "StartSelfUIAbility called");
    auto ret = CheckWant(want);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "CheckWant failed: %{public}d", ret);
        return ret;
    }
    Want abilityWant;
    AbilityBase_ErrorCode errCode = CWantManager::TransformToWant(*want, false, abilityWant);
    if (errCode != ABILITY_BASE_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "transform error:%{public}d", errCode);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ConvertToCommonBusinessErrorCode(AbilityManagerClient::GetInstance()->StartSelfUIAbility(abilityWant));
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_StartSelfUIAbilityWithStartOptions(AbilityBase_Want *want,
    AbilityRuntime_StartOptions *options)
{
    TAG_LOGD(AAFwkTag::APPKIT, "StartSelfUIAbilityWithStartOptions called");
    auto ret = CheckWant(want);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "CheckWant failed: %{public}d", ret);
        return ret;
    }
    if (options == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null options");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    Want abilityWant;
    AbilityBase_ErrorCode errCode = CWantManager::TransformToWant(*want, false, abilityWant);
    if (errCode != ABILITY_BASE_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "transform error:%{public}d", errCode);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    OHOS::AAFwk::StartOptions startOptions = options->GetInnerStartOptions();
    return ConvertToAPI17BusinessErrorCode(AbilityManagerClient::GetInstance()->StartSelfUIAbilityWithStartOptions(
        abilityWant, startOptions));
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetVersionCode(int64_t* versionCode)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getVersionCode called");
    if (versionCode == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "versionCode is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const auto appApplicationInfo = appContext->GetApplicationInfo();
    if (appApplicationInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo is null");
        return ABILITY_RUNTIME_ERROR_CODE_GET_APPLICATION_INFO_FAILED;
    }
    *versionCode = static_cast<int64_t>(appApplicationInfo->versionCode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode StartSelfUIAbilityWithPidResultPrecheck(AbilityBase_Want *want,
    AbilityRuntime_StartOptions *options, int32_t *targetPid)
{
    auto ret = CheckAppMainThread();
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "CheckAppMainThread failed, ret=%{public}d", ret);
        return ret;
    }
    ret = CheckWant(want);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "CheckWant failed: %{public}d", ret);
        return ret;
    }
    if (options == nullptr || targetPid == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null options or targetPid");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_StartSelfUIAbilityWithPidResult(AbilityBase_Want *want,
    AbilityRuntime_StartOptions *options, int32_t *targetPid)
{
    auto ret = StartSelfUIAbilityWithPidResultPrecheck(want, options, targetPid);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    Want abilityWant;
    AbilityBase_ErrorCode errCode = CWantManager::TransformToWant(*want, false, abilityWant);
    if (errCode != ABILITY_BASE_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "transform error:%{public}d", errCode);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    StartOptions startOptions = options->GetInnerStartOptions();
    ffrt::condition_variable callbackDoneCv;
    std::atomic_bool done = false;
    auto task = [targetPid, &callbackDoneCv, &done](int32_t pidResult) {
        *targetPid = pidResult;
        done.store(true);
        callbackDoneCv.notify_all();
    };
    sptr<LoadAbilityCallbackImpl> callback = sptr<LoadAbilityCallbackImpl>::MakeSptr(std::move(task));
    uint64_t callbackId = GenerateCallbackId();
    auto result = LoadAbilityCallbackManager::GetInstance().AddLoadAbilityCallback(callbackId, callback);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "add callback error:%{public}d", result);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    result = AbilityManagerClient::GetInstance()->StartSelfUIAbilityWithPidResult(
        abilityWant, startOptions, callbackId);
    if (result != ERR_OK) {
        callback->Cancel();
        LoadAbilityCallbackManager::GetInstance().RemoveCallback(callback);
        return ConvertToAPI21BusinessErrorCode(result);
    }
    ffrt::mutex callbackDoneMutex;
    std::unique_lock<ffrt::mutex> lock(callbackDoneMutex);
    if (!callbackDoneCv.wait_for(lock, std::chrono::milliseconds(ATTACH_ABILITY_THREAD_TIMEOUT_TIME),
        [&done] { return done.load(); })) {
        TAG_LOGE(AAFwkTag::APPKIT, "wait for loadability callback timeout");
        callback->Cancel();
        LoadAbilityCallbackManager::GetInstance().RemoveCallback(callback);
        return ABILITY_RUNTIME_ERROR_CODE_START_TIMEOUT;
    }
    if (*targetPid < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "loadability failed");
        return ABILITY_RUNTIME_ERROR_CODE_START_TIMEOUT;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextGetLogFileDir(
    char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    TAG_LOGD(AAFwkTag::APPKIT, "getLogFileDir called");
    auto ret = CheckParameters(buffer, writeLength);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContext is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    const std::string logFileDir = appContext->GetLogFileDir();
    if (logFileDir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "logFileDir is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return WriteStringToBuffer(logFileDir, buffer, bufferSize, writeLength);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ApplicationContextNotifyPageChanged(
    const char* targetPageName, int32_t targetPageNameLength, int32_t windowId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NotifyPageChanged called");
    if (targetPageName == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "targetPageName null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (targetPageNameLength <= 0 || static_cast<size_t>(targetPageNameLength) != strlen(targetPageName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "targetPageNameLength invalid");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto err = PageConfigManager::GetInstance().NotifyPageChanged(targetPageName, targetPageNameLength, windowId);
    if (err == ERR_NO_INIT) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid windowId");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (err != ERR_OK) {
        return ConvertToAPI21BusinessErrorCode(err);
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_AcquireUIAbilityChildProcessInfos(
    OH_AbilityRuntime_ChildProcessInfosHandle* infos, uint32_t* count)
{
    if (infos == nullptr || count == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::vector<AppExecFwk::ChildProcessInfo> childInfos;
    auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (client == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "AppMgrClient is nullptr");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    int32_t ret = client->GetSelfUIAbilityChildProcesses(childInfos);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetSelfUIAbilityChildProcesses failed, ret=%{public}d", ret);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    if (!CreateAndFillChildProcessInfos(childInfos, infos, count)) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}