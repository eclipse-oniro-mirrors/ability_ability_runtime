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

#include "form_runtime/js_form_extension_context.h"

#include <cinttypes>
#include <cstdint>
#include <charconv>

#include "hilog_tag_wrapper.h"
#include "form_mgr_errors.h"
#include "ipc_skeleton.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_start_options.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "napi_form_util.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
const int UPDATE_FORM_PARAMS_SIZE = 2;

std::map<ConnectionKey, sptr<JSFormExtensionConnection>, key_compare> g_connects;
std::mutex g_connectsMutex_;
int64_t g_serialNumber = 0;

void RemoveConnection(int64_t connectId)
{
    std::lock_guard<std::mutex> lock(g_connectsMutex_);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        TAG_LOGD(AAFwkTag::FORM_EXT, "ability exist");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::FORM_EXT, "ability not exist");
    }
}
class JsFormExtensionContext final {
public:
    explicit JsFormExtensionContext(const std::shared_ptr<FormExtensionContext>& context) : context_(context) {}
    ~JsFormExtensionContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::FORM_EXT, "called");
        std::unique_ptr<JsFormExtensionContext>(static_cast<JsFormExtensionContext*>(data));
    }

    static napi_value UpdateForm(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsFormExtensionContext, OnUpdateForm);
    }

    static napi_value StartAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsFormExtensionContext, OnStartAbility);
    }

    static napi_value ConnectAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsFormExtensionContext, OnConnectAbility);
    }

    static napi_value DisconnectAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsFormExtensionContext, OnDisconnectAbility);
    }

private:
    std::weak_ptr<FormExtensionContext> context_;

    bool CheckCallerIsSystemApp() const
    {
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken);
    }

    napi_value OnUpdateForm(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::FORM_EXT, "called");
        if (info.argc < UPDATE_FORM_PARAMS_SIZE) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "invalid argc");
            return CreateJsUndefined(env);
        }

        std::string strFormId;
        ConvertFromJsValue(env, info.argv[0], strFormId);
        int64_t formId = 0;
        auto res = std::from_chars(strFormId.c_str(), strFormId.c_str() + strFormId.size(), formId);
        if (res.ec != std::errc()) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "from_chars error strFormId:%{public}s", strFormId.c_str());
            formId = -1;
        }

        AppExecFwk::FormProviderData formProviderData;
        std::string formDataStr = "{}";
        if (CheckTypeForNapiValue(env, info.argv[1], napi_object)) {
            napi_value nativeDataValue = nullptr;
            napi_get_named_property(env, info.argv[1], "data", &nativeDataValue);
            if (nativeDataValue == nullptr || !ConvertFromJsValue(env, nativeDataValue, formDataStr)) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "null NativeDataValue or ConvertFromJsValue failed");
            }
        } else {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Not object");
        }

        formProviderData = AppExecFwk::FormProviderData(formDataStr);
        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, formId, formProviderData](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    TAG_LOGW(AAFwkTag::FORM_EXT, "null context");
                    task.Reject(env, CreateJsError(env, 1, "Context is released"));
                    return;
                }
                auto errcode = context->UpdateForm(formId, formProviderData);
                if (errcode == ERR_OK) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsError(env, errcode, "update form failed."));
                }
            };

        napi_value lastParam =
            (info.argc == UPDATE_FORM_PARAMS_SIZE) ? nullptr : info.argv[info.argc - 1];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsFormExtensionContext::OnUpdateForm",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::FORM_EXT, "called");
        // only support one or two params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Not enough params");
            NapiFormUtil::ThrowParamNumError(env, std::to_string(info.argc), "1 or 2");
            return CreateJsUndefined(env);
        }

        decltype(info.argc) unwrapArgc = 0;
        AAFwk::Want want;
        bool unwrapResult = OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
        if (!unwrapResult) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "unwrap want failed");
            NapiFormUtil::ThrowParamTypeError(env, "want", "Want");
            return CreateJsUndefined(env);
        }
        TAG_LOGI(AAFwkTag::FORM_EXT, "Start bundle: %{public}s ability: %{public}s",
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        unwrapArgc++;

        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want](napi_env env, NapiAsyncTask& task, int32_t status) {
                TAG_LOGI(AAFwkTag::FORM_EXT, "startAbility begin");
                auto context = weak.lock();
                if (!context) {
                    TAG_LOGW(AAFwkTag::FORM_EXT, "null context");
                    task.Reject(env, NapiFormUtil::CreateErrorByInternalErrorCode(
                        env, ERR_APPEXECFWK_FORM_COMMON_CODE));
                    return;
                }

                // entry to the core functionality.
                ErrCode innerErrorCode = context->StartAbility(want);
                if (innerErrorCode == ERR_OK) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    TAG_LOGE(AAFwkTag::FORM_EXT, "Start failed: %{public}d", innerErrorCode);
                    task.Reject(env, NapiFormUtil::CreateErrorByInternalErrorCode(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsFormExtensionContext::OnStartAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnConnectAbility(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::FORM_EXT, "called");
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "not system app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        // Check params count
        if (info.argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        // Unwrap want and connection
        AAFwk::Want want;
        sptr<JSFormExtensionConnection> connection = new JSFormExtensionConnection(env);
        if (!AppExecFwk::UnwrapWant(env, info.argv[0], want) ||
            !CheckConnectionParam(env, info.argv[1], connection, want)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int64_t connectId = connection->GetConnectionId();
        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want, connection, connectId](napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    RemoveConnection(connectId);
                    return;
                }
                TAG_LOGD(AAFwkTag::FORM_EXT, "ConnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
                auto innerErrorCode = context->ConnectAbility(want, connection);
                int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
                if (errcode) {
                    connection->CallJsFailed(errcode);
                    RemoveConnection(connectId);
                }
                task.Resolve(env, CreateJsUndefined(env));
            };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSFormExtensionConnection::OnConnectAbility",
            env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
        return CreateJsValue(env, connectId);
    }

    napi_value OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::FORM_EXT, "DisconnectAbility");
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "not system app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int64_t connectId = -1;
        if (!AppExecFwk::UnwrapInt64FromJS2(env, info.argv[INDEX_ZERO], connectId)) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        AAFwk::Want want;
        sptr<JSFormExtensionConnection> connection = nullptr;
        FindConnection(want, connection, connectId);
        // begin disconnect
        NapiAsyncTask::CompleteCallback complete =
            [weak = context_, want, connection](
                napi_env env, NapiAsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    TAG_LOGW(AAFwkTag::FORM_EXT, "null context");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                    return;
                }
                if (connection == nullptr) {
                    TAG_LOGW(AAFwkTag::FORM_EXT, "null connection");
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                    return;
                }
                auto innerErrorCode = context->DisconnectAbility(want, connection);
                if (innerErrorCode == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };

        napi_value lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSFormExtensionConnection::OnDisconnectAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    bool CheckConnectionParam(napi_env env, napi_value value,
        sptr<JSFormExtensionConnection>& connection, AAFwk::Want& want) const
    {
        if (!CheckTypeForNapiValue(env, value, napi_object)) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "get connection object failed");
            return false;
        }
        connection->SetJsConnectionObject(value);
        ConnectionKey key;
        key.id = g_serialNumber;
        key.want = want;
        connection->SetConnectionId(key.id);
        {
            std::lock_guard<std::mutex> lock(g_connectsMutex_);
            g_connects.emplace(key, connection);
        }
        if (g_serialNumber < INT32_MAX) {
            g_serialNumber++;
        } else {
            g_serialNumber = 0;
        }
        TAG_LOGD(AAFwkTag::FORM_EXT, "not find connection");
        return true;
    }

    void FindConnection(AAFwk::Want& want, sptr<JSFormExtensionConnection>& connection, int64_t& connectId) const
    {
        TAG_LOGD(AAFwkTag::FORM_EXT, "connection:%{public}d", static_cast<int32_t>(connectId));
        std::lock_guard<std::mutex> lock(g_connectsMutex_);
        auto item = std::find_if(g_connects.begin(),
            g_connects.end(),
            [&connectId](const auto &obj) {
                return connectId == obj.first.id;
            });
        if (item != g_connects.end()) {
            // match id
            want = item->first.want;
            connection = item->second;
            TAG_LOGD(AAFwkTag::FORM_EXT, "ability not exist");
        }
        return;
    }
};
} // namespace

napi_value CreateJsFormExtensionContext(napi_env env, std::shared_ptr<FormExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "called");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value object = CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JsFormExtensionContext> jsContext = std::make_unique<JsFormExtensionContext>(context);
    napi_wrap(env, object, jsContext.release(), JsFormExtensionContext::Finalizer, nullptr, nullptr);

    std::string type = "FormExtensionContext";
    napi_set_named_property(env, object, "contextType", CreateJsValue(env, type));

    const char *moduleName = "JsFormExtensionContext";
    BindNativeFunction(env, object, "updateForm", moduleName, JsFormExtensionContext::UpdateForm);
    BindNativeFunction(env, object, "startAbility", moduleName, JsFormExtensionContext::StartAbility);
    BindNativeFunction(
        env, object, "connectServiceExtensionAbility", moduleName, JsFormExtensionContext::ConnectAbility);
    BindNativeFunction(env, object, "disconnectServiceExtensionAbility",
        moduleName, JsFormExtensionContext::DisconnectAbility);

    return object;
}

JSFormExtensionConnection::JSFormExtensionConnection(napi_env env) : env_(env) {}

JSFormExtensionConnection::~JSFormExtensionConnection()
{
    if (jsConnectionObject_ == nullptr) {
        return;
    }

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    work->data = reinterpret_cast<void *>(jsConnectionObject_.release());
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work == nullptr) {
            return;
        }
        if (work->data == nullptr) {
            delete work;
            work = nullptr;
            return;
        }
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    }
}

void JSFormExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

int64_t JSFormExtensionConnection::GetConnectionId()
{
    return connectionId_;
}

void JSFormExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "called, resultCode:%{public}d", resultCode);
    wptr<JSFormExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSFormExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "null connectionSptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSFormExtensionConnection::OnAbilityConnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSFormExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "called, resultCode:%{public}d", resultCode);
    HandleScope handleScope(env_);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);

    // wrap RemoteObject
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = {napiElementName, napiRemoteObject};
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null jsConnectionObject");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get object error");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get methodOnConnect");
        return;
    }
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
}

void JSFormExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "called, resultCode:%{public}d", resultCode);
    wptr<JSFormExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSFormExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGI(AAFwkTag::FORM_EXT, "null connectionSptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSFormExtensionConnection::OnAbilityDisconnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSFormExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "called, resultCode:%{public}d", resultCode);
    HandleScope handleScope(env_);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = {napiElementName};
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null jsConnectionObject");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get object fail");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get method");
        return;
    }

    // release connect
    TAG_LOGD(AAFwkTag::FORM_EXT, "size:%{public}zu", g_connects.size());
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    std::lock_guard<std::mutex> lock(g_connectsMutex_);
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [bundleName, abilityName, connectionId = connectionId_](
            const auto &obj) {
            return (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName()) &&
                   connectionId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match bundleName && abilityName
        g_connects.erase(item);
        TAG_LOGD(AAFwkTag::FORM_EXT, "erase size:%{public}zu", g_connects.size());
    }
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}

void JSFormExtensionConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &ref);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

void JSFormExtensionConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

void JSFormExtensionConnection::CallJsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "called");
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null jsConnectionObject");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get object error");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null method");
        return;
    }
    napi_value argv[] = {CreateJsValue(env_, errorCode)};
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    TAG_LOGD(AAFwkTag::FORM_EXT, "CallJsFailed exit");
}
} // namespace AbilityRuntime
} // namespace OHOS
