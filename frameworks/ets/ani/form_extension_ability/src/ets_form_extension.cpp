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

#include "ets_form_extension.h"

#include <sstream>
#include <vector>

#include "ability_info.h"
#include "ani_common_util.h"
#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "ets_runtime.h"
#include "form_provider_data.h"
#include "form_runtime/form_extension_provider_client.h"
#include "hilog_tag_wrapper.h"
#include "ets_form_extension_context.h"
#include "connection_manager.h"
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
#include "histogram_plugin_macros.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* FORM_BINDING_DATA_CLASS_NAME =
    "@ohos.app.form.formBindingData.formBindingData.FormBindingDataInner";
constexpr const char* RECORD_CLASS_NAME = "std.core.Record";
constexpr const char *FORM_LOCATION_STATE_ENUM_NAME = "@ohos.app.form.formInfo.formInfo.FormLocation";
constexpr const char *FORM_LOCATION_CHANGED_STATE_NAME =
    "C{std.core.String}C{@ohos.app.form.formInfo.formInfo.FormLocation}:";
constexpr const char *FORM_DIMENSION_ENUM_NAME = "@ohos.app.form.formInfo.formInfo.FormDimension";
constexpr const char *FORM_SIZE_CHANGED_NAME =
    "C{std.core.String}C{@ohos.app.form.formInfo.formInfo.FormDimension}C{@ohos.app.form.formInfo.formInfo.Rect}:";
constexpr const char* FORM_RECT_NAME = "@ohos.app.form.formInfo.formInfo.RectInner";
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
constexpr bool HISTOGRAM_BOOLEAN_SAMPLE = true;
#endif
}

extern "C" __attribute__((visibility("default"))) FormExtension *OHOS_ABILITY_ETSFormExtension(
    const std::unique_ptr<Runtime> &runtime)
{
    return new ETSFormExtension(static_cast<ETSRuntime &>(*runtime));
}

ETSFormExtension *ETSFormExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call___%{public}d", runtime->GetLanguage());
    return new ETSFormExtension(static_cast<ETSRuntime &>(*runtime));
}

const ETSRuntime &ETSFormExtension::GetETSRuntime()
{
    return etsRuntime_;
}

ETSFormExtension::ETSFormExtension(ETSRuntime &etsRuntime) : etsRuntime_(etsRuntime) {}

ETSFormExtension::~ETSFormExtension()
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "destructor");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env null");
        return;
    }
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "etsAbilityObj_ null");
        return;
    }
    if (etsAbilityObj_->aniRef) {
        env->GlobalReference_Delete(etsAbilityObj_->aniRef);
    }
}

void ETSFormExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "Init call");
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null localAbilityRecord");
        return;
    }
    auto abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null abilityInfo");
        return;
    }
    FormExtension::Init(record, application, handler, token);

    std::string srcPath;
    GetSrcPath(srcPath);

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::FORM_EXT,
        "moduleName:%{public}s,srcPath:%{public}s,"
        "compileMode :%{public}d",
        moduleName.c_str(), srcPath.c_str(), abilityInfo_->compileMode);

    BindContext(abilityInfo, record->GetWant(), moduleName, srcPath);
    TAG_LOGI(AAFwkTag::FORM_EXT, "Init End");
}

void ETSFormExtension::GetSrcPath(std::string &srcPath)
{
    if (!Extension::abilityInfo_->isModuleJson) {
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        auto pos = srcPath.rfind(".");
        if (pos != std::string::npos) {
            srcPath.erase(pos);
            srcPath.append(".abc");
        }
    }
}

void ETSFormExtension::UpdateFormExtensionObj(
    std::shared_ptr<AbilityInfo> &abilityInfo, const std::string &moduleName, const std::string &srcPath)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "UpdateFormExtensionObj call");
    etsAbilityObj_ = etsRuntime_.LoadModule(moduleName, srcPath, abilityInfo->hapPath,
        abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE, false, abilityInfo_->srcEntrance);
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsAbilityObj_");
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "UpdateFormExtensionObj End");
}

void ETSFormExtension::BindContext(std::shared_ptr<AbilityInfo> &abilityInfo, std::shared_ptr<AAFwk::Want> want,
    const std::string &moduleName, const std::string &srcPath)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "BindContext call");

    UpdateFormExtensionObj(abilityInfo, moduleName, srcPath);

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env null");
        return;
    }
    if (etsAbilityObj_ == nullptr || want == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "etsAbilityObj_ or abilityContext_ or want is null");
        return;
    }

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get context error");
        return;
    }
    ani_ref contextObj = CreateEtsFormExtensionContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Create context obj error");
        return;
    }
    ani_ref contextGlobalRef = nullptr;
    ani_field field = nullptr;
    ani_status status = ANI_ERROR;

    if ((status = env->GlobalReference_Create(contextObj, &contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "GlobalReference_Create failed, status : %{public}d", status);
        return;
    }
    if ((status = env->Class_FindField(etsAbilityObj_->aniCls, "context", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindField failed, status : %{public}d", status);
        return;
    }

    if ((status = env->Object_SetField_Ref(etsAbilityObj_->aniObj, field, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_SetField_Ref failed, status : %{public}d", status);
        return;
    }

    TAG_LOGI(AAFwkTag::FORM_EXT, "BindContext End");
}

std::string ETSFormExtension::ANIUtils_ANIStringToStdString(ani_env *env, ani_string ani_str)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "ANIUtils_ANIStringToStdString Call");
    ani_size strSize;
    if (ANI_OK != env->String_GetUTF8Size(ani_str, &strSize)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_GetUTF8Size Failed");
        return "";
    }

    std::vector<char> buffer(strSize + 1);
    char *utf8_buffer = buffer.data();

    ani_size bytes_written = 0;
    if (ANI_OK != env->String_GetUTF8(ani_str, utf8_buffer, strSize + 1, &bytes_written)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_GetUTF8 Failed");
        return "";
    }

    if (bytes_written <= strSize) {
        utf8_buffer[bytes_written] = '\0';
    } else {
        utf8_buffer[strSize] = '\0';
        TAG_LOGW(AAFwkTag::FORM_EXT, "ANIUtils_ANIStringToStdString bytes_written exceeds buffer size");
    }

    std::string content = std::string(utf8_buffer);
    TAG_LOGI(AAFwkTag::FORM_EXT, "ANIUtils_ANIStringToStdString End");
    return content;
}

bool ETSFormExtension::ConvertFromDataProxies(
    ani_env *env, ani_object arrayValue, std::vector<FormDataProxy> &formDataProxies)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "ConvertFromDataProxies Call");
    ani_status status = ANI_OK;
    if (arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "arrayValue null");
        return false;
    }

    ani_double length;
    if (ANI_OK != env->Object_GetPropertyByName_Double(arrayValue, "length", &length)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetPropertyByName_Double length Failed");
        return false;
    }

    int proxyLength = static_cast<int>(std::round(length));
    for (int i = 0; i < proxyLength; i++) {
        FormDataProxy formDataProxy("", "");
        ani_ref stringEntryRef;
        if (ANI_OK !=
            env->Object_CallMethodByName_Ref(arrayValue, "$_get", "i:Y", &stringEntryRef, (ani_int)i)) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Object_CallMethodByName_Ref _get Failed");
            return false;
        }

        ani_field key;
        if ((status = env->Class_FindField(etsAbilityObj_->aniCls, "key", &key)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindField status : %{public}d", status);
            return false;
        }

        ani_ref keyRef;
        if ((status = env->Object_GetField_Ref(static_cast<ani_object>(stringEntryRef), key, &keyRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetField_Ref status : %{public}d", status);
            return false;
        }

        formDataProxy.key = ANIUtils_ANIStringToStdString(env, static_cast<ani_string>(keyRef));

        ani_field subscriberId;
        if ((status = env->Class_FindField(etsAbilityObj_->aniCls, "subscriberId", &subscriberId)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindField status : %{public}d", status);
            return false;
        }

        ani_ref subscriberIdRef;
        status = env->Object_GetField_Ref(static_cast<ani_object>(stringEntryRef), subscriberId, &subscriberIdRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetField_Ref status : %{public}d", status);
            return false;
        }

        formDataProxy.subscribeId = ANIUtils_ANIStringToStdString(env, static_cast<ani_string>(subscriberIdRef));
        formDataProxies.push_back(formDataProxy);
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "ConvertFromDataProxies End");
    return true;
}

sptr<IRemoteObject> ETSFormExtension::OnConnect(const OHOS::AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "call");
    Extension::OnConnect(want);
    if (providerRemoteObject_ == nullptr) {
        TAG_LOGD(AAFwkTag::FORM_EXT, "null providerRemoteObject");
        sptr<FormExtensionProviderClient> providerClient = new (std::nothrow) FormExtensionProviderClient();
        std::shared_ptr<ETSFormExtension> formExtension =
            std::static_pointer_cast<ETSFormExtension>(shared_from_this());
        providerClient->SetOwner(formExtension);
        providerRemoteObject_ = providerClient->AsObject();
    }
    return providerRemoteObject_;
}

OHOS::AppExecFwk::FormProviderInfo ETSFormExtension::OnCreate(const OHOS::AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnCreate call");

    OHOS::AppExecFwk::FormProviderInfo formProviderInfo;
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env null");
        return formProviderInfo;
    }

    ani_object aniWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null aniWant");
        return formProviderInfo;
    }

    ani_ref nativeResult;
    if (!CallNativeFormMethod(env, aniWant, nativeResult)) {
        return formProviderInfo;
    }

    AppExecFwk::FormProviderData formData;
    std::vector<FormDataProxy> formDataProxies;
    if (ExtractFormData(env, nativeResult, formData, formDataProxies)) {
        formProviderInfo.SetFormData(formData);
        if (!formDataProxies.empty()) {
            formProviderInfo.SetFormDataProxies(formDataProxies);
        }
    }

    TAG_LOGI(AAFwkTag::FORM_EXT, "OnCreate End");
    return formProviderInfo;
}

bool ETSFormExtension::CallNativeFormMethod(ani_env *env, ani_object aniWant, ani_ref &nativeResult)
{
    ani_status status = ANI_OK;

    ani_method function;
    if ((status = env->Class_FindMethod(etsAbilityObj_->aniCls, "onAddForm",
        "C{@ohos.app.ability.Want.Want}:C{@ohos.app.form.formBindingData.formBindingData.FormBindingData}",
        &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status: %{public}d", status);
        return false;
    }

    status = env->Object_CallMethod_Ref(etsAbilityObj_->aniObj, function, &nativeResult, aniWant);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_CallMethod_Ref status: %{public}d", status);
        return false;
    }

    return true;
}

bool ETSFormExtension::ExtractFormData(ani_env *env, ani_ref nativeResult, AppExecFwk::FormProviderData &formData,
    std::vector<FormDataProxy> &formDataProxies)
{
    ani_status status = ANI_OK;

    ani_class cls{};
    status = env->FindClass(FORM_BINDING_DATA_CLASS_NAME, &cls);
    if (status != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FindClass status: %{public}d", status);
        return false;
    }

    ani_method data{};
    status = env->Class_FindMethod(cls, "<get>data", nullptr, &data);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod data status: %{public}d", status);
        return false;
    }

    ani_ref dataRef;
    status = env->Object_CallMethod_Ref(static_cast<ani_object>(nativeResult), data, &dataRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_CallMethod_Ref data status: %{public}d", status);
        return false;
    }

    std::string dataStr = ANIUtils_ANIStringToStdString(env, static_cast<ani_string>(dataRef));
    formData = AppExecFwk::FormProviderData(dataStr);

    ani_method proxies;
    status = env->Class_FindMethod(cls, "<get>proxies", nullptr, &proxies);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod proxies status: %{public}d", status);
        return true;
    }

    ani_ref proxiesRef;
    status = env->Object_CallMethod_Ref(static_cast<ani_object>(nativeResult), proxies, &proxiesRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_CallMethod_Ref proxies status: %{public}d", status);
        return true;
    }

    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(proxiesRef, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Reference_IsUndefined status: %{public}d", status);
    }

    if (!isUndefined && proxiesRef != nullptr) {
        ConvertFromDataProxies(env, static_cast<ani_object>(proxiesRef), formDataProxies);
    }

    return true;
}

ani_status ETSFormExtension::ANIUtils_FormIdToAniString(ani_env *env, int64_t formId, ani_string &formIdStr)
{
    ani_status status = ANI_OK;
    std::string str = std::to_string(formId);
    ani_string formIdStrTmp;
    if ((status = env->String_NewUTF8(str.c_str(), str.size(), &formIdStrTmp))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_NewUTF8 status : %{public}d", status);
        return status;
    }
    formIdStr = formIdStrTmp;
    return status;
}

void ETSFormExtension::OnDestroy(const int64_t formId)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnDestroy formId: %{public}" PRId64, formId);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env null");
        return;
    }
    ani_status status = ANI_OK;

    ani_string formIdStr;
    if (ANIUtils_FormIdToAniString(env, formId, formIdStr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "formId change failed.");
        return;
    }

    ani_method function;
    if ((status = env->Class_FindMethod(etsAbilityObj_->aniCls, "onRemoveForm", "C{std.core.String}:", &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return;
    }

    status = env->Object_CallMethod_Void(etsAbilityObj_->aniObj, function, formIdStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnDestroy End");
}

void ETSFormExtension::OnEvent(const int64_t formId, const std::string &message)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnEvent formId: %{public}" PRId64, formId);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env null");
        return;
    }
    ani_status status = ANI_OK;

    ani_string formIdStr;
    if (ANIUtils_FormIdToAniString(env, formId, formIdStr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "formId change failed.");
        return;
    }
    ani_string aniMessage;
    if ((status = env->String_NewUTF8(message.c_str(), message.size(), &aniMessage))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_NewUTF8 status : %{public}d", status);
        return;
    }
    ani_method function;
    if ((status = env->Class_FindMethod(
        etsAbilityObj_->aniCls, "onFormEvent", "C{std.core.String}C{std.core.String}:", &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return;
    }

    status = env->Object_CallMethod_Void(etsAbilityObj_->aniObj, function, formIdStr, aniMessage);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnEvent End");
}

void ETSFormExtension::OnUpdate(const int64_t formId, const AAFwk::WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnUpdate formId: %{public}" PRId64, formId);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env null");
        return;
    }
    ani_status status = ANI_OK;

    ani_string formIdStr;
    if (ANIUtils_FormIdToAniString(env, formId, formIdStr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "formId change failed.");
        return;
    }

    ani_ref aniWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParams);

    ani_method function;
    if ((status = env->Class_FindMethod(
        etsAbilityObj_->aniCls, "onUpdateForm", "C{std.core.String}C{std.core.Record}:", &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return;
    }
    status = env->Object_CallMethod_Void(
        etsAbilityObj_->aniObj, function, formIdStr, static_cast<ani_object>(aniWantParams));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnUpdate End");
}

void ETSFormExtension::OnCastToNormal(const int64_t formId)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnCastToNormal formId: %{public}" PRId64, formId);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env null");
        return;
    }
    ani_status status = ANI_OK;

    ani_string formIdStr;
    if (ANIUtils_FormIdToAniString(env, formId, formIdStr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "formId change failed.");
        return;
    }

    ani_method function;
    if ((status = env->Class_FindMethod(
        etsAbilityObj_->aniCls, "onCastToNormalForm", "C{std.core.String}:", &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return;
    }

    status = env->Object_CallMethod_Void(etsAbilityObj_->aniObj, function, formIdStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnCastToNormal End");
}

void ETSFormExtension::OnVisibilityChange(const std::map<int64_t, int32_t> &formEventsMap)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnVisibilityChange call");

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env null");
        return;
    }

    ani_method function;
    ani_status status =
        env->Class_FindMethod(etsAbilityObj_->aniCls, "onChangeFormVisibility", "C{std.core.Record}:", &function);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status: %{public}d", status);
        return;
    }

    ani_object recordObject = {};
    if (!CreateAndFillRecordObject(env, formEventsMap, recordObject)) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "formEventsMap empty");
    }

    status = env->Object_CallMethod_Void(etsAbilityObj_->aniObj, function, recordObject);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_CallMethod_Void status: %{public}d", status);
        return;
    }

    TAG_LOGI(AAFwkTag::FORM_EXT, "OnVisibilityChange End");
}

bool ETSFormExtension::CreateAndFillRecordObject(ani_env *env, const std::map<int64_t, int32_t> &formEventsMap,
    ani_object &recordObject)
{
    ani_status status = ANI_OK;
    ani_class recordCls;
    status = env->FindClass(RECORD_CLASS_NAME, &recordCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FindClass failed status: %{public}d", status);
        return false;
    }
    ani_method objectMethod;
    if ((status = env->Class_FindMethod(recordCls, "<ctor>", ":", &objectMethod)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod constructor failed: %{public}d", status);
        return false;
    }
    if ((status = env->Object_New(recordCls, objectMethod, &recordObject)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New failed: %{public}d", status);
        return false;
    }
    ani_method recordSetMethod;
    status = env->Class_FindMethod(recordCls, "$_set",
        "X{C{std.core.Numeric}C{std.core.String}C{std.core.BaseEnum}}Y:",
        &recordSetMethod);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod set failed: %{public}d", status);
        return false;
    }
    for (auto iter = formEventsMap.begin(); iter != formEventsMap.end(); ++iter) {
        std::string key = std::to_string(iter->first);
        ani_string ani_key;
        ani_int ani_value = iter->second;
        if ((status = env->String_NewUTF8(key.c_str(), key.length(), &ani_key)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "String_NewUTF8 key failed status : %{public}d", status);
            return false;
        }
        static const char *className = "std.core.Int";
        ani_class persion_cls;
        if (ANI_OK != env->FindClass(className, &persion_cls)) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "FindClass failed status: %{public}d", status);
            return false;
        }
        ani_method personInfoCtor;
        env->Class_FindMethod(persion_cls, "<ctor>", "i:", &personInfoCtor);
        ani_object personInfoObj;
        env->Object_New(persion_cls, personInfoCtor, &personInfoObj, ani_value);
        if ((status = env->Object_CallMethod_Void(recordObject, recordSetMethod, ani_key, personInfoObj)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Object_CallMethod_Void failed status : %{public}d", status);
            return false;
        }
    }
    return true;
}

void ETSFormExtension::OnStop()
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnStop begin");

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env null");
        return;
    }

    ani_ref nameRef;
    ani_status status = env->Object_GetFieldByName_Ref(
        static_cast<ani_object>(etsAbilityObj_->aniRef), "onStop", &nameRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetFieldByName status: %{public}d", status);
        return;
    }

    ani_ref result;
    status = env->FunctionalObject_Call(static_cast<ani_fn_object>(nameRef), 0, nullptr, &result);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FunctionalObject_Call status: %{public}d", status);
        return;
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGI(AAFwkTag::FORM_EXT, "disconnected failed");
        return;
    }

    TAG_LOGI(AAFwkTag::FORM_EXT, "OnStop End");
}

bool ETSFormExtension::OnShare(int64_t formId, AAFwk::WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnShare Call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env nullptr");
        return false;
    }
    ani_string formIdStr = AppExecFwk::GetAniString(env, std::to_string(formId));
    ani_ref nameRef;
    ani_status status = env->Object_GetFieldByName_Ref(
        static_cast<ani_object>(etsAbilityObj_->aniRef), "onShareForm", &nameRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetFieldByName status: %{public}d", status);
        return false;
    }
    ani_ref argv[] = { formIdStr };
    ani_ref result;
    status = env->FunctionalObject_Call(static_cast<ani_fn_object>(nameRef), 1, argv, &result);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FunctionalObject_Call status: %{public}d", status);
        return false;
    }
    if (!AppExecFwk::UnwrapWantParams(env, result, wantParams)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "unwrap want failed");
        return false;
    }
    return true;
}

bool ETSFormExtension::OnAcquireData(int64_t formId, AAFwk::WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnShare Call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env nullptr");
        return false;
    }
    ani_string formIdStr = AppExecFwk::GetAniString(env, std::to_string(formId));
    ani_ref nameRef;
    ani_status status = env->Object_GetFieldByName_Ref(
        static_cast<ani_object>(etsAbilityObj_->aniRef), "onAcquireFormData", &nameRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetFieldByName status: %{public}d", status);
        return false;
    }
    ani_ref argv[] = { formIdStr };
    ani_ref result;
    status = env->FunctionalObject_Call(static_cast<ani_fn_object>(nameRef), 1, argv, &result);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FunctionalObject_Call status: %{public}d", status);
        return false;
    }
    if (!AppExecFwk::UnwrapWantParams(env, result, wantParams)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "unwrap want failed");
        return false;
    }
    return true;
}

void ETSFormExtension::OnFormLocationChanged(const int64_t formId, const int32_t formLocation)
{
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
    HISTOGRAM_BOOLEAN("Form.ExtensionAbility.onFormLocationChanged", HISTOGRAM_BOOLEAN_SAMPLE);
#endif
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnFormLocationChanged Call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env nullptr");
        return;
    }
    ani_string formIdStr = AppExecFwk::GetAniString(env, std::to_string(formId));

    ani_enum_item formLocationStateItem {};
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env, FORM_LOCATION_STATE_ENUM_NAME, formLocation, formLocationStateItem);

    ani_method function;
    ani_status status = ANI_ERROR;
    if ((status = env->Class_FindMethod(
        etsAbilityObj_->aniCls, "onFormLocationChanged", FORM_LOCATION_CHANGED_STATE_NAME, &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status: %{public}d", status);
        return;
    }

    status = env->Object_CallMethod_Void(etsAbilityObj_->aniObj, function, formIdStr, formLocationStateItem);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
}

void ETSFormExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    FormExtension::OnConfigurationUpdated(configuration);
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnConfigurationUpdated Call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env nullptr");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        return;
    }
    auto fullConfig = context->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null configuration");
        return;
    }

    ani_object aniConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(etsAbilityObj_->aniCls,
        "onConfigurationUpdate", "C{@ohos.app.ability.Configuration.Configuration}:", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod failed, status: %{public}d", status);
        return;
    }
    status = env->Object_CallMethod_Void(etsAbilityObj_->aniObj, method, aniConfiguration);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "CALL Object_CallMethod failed, status: %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnConfigurationUpdated End");
}

FormState ETSFormExtension::OnAcquireFormState(const Want &want)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnAcquireFormState Call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env nullptr");
        return AppExecFwk::FormState::DEFAULT;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "wantRef nullptr");
        return AppExecFwk::FormState::DEFAULT;
    }

    ani_ref nameRef;
    ani_status status = env->Object_GetFieldByName_Ref(
        static_cast<ani_object>(etsAbilityObj_->aniRef), "onAcquireFormState", &nameRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetFieldByName status: %{public}d", status);
        return AppExecFwk::FormState::DEFAULT;
    }
    ani_ref argv[] = { wantRef };
    ani_ref result;
    status = env->FunctionalObject_Call(static_cast<ani_fn_object>(nameRef), 1, argv, &result);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FunctionalObject_Call status: %{public}d", status);
        return AppExecFwk::FormState::DEFAULT;
    }
    int32_t state = static_cast<int32_t>(FormState::DEFAULT);
    AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, static_cast<ani_enum_item>(result), state);
    TAG_LOGI(AAFwkTag::FORM_EXT, "state: %{public}d", state);
    if (state <= static_cast<int32_t>(AppExecFwk::FormState::UNKNOWN) ||
        state > static_cast<int32_t>(AppExecFwk::FormState::READY)) {
        return AppExecFwk::FormState::UNKNOWN;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnAcquireFormState End");
    return static_cast<AppExecFwk::FormState>(state);
}

void ETSFormExtension::OnSizeChanged(int64_t formId, int32_t newDimension, const Rect &newRect)
{
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
    HISTOGRAM_BOOLEAN("Form.ExtensionAbility.onSizeChanged", HISTOGRAM_BOOLEAN_SAMPLE);
#endif
    TAG_LOGI(AAFwkTag::FORM_EXT, "Form size changed, formId: %{public}" PRId64, formId);
    FormExtension::OnSizeChanged(formId, newDimension, newRect);

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env nullptr");
        return;
    }
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "etsAbilityObj nullptr");
        return;
    }

    ani_string formIdStr = AppExecFwk::GetAniString(env, std::to_string(formId));
    ani_enum_item newDimensionItem = nullptr;
    if (!OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env, FORM_DIMENSION_ENUM_NAME,
        newDimension, newDimensionItem) || newDimensionItem == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "newDimensionItem NativeToEts failed");
        return;
    }
    
    ani_object formRectObject = WrapFormRect(env, newRect);
    if (formRectObject == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "wrap formRectObject failed");
        return;
    }

    ani_method function;
    ani_status status = ANI_ERROR;
    if ((status = env->Class_FindMethod(
        etsAbilityObj_->aniCls, "onSizeChanged", FORM_SIZE_CHANGED_NAME, &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status: %{public}d", status);
        return;
    }

    status = env->Object_CallMethod_Void(etsAbilityObj_->aniObj, function,
        formIdStr, newDimensionItem, formRectObject);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_CallMethod status : %{public}d", status);
        return;
    }
}

ani_object ETSFormExtension::WrapFormRect(ani_env *env, const Rect &rect)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env nullptr");
        return nullptr;
    }

    ani_object etsObject = nullptr;
    if (!CreateObject(env, etsObject, FORM_RECT_NAME)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "CreateObject failed");
        return nullptr;
    }

    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Double(etsObject, "left", rect.left)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetPropertyByName_Double(etsObject, "top", rect.top)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetPropertyByName_Double(etsObject, "width", rect.width)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetPropertyByName_Double(etsObject, "height", rect.height)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
        return nullptr;
    }
    return etsObject;
}

bool ETSFormExtension::CreateObject(ani_env *env, ani_object &object, const std::string &className)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(className.c_str(), &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return false;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null cls");
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return false;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null method");
        return false;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return false;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null object");
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
