/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "form_extension.h"

#include "configuration_utils.h"
#include "form_extension_context.h"
#include "form_runtime/js_form_extension.h"
#include "hilog_tag_wrapper.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
FormExtension* FormExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new FormExtension();
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsFormExtension::Create(runtime);
        default:
            return new FormExtension();
    }
}

void FormExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<FormExtensionContext>::Init(record, application, handler, token);
    TAG_LOGD(AAFwkTag::FORM_EXT, "init");
}

std::shared_ptr<FormExtensionContext> FormExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<FormExtensionContext> context =
        ExtensionBase<FormExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null record");
        return context;
    }
    context->SetAbilityInfo(record->GetAbilityInfo());
    return context;
}

OHOS::AppExecFwk::FormProviderInfo FormExtension::OnCreate(const OHOS::AAFwk::Want& want)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    OHOS::AppExecFwk::FormProviderInfo formProviderInfo;
    return formProviderInfo;
}

void FormExtension::OnDestroy(const int64_t formId)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "call");
}

void FormExtension::OnEvent(const int64_t formId, const std::string& message)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
}

void FormExtension::OnUpdate(const int64_t formId, const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "call");
}

void FormExtension::OnCastToNormal(const int64_t formId)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
}

void FormExtension::OnVisibilityChange(const std::map<int64_t, int32_t>& formEventsMap)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
}

FormState FormExtension::OnAcquireFormState(const Want &want)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    return FormState::DEFAULT;
}

bool FormExtension::OnShare(int64_t formId, AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "call");
    return false;
}

bool FormExtension::OnAcquireData(int64_t formId, AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "call");
    return false;
}

void FormExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    Extension::OnConfigurationUpdated(configuration);

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    if (configUtils) {
        configUtils->UpdateGlobalConfig(configuration, context->GetResourceManager());
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
