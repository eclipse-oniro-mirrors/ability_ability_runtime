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

#include "ability_native_thread.h"

#include <pthread.h>

#include "hilog_tag_wrapper.h"
#include "module_manager/native_module_manager.h"
#include "native_ability_util.h"

namespace OHOS {
namespace AppExecFwk {
constexpr const char* DEFAULT_NAMESPACE = "default";
AbilityNativeThread::~AbilityNativeThread()
{
    // Clean up resources
    if (nativeThread_.joinable()) {
        TAG_LOGW(AAFwkTag::ABILITY, "Native thread is still running, forcing detach");
        nativeThread_.detach();
    }

    if (moduleHandle_ != nullptr) {
        dlclose(moduleHandle_);
        moduleHandle_ = nullptr;
        TAG_LOGI(AAFwkTag::ABILITY, "Native module unloaded");
    }
}

bool AbilityNativeThread::LoadNativeModule(const AAFwk::NativeAbilityMetaData& metaData,
    const std::string &bundleName, const std::string &moduleName)
{
    if (metaData.nativeModuleSource.empty() || metaData.nativeModuleFunc.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Native config source or OHMain is empty");
        return false;
    }

    if (moduleHandle_ != nullptr) {
        return true;
    }

    // Load the dynamic library
    std::string bundleModuleName = bundleName + "/" + moduleName;
    moduleHandle_ = OpenNativeLibrary(bundleModuleName, metaData.nativeModuleSource);
    if (moduleHandle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to load native module: %{public}s-%{public}s",
            bundleModuleName.c_str(), metaData.nativeModuleSource.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Native module loaded: %{public}s", metaData.nativeModuleSource.c_str());
    // Find the OHMain function
    auto rawMain = reinterpret_cast<void(*)()>(dlsym(moduleHandle_, metaData.nativeModuleFunc.c_str()));
    if (rawMain == nullptr) {
        char* error = dlerror();
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to find OHMain function: %{public}s, error: %{public}s",
            metaData.nativeModuleFunc.c_str(), error ? error : "null");
        dlclose(moduleHandle_);
        moduleHandle_ = nullptr;
        return false;
    }
    ohMainFun_ = rawMain;

    TAG_LOGI(AAFwkTag::ABILITY, "OHMain function found: %{public}s", metaData.nativeModuleFunc.c_str());

    // Find the PostAbility function (optional but recommended)
    auto rawPostAbility = reinterpret_cast<void(*)(const NativeAbilityWrapper*)>(dlsym(moduleHandle_, "PostAbility"));
    if (rawPostAbility != nullptr) {
        postAbilityFunc_ = rawPostAbility;
        TAG_LOGI(AAFwkTag::ABILITY, "PostAbility function found");
    }

    // Find the DestroyAbility function (optional)
    auto rawDestroyAbility = reinterpret_cast<void(*)(const NativeAbilityWrapper*)>(
        dlsym(moduleHandle_, "DestroyAbility"));
    if (rawDestroyAbility != nullptr) {
        destroyAbilityFunc_ = rawDestroyAbility;
        TAG_LOGI(AAFwkTag::ABILITY, "DestroyAbility function found");
    }

    // Find the NotifyProcessExit function (optional)
    auto rawNotifyProcessExit = reinterpret_cast<void(*)()>(dlsym(moduleHandle_, "NotifyProcessExit"));
    if (rawNotifyProcessExit != nullptr) {
        notifyProcessExitFunc_ = rawNotifyProcessExit;
        TAG_LOGI(AAFwkTag::ABILITY, "NotifyProcessExit function found");
    }
    return true;
}

void AbilityNativeThread::RunMain()
{
    if (ohMainFun_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "OHMain function is null, cannot run native thread");
        return;
    }

    if (nativeThread_.joinable()) {
        TAG_LOGW(AAFwkTag::ABILITY, "Native thread running");
        return;
    }

    // Capture ohMainFun_ by value to avoid capturing this
    auto mainFunc = ohMainFun_;
    nativeThread_ = std::thread([mainFunc]() {
        pthread_setname_np(pthread_self(), "native_main");
        TAG_LOGI(AAFwkTag::ABILITY, "Native thread started");

        // Call the main function
        mainFunc();

        TAG_LOGI(AAFwkTag::ABILITY, "Native thread finished");
    });

    TAG_LOGI(AAFwkTag::ABILITY, "Native thread created");
}

void AbilityNativeThread::PostAbility(const NativeAbilityWrapper* nativeAbilityWrapper)
{
    if (nativeAbilityWrapper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "NativeAbilityWrapper is null");
        return;
    }

    if (postAbilityFunc_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "PostAbility function is null");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Posting ability to native thread: %{public}s (instanceId: %{public}s)",
        nativeAbilityWrapper->abilityName.c_str(), nativeAbilityWrapper->instanceId.c_str());

    // Call the PostAbility function from the native module
    postAbilityFunc_(nativeAbilityWrapper);
}

void AbilityNativeThread::DestroyAbility(const NativeAbilityWrapper* nativeAbilityWrapper)
{
    if (destroyAbilityFunc_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "DestroyAbility function is null");
        return;
    }

    if (nativeAbilityWrapper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "nativeAbilityWrapper is null");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Destroying ability with instanceId: %{public}s",
        nativeAbilityWrapper->instanceId.c_str());
    destroyAbilityFunc_(nativeAbilityWrapper);
}

void AbilityNativeThread::NotifyProcessExit()
{
    if (notifyProcessExitFunc_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "NotifyProcessExit function is null");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Notifying process exit");
    notifyProcessExitFunc_();
}

LIBHANDLE AbilityNativeThread::OpenNativeLibrary(const std::string& bundleModuleName, const std::string& fileName)
{
    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null moduleManager");
        return nullptr;
    }
    std::string namespaceName;
    if (!moduleManager->GetLdNamespaceName(bundleModuleName, namespaceName)) {
        if (!moduleManager->GetLdNamespaceName(DEFAULT_NAMESPACE, namespaceName)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "GetLdNamespaceName failed");
            return nullptr;
        }
    }
    Dl_namespace ns;
    if (dlns_get(namespaceName.data(), &ns) != 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "namespaceName not found: %{public}s", namespaceName.c_str());
        return nullptr;
    }
    LIBHANDLE nativeHandle = nullptr;
    auto libName = fileName;
    auto pos = fileName.find_last_of('/');
    if (pos != std::string::npos) {
        libName = fileName.substr(pos);
    }
    nativeHandle = dlopen_ns(&ns, libName.c_str(), RTLD_LAZY);
    return nativeHandle;
}
} // namespace AppExecFwk
} // namespace OHOS
