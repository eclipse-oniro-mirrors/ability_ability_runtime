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

#include "preload_ui_extension_host_client.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AbilityRuntime {
sptr<PreloadUIExtensionHostClient> PreloadUIExtensionHostClient::instance_ = nullptr;
std::mutex PreloadUIExtensionHostClient::instanceMutex_;
std::once_flag PreloadUIExtensionHostClient::singletonFlag_;

sptr<PreloadUIExtensionHostClient> PreloadUIExtensionHostClient::GetInstance()
{
    std::call_once(singletonFlag_, []() {
        instance_ = new (std::nothrow) PreloadUIExtensionHostClient();
        if (instance_ == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null instance");
        }
    });
    return instance_;
}

void PreloadUIExtensionHostClient::RegisterPreloadUIExtensionHostClient()
{
    std::lock_guard<std::mutex> lock(registrationMutex_);
    if (isRegistered_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "PreloadUIExtensionHostClient already registered");
        return;
    }
    if (loadedCallbackMap_.empty() && destroyCallbackMap_.empty() && resultCallbacks_.empty()) {
        TAG_LOGD(AAFwkTag::UI_EXT, "No callbacks to register, skip registration");
        return;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->RegisterPreloadUIExtensionHostClient(GetInstance());
    if (ret == 0) {
        isRegistered_ = true;
        TAG_LOGI(AAFwkTag::UI_EXT, "RegisterPreloadUIExtensionHostClient success");
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "RegisterPreloadUIExtensionHostClient failed, ret: %{public}d", ret);
    }
}

void PreloadUIExtensionHostClient::UnRegisterPreloadUIExtensionHostClient()
{
    std::lock_guard<std::mutex> lock(registrationMutex_);
    if (!isRegistered_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "PreloadUIExtensionHostClient not registered, skip unregistration");
        return;
    }
    if (!loadedCallbackMap_.empty() || !destroyCallbackMap_.empty() || !resultCallbacks_.empty()) {
        TAG_LOGD(AAFwkTag::UI_EXT,
            "Still have callbacks, skip unregistration loaded: %{public}zu, destroy: %{public}zu",
            loadedCallbackMap_.size(), destroyCallbackMap_.size());
        return;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->UnRegisterPreloadUIExtensionHostClient();
    if (ret == 0) {
        isRegistered_ = false;
        TAG_LOGI(AAFwkTag::UI_EXT, "UnRegisterPreloadUIExtensionHostClient success");
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "UnRegisterPreloadUIExtensionHostClient failed, ret: %{public}d", ret);
    }
}

int32_t PreloadUIExtensionHostClient::AddLoadedCallback(
    const std::shared_ptr<PreloadUIExtensionCallbackInterface> &callback)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
    int32_t key = ++key_;
    loadedCallbackMap_[key] = callback;
    if (loadedCallbackMap_.size() == 1) {
        RegisterPreloadUIExtensionHostClient();
    }
    return key;
}

int32_t PreloadUIExtensionHostClient::AddDestroyCallback(
    const std::shared_ptr<PreloadUIExtensionCallbackInterface> &callback)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
    int32_t key = ++key_;
    destroyCallbackMap_[key] = callback;
    if (destroyCallbackMap_.size() == 1) {
        RegisterPreloadUIExtensionHostClient();
    }
    return key;
}

int32_t PreloadUIExtensionHostClient::RemoveLoadedCallback(int32_t key)
{
    bool needUnregister = false;
    {
        std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
        auto callbackIter = loadedCallbackMap_.find(key);
        if (callbackIter == loadedCallbackMap_.end()) {
            TAG_LOGE(AAFwkTag::UI_EXT, "No callback found with key: %{public}d", key);
            return ERR_INVALID_VALUE;
        }
        loadedCallbackMap_.erase(callbackIter);
        needUnregister = loadedCallbackMap_.empty();
    }
    if (needUnregister) {
        UnRegisterPreloadUIExtensionHostClient();
    }
    return ERR_OK;
}

int32_t PreloadUIExtensionHostClient::RemoveDestroyCallback(int32_t key)
{
    bool needUnregister = false;
    {
        std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
        auto callbackIter = destroyCallbackMap_.find(key);
        if (callbackIter == destroyCallbackMap_.end()) {
            TAG_LOGE(AAFwkTag::UI_EXT, "No callback found with key: %{public}d", key);
            return ERR_INVALID_VALUE;
        }
        destroyCallbackMap_.erase(callbackIter);
        needUnregister = destroyCallbackMap_.empty();
    }
    if (needUnregister) {
        UnRegisterPreloadUIExtensionHostClient();
    }
    return ERR_OK;
}

void PreloadUIExtensionHostClient::RemoveAllLoadedCallback()
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
    loadedCallbackMap_.clear();
    UnRegisterPreloadUIExtensionHostClient();
}

void PreloadUIExtensionHostClient::RemoveAllDestroyCallback()
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
    destroyCallbackMap_.clear();
    UnRegisterPreloadUIExtensionHostClient();
}

int32_t PreloadUIExtensionHostClient::GenerateRequestCode()
{
    requestCode_ = (requestCode_ == INT32_MAX) ? 0 : (requestCode_ + 1);
    return requestCode_;
}

void PreloadUIExtensionHostClient::PreloadUIExtensionAbility(
    const Want &want, std::string &hostBundleName, PreloadTask &&task)
{
    int32_t requestCode;
    std::shared_ptr<PreloadByCallData> callData;
    {
        std::lock_guard lock(requestCodeMutex_);
        requestCode = GenerateRequestCode();
        callData = std::make_shared<PreloadByCallData>(std::move(task));
        resultCallbacks_.emplace(requestCode, callData);
        if (resultCallbacks_.size() == 1) {
            RegisterPreloadUIExtensionHostClient();
        }
    }
    int32_t userId = AAFwk::DEFAULT_INVAL_VALUE;
    int32_t hostPid = AAFwk::DEFAULT_INVAL_VALUE;
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->PreloadUIExtensionAbility(
        want, hostBundleName, userId, hostPid, requestCode);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "PreloadUIExtensionAbility failed, ret: %{public}d", ret);
        OnPreloadSuccess(requestCode, -1, ret);
        return;
    }
}

void PreloadUIExtensionHostClient::OnLoadedDone(int32_t extensionAbilityId)
{
    std::vector<std::shared_ptr<PreloadUIExtensionCallbackInterface>> callbacksToInvoke;
    {
        std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
        for (auto &callbackPair : loadedCallbackMap_) {
            if (callbackPair.second != nullptr) {
                callbacksToInvoke.push_back(callbackPair.second);
            }
        }
    }
    for (auto &callback : callbacksToInvoke) {
        if (callback != nullptr) {
            callback->ProcessOnLoadedDone(extensionAbilityId);
        }
    }
}

void PreloadUIExtensionHostClient::OnDestroyDone(int32_t extensionAbilityId)
{
    std::vector<std::shared_ptr<PreloadUIExtensionCallbackInterface>> callbacksToInvoke;
    {
        std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
        for (auto &callbackPair : destroyCallbackMap_) {
            if (callbackPair.second != nullptr) {
                callbacksToInvoke.push_back(callbackPair.second);
            }
        }
    }
    for (auto &callback : callbacksToInvoke) {
        if (callback != nullptr) {
            callback->ProcessOnDestroyDone(extensionAbilityId);
        }
    }
}

void PreloadUIExtensionHostClient::OnPreloadSuccess(
    int32_t requestCode, int32_t extensionAbilityId, int32_t innerErrCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT,
        "OnPreloadSuccess called requestCode: %{public}d extensionAbilityId: %{public}d innerErrCode: %{public}d",
        requestCode, extensionAbilityId, innerErrCode);
    std::shared_ptr<PreloadByCallData> callData;
    bool needUnregister = false;
    {
        std::lock_guard lock(requestCodeMutex_);
        auto it = resultCallbacks_.find(requestCode);
        if (it == resultCallbacks_.end()) {
            TAG_LOGW(AAFwkTag::UI_EXT, "Callback not found for requestCode: %{public}d", requestCode);
            return;
        }
        callData = it->second;
        resultCallbacks_.erase(it);
        needUnregister = resultCallbacks_.empty();
    }
    if (callData != nullptr && callData->task != nullptr && callData->handler_ != nullptr) {
        std::lock_guard lock(callData->mutexlock);
        auto task = [callData, extensionAbilityId, innerErrCode]() {
            callData->task(extensionAbilityId, innerErrCode);
        };
        callData->handler_->PostTask(task, "OnPreloadSuccess");
    }
    if (needUnregister) {
        UnRegisterPreloadUIExtensionHostClient();
    }
}
} // namespace AbilityRuntime
} // namespace OHOS