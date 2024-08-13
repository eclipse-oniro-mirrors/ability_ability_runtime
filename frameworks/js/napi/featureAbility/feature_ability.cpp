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
#include "feature_ability.h"

#include <cstring>
#include <uv.h>
#include <vector>

#include "napi_common_ability.h"
#include "js_napi_common_ability.h"
#include "ability_process.h"
#include "element_name.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_runtime_utils.h"
#ifdef SUPPORT_SCREEN
#include "js_window.h"
#endif
#include "napi_common_util.h"
#include "napi_context.h"
#include "napi_data_ability_helper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityRuntime;
static int64_t dummyRequestCode_ = 0;
CallbackInfo g_aceCallbackInfo;

const int PARA_SIZE_IS_ONE = 1;
const int PARA_SIZE_IS_TWO = 2;

napi_value FeatureAbilityInit(napi_env env, napi_value exports)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("finishWithResult", NAPI_SetResult),
        DECLARE_NAPI_FUNCTION("getAppType", NAPI_GetAppType),
        DECLARE_NAPI_FUNCTION("getAbilityName", NAPI_GetAbilityName),
        DECLARE_NAPI_FUNCTION("getAbilityInfo", NAPI_GetAbilityInfo),
        DECLARE_NAPI_FUNCTION("getHapModuleInfo", NAPI_GetHapModuleInfo),
        DECLARE_NAPI_FUNCTION("getDataAbilityHelper", NAPI_GetDataAbilityHelper),
        DECLARE_NAPI_FUNCTION("acquireDataAbilityHelper", NAPI_AcquireDataAbilityHelper),
        DECLARE_NAPI_FUNCTION("continueAbility", NAPI_FAContinueAbility),
        DECLARE_NAPI_FUNCTION("getWantSync", NAPI_GetWantSync),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return JsFeatureAbilityInit(env, exports);
}

class JsFeatureAbility : public JsNapiCommon {
public:
    JsFeatureAbility() = default;
    virtual ~JsFeatureAbility() override = default;

    Ability* GetAbility(napi_env env);
    static void Finalizer(napi_env env, void *data, void *hint);
    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value HasWindowFocus(napi_env env, napi_callback_info info);
    static napi_value ConnectAbility(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbility(napi_env env, napi_callback_info info);
    static napi_value GetWant(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value GetContext(napi_env env, napi_callback_info info);
    static napi_value FinishWithResult(napi_env env, napi_callback_info info);
    static napi_value TerminateAbility(napi_env env, napi_callback_info info);
    static napi_value GetWindow(napi_env env, napi_callback_info info);
    std::shared_ptr<NativeReference> GetFAContext();
    void SetFAContext(std::shared_ptr<NativeReference> context);
private:
    napi_value OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnFinishWithResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetWindow(napi_env env, napi_callback_info info);
#ifdef SUPPORT_SCREEN
    napi_value OnHasWindowFocus(napi_env env, const NapiCallbackInfo& info);
#endif
    std::shared_ptr<NativeReference> context_;
};

void JsFeatureAbility::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    std::unique_ptr<JsFeatureAbility>(static_cast<JsFeatureAbility*>(data));
}

napi_value JsFeatureAbilityInit(napi_env env, napi_value exports)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (env == nullptr || exports == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "Invalid input parameters");
        return exports;
    }

    if (!AppExecFwk::CheckTypeForNapiValue(env, exports, napi_object)) {
        TAG_LOGE(AAFwkTag::FA, "null object");
        return exports;
    }

    std::unique_ptr<JsFeatureAbility> jsFeatureAbility = std::make_unique<JsFeatureAbility>();
    jsFeatureAbility->ability_ = jsFeatureAbility->GetAbility(env);
    napi_value contextValue = CreateNapiJSContext(env);
    if (contextValue != nullptr) {
        napi_ref contextRef = nullptr;
        napi_create_reference(env, contextValue, 1, &contextRef);
        jsFeatureAbility->SetFAContext(
            std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(contextRef)));
    }
    napi_wrap(env, exports, jsFeatureAbility.release(), JsFeatureAbility::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsFeatureAbility";
    BindNativeFunction(env, exports, "startAbility", moduleName, JsFeatureAbility::StartAbility);
    BindNativeFunction(env, exports, "getWant", moduleName, JsFeatureAbility::GetWant);
    BindNativeFunction(env, exports, "hasWindowFocus", moduleName, JsFeatureAbility::HasWindowFocus);
    BindNativeFunction(env, exports, "connectAbility", moduleName, JsFeatureAbility::ConnectAbility);
    BindNativeFunction(env, exports, "disconnectAbility", moduleName, JsFeatureAbility::DisconnectAbility);
    BindNativeFunction(env, exports, "startAbilityForResult", moduleName, JsFeatureAbility::StartAbilityForResult);
    BindNativeFunction(env, exports, "getContext", moduleName, JsFeatureAbility::GetContext);
    BindNativeFunction(env, exports, "getWindow", moduleName, JsFeatureAbility::GetWindow);
    BindNativeFunction(env, exports, "terminateSelfWithResult", moduleName, JsFeatureAbility::FinishWithResult);
    BindNativeFunction(env, exports, "terminateSelf", moduleName, JsFeatureAbility::TerminateAbility);
    return exports;
}

void JsFeatureAbility::SetFAContext(std::shared_ptr<NativeReference> context)
{
    context_ = context;
}

std::shared_ptr<NativeReference> JsFeatureAbility::GetFAContext()
{
    return context_;
}

napi_value JsFeatureAbility::StartAbility(napi_env env, napi_callback_info info)
{
    JsFeatureAbility *me = CheckParamsAndGetThis<JsFeatureAbility>(env, info);
    return (me != nullptr) ? me->JsStartAbility(env, info, AbilityType::PAGE) : nullptr;
}

napi_value JsFeatureAbility::GetWant(napi_env env, napi_callback_info info)
{
    JsFeatureAbility *me = CheckParamsAndGetThis<JsFeatureAbility>(env, info);
    return (me != nullptr) ? me->JsGetWant(env, info, AbilityType::PAGE) : nullptr;
}

napi_value JsFeatureAbility::HasWindowFocus(napi_env env, napi_callback_info info)
{
#ifdef SUPPORT_SCREEN
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnHasWindowFocus);
#else
    return nullptr;
#endif
}

napi_value JsFeatureAbility::ConnectAbility(napi_env env, napi_callback_info info)
{
    JsFeatureAbility *me = CheckParamsAndGetThis<JsFeatureAbility>(env, info);
    return (me != nullptr) ? me->JsConnectAbility(env, info, AbilityType::PAGE) : nullptr;
}

napi_value JsFeatureAbility::DisconnectAbility(napi_env env, napi_callback_info info)
{
    JsFeatureAbility *me = CheckParamsAndGetThis<JsFeatureAbility>(env, info);
    return (me != nullptr) ? me->JsDisConnectAbility(env, info, AbilityType::PAGE) : nullptr;
}

napi_value JsFeatureAbility::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnStartAbilityForResult);
}

napi_value JsFeatureAbility::GetContext(napi_env env, napi_callback_info info)
{
    JsFeatureAbility *me = CheckParamsAndGetThis<JsFeatureAbility>(env, info);
    if (me != nullptr) {
        std::shared_ptr<NativeReference> contextObj = me->GetFAContext();
        if (contextObj != nullptr) {
            return contextObj->GetNapiValue();
        }
        napi_value contextValue = me->JsGetContext(env, info, AbilityType::PAGE);
        if (contextValue != nullptr) {
            napi_ref contextRef = nullptr;
            napi_create_reference(env, contextValue, 1, &contextRef);
            me->SetFAContext(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(contextRef)));
            return contextValue;
        }
    }
    return nullptr;
}

napi_value JsFeatureAbility::FinishWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, OnFinishWithResult);
}

napi_value JsFeatureAbility::TerminateAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFeatureAbility, JsTerminateAbility);
}

#ifdef SUPPORT_SCREEN
napi_value JsFeatureAbility::OnHasWindowFocus(napi_env env, const NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (info.argc > ARGS_ONE || info.argc < ARGS_ZERO) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return CreateJsUndefined(env);
    }
    NapiAsyncTask::CompleteCallback complete =
        [obj = this](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (obj->ability_ == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "HasWindowFocus execute error, the ability is nullptr");
                task.Reject(env, CreateJsError(env, NAPI_ERR_ACE_ABILITY, "HasWindowFocus failed"));
                return;
            }
            auto ret = obj->ability_->HasWindowFocus();
            task.Resolve(env, CreateJsValue(env, ret));
        };
    napi_value result = nullptr;
    napi_value lastParam = (info.argc == ARGS_ZERO) ? nullptr : info.argv[PARAM0];
    NapiAsyncTask::ScheduleHighQos("JSFeatureAbility::OnHasWindowFocus",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    TAG_LOGD(AAFwkTag::FA, "end");
    return result;
}
#endif

Ability* JsFeatureAbility::GetAbility(napi_env env)
{
    napi_status ret;
    napi_value global = nullptr;
    const napi_extended_error_info *errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::FA, "get_global=%{public}d err:%{public}s", ret, errorInfo->error_message);
        return nullptr;
    }

    napi_value abilityObj = nullptr;
    ret = napi_get_named_property(env, global, "ability", &abilityObj);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::FA, "get_named_property=%{public}d err:%{public}s", ret, errorInfo->error_message);
        return nullptr;
    }

    Ability *ability = nullptr;
    ret = napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability));
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::FA, "get_value_external=%{public}d err:%{public}s", ret, errorInfo->error_message);
        return nullptr;
    }

    return ability;
}

napi_value JsFeatureAbility::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (info.argc < ARGS_ONE || info.argc > ARGS_TWO) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return CreateJsUndefined(env);
    }

    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ability");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<CallAbilityParam> abilityParam = std::make_shared<CallAbilityParam>();
    std::shared_ptr<CallbackInfo> startAbilityCallback = std::make_shared<CallbackInfo>();
    startAbilityCallback->env = env;

    if (UnwrapForResultParam(*abilityParam, env, info.argv[0]) == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "unwrapForResultParam failed");
        startAbilityCallback->errCode = NAPI_ERR_PARAM_INVALID;
    }

    napi_value result = nullptr;
    napi_value lastParam = (info.argc == ARGS_TWO) ? info.argv[ARGS_ONE] : nullptr;
    startAbilityCallback->napiAsyncTask =
        AbilityRuntime::CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result).release();

    AbilityProcess::GetInstance()->AddAbilityResultCallback(ability_,
        *abilityParam, startAbilityCallback->errCode, *startAbilityCallback);

    if (startAbilityCallback->errCode == NAPI_ERR_NO_ERROR) {
        startAbilityCallback->errCode = AbilityProcess::GetInstance()->StartAbility(ability_,
            *abilityParam, *startAbilityCallback);
    }

    if (startAbilityCallback->errCode != NAPI_ERR_NO_ERROR) {
        // Callback the errcode when StartAbilityForResult failed.
        Want resultData;
        AbilityProcess::GetInstance()->OnAbilityResult(ability_, abilityParam->requestCode, 0, resultData);
    }

    return result;
}

napi_value JsFeatureAbility::OnFinishWithResult(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (info.argc > ARGS_TWO || info.argc < ARGS_ONE) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return CreateJsUndefined(env);
    }

    if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::FA, "param not object");
        return CreateJsUndefined(env);
    }

    CallAbilityParam param;
    napi_value jsRequestCode = nullptr;
    napi_get_named_property(env, info.argv[0], "resultCode", &jsRequestCode);
    if (!AppExecFwk::IsTypeForNapiValue(env, jsRequestCode, napi_number)) {
        TAG_LOGE(AAFwkTag::FA, "resultCode type failed");
        return CreateJsUndefined(env);
    }
    if (!ConvertFromJsValue(env, jsRequestCode, param.requestCode)) {
        TAG_LOGE(AAFwkTag::FA, "convert resultCode failed");
        return CreateJsUndefined(env);
    }
    bool hasWant = false;
    napi_has_named_property(env, info.argv[0], "want", &hasWant);
    if (hasWant) {
        napi_value jsWant = nullptr;
        napi_get_named_property(env, info.argv[0], "want", &jsWant);
        if (!AppExecFwk::IsTypeForNapiValue(env, jsWant, napi_object)) {
            TAG_LOGE(AAFwkTag::FA, "want type failed");
            return CreateJsUndefined(env);
        }
        if (!UnwrapWant(env, jsWant, param.want)) {
            TAG_LOGE(AAFwkTag::FA, "unwrapWant failed");
            return CreateJsUndefined(env);
        }
    }

    NapiAsyncTask::CompleteCallback complete = [obj = this, param](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (obj->ability_ != nullptr) {
            obj->ability_->SetResult(param.requestCode, param.want);
            obj->ability_->TerminateAbility();
        } else {
            TAG_LOGE(AAFwkTag::FA, "null ability");
        }
        task.Resolve(env, CreateJsNull(env));
    };
    napi_value result = nullptr;
    napi_value lastParam = (info.argc >= ARGS_TWO) ? info.argv[ARGS_ONE] : nullptr;
    NapiAsyncTask::ScheduleHighQos("JSFeatureAbility::OnFinishWithResult",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

#ifdef SUPPORT_SCREEN
napi_value JsFeatureAbility::GetWindow(napi_env env, napi_callback_info info)
{
    if (env == nullptr || info == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null %{public}s", ((env == nullptr) ? "env" : "info"));
        return nullptr;
    }

    auto object = CheckParamsAndGetThis<JsFeatureAbility>(env, info);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null obj");
        return nullptr;
    }

    return object->OnGetWindow(env, info);
}

napi_value JsFeatureAbility::OnGetWindow(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::FA, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto complete = [obj = this] (napi_env env, NapiAsyncTask& task, int32_t status) {
        if (obj->ability_ == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null ability");
            task.Resolve(env, CreateJsNull(env));
            return;
        }
        auto window = obj->ability_->GetWindow();
        task.Resolve(env, OHOS::Rosen::CreateJsWindowObject(env, window));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsFeatureAbility::OnGetWindow",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));

    return result;
}
#else

napi_value JsFeatureAbility::GetWindow(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsFeatureAbility::OnGetWindow(napi_env env, napi_callback_info info)
{
    return nullptr;
}
#endif

napi_value NAPI_SetResult(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
        return WrapVoidToJS(env);
    }

    napi_value ret = SetResultWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return ret;
}

napi_value SetResultWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = 2;
    const size_t argcPromise = 1;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    CallAbilityParam param;
    if (UnwrapAbilityResult(param, env, args[0]) == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "call unwrapWant failed");
        return nullptr;
    }
    asyncCallbackInfo->param = param;

    if (argcAsync > argcPromise) {
        ret = SetResultAsync(env, args, 1, asyncCallbackInfo);
    } else {
        ret = SetResultPromise(env, asyncCallbackInfo);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return ret;
}

napi_value CreateAsyncWork(napi_env env, napi_value &resourceName, AsyncCallbackInfo *asyncCallbackInfo)
{
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName,
    [](napi_env env, void *data) {
        TAG_LOGI(AAFwkTag::FA, "worker pool thread");
        AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
        if (asyncCallbackInfo == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
            return;
        }

        if (asyncCallbackInfo->ability != nullptr) {
            asyncCallbackInfo->ability->SetResult(
                asyncCallbackInfo->param.requestCode, asyncCallbackInfo->param.want);
            asyncCallbackInfo->ability->TerminateAbility();
        } else {
            TAG_LOGE(AAFwkTag::FA, "null ability");
        }
        TAG_LOGI(AAFwkTag::FA, "worker pool thread execute exit");
    },
    [](napi_env env, napi_status status, void *data) {
        TAG_LOGI(AAFwkTag::FA, "complete");
        AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
        if (asyncCallbackInfo == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
            return;
        }
        napi_value result[ARGS_TWO] = {nullptr};
        napi_value callback = nullptr;
        napi_value undefined = nullptr;
        napi_value callResult = nullptr;
        napi_get_undefined(env, &undefined);
        result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
        napi_get_null(env, &result[PARAM1]);
        napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
        napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

        if (asyncCallbackInfo->cbInfo.callback != nullptr) {
            TAG_LOGD(AAFwkTag::FA, "napi_delete_reference");
            napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
        }
        napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
        delete asyncCallbackInfo;
        TAG_LOGI(AAFwkTag::FA, "complete end");
    },
    static_cast<void *>(asyncCallbackInfo),
    &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value SetResultAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }

    return CreateAsyncWork(env, resourceName, asyncCallbackInfo);
}

napi_value SetResultPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::FA, "promise");
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void *data) {
            TAG_LOGI(AAFwkTag::FA, "worker pool thread execute");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            if (asyncCallbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
                return;
            }

            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->SetResult(
                    asyncCallbackInfo->param.requestCode, asyncCallbackInfo->param.want);
                asyncCallbackInfo->ability->TerminateAbility();
            } else {
                TAG_LOGE(AAFwkTag::FA, "ability == nullptr");
            }
            TAG_LOGI(AAFwkTag::FA, "execute end");
        },
        [](napi_env env, napi_status status, void *data) {
            TAG_LOGI(AAFwkTag::FA, "complete called");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            if (asyncCallbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
                return;
            }
            napi_value result = nullptr;
            napi_get_null(env, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            TAG_LOGI(AAFwkTag::FA, "complete end");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

EXTERN_C_START
int CreateUVQueueWork(uv_loop_t *loop, uv_work_t *work)
{
    int rev = uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            // JS Thread
            if (work == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null work");
                return;
            }
            auto onAbilityCB = static_cast<OnAbilityCallback *>(work->data);
            if (onAbilityCB == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null onAbilityCB");
                delete work;
                work = nullptr;
                return;
            }

            if (onAbilityCB->cb.errCode != ERR_OK) {
                int32_t errCode = GetStartAbilityErrorCode(onAbilityCB->cb.errCode);
                onAbilityCB->cb.napiAsyncTask->Reject(onAbilityCB->cb.env,
                    CreateJsError(onAbilityCB->cb.env, errCode, "StartAbilityForResult Error"));
                delete onAbilityCB->cb.napiAsyncTask;
                onAbilityCB->cb.napiAsyncTask = nullptr;
                delete onAbilityCB;
                onAbilityCB = nullptr;
                delete work;
                work = nullptr;
                return;
            }

            napi_value objValue = nullptr;
            napi_create_object(onAbilityCB->cb.env, &objValue);

            napi_set_named_property(onAbilityCB->cb.env,
                objValue, "resultCode", CreateJsValue(onAbilityCB->cb.env, onAbilityCB->resultCode));
            napi_set_named_property(onAbilityCB->cb.env,
                objValue, "want", CreateJsWant(onAbilityCB->cb.env, onAbilityCB->resultData));

            onAbilityCB->cb.napiAsyncTask->Resolve(onAbilityCB->cb.env, objValue);
            delete onAbilityCB->cb.napiAsyncTask;
            onAbilityCB->cb.napiAsyncTask = nullptr;
            delete onAbilityCB;
            onAbilityCB = nullptr;
            delete work;
            work = nullptr;
            TAG_LOGI(AAFwkTag::FA, "uv_queue_work end");
        });
    return rev;
}

void CallOnAbilityResult(int requestCode, int resultCode, const Want &resultData, CallbackInfo callbackInfo)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (callbackInfo.env == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null cb.env");
        return;
    }

    if (callbackInfo.napiAsyncTask == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null cb.asyncTask");
        return;
    }

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo.env, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null loop");
        return;
    }

    auto work = new uv_work_t;
    auto onAbilityCB = new (std::nothrow) OnAbilityCallback;
    if (onAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "Failed to allocate OnAbilityCallback");
        delete work;
        return;
    }

    onAbilityCB->requestCode = requestCode;
    onAbilityCB->resultCode = resultCode;
    onAbilityCB->resultData = resultData;
    onAbilityCB->cb = callbackInfo;

    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null work");
        return;
    }
    work->data = static_cast<void *>(onAbilityCB);

    int rev = CreateUVQueueWork(loop, work);
    if (rev != 0) {
        if (onAbilityCB != nullptr) {
            delete onAbilityCB;
            onAbilityCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}
EXTERN_C_END

bool InnerUnwrapWant(napi_env env, napi_value args, Want &want)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, args, &valueType), false);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::FA, "wrong argument type");
        return false;
    }

    napi_value jsWant = GetPropertyValueByPropertyName(env, args, "want", napi_object);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null jsWant");
        return false;
    }

    return UnwrapWant(env, jsWant, want);
}

napi_value UnwrapForResultParam(CallAbilityParam &param, napi_env env, napi_value args)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    // dummy requestCode for NativeC++ interface and onabilityresult callback
    param.requestCode = dummyRequestCode_;
    param.forResultOption = true;
    dummyRequestCode_ = (dummyRequestCode_ < INT32_MAX) ? (dummyRequestCode_ + 1) : 0;
    TAG_LOGI(AAFwkTag::FA, "reqCode=%{public}d forResultOption=%{public}d",
        param.requestCode,
        param.forResultOption);

    // unwrap the param : want object
    if (!InnerUnwrapWant(env, args, param.want)) {
        TAG_LOGE(AAFwkTag::FA, "Failed to InnerUnwrapWant");
        return nullptr;
    }

    // unwrap the param : abilityStartSetting (optional)
    napi_value jsSettingObj = GetPropertyValueByPropertyName(env, args, "abilityStartSettings", napi_object);
    if (jsSettingObj == nullptr) {
        jsSettingObj = GetPropertyValueByPropertyName(env, args, "abilityStartSetting", napi_object);
    }
    if (jsSettingObj != nullptr) {
        param.setting = AbilityStartSetting::GetEmptySetting();
        if (!UnwrapAbilityStartSetting(env, jsSettingObj, *(param.setting))) {
            TAG_LOGE(AAFwkTag::FA, "unwrap abilityStartSetting failed");
        }
        TAG_LOGI(AAFwkTag::FA, "abilityStartSetting");
    }

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value UnwrapAbilityResult(CallAbilityParam &param, napi_env env, napi_value args)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    // unwrap the param
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "param type mismatch!");
    // get resultCode property
    napi_value property = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, args, "resultCode", &property));
    NAPI_CALL(env, napi_typeof(env, property, &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "property type mismatch!");
    NAPI_CALL(env, napi_get_value_int32(env, property, &param.requestCode));
    TAG_LOGI(AAFwkTag::FA, "requestCode=%{public}d", param.requestCode);

    // unwrap the param : want object
    InnerUnwrapWant(env, args, param.want);

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

/**
 * @brief GetWantSyncWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetWantSyncWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
        return nullptr;
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ability");
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return nullptr;
    }

    std::shared_ptr<AAFwk::Want> ptrWant = asyncCallbackInfo->ability->GetWant();
    if (ptrWant != nullptr) {
        asyncCallbackInfo->param.want = *ptrWant;
    } else {
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }

    napi_value result = nullptr;
    if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
        result = WrapWant(env, asyncCallbackInfo->param.want);
    } else {
        result = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

/**
 * @brief Get want(Sync).
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWantSync(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetWantSyncWrap(env, info, asyncCallbackInfo);

    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;

    if (ret == nullptr) {
        ret = WrapVoidToJS(env);
        TAG_LOGE(AAFwkTag::FA, "null ret");
    } else {
        TAG_LOGI(AAFwkTag::FA, "exit");
    }
    return ret;
}

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppType(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    return NAPI_GetAppTypeCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityName(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    return NAPI_GetAbilityNameCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityInfo(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    return NAPI_GetAbilityInfoCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetHapModuleInfo(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    return NAPI_GetHapModuleInfoCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief FeatureAbility NAPI method : getDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetDataAbilityHelper(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DataAbilityHelperCB *dataAbilityHelperCB = new (std::nothrow) DataAbilityHelperCB;
    if (dataAbilityHelperCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelperCB");
        return WrapVoidToJS(env);
    }
    dataAbilityHelperCB->cbBase.cbInfo.env = env;
    napi_value ret = GetDataAbilityHelperWrap(env, info, dataAbilityHelperCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        if (dataAbilityHelperCB != nullptr) {
            delete dataAbilityHelperCB;
            dataAbilityHelperCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return ret;
}

/**
 * @brief getDataAbilityHelper processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetDataAbilityHelperWrap(napi_env env, napi_callback_info info, DataAbilityHelperCB *dataAbilityHelperCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (dataAbilityHelperCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelperCB");
        return nullptr;
    }

    size_t argcAsync = 2;
    const size_t argcPromise = 1;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        NAPI_CALL(env, napi_create_reference(env, args[PARAM0], 1, &dataAbilityHelperCB->uri));
    }

    if (argcAsync > argcPromise) {
        ret = GetDataAbilityHelperAsync(env, args, 1, dataAbilityHelperCB);
    } else {
        ret = GetDataAbilityHelperPromise(env, dataAbilityHelperCB);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return ret;
}

napi_value GetDataAbilityHelperAsync(
    napi_env env, napi_value *args, const size_t argCallback, DataAbilityHelperCB *dataAbilityHelperCB)
{
    TAG_LOGI(AAFwkTag::FA, "asyncCallback");
    if (args == nullptr || dataAbilityHelperCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &dataAbilityHelperCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env, nullptr, resourceName,
            [](napi_env env, void *data) {
                TAG_LOGI(AAFwkTag::FA, "worker pool thread execute");
            },
            GetDataAbilityHelperAsyncCompleteCB,
            static_cast<void *>(dataAbilityHelperCB),
            &dataAbilityHelperCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, dataAbilityHelperCB->cbBase.asyncWork,
        napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value GetDataAbilityHelperPromise(napi_env env, DataAbilityHelperCB *dataAbilityHelperCB)
{
    TAG_LOGI(AAFwkTag::FA, "promise");
    if (dataAbilityHelperCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    dataAbilityHelperCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env, nullptr, resourceName,
            [](napi_env env, void *data) {
                TAG_LOGI(AAFwkTag::FA, "worker pool thread execute");
            },
            GetDataAbilityHelperPromiseCompleteCB,
            static_cast<void *>(dataAbilityHelperCB),
            &dataAbilityHelperCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, dataAbilityHelperCB->cbBase.asyncWork,
        napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void GetDataAbilityHelperAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DataAbilityHelperCB *dataAbilityHelperCB = static_cast<DataAbilityHelperCB *>(data);
    if (dataAbilityHelperCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelperCB");
        return;
    }

    std::unique_ptr<DataAbilityHelperCB> callbackPtr {dataAbilityHelperCB};
    napi_value uri = nullptr;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    napi_get_undefined(env, &undefined);
    napi_get_reference_value(env, dataAbilityHelperCB->uri, &uri);
    napi_get_reference_value(env, dataAbilityHelperCB->cbBase.cbInfo.callback, &callback);
    napi_new_instance(env, GetGlobalDataAbilityHelper(env), 1, &uri, &dataAbilityHelperCB->result);
    if (IsTypeForNapiValue(env, dataAbilityHelperCB->result, napi_object)) {
        result[PARAM1] = dataAbilityHelperCB->result;
    } else {
        TAG_LOGI(AAFwkTag::FA, "helper is nullptr");
        result[PARAM1] = WrapVoidToJS(env);
    }
    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);
    if (dataAbilityHelperCB->cbBase.cbInfo.callback != nullptr) {
        napi_delete_reference(env, dataAbilityHelperCB->cbBase.cbInfo.callback);
    }
    if (dataAbilityHelperCB->uri != nullptr) {
        napi_delete_reference(env, dataAbilityHelperCB->uri);
    }
    napi_delete_async_work(env, dataAbilityHelperCB->cbBase.asyncWork);
    TAG_LOGI(AAFwkTag::FA, "end");
}

void GetDataAbilityHelperPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DataAbilityHelperCB *dataAbilityHelperCB = static_cast<DataAbilityHelperCB *>(data);
    if (dataAbilityHelperCB == nullptr) {
        TAG_LOGI(AAFwkTag::FA, "null dataAbilityHelperCB");
        return;
    }

    napi_value uri = nullptr;
    napi_value result = nullptr;
    napi_get_reference_value(env, dataAbilityHelperCB->uri, &uri);
    napi_new_instance(env, GetGlobalDataAbilityHelper(env), 1, &uri, &dataAbilityHelperCB->result);
    if (IsTypeForNapiValue(env, dataAbilityHelperCB->result, napi_object)) {
        result = dataAbilityHelperCB->result;
        napi_resolve_deferred(env, dataAbilityHelperCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, dataAbilityHelperCB->cbBase.errCode);
        napi_reject_deferred(env, dataAbilityHelperCB->cbBase.deferred, result);
        TAG_LOGI(AAFwkTag::FA, "null helper");
    }

    if (dataAbilityHelperCB->uri != nullptr) {
        napi_delete_reference(env, dataAbilityHelperCB->uri);
    }
    napi_delete_async_work(env, dataAbilityHelperCB->cbBase.asyncWork);
    TAG_LOGI(AAFwkTag::FA, "end");
}

/**
 * @brief FeatureAbility NAPI method : acquireDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_AcquireDataAbilityHelper(napi_env env, napi_callback_info info)
{
    return NAPI_AcquireDataAbilityHelperCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief FeatureAbility NAPI method : continueAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_FAContinueAbility(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
        return WrapVoidToJS(env);
    }

    napi_value ret = ContinueAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return ret;
}

/**
 * @brief ContinueAbilityWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ContinueAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argc = 2;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_valuetype valueType = napi_undefined;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_CALL(env, napi_typeof(env, args[0], &valueType));
    if (valueType != napi_object && valueType != napi_function) {
        TAG_LOGE(AAFwkTag::FA, "wrong argument type. Object or function expected");
        return nullptr;
    }
    if (argc == 0) {
        ret = ContinueAbilityPromise(env, args, asyncCallbackInfo, argc);
    } else if (PARA_SIZE_IS_ONE == argc) {
        if (valueType == napi_function) {
            ret = ContinueAbilityAsync(env, args, asyncCallbackInfo, argc);
        } else {
            ret = ContinueAbilityPromise(env, args, asyncCallbackInfo, argc);
        }
    } else if (PARA_SIZE_IS_TWO == argc) {
        napi_valuetype value = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, args[1], &value));
        if (value != napi_function) {
            TAG_LOGE(AAFwkTag::FA, "function expected");
            return nullptr;
        }
        ret = ContinueAbilityAsync(env, args, asyncCallbackInfo, argc);
    } else {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return ret;
}

void CreateContinueAsyncWork(napi_env env, napi_value &resourceName, AsyncCallbackInfo *asyncCallbackInfo)
{
    napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void *data) {
            TAG_LOGI(AAFwkTag::FA, "worker pool thread execute");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            if (asyncCallbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
                return;
            }

            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->ContinueAbility(asyncCallbackInfo->optionInfo.deviceId);
            } else {
                TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
            }
            TAG_LOGI(AAFwkTag::FA, "worker pool thread execute exit");
        },
        [](napi_env env, napi_status status, void *data) {
            TAG_LOGI(AAFwkTag::FA, "main event thread end");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            if (asyncCallbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
                return;
            }
            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value result[ARGS_TWO] = {nullptr};
            napi_value callResult = nullptr;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_get_null(env, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            TAG_LOGI(AAFwkTag::FA, "end");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
}

napi_value ContinueAbilityAsync(napi_env env, napi_value *args, AsyncCallbackInfo *asyncCallbackInfo, size_t argc)
{
    TAG_LOGI(AAFwkTag::FA, "asyncCallback");
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    if (PARA_SIZE_IS_TWO == argc) {
        // args[0] : ContinueAbilityOptions
        napi_valuetype valueTypeOptions = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, args[0], &valueTypeOptions));
        if (valueTypeOptions != napi_object) {
            TAG_LOGE(AAFwkTag::FA, "object expected");
            return nullptr;
        }
        if (GetContinueAbilityOptionsInfoCommon(env, args[0], asyncCallbackInfo->optionInfo) == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "getContinueAbilityOptionsInfoCommon fail");
            return nullptr;
        }

        // args[1] : callback
        napi_valuetype valueTypeCallBack = napi_undefined;
        napi_typeof(env, args[1], &valueTypeCallBack);
        if (valueTypeCallBack == napi_function) {
            napi_create_reference(env, args[1], 1, &asyncCallbackInfo->cbInfo.callback);
        }
    } else {
        // args[0] : callback
        napi_valuetype valueTypeCallBack = napi_undefined;
        napi_typeof(env, args[1], &valueTypeCallBack);
        if (valueTypeCallBack == napi_function) {
            napi_create_reference(env, args[0], 1, &asyncCallbackInfo->cbInfo.callback);
        }
    }

    CreateContinueAsyncWork(env, resourceName, asyncCallbackInfo);

    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value ContinueAbilityPromise(napi_env env, napi_value *args, AsyncCallbackInfo *asyncCallbackInfo, size_t argc)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
        return nullptr;
    }

    if (argc == PARA_SIZE_IS_ONE) {
        // args[0] : ContinueAbilityOptions
        napi_valuetype valueTypeOptions = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, args[0], &valueTypeOptions));
        if (valueTypeOptions != napi_object) {
            TAG_LOGE(AAFwkTag::FA, "object expected");
            return nullptr;
        }
        if (GetContinueAbilityOptionsInfoCommon(env, args[0], asyncCallbackInfo->optionInfo) == nullptr) {
            return nullptr;
        }
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void *data) {
            TAG_LOGI(AAFwkTag::FA, "execute called");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            if (asyncCallbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
                return;
            }
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->ContinueAbility(asyncCallbackInfo->optionInfo.deviceId);
            } else {
                TAG_LOGE(AAFwkTag::FA, "null ability");
            }
            TAG_LOGI(AAFwkTag::FA, "execute end");
        },
        [](napi_env env, napi_status status, void *data) {
            TAG_LOGI(AAFwkTag::FA, "complete called");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            if (asyncCallbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "null asyncCallbackInfo");
                return;
            }
            napi_value result = nullptr;
            napi_get_null(env, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            TAG_LOGI(AAFwkTag::FA, "complete end");
        },
        static_cast<void *>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}
}  // namespace AppExecFwk
}  // namespace OHOS
