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

#include "app_image_observer_manager.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t IMAGE_PROCESS_TYPE_TEMPLATE = 1;
}

AppImageLifeCycleCallbackVector AppImageObserverManager::appImageLifeCycleCallback_;

AppImageObserverManager& AppImageObserverManager::GetInstance()
{
    static AppImageObserverManager appImageObserverManager;
    return appImageObserverManager;
}

void AppImageObserverManager::RegisterImageLifecycleCallback(
    const std::weak_ptr<AbilityRuntime::AppImageLifeCycleCallback> &appImageLifeCycleCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "register update callback");
    std::lock_guard<std::recursive_mutex> lock(appImageLifeCycleCallbackLock_);
    appImageLifeCycleCallback_.push_back(appImageLifeCycleCallback);
}

void AppImageObserverManager::NotifyApplicationUpdate()
{
    TAG_LOGD(AAFwkTag::APPKIT, "notify onUpdate");
    auto appImageLifeCycleCallbackCopy = GetAppImageLifeCycleCallbackCopy();
    for (auto& callback : appImageLifeCycleCallbackCopy) {
        auto callbackSptr = callback.lock();
        if (callbackSptr != nullptr) {
            callbackSptr->NotifyApplicationUpdate();
        }
    }
}

void AppImageObserverManager::NotifyApplicationPreAbilityCreate()
{
    TAG_LOGD(AAFwkTag::APPKIT, "notify onPreAbilityCreate");
    auto appImageLifeCycleCallbackCopy = GetAppImageLifeCycleCallbackCopy();
    for (auto& callback : appImageLifeCycleCallbackCopy) {
        auto callbackSptr = callback.lock();
        if (callbackSptr != nullptr) {
            callbackSptr->NotifyApplicationPreAbilityCreate();
        }
    }
}

AppImageLifeCycleCallbackVector AppImageObserverManager::GetAppImageLifeCycleCallbackCopy()
{
    std::lock_guard<std::recursive_mutex> lock(appImageLifeCycleCallbackLock_);
    return appImageLifeCycleCallback_;
}

void AppImageObserverManager::SetImageProcessType(int32_t imageProcessType)
{
    imageProcessType_.store(imageProcessType);
}

int32_t AppImageObserverManager::GetImageProcessType() const
{
    return imageProcessType_.load();
}

void AppImageObserverManager::SetAbilityCreated(bool flag)
{
    isAbilityCreated_.store(flag);
}

bool AppImageObserverManager::IsAbilityCreated() const
{
    return isAbilityCreated_.load();
}

bool AppImageObserverManager::IsBeforeImageCreationPoint() const
{
    return (GetImageProcessType() == IMAGE_PROCESS_TYPE_TEMPLATE) && !IsAbilityCreated();
}
}  // namespace AppExecFwk
}  // namespace OHOS