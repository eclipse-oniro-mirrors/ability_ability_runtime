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

#include "ani_common_execute_result.h"

#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "want_params.h"
#include <memory>

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr const char *CLASSNAME_ARRAY = "std.core.Array";

void SetModalStringField(ani_env *env, ani_object &obj, const char *name,
    const std::string &value)
{
    ani_string tmpStr = nullptr;
    env->String_NewUTF8(value.c_str(), value.size(), &tmpStr);
    env->Object_SetPropertyByName_Ref(obj, name, tmpStr);
}
}

bool UnwrapResultOfExecuteResult(ani_env *env, ani_object &param, InsightIntentExecuteResult &executeResult)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return false;
    }
    ani_status status = ANI_OK;
    ani_ref wantParamRef = nullptr;
    if (!GetRefProperty(env, param, "result", wantParamRef)) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return false;
    }

    auto wantParams = std::make_shared<AAFwk::WantParams>();
    if (!UnwrapWantParams(env, wantParamRef, *wantParams)) {
        TAG_LOGE(AAFwkTag::INTENT, "failed to unwrap want parameter");
        return false;
    }

    if (!executeResult.CheckResult(wantParams)) {
        TAG_LOGE(AAFwkTag::INTENT, "Check wp fail");
        return false;
    }
    executeResult.result = wantParams;

    return true;
}

bool UnwrapResultOfDecoratorExecuteResult(ani_env *env, ani_object &param, InsightIntentExecuteResult &executeResult)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return false;
    }
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "decorator param is nullptr");
        return false;
    }

    auto wantParams = std::make_shared<AAFwk::WantParams>();
    if (!UnwrapWantParams(env, param, *wantParams)) {
        TAG_LOGE(AAFwkTag::INTENT, "failed to unwrap want parameter");
        return false;
    }

    if (!executeResult.CheckResult(wantParams)) {
        TAG_LOGE(AAFwkTag::INTENT, "Check wp fail");
        return false;
    }
    executeResult.result = wantParams;
    executeResult.code = wantParams->GetIntParam("code", 0);
    if (!UnwrapInteractionInfoOfExecuteResult(env, param, executeResult)) {
        TAG_LOGE(AAFwkTag::INTENT, "decorator unwrap interactionInfo fail");
        return false;
    }
    return true;
}

bool UnwrapModalUIExtensionFields(ani_env *env, ani_object &obj,
    std::shared_ptr<AppExecFwk::InteractionModalUIExtension> &modalUI)
{
    if (modalUI == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null modalUI");
        return false;
    }
    if (!GetStringProperty(env, obj, "bundleName", modalUI->bundleName) ||
        !GetStringProperty(env, obj, "abilityName", modalUI->abilityName) ||
        !GetStringProperty(env, obj, "moduleName", modalUI->moduleName) ||
        !GetStringProperty(env, obj, "uiExtensionType", modalUI->uiExtensionType) ||
        !GetStringProperty(env, obj, "uri", modalUI->uri)) {
        TAG_LOGE(AAFwkTag::INTENT, "get modal UI extension field fail");
        return false;
    }
    if (!IsExistsProperty(env, obj, "parameters")) {
        return true;
    }
    ani_boolean isUndefined = false;
    ani_ref paramsRef = nullptr;
    if (!GetPropertyRef(env, obj, "parameters", paramsRef, isUndefined) ||
        isUndefined || paramsRef == nullptr) {
        return true;
    }
    auto params = std::make_shared<AAFwk::WantParams>();
    if (!UnwrapWantParams(env, paramsRef, *params)) {
        TAG_LOGE(AAFwkTag::INTENT, "unwrap parameters fail");
        return false;
    }
    modalUI->parameters = params;
    return true;
}

bool UnwrapInteractionInfoOfExecuteResult(
    ani_env *env, ani_object &param, InsightIntentExecuteResult &executeResult)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return false;
    }
    if (!IsExistsProperty(env, param, "interactionInfo")) {
        return true;
    }
    ani_boolean isUndefined = false;
    ani_ref intentRef = nullptr;
    if (!GetPropertyRef(env, param, "interactionInfo", intentRef, isUndefined) ||
        isUndefined || intentRef == nullptr) {
        return true;
    }
    ani_object intentObj = static_cast<ani_object>(intentRef);
    ani_ref interactionUIRef = nullptr;
    if (!GetPropertyRef(env, intentObj, "interactionUI", interactionUIRef, isUndefined) ||
        isUndefined || interactionUIRef == nullptr) {
        return true;
    }
    ani_object interactionUIObj = static_cast<ani_object>(interactionUIRef);
    std::string uiType;
    if (!GetStringProperty(env, interactionUIObj, "interactionUIType", uiType)) {
        TAG_LOGE(AAFwkTag::INTENT, "get interactionUIType fail");
        return false;
    }
    if (uiType == INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
        auto modalUI = std::make_shared<AppExecFwk::InteractionModalUIExtension>();
        modalUI->interactionUIType = uiType;
        if (!UnwrapModalUIExtensionFields(env, interactionUIObj, modalUI)) {
            return false;
        }
        executeResult.interactionInfo = std::make_shared<AppExecFwk::InteractionInfo>();
        executeResult.interactionInfo->interactionUI = modalUI;
    } else {
        auto ui = std::make_shared<AppExecFwk::InteractionUI>();
        ui->interactionUIType = uiType;
        executeResult.interactionInfo = std::make_shared<AppExecFwk::InteractionInfo>();
        executeResult.interactionInfo->interactionUI = ui;
    }
    if (!InsightIntentExecuteResult::CheckInteractionInfo(*executeResult.interactionInfo)) {
        TAG_LOGE(AAFwkTag::INTENT, "Check interactionInfo fail");
        return false;
    }
    return true;
}

bool UnwrapOptionalFields(
    ani_env *env, ani_object &param, InsightIntentExecuteResult &executeResult)
{
    if (IsExistsProperty(env, param, "uris")) {
        std::vector<std::string> uris;
        if (!GetStringArrayProperty(env, param, "uris", uris)) {
            TAG_LOGE(AAFwkTag::INTENT, "unwrap uris is null");
            return false;
        }
        executeResult.uris = uris;
    }

    if (IsExistsProperty(env, param, "flags")) {
        int32_t flags = 0;
        if (!GetIntPropertyObject(env, param, "flags", flags)) {
            TAG_LOGE(AAFwkTag::INTENT, "unwrap flags is null");
            return false;
        }
        executeResult.flags = flags;
    }

    if (IsExistsProperty(env, param, "interactionInfo")) {
        if (!UnwrapInteractionInfoOfExecuteResult(env, param, executeResult)) {
            TAG_LOGE(AAFwkTag::INTENT, "unwrap interactionInfo fail");
            return false;
        }
    }
    return true;
}

bool UnwrapExecuteResult(ani_env *env, ani_object &param, InsightIntentExecuteResult &executeResult, bool isDecorator)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return false;
    }

    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "param is nullptr");
        return false;
    }

    if (isDecorator) {
        executeResult.isDecorator = true;
        if (!UnwrapResultOfDecoratorExecuteResult(env, param, executeResult)) {
            TAG_LOGE(AAFwkTag::INTENT, "unwrap decorator result fail");
            return false;
        }
        return true;
    }

    int32_t code = 0;
    if (!GetIntPropertyValue(env, param, "code", code)) {
        TAG_LOGE(AAFwkTag::INTENT, "parse code fail");
        return false;
    }
    executeResult.code = code;

    if (IsExistsProperty(env, param, "result")) {
        if (!UnwrapResultOfExecuteResult(env, param, executeResult)) {
            TAG_LOGE(AAFwkTag::INTENT, "unwrap execute result fail");
            return false;
        }
    }

    return UnwrapOptionalFields(env, param, executeResult);
}

ani_object WrapInteractionInfo(ani_env *env, const AppExecFwk::InteractionInfo &interactionInfo)
{
    if (env == nullptr || interactionInfo.interactionUI == nullptr) {
        return nullptr;
    }
    ani_class infoCls = nullptr;
    ani_method infoCtor = nullptr;
    if (env->FindClass("@ohos.app.ability.insightIntent.insightIntent.InteractionInfoInner",
        &infoCls) != ANI_OK ||
        env->Class_FindMethod(infoCls, "<ctor>", nullptr, &infoCtor) != ANI_OK) {
        return nullptr;
    }
    ani_object infoObj = nullptr;
    if (env->Object_New(infoCls, infoCtor, &infoObj) != ANI_OK || infoObj == nullptr) {
        return nullptr;
    }
    std::string uiClassName = interactionInfo.interactionUI->interactionUIType ==
        INTERACTION_UI_TYPE_MODAL_UIEXTENSION
        ? "@ohos.app.ability.insightIntent.insightIntent.InteractionModalUIExtensionInner"
        : "@ohos.app.ability.insightIntent.insightIntent.InteractionUIInner";
    ani_class uiCls = nullptr;
    ani_method uiCtor = nullptr;
    if (env->FindClass(uiClassName.c_str(), &uiCls) != ANI_OK ||
        env->Class_FindMethod(uiCls, "<ctor>", nullptr, &uiCtor) != ANI_OK) {
        return infoObj;
    }
    ani_object uiObj = nullptr;
    if (env->Object_New(uiCls, uiCtor, &uiObj) != ANI_OK || uiObj == nullptr) {
        return infoObj;
    }
    SetModalStringField(env, uiObj, "interactionUIType",
        interactionInfo.interactionUI->interactionUIType);
    if (interactionInfo.interactionUI->interactionUIType == INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
        auto modalUI = std::static_pointer_cast<AppExecFwk::InteractionModalUIExtension>(
            interactionInfo.interactionUI);
        SetModalStringField(env, uiObj, "bundleName", modalUI->bundleName);
        SetModalStringField(env, uiObj, "abilityName", modalUI->abilityName);
        SetModalStringField(env, uiObj, "moduleName", modalUI->moduleName);
        SetModalStringField(env, uiObj, "uiExtensionType", modalUI->uiExtensionType);
        SetModalStringField(env, uiObj, "uri", modalUI->uri);
        if (modalUI->parameters != nullptr) {
            SetRefProperty(env, uiObj, "parameters",
                WrapWantParams(env, *modalUI->parameters));
        }
    }
    SetRefProperty(env, infoObj, "interactionUI", uiObj);
    return infoObj;
}

ani_object WrapExecuteResult(ani_env *env, const AppExecFwk::InsightIntentExecuteResult &executeResult)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    if (executeResult.isQueryEntity) {
        return WrapQueryEntityResult(env, executeResult);
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntent.insightIntent.ExecuteResultInner",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if (!SetIntPropertyValue(env, objValue, "code", executeResult.code)) {
        TAG_LOGE(AAFwkTag::INTENT, "SetIntPropertyValue failded");
        return nullptr;
    }
    if (executeResult.result != nullptr) {
        SetRefProperty(env, objValue, "result", WrapWantParams(env, *executeResult.result));
    }
    if (executeResult.uris.size() > 0) {
        SetStringArrayProperty(env, objValue, "uris", executeResult.uris);
    }
    SetIntPropertyObject(env, objValue, "flags", executeResult.flags);
    if (executeResult.interactionInfo != nullptr &&
        executeResult.interactionInfo->interactionUI != nullptr) {
        SetRefProperty(env, objValue, "interactionInfo",
            WrapInteractionInfo(env, *executeResult.interactionInfo));
    }

    return objValue;
}

ani_object WrapQueryEntityResult(ani_env *env, const AppExecFwk::InsightIntentExecuteResult &executeResult)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    if (executeResult.queryResults.size() == 0) {
        return CreateEmptyArray(env);
    }

    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "FindClass failed status: %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "find ctor failed status: %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, executeResult.queryResults.size());
    if (status != ANI_OK || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "Object_New array status: %{public}d", status);
        return nullptr;
    }
    ani_size index = 0;
    for (size_t i = 0; i < executeResult.queryResults.size(); i++) {
        if (executeResult.queryResults[i] == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "queryResult: %{public}zu is nullptr", i);
            continue;
        }
        ani_object aniInfo = static_cast<ani_object>(WrapWantParams(env, *executeResult.queryResults[i]));
        if (aniInfo == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "queryResult: %{public}zu is nullptr", i);
            continue;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, aniInfo);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "queryResult: %{public}zu SetObject failed status: %{public}d", i, status);
            continue;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateNullExecuteResult(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("@ohos.app.ability.insightIntent.insightIntent.ExecuteResultInner",
        &cls))
        != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
