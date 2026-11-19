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

#include "service_extension.h"

#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "hilog_tag_wrapper.h"
#include "js_service_extension.h"
#include "runtime.h"
#include "service_extension_context.h"
#include "ets_service_extension_instance.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

CreatorFunc ServiceExtension::creator_ = nullptr;
void ServiceExtension::SetCreator(const CreatorFunc& creator)
{
    creator_ = creator;
}

ServiceExtension* ServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new ServiceExtension();
    }

    if (creator_) {
        return creator_(runtime);
    }

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsServiceExtension::Create(runtime);
        case Runtime::Language::ETS:
            return CreateETSServiceExtension(runtime);
        default:
            return new ServiceExtension();
    }
}

void ServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<ServiceExtensionContext>::Init(record, application, handler, token);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "begin init context");
}

std::shared_ptr<ServiceExtensionContext> ServiceExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<ServiceExtensionContext> context =
        ExtensionBase<ServiceExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return context;
    }
    return context;
}

void ServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    Extension::OnConfigurationUpdated(configuration);

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(configuration, context->GetResourceManager());
    auto contextConfig = context->GetConfiguration();
    if (contextConfig != nullptr) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
        std::vector<std::string> changeKeyV;
        contextConfig->CompareDifferent(changeKeyV, configuration);
        if (!changeKeyV.empty()) {
            contextConfig->Merge(changeKeyV, configuration);
        }
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump after merge: %{public}s", contextConfig->GetName().c_str());
    }
}

bool ServiceExtension::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo &executorInfo)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "GetInsightIntentExecutorInfo called");
    auto context = GetContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Param invalid");
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    executorInfo.srcEntry = wantParams.GetStringParam(AppExecFwk::INSIGHT_INTENT_SRC_ENTRY);
    executorInfo.decoratorClass = wantParams.GetStringParam(AppExecFwk::INSIGHT_INTENT_DECORATOR_CLASS);
    executorInfo.hapPath = abilityInfo_->hapPath;
    executorInfo.esmodule = abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    executorInfo.token = context->GetToken();
    executorInfo.executeParam = executeParam;
    return true;
}

bool ServiceExtension::OnInsightIntentExecuteDone(uint64_t intentId,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "Notify execute done, intentId %{public}" PRIu64"", intentId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return false;
    }
    auto token = context->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null token");
        return false;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token, intentId, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Notify execute done failed");
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
