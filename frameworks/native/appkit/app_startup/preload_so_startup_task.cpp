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

#include "preload_so_startup_task.h"

#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "native_module_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char OHM_URL_APP_PREFIX[] = "@app:";
constexpr char OHM_URL_NORMALIZED_SO_PREFIX[] = "@normalized:Y";
constexpr char OHM_URL_NORMALIZED_TAG = '&';
constexpr char OHM_URL_APP_COLON_TAG = ':';
constexpr char OHM_URL_SLASH_TAG = '/';
constexpr size_t OHM_URL_NORMALIZED_ARGS_NUM = 5;
constexpr size_t OHM_URL_NORMALIZED_IMPORT_PATH_INDEX = 3;
constexpr size_t SO_PREFIX_LEN = 3;
constexpr size_t SO_SUFFIX_LEN = 3;

bool StringStartWith(const std::string& str, const std::string& startStr)
{
    size_t startStrLen = startStr.length();
    return ((str.length() >= startStrLen) && (str.compare(0, startStrLen, startStr) == 0));
}

int32_t ParseNormalizedOhmUrl(const std::string& ohmUrl, std::string& soName)
{
    // @normalized:Y&&&<IMPORT_PATH>&<VERSION>  or  @normalized:Y&&<bundleName>&<IMPORT_PATH>&<VERSION>
    std::vector<std::string> res;
    size_t start = 0;
    size_t pos = ohmUrl.find(OHM_URL_NORMALIZED_TAG);
    while (pos != std::string::npos) {
        std::string element = ohmUrl.substr(start, pos - start);
        res.emplace_back(element);
        start = pos + 1;
        pos = ohmUrl.find(OHM_URL_NORMALIZED_TAG, start);
    }
    std::string tail = ohmUrl.substr(start);
    res.emplace_back(tail);

    if (res.size() != OHM_URL_NORMALIZED_ARGS_NUM) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid normalized ohmUrl: %{public}s", ohmUrl.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }

    soName = res[OHM_URL_NORMALIZED_IMPORT_PATH_INDEX];
    // Delete the prefix "lib" and suffix ".so".
    if (soName.size() < SO_PREFIX_LEN + SO_SUFFIX_LEN) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid import path in ohmUrl: %{public}s", ohmUrl.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    soName = soName.substr(SO_PREFIX_LEN, soName.size() - SO_PREFIX_LEN - SO_SUFFIX_LEN);
    if (soName.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "empty soName after strip in ohmUrl: %{public}s", ohmUrl.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t ParseAppOhmUrl(const std::string& ohmUrl, std::string& soName)
{
    // @app:<bundleName>/<moduleName>/<libName>
    size_t pos = ohmUrl.find(OHM_URL_APP_COLON_TAG);
    if (pos == std::string::npos) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid app ohmUrl: %{public}s", ohmUrl.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    std::string soFullPath = ohmUrl.substr(pos + 1);

    pos = soFullPath.rfind(OHM_URL_SLASH_TAG);
    if (pos == std::string::npos) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid app ohmUrl: %{public}s", ohmUrl.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    soName = soFullPath.substr(pos + 1);
    return ERR_OK;
}

int32_t ParseOhmUrl(const std::string& ohmUrl, std::string& soName)
{
    if (ohmUrl.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "ohmUrl is empty");
        return ERR_STARTUP_INVALID_VALUE;
    }
    if (StringStartWith(ohmUrl, OHM_URL_NORMALIZED_SO_PREFIX)) {
        return ParseNormalizedOhmUrl(ohmUrl, soName);
    }
    if (StringStartWith(ohmUrl, OHM_URL_APP_PREFIX)) {
        return ParseAppOhmUrl(ohmUrl, soName);
    }
    TAG_LOGE(AAFwkTag::STARTUP, "invalid ohmUrl: %{public}s", ohmUrl.c_str());
    return ERR_STARTUP_INVALID_VALUE;
}
} // namespace

PreloadSoStartupTask::PreloadSoStartupTask(const std::string& name, const std::string& ohmUrl, const std::string& path)
    : AppStartupTask(name), ohmUrl_(ohmUrl), path_(path)
{
    SetWaitOnMainThread(false);
    SetCallCreateOnMainThread(false);
}

PreloadSoStartupTask::~PreloadSoStartupTask() = default;

const std::string &PreloadSoStartupTask::GetType() const
{
    return AppStartupTask::TASK_TYPE_PRELOAD_SO;
}

int32_t PreloadSoStartupTask::RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback)
{
    std::string soName;
    int32_t code = ParseOhmUrl(ohmUrl_, soName);
    AAFwk::EventInfo eventInfo;
    if (code != ERR_OK) {
        TAG_LOGW(AAFwkTag::STARTUP, "task %{public}s, parse ohmUrl failed: %{public}s", name_.c_str(), ohmUrl_.c_str());
        return ERR_OK;
    }
    TAG_LOGI(AAFwkTag::STARTUP, "task: %{public}s, soName: %{public}s", name_.c_str(), soName.c_str());

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "moduleManager is null");
        OnCompletedCallback::OnCallback(std::move(callback), ERR_STARTUP_INTERNAL_ERROR);
        eventInfo.errCode = ERR_NATIVE_MODULE_MANAGER_CONSTRUCTION;
        eventInfo.errReason = "moduleManager is null";
        AAFwk::EventReport::SendLaunchFrameworkEvent(
            AAFwk::EventName::STARTUP_TASK_ERROR, HISYSEVENT_FAULT, eventInfo);
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    std::string errInfo;
    NativeModule* module = moduleManager->LoadNativeModule(soName.c_str(), path_.empty() ? nullptr : path_.c_str(),
        true, errInfo, false, "");
    if (module == nullptr) {
        TAG_LOGW(AAFwkTag::STARTUP, "module is null, errInfo: %{public}s", errInfo.c_str());
        OnCompletedCallback::OnCallback(std::move(callback), ERR_OK);
        eventInfo.errCode = ERR_LOAD_NATIVE_MODULE;
        eventInfo.errReason = errInfo;
        AAFwk::EventReport::SendLaunchFrameworkEvent(
            AAFwk::EventName::STARTUP_TASK_ERROR, HISYSEVENT_FAULT, eventInfo);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    OnCompletedCallback::OnCallback(std::move(callback), ERR_OK);
    return ERR_OK;
}

int32_t PreloadSoStartupTask::RunTaskOnDependencyCompleted(const std::string& dependencyName,
    const std::shared_ptr<StartupTaskResult>& result)
{
    // no onDependencyCompleted callback, do nothing
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
