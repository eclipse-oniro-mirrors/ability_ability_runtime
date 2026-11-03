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

#include "ets_insight_intent_driver.h"

#include <mutex>

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ets_insight_intent_driver_utils.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_callback_interface.h"
#include "insight_intent_host_client.h"
#include "insight_intent_execute_result.h"
#include "ani_common_execute_param.h"
#include "ani_common_execute_result.h"
#include "ani_common_intent_info_filter.h"
#include "ani_common_query_entity_param.h"
#include "ani_common_util.h"
#include "ets_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr const char *DRIVER_CLASS_NAME = "@ohos.app.ability.insightIntentDriver.insightIntentDriver";
}
class EtsInsightIntentExecuteCallbackClient : public InsightIntentExecuteCallbackInterface,
    public std::enable_shared_from_this<EtsInsightIntentExecuteCallbackClient> {
public:
    EtsInsightIntentExecuteCallbackClient(ani_vm *vm, ani_ref callbackRef, ani_ref promiseRef)
        : vm_(vm), callbackRef_(callbackRef), promiseRef_(promiseRef) {}

    virtual ~EtsInsightIntentExecuteCallbackClient()
    {
        ani_env *env = AttachCurrentThread();
        if (env != nullptr) {
            if (promiseRef_) {
                env->GlobalReference_Delete(promiseRef_);
                promiseRef_ = nullptr;
            }
            if (callbackRef_) {
                env->GlobalReference_Delete(callbackRef_);
                callbackRef_ = nullptr;
            }
            DetachCurrentThread();
        }
    }

    void ProcessInsightIntentExecute(int32_t resultCode,
        AppExecFwk::InsightIntentExecuteResult executeResult) override
    {
        ani_env *env = AttachCurrentThread();
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "GetEnv failed");
            return;
        }

        ani_object error;
        ani_object result;
        if (resultCode != 0) {
            error = EtsErrorUtil::CreateErrorByNativeErr(env, resultCode);
            result = CreateNullExecuteResult(env);
        } else {
            error = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
            result = WrapExecuteResult(env, executeResult);
        }
        if (callbackRef_) {
            AsyncCallback(env, static_cast<ani_object>(callbackRef_), error, result);
        }
        if (promiseRef_) {
            AsyncCallback(env, static_cast<ani_object>(promiseRef_), error, result);
        }
        DetachCurrentThread();
    }

    ani_env *AttachCurrentThread()
    {
        ani_env *env = nullptr;
        ani_status status = ANI_ERROR;
        if ((status = vm_->GetEnv(ANI_VERSION_1, &env)) == ANI_OK) {
            return env;
        }

        ani_option interopEnabled { "--interop=disable", nullptr };
        ani_options aniArgs { 1, &interopEnabled };
        if ((status = vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
            return nullptr;
        }
        isAttachThread_ = true;
        return env;
    }

    void DetachCurrentThread()
    {
        if (isAttachThread_) {
            vm_->DetachCurrentThread();
            isAttachThread_ = false;
        }
    }

private:
    ani_vm *vm_ = nullptr;
    ani_ref callbackRef_ = nullptr;
    ani_ref promiseRef_ = nullptr;
    bool isAttachThread_ = false;
};

class EtsInsightIntentDriver {
public:
    EtsInsightIntentDriver() = default;
    ~EtsInsightIntentDriver() = default;

    static void OnExecute(ani_env *env, ani_object exparam, ani_object callback, ani_boolean isCallback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnExecute called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        ani_object error;
        if (exparam == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid param");
            error = EtsErrorUtil::CreateInvalidParamError(env, "invalid param");
            AsyncCallback(env, callback, error, CreateNullExecuteResult(env));
            return;
        }

        InsightIntentExecuteParam param;
        if (!UnwrapExecuteParam(env, exparam, param)) {
            TAG_LOGE(AAFwkTag::INTENT, "parse execute param failed");
            error = EtsErrorUtil::CreateInvalidParamError(env,
                "Parameter error: Parse param failed, param must be a ExecuteParam.");
            AsyncCallback(env, callback, error, CreateNullExecuteResult(env));
            return;
        }

        ani_ref callbackRef = nullptr;
        if (env->GlobalReference_Create(callback, &callbackRef) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "GlobalReference_Create failed");
            error = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(
                AbilityErrorCode::ERROR_CODE_INNER));
            AsyncCallback(env, callback, error, CreateNullExecuteResult(env));
            return;
        }

        ani_vm *vm = nullptr;
        if (env->GetVM(&vm) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "GetVM failed");
            error = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(
                AbilityErrorCode::ERROR_CODE_INNER));
            AsyncCallback(env, callback, error, CreateNullExecuteResult(env));
            return;
        }

        std::shared_ptr<EtsInsightIntentExecuteCallbackClient> client;
        if (isCallback) {
            client = std::make_shared<EtsInsightIntentExecuteCallbackClient>(vm, callbackRef, nullptr);
        } else {
            client = std::make_shared<EtsInsightIntentExecuteCallbackClient>(vm, nullptr, callbackRef);
        }
        uint64_t key = InsightIntentHostClient::GetInstance()->AddInsightIntentExecute(client);
        auto err = AbilityManagerClient::GetInstance()->ExecuteIntent(key,
            InsightIntentHostClient::GetInstance(), param);
        if (err != 0) {
            error = EtsErrorUtil::CreateErrorByNativeErr(env, err);
            AsyncCallback(env, callback, error, CreateNullExecuteResult(env));
            InsightIntentHostClient::GetInstance()->RemoveInsightIntentExecute(key);
        }
        return;
    }

    static void OnGetAllInfoCheck(ani_env *env, ani_int intentFlags)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnGetAllInfoCheck called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        GetInsightIntentFlag flag = static_cast<GetInsightIntentFlag>(intentFlags);
        if (flag != GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
            flag != GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
            flag != (GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO) &&
            flag != (GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse flag failed");
            EtsErrorUtil::ThrowInvalidParamError(env, "Parse param flag failed, flag must be GetInsightIntentFlag.");
            return;
        }
    }

    static void OnGetAllInsightIntentInfo(ani_env *env, ani_int intentFlags, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnGetAllInsightIntentInfo called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        GetInsightIntentFlag flag = static_cast<GetInsightIntentFlag>(intentFlags);
        if (flag != GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
            flag != GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
            flag != (GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO) &&
            flag != (GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse flag failed");
            EtsErrorUtil::ThrowInvalidParamError(env, "Parse param flag failed, flag must be GetInsightIntentFlag.");
            return;
        }
        auto infos = std::make_shared<std::vector<InsightIntentInfoForQuery>>();
        auto innerErrorCode = AbilityManagerClient::GetInstance()->GetAllInsightIntentInfo(flag, *infos);
        if (innerErrorCode == 0) {
            ani_object result = CreateEtsInsightIntentInfoForQueryArray(env, *infos);
            AsyncCallback(env, callback, nullptr, result);
        } else {
            AsyncCallback(env, callback,
                AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrorCode), nullptr);
        }
        return;
    }

    static void OnGetInfoByBundleNameCheck(ani_env *env, ani_string bundleNameObj, ani_int intentFlags)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnGetInfoByBundleNameCheck called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        std::string bundleName;
        if (!GetStdString(env, bundleNameObj, bundleName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse bundleName failed");
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param bundleName failed, bundleName must be string.");
            return;
        }
        GetInsightIntentFlag flag = static_cast<GetInsightIntentFlag>(intentFlags);
        if (flag != GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
            flag != GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
            flag != (GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO) &&
            flag != (GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse flag failed");
            EtsErrorUtil::ThrowInvalidParamError(env, "Parse param flag failed, flag must be GetInsightIntentFlag.");
            return;
        }
    }

    static void OnGetInsightIntentInfoByBundleName(
        ani_env *env, ani_string bundleNameObj, ani_int intentFlags, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnGetInsightIntentInfoByBundleName called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        std::string bundleName;
        if (!GetStdString(env, bundleNameObj, bundleName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse bundleName failed");
            AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param bundleName failed, bundleName must be string."), nullptr);
            return;
        }
        GetInsightIntentFlag flag = static_cast<GetInsightIntentFlag>(intentFlags);
        if (flag != GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
            flag != GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
            flag != (GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO) &&
            flag != (GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse flag failed");
            EtsErrorUtil::ThrowInvalidParamError(env, "Parse param flag failed, flag must be GetInsightIntentFlag.");
            return;
        }
        auto infos = std::make_shared<std::vector<InsightIntentInfoForQuery>>();
        auto innerErrorCode = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByBundleName(
            flag, bundleName, *infos);
        if (innerErrorCode == 0) {
            ani_object result = CreateEtsInsightIntentInfoForQueryArray(env, *infos);
            AsyncCallback(env, callback, nullptr, result);
        } else {
            AsyncCallback(env, callback,
                AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrorCode), nullptr);
        }
        return;
    }

    static void OnGetInfoByIntentNameCheck(ani_env *env, ani_string bundleNameObj,
        ani_string moduleNameObj, ani_string intentNameObj, ani_int intentFlags)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnGetInfoByIntentNameCheck called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        std::string bundleName;
        if (!GetStdString(env, bundleNameObj, bundleName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse bundleName failed");
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param bundleName failed, bundleName must be string.");
            return;
        }
        std::string moduleName;
        if (!GetStdString(env, moduleNameObj, moduleName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse moduleName failed");
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param moduleName failed, moduleName must be string.");
            return;
        }
        std::string intentName;
        if (!GetStdString(env, intentNameObj, intentName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse intentName failed");
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param intentName failed, intentName must be string.");
            return;
        }
        GetInsightIntentFlag flag = static_cast<GetInsightIntentFlag>(intentFlags);
        if (flag != GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
            flag != GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
            flag != (GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO) &&
            flag != (GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse flag failed");
            EtsErrorUtil::ThrowInvalidParamError(env, "Parse param flag failed, flag must be GetInsightIntentFlag.");
            return;
        }
    }

    static void OnGetInsightIntentInfoByIntentName(ani_env *env,
        ani_string bundleNameObj, ani_string moduleNameObj,
        ani_string intentNameObj, ani_int intentFlags, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnGetInsightIntentInfoByIntentName called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        std::string bundleName;
        if (!GetStdString(env, bundleNameObj, bundleName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse bundleName failed");
            AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param bundleName failed, bundleName must be string."), nullptr);
            return;
        }
        std::string moduleName;
        if (!GetStdString(env, moduleNameObj, moduleName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse moduleName failed");
            AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param moduleName failed, moduleName must be string."), nullptr);
            return;
        }
        std::string intentName;
        if (!GetStdString(env, intentNameObj, intentName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse intentName failed");
            AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param intentName failed, intentName must be string."), nullptr);
            return;
        }
        GetInsightIntentFlag flag = static_cast<GetInsightIntentFlag>(intentFlags);
        if (flag != GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
            flag != GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
            flag != (GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO) &&
            flag != (GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse flag failed");
            EtsErrorUtil::ThrowInvalidParamError(env, "Parse param flag failed, flag must be GetInsightIntentFlag.");
            return;
        }
        auto info = std::make_shared<InsightIntentInfoForQuery>();
        auto innerErrorCode = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByIntentName(
            flag, bundleName, moduleName, intentName, *info);
        if (innerErrorCode == 0) {
            ani_object result = CreateEtsInsightIntentInfoForQuery(env, *info);
            AsyncCallback(env, callback, nullptr, result);
        } else {
            AsyncCallback(env, callback,
                AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, innerErrorCode), nullptr);
        }
        return;
    }
    
    static void OnGetInsightIntentInfoByFilterCheck(ani_env *env, ani_object aniFilter)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }

        if (aniFilter == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid param");
            EtsErrorUtil::ThrowInvalidParamError(env, "invalid param");
            return;
        }

        if (!CheckValidIntentInfoFilter(env, aniFilter)) {
            TAG_LOGE(AAFwkTag::INTENT, "check filter failed");
            EtsErrorUtil::ThrowInvalidParamError(env, "Param error: filter must be a valid InsightIntentInfoFilter.");
            return;
        }

        InsightIntentInfoFilter filter;
        if (!UnwrapIntentInfoFilter(env, aniFilter, filter)) {
            TAG_LOGE(AAFwkTag::INTENT, "parse filter failed");
            EtsErrorUtil::ThrowInvalidParamError(env, "Param error: filter must be a InsightIntentInfoFilter.");
            return;
        }
    }

    static void OnGetInsightIntentInfoByFilter(ani_env *env, ani_object aniFilter, ani_object callback)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        if (aniFilter == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid param");
            return;
        }
        if (!CheckValidIntentInfoFilter(env, aniFilter)) {
            TAG_LOGE(AAFwkTag::INTENT, "check filter failed");
            return;
        }
        InsightIntentInfoFilter filter;
        if (!UnwrapIntentInfoFilter(env, aniFilter, filter)) {
            TAG_LOGE(AAFwkTag::INTENT, "parse filter failed");
            return;
        }

        ani_object error;
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        auto infos = std::make_shared<std::vector<InsightIntentInfoForQuery>>();
        if (filter.bundleName_.empty()) {
            *innerErrorCode = AbilityManagerClient::GetInstance()->GetAllInsightIntentInfo(
                filter.intentFlags_, *infos, filter.userId_);
        } else if (!filter.moduleName_.empty() && !filter.intentName_.empty()) {
            auto intentInfo = std::make_shared<InsightIntentInfoForQuery>();
            *innerErrorCode = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByIntentName(
                filter.intentFlags_, filter.bundleName_, filter.moduleName_,
                filter.intentName_, *intentInfo, filter.userId_);
            if (intentInfo != nullptr && (!intentInfo->intentType.empty() || intentInfo->isConfig)) {
                infos->push_back(*intentInfo);
            }
        } else if (filter.moduleName_.empty() && filter.intentName_.empty()) {
            *innerErrorCode = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByBundleName(
                filter.intentFlags_, filter.bundleName_, *infos, filter.userId_);
        }

        if (*innerErrorCode != 0) {
            error = EtsErrorUtil::CreateErrorByNativeErr(env, *innerErrorCode);
            AsyncCallback(env, callback, error, nullptr);
        } else {
            ani_object result = CreateEtsInsightIntentInfoForQueryArray(env, *infos);
            AsyncCallback(env, callback, nullptr, result);
        }
        return;
    }

    static void OnQueryEntityInfoCheck(ani_env *env, ani_object aniParam)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }

        if (aniParam == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid param");
            EtsErrorUtil::ThrowInvalidParamError(env, "invalid param");
            return;
        }

        InsightIntentQueryParam param;
        if (!UnwrapQueryEntityParam(env, aniParam, param)) {
            TAG_LOGE(AAFwkTag::INTENT, "parse query entity param failed");
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parameter error: Parse param failed, param must be a QueryEntityParam.");
            return;
        }

        if (param.queryEntityParam_.queryType_.compare("byProperty") == 0 &&
            param.queryEntityParam_.parameters_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "byProperty queryType must have parameters.");
            EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: byProperty queryType must have parameters.");
            return;
        }
    }

    static void OnQueryEntityInfo(ani_env *env, ani_object aniParam, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnQueryEntityInfo called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }

        ani_object error;
        if (aniParam == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid param");
            return;
        }

        InsightIntentQueryParam param;
        if (!UnwrapQueryEntityParam(env, aniParam, param)) {
            TAG_LOGE(AAFwkTag::INTENT, "parse query entity param failed");
            return;
        }

        if (param.queryEntityParam_.queryType_.compare("byProperty") == 0 &&
            param.queryEntityParam_.parameters_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "byProperty queryType must have parameters.");
            return;
        }

        ani_ref callbackRef = nullptr;
        if (env->GlobalReference_Create(callback, &callbackRef) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "GlobalReference_Create failed");
            error = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
            AsyncCallback(env, callback, error, CreateEmptyArray(env));
            return;
        }

        ani_vm *vm = nullptr;
        if (env->GetVM(&vm) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "GetVM failed");
            env->GlobalReference_Delete(callbackRef);
            error = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
            AsyncCallback(env, callback, error, CreateEmptyArray(env));
            return;
        }

        auto client = std::make_shared<EtsInsightIntentExecuteCallbackClient>(vm, nullptr, callbackRef);
        uint64_t key = InsightIntentHostClient::GetInstance()->AddInsightIntentExecute(client);
        auto err = AbilityManagerClient::GetInstance()->QueryEntityInfo(key,
            InsightIntentHostClient::GetInstance(), param);
        if (err == INTENT_NOT_EXIST) {
            AsyncCallback(env, callback, nullptr, CreateEmptyArray(env));
            InsightIntentHostClient::GetInstance()->RemoveInsightIntentExecute(key);
        } else if (err != 0) {
            error = EtsErrorUtil::CreateErrorByNativeErr(env, err);
            AsyncCallback(env, callback, error, CreateEmptyArray(env));
            InsightIntentHostClient::GetInstance()->RemoveInsightIntentExecute(key);
        }
        return;
    }
};

void EtsInsightIntentDriverInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentDriverInit called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return;
    }

    ani_namespace ns;
    ani_status status = env->FindNamespace(DRIVER_CLASS_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "FindNamespace insightIntentDriver failed status: %{public}d", status);
        return;
    }

    std::array kitFunctions = {
        ani_native_function {"nativeExecuteSync", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnExecute)},
        ani_native_function {"nativeGetAllInsightIntentInfo", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnGetAllInsightIntentInfo)},
        ani_native_function {"nativeGetAllInfoCheck", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnGetAllInfoCheck)},
        ani_native_function {"nativeGetInsightIntentInfoByBundleName", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnGetInsightIntentInfoByBundleName)},
        ani_native_function {"nativeGetInfoByBundleNameCheck", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnGetInfoByBundleNameCheck)},
        ani_native_function {"nativeGetInsightIntentInfoByIntentName", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnGetInsightIntentInfoByIntentName)},
        ani_native_function {"nativeGetInfoByIntentNameCheck", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnGetInfoByIntentNameCheck)},
        ani_native_function {"nativeGetInsightIntentInfoByFilterCheck", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnGetInsightIntentInfoByFilterCheck)},
        ani_native_function {"nativeGetInsightIntentInfoByFilter", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnGetInsightIntentInfoByFilter)},
        ani_native_function {"nativeQueryEntityInfoCheck", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnQueryEntityInfoCheck)},
        ani_native_function {"nativeQueryEntityInfo", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentDriver::OnQueryEntityInfo)},
    };

    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "bind nativeExecuteSync failed status: %{public}d", status);
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::INTENT, "ANI_Constructor");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGD(AAFwkTag::INTENT, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsInsightIntentDriverInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::INTENT, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS
