/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "js_auto_fill_extension_context.h"

#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "js_auto_fill_extension_util.h"
#include "js_error_utils.h"
#include "js_extension_context.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

static std::mutex g_connectsMutex;
static std::map<AutoFillExtensionConnectionKey, sptr<JSAutoFillExtensionConnection>, AutoFillKeyCompare> g_connects;
static int64_t g_serialNumber = 0;

bool CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Non-system app forbidden to call");
        return false;
    }
    return true;
}

void RemoveConnection(int64_t connectId)
{
    std::lock_guard guard(g_connectsMutex);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
        [connectId](const auto &obj) { return connectId == obj.first.id; });
    if (item != g_connects.end()) {
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    }
}

bool CheckDisconnectAbilityParam(napi_env env, napi_value value,
    AAFwk::Want &want, sptr<JSAutoFillExtensionConnection> &connection)
{
    int64_t connectId = -1;
    if (!AppExecFwk::UnwrapInt64FromJS2(env, value, connectId)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Invalid connectId");
        ThrowInvalidParamError(env, "Parse param connectId failed, connectId must be number.");
        return false;
    }
    std::lock_guard guard(g_connectsMutex);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
        [connectId](const auto &obj) { return connectId == obj.first.id; });
    if (item != g_connects.end()) {
        want = item->first.want;
        connection = item->second;
    }
    return true;
}

bool CheckConnectionParam(napi_env env, NapiCallbackInfo &info,
    sptr<JSAutoFillExtensionConnection> &connection, AAFwk::Want &want)
{
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "UnwrapWant failed");
        return false;
    }
    if (!CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "get object failed");
        return false;
    }
    connection->SetJsConnectionObject(info.argv[INDEX_ONE]);
    std::lock_guard guard(g_connectsMutex);
    AutoFillExtensionConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    if (g_serialNumber < INT64_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    return true;
}

int32_t ExecuteConnectAbility(const std::weak_ptr<AutoFillExtensionContext> &weak,
    const AAFwk::Want &want, const sptr<JSAutoFillExtensionConnection> &connection)
{
    auto context = weak.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
    }
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "ConnectAbility connection:%{public}d",
        static_cast<int32_t>(connection->GetConnectionId()));
    return context->ConnectAbility(want, connection);
}

int32_t ExecuteDisconnectAbility(const std::weak_ptr<AutoFillExtensionContext> &weak,
    const AAFwk::Want &want, const sptr<JSAutoFillExtensionConnection> &connection)
{
    auto context = weak.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
    }
    if (!connection) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null connection");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "context->DisconnectAbility");
    return context->DisconnectAbility(want, connection);
}

} // namespace

void JsAutoFillExtensionContext::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    std::unique_ptr<JsAutoFillExtensionContext>(static_cast<JsAutoFillExtensionContext*>(data));
}

napi_value JsAutoFillExtensionContext::ReloadInModal(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAutoFillExtensionContext, OnReloadInModal);
}

napi_value JsAutoFillExtensionContext::OnReloadInModal(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "invalid argc");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    HandleEscape handleEscape(env);
    napi_value jsCustomData = GetPropertyValueByPropertyName(env, info.argv[INDEX_ZERO], "data", napi_object);
    CustomData customData;
    if (jsCustomData == nullptr || !AppExecFwk::UnwrapWantParams(env, jsCustomData, customData.data)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Parse custom data failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, customData, ret = retVal, env]() {
        auto context = weak.lock();
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null ret");
            return;
        }
        *ret = context->ReloadInModal(customData);
    };

    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null ret");
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::RELOAD_IN_MODAL_RESULT_NULL)));
            return;
        }
        if (*ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "error %{public}d", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAutoFillExtensionContext::OnReloadInModal",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsAutoFillExtensionContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAutoFillExtensionContext, OnConnectAbility);
}

napi_value JsAutoFillExtensionContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAutoFillExtensionContext, OnDisconnectAbility);
}

napi_value JsAutoFillExtensionContext::OnConnectAbility(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "OnConnectAbility called");
    if (!CheckCallerIsSystemApp()) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    auto connection = sptr<JSAutoFillExtensionConnection>::MakeSptr(env);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null connection");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    if (!CheckConnectionParam(env, info, connection, want)) {
        ThrowInvalidParamError(env,
            "Parse param want or connection failed, want must be Want and connection must be Connection.");
        return CreateJsUndefined(env);
    }
    int64_t connectId = connection->GetConnectionId();
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, connection, innerErrCode]() {
        *innerErrCode = ExecuteConnectAbility(weak, want, connection);
    };
    NapiAsyncTask::CompleteCallback complete =
        [connection, connectId, innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            int32_t errcode = (*innerErrCode == static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)) ?
                *innerErrCode : static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrCode));
            if (errcode != ERR_OK) {
                connection->CallJsFailed(errcode);
                RemoveConnection(connectId);
            }
            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAutoFillExtensionContext::OnConnectAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return CreateJsValue(env, connectId);
}

napi_value JsAutoFillExtensionContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "OnDisconnectAbility called");
    if (!CheckCallerIsSystemApp()) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    sptr<JSAutoFillExtensionConnection> connection = nullptr;
    if (!CheckDisconnectAbilityParam(env, info.argv[INDEX_ZERO], want, connection)) {
        return CreateJsUndefined(env);
    }
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, connection, innerErrCode]() {
        *innerErrCode = ExecuteDisconnectAbility(weak, want, connection);
    };
    NapiAsyncTask::CompleteCallback complete =
        [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                task.Resolve(env, CreateJsUndefined(env));
            } else if (*innerErrCode == static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
            } else if (*innerErrCode == static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER)) {
                task.Reject(env, CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                    GetInnerErrorMsg(AbilityInnerErrorMsg::CONNECTION_NOT_FOUND)));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAutoFillExtensionContext::OnDisconnectAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAutoFillExtensionContext::CreateJsAutoFillExtensionContext(
    napi_env env, const std::shared_ptr<AutoFillExtensionContext> &context)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context != nullptr) {
        abilityInfo = context->GetAbilityInfo();
    }
    HandleEscape handleEscape(env);
    napi_value objValue = CreateJsExtensionContext(env, context, abilityInfo);

    auto jsContext = std::make_unique<JsAutoFillExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsAutoFillExtensionContext";
    BindNativeFunction(env, objValue, "reloadInModal", moduleName, ReloadInModal);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);

    std::string type = "AutoFillExtensionContext";
    napi_set_named_property(env, objValue, "contextType", CreateJsValue(env, type));

    return handleEscape.Escape(objValue);
}

JSAutoFillExtensionConnection::JSAutoFillExtensionConnection(napi_env env) : env_(env) {}

JSAutoFillExtensionConnection::~JSAutoFillExtensionConnection()
{
    ReleaseNativeReference(jsConnectionObject_.release());
}

void JSAutoFillExtensionConnection::ReleaseNativeReference(NativeReference *ref)
{
    if (ref == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null ref");
        return;
    }
    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null loop");
        delete ref;
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null work");
        delete ref;
        return;
    }
    work->data = reinterpret_cast<void *>(ref);
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            if (work == nullptr) {
                TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null work");
                return;
            }
            if (work->data == nullptr) {
                TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null data");
                delete work;
                work = nullptr;
                return;
            }
            NativeReference *refPtr = reinterpret_cast<NativeReference *>(work->data);
            delete refPtr;
            refPtr = nullptr;
            delete work;
            work = nullptr;
        });
    if (ret != 0) {
        delete ref;
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

void JSAutoFillExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

void JSAutoFillExtensionConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref value = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &value);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(value));
}

void JSAutoFillExtensionConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

void JSAutoFillExtensionConnection::CallJsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "CallJsFailed enter");
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null jsConnectionObject_");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "get object wrong");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null method");
        return;
    }
    napi_value argv[] = { CreateJsValue(env_, errorCode) };
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "CallJsFailed end");
}

void JSAutoFillExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "resultCode:%{public}d", resultCode);
    wptr<JSAutoFillExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<NapiAsyncTask::CompleteCallback>(
            [connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
                sptr<JSAutoFillExtensionConnection> connectionSptr = connection.promote();
                if (!connectionSptr) {
                    TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null connectionSptr");
                    return;
                }
                connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
            });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAutoFillExtensionConnection::OnAbilityConnectDone", env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAutoFillExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "resultCode:%{public}d", resultCode);
    wptr<JSAutoFillExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<NapiAsyncTask::CompleteCallback>(
            [connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
                sptr<JSAutoFillExtensionConnection> connectionSptr = connection.promote();
                if (!connectionSptr) {
                    TAG_LOGI(AAFwkTag::AUTOFILL_EXT, "null connectionSptr");
                    return;
                }
                connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
            });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSAutoFillExtensionConnection::OnAbilityDisconnectDone", env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSAutoFillExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "start, resultCode:%{public}d", resultCode);
    HandleScope handleScope(env_);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = {napiElementName, napiRemoteObject};
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null jsConnectionObject_");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "get object failed");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null methodOnConnect");
        return;
    }
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Call onConnect");
    napi_status status = napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "call js func failed %{public}d", status);
    }
}

void JSAutoFillExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "resultCode:%{public}d", resultCode);
    HandleScope handleScope(env_);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = {napiElementName};
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null jsConnectionObject_");
        RemoveConnection(connectionId_);
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "get object failed");
        RemoveConnection(connectionId_);
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    RemoveConnection(connectionId_);
    if (!CheckTypeForNapiValue(env_, method, napi_function)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "invalid onDisconnect");
        return;
    }

    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Call onDisconnect");
    napi_status status = napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "call js func failed %{public}d", status);
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
