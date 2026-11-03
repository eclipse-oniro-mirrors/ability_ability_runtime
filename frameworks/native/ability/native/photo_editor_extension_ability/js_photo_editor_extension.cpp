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

#include "js_photo_editor_extension.h"
#include "js_photo_editor_extension_context.h"
#include "js_photo_editor_extension_impl.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

JsPhotoEditorExtension *JsPhotoEditorExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) JsPhotoEditorExtension(runtime);
}

JsPhotoEditorExtension::JsPhotoEditorExtension(const std::unique_ptr<Runtime> &runtime)
{
    impl_ = std::make_shared<JsPhotoEditorExtensionImpl>(runtime);
    SetUIExtensionBaseImpl(impl_);
}

void JsPhotoEditorExtension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
                                  const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
                                  std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
                                  const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "Begin init photo editor extension");
    std::shared_ptr<PhotoEditorExtensionContext> context = std::make_shared<PhotoEditorExtensionContext>();
    context->SetToken(token);
    auto appContext = Context::GetApplicationContext();
    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null appContext");
        return;
    }
    context->SetApplicationInfo(appContext->GetApplicationInfo());
    context->SetResourceManager(appContext->GetResourceManager());
    context->SetParentContext(appContext);

    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null record");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Begin init abilityInfo");
    auto abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null abilityInfo");
        return;
    }
    context->SetAbilityInfo(abilityInfo);
    context->InitHapModuleInfo(abilityInfo);
    context->SetConfiguration(appContext->GetConfiguration());
    if (abilityInfo->applicationInfo.multiProjects) {
        std::shared_ptr<Context> moduleContext = context->CreateModuleContext(abilityInfo->moduleName);
        if (moduleContext != nullptr) {
            auto rm = moduleContext->GetResourceManager();
            context->SetResourceManager(rm);
        }
    }

    Extension::Init(record, application, handler, token);
    impl_->SetContext(context);
    impl_->SetAbilityInfo(Extension::abilityInfo_);
    auto extensionCommon = impl_->Init(record, application, handler, token);
    ExtensionBase<PhotoEditorExtensionContext>::SetExtensionCommon(extensionCommon);
}

} // namespace AbilityRuntime
} // namespace OHOS
