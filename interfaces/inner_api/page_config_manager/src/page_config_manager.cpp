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

#include "page_config_manager.h"

#include <dlfcn.h>
#include <unistd.h>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "window.h"

namespace OHOS::AbilityRuntime {
namespace {
constexpr const char *PAGE_CONFIG_LIBNAME = "libhmos_pageconfig.z.so";
using InitializeFunc = int32_t (*)(const std::string&, const wptr<Rosen::Window>&);
InitializeFunc g_initializeFunc = nullptr;
using NotifyPageChangedFunc = int32_t (*)(const char*, int32_t, int32_t);
NotifyPageChangedFunc g_notifyPageChangedFunc = nullptr;
}

PageConfigManager &PageConfigManager::GetInstance()
{
    static PageConfigManager instance;
    return instance;
}

int32_t PageConfigManager::Initialize(const std::string& configJson, const wptr<Rosen::Window>& window)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    std::lock_guard guard(g_pageConfigMutex);
    if (isInitialized_) {
        TAG_LOGI(AAFwkTag::ABILITY, "has initialized, update window");
        if (g_initializeFunc != nullptr) {
            g_initializeFunc(configJson, window);
        }
        return ERR_OK;
    }
    if (configJson.empty()) {
        TAG_LOGI(AAFwkTag::ABILITY, "configJson is empty");
        return ERR_NO_INIT;
    }
    LoadPageConfigSo();
    if (pageConfigSo_ == nullptr) {
        return ERR_NO_INIT;
    }
    auto symbol = dlsym(pageConfigSo_, "Initialize");
    if (symbol == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "dlsym failed %{public}s, %{public}s", "Initialize", dlerror());
        return ERR_NO_INIT;
    }
    g_initializeFunc = reinterpret_cast<InitializeFunc>(symbol);
    auto result = g_initializeFunc(configJson, window);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Initialize failed");
        return ERR_NO_INIT;
    }
    isInitialized_ = true;
    TAG_LOGD(AAFwkTag::ABILITY, "Initialize success");
    return ERR_OK;
}

void PageConfigManager::LoadPageConfigSo()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (pageConfigSo_ == nullptr) {
        pageConfigSo_ = dlopen(PAGE_CONFIG_LIBNAME, RTLD_LAZY);
        if (pageConfigSo_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "dlopen failed %{public}s, %{public}s", PAGE_CONFIG_LIBNAME, dlerror());
        } else {
            TAG_LOGD(AAFwkTag::ABILITY, "Success loaded page config");
        }
    }
}

int32_t PageConfigManager::NotifyPageChanged(const char* targetPageName,
    int32_t targetPageNameLength, int32_t windowId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    std::lock_guard lock(g_pageConfigMutex);
    if (targetPageName == nullptr || targetPageNameLength <= 0 ||
            static_cast<size_t>(targetPageNameLength) != strlen(targetPageName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "targetPageName null or length invalid");
        return ERR_NO_INIT;
    }
    if (pageConfigSo_ == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "pageConfigSo_ not loaded");
        return ERR_OK;
    }
    if (!isInitialized_) {
        TAG_LOGE(AAFwkTag::ABILITY, "pageConfigSo_ loaded but not initialized");
        return ERR_OK;
    }
    auto symbol = dlsym(pageConfigSo_, "NotifyPageChanged");
    if (symbol == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "dlsym failed %{public}s, %{public}s", "NotifyPageChanged", dlerror());
        return AAFwk::INNER_ERR;
    }
    g_notifyPageChangedFunc = reinterpret_cast<NotifyPageChangedFunc>(symbol);
    auto result = g_notifyPageChangedFunc(targetPageName, targetPageNameLength, windowId);
    TAG_LOGD(AAFwkTag::ABILITY, "NotifyPageChanged completed with result: %{public}d", result);
    return result;
}

} // namespace AbilityRuntime
