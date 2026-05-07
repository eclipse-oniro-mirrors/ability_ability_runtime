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

#include "modular_object_extension.h"

#include <new>

#include "hilog_tag_wrapper.h"
#include "ipc_inner_object.h"
#include "native_runtime.h"
#include "securec.h"
#include "want_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char PATH_SEPARATOR = '/';
}

ModularObjectExtension* ModularObjectExtension::Create()
{
    return new ModularObjectExtension();
}

void ModularObjectExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<ModularObjectExtensionContext>::Init(record, application, handler, token);

    moeInstance_ = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionInstance>();
    moeInstance_->type = AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    moeInstance_->extension = weak_from_this();

    moeContext_ = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    if (moeContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "failed to create modular object extension context");
        return;
    }
    moeContext_->type = AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto context = GetContext();
    if (context != nullptr) {
        moeContext_->context = context->weak_from_this();
    }
    moeInstance_->context = moeContext_;

    if (!LoadNativeExtensionModule()) {
        TAG_LOGE(AAFwkTag::EXT, "failed to load modular object native extension module");
    }
}

std::shared_ptr<ModularObjectExtensionContext> ModularObjectExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    return ExtensionBase<ModularObjectExtensionContext>::CreateAndInitContext(record, application, handler, token);
}

void ModularObjectExtension::OnStart(const AAFwk::Want &want)
{
    if (moeInstance_ == nullptr || moeInstance_->onCreateFunc == nullptr) {
        return;
    }

    AbilityBase_Want cWant;
    AbilityBase_Element element;
    if (!BuildCWant(want, cWant, element)) {
        TAG_LOGE(AAFwkTag::EXT, "failed to build c want for OnStart");
        return;
    }
    moeInstance_->onCreateFunc(moeInstance_.get(), &cWant);
    DestroyElement(element);
}

void ModularObjectExtension::OnStop()
{
    if (moeInstance_ != nullptr && moeInstance_->onDestroyFunc != nullptr) {
        moeInstance_->onDestroyFunc(moeInstance_.get());
    }
}

sptr<IRemoteObject> ModularObjectExtension::OnConnect(const AAFwk::Want &want)
{
    Extension::OnConnect(want);
    if (moeInstance_ == nullptr || moeInstance_->onConnectFunc == nullptr) {
        return nullptr;
    }

    AbilityBase_Want cWant;
    AbilityBase_Element element;
    if (!BuildCWant(want, cWant, element)) {
        TAG_LOGE(AAFwkTag::EXT, "failed to build c want for OnConnect");
        return nullptr;
    }
    OHIPCRemoteStub *stub = moeInstance_->onConnectFunc(moeInstance_.get(), &cWant);
    DestroyElement(element);
    if (stub == nullptr || stub->remote == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "invalid remote stub returned from OnConnect callback");
        return nullptr;
    }
    return stub->remote;
}

void ModularObjectExtension::OnDisconnect(const AAFwk::Want &want)
{
    Extension::OnDisconnect(want);
    if (moeInstance_ != nullptr && moeInstance_->onDisconnectFunc != nullptr) {
        moeInstance_->onDisconnectFunc(moeInstance_.get());
    }
}

bool ModularObjectExtension::LoadNativeExtensionModule()
{
    if (moeInstance_ == nullptr || abilityInfo_ == nullptr) {
        return false;
    }
    if (abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "srcEntrance is empty");
        return false;
    }

    std::string srcPath = abilityInfo_->moduleName + PATH_SEPARATOR + abilityInfo_->srcEntrance;
    std::string bundleModuleName = abilityInfo_->bundleName + PATH_SEPARATOR + abilityInfo_->moduleName;
    size_t pos = srcPath.find_last_of(PATH_SEPARATOR);
    std::string fileName = pos == std::string::npos ? srcPath : srcPath.substr(pos + 1);
    return NativeRuntime::LoadModule(bundleModuleName, fileName, abilityInfo_->name, *moeInstance_);
}

bool ModularObjectExtension::BuildCWant(const AAFwk::Want &want, AbilityBase_Want &cWant,
    AbilityBase_Element &element) const
{
    auto ret = AAFwk::CWantManager::TransformToCWantWithoutElement(want, false, cWant);
    if (ret != ABILITY_BASE_ERROR_CODE_NO_ERROR) {
        return false;
    }
    element.bundleName = nullptr;
    element.moduleName = nullptr;
    element.abilityName = nullptr;
    if (!BuildElement(want.GetElement(), element)) {
        return false;
    }
    cWant.element = element;
    return true;
}

bool ModularObjectExtension::BuildElement(const AppExecFwk::ElementName &elementName, AbilityBase_Element &element)
{
    auto copyField = [](const std::string &src, char *&dst) -> bool {
        dst = new (std::nothrow) char[src.size() + 1];
        if (dst == nullptr) {
            return false;
        }
        if (strcpy_s(dst, src.size() + 1, src.c_str()) != EOK) {
            delete[] dst;
            dst = nullptr;
            return false;
        }
        return true;
    };
    if (!copyField(elementName.GetBundleName(), element.bundleName)) {
        return false;
    }
    if (!copyField(elementName.GetModuleName(), element.moduleName)) {
        delete[] element.bundleName;
        element.bundleName = nullptr;
        return false;
    }
    if (!copyField(elementName.GetAbilityName(), element.abilityName)) {
        delete[] element.bundleName;
        delete[] element.moduleName;
        element.bundleName = nullptr;
        element.moduleName = nullptr;
        return false;
    }
    return true;
}

void ModularObjectExtension::DestroyElement(AbilityBase_Element &element)
{
    delete[] element.bundleName;
    delete[] element.moduleName;
    delete[] element.abilityName;
    element.bundleName = nullptr;
    element.moduleName = nullptr;
    element.abilityName = nullptr;
}
} // namespace AbilityRuntime
} // namespace OHOS
