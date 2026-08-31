/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "js_ui_service_extension_context.h"

#include <chrono>
#include <cstdint>
#include "js_service_extension_context.h"
#include "ability_manager_client.h"
#include "ability_runtime/js_caller_complex.h"
#include "hilog_tag_wrapper.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_callback.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "napi_common_start_options.h"
#include "start_options.h"
#include "hitrace_meter.h"
#include "js_free_install_observer.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr int32_t ERROR_CODE_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int32_t INVALID_PARAM = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM);

static std::mutex g_connectsMutex;
static std::map<ConnectionKey, sptr<JSUIServiceExtensionConnection>, key_compare> g_connects;
static int64_t g_serialNumber = 0;

class JSUIServiceExtensionContext final {
public:
    explicit JSUIServiceExtensionContext(
        const std::shared_ptr<UIServiceExtensionContext>& context) : context_(context) {}
    ~JSUIServiceExtensionContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "JsAbilityContext::Finalizer is called");
        std::unique_ptr<JSUIServiceExtensionContext>(static_cast<JSUIServiceExtensionContext*>(data));
    }

    static napi_value StartAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JSUIServiceExtensionContext, OnStartAbility);
    }

    static napi_value TerminateSelf(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JSUIServiceExtensionContext, OnTerminateSelf);
    }

    static napi_value StartAbilityByType(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JSUIServiceExtensionContext, OnStartAbilityByType);
    }

    static napi_value ConnectServiceExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JSUIServiceExtensionContext, OnConnectServiceExtensionAbility);
    }

    static napi_value DisConnectServiceExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JSUIServiceExtensionContext, OnDisConnectServiceExtensionAbility);
    }

private:
    std::weak_ptr<UIServiceExtensionContext> context_;

#ifdef SUPPORT_SCREEN
    void InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions, napi_env &env, NapiCallbackInfo& info)
    {
        auto context = context_.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
            return;
        }

        auto window = context->GetWindow();
        if (window == nullptr) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "null window");
            return;
        }

        int32_t displayId = 0;
        if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)
            && AppExecFwk::UnwrapInt32ByPropertyName(env, info.argv[1], "displayId", displayId)) {
            TAG_LOGI(AAFwkTag::UISERVC_EXT, "startOption displayId %{public}d", startOptions.GetDisplayID());
            return;
        }

        TAG_LOGI(AAFwkTag::UISERVC_EXT, "window displayId %{public}" PRIu64, window->GetDisplayId());
        startOptions.SetDisplayID(window->GetDisplayId());
    }
#endif

    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info)
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "Call");
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        size_t unwrapArgc = 0;
        AAFwk::Want want;
        AAFwk::StartOptions startOptions;
        if (!CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc)) {
            ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
            return CreateJsUndefined(env);
        }
#ifdef SUPPORT_SCREEN
        InitDisplayId(want, startOptions, env, info);
#endif
        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, startOptions, unwrapArgc, innerErrCode]() {
            TAG_LOGD(AAFwkTag::UI_EXT, "JSUIServiceExtensionContext OnStartAbility");
            auto context = weak.lock();
            if (!context) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null context");
                *innerErrCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
                return;
            }
            *innerErrCode = context->StartAbility(want, startOptions);
        };
        NapiAsyncTask::CompleteCallback complete =
        [innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else if (*innerErrCode == static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIServiceExtensionContext::OnStartAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

    bool CheckStartAbilityInputParam(napi_env env, NapiCallbackInfo& info,
        AAFwk::Want& want, AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const
    {
        if (info.argc < ARGC_ONE) {
            return false;
        }
        unwrapArgc = ARGC_ZERO;
        // Check input want
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            return false;
        }
        ++unwrapArgc;
        if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)) {
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnStartAbility start options is used.");
            AppExecFwk::UnwrapStartOptions(env, info.argv[1], startOptions);
            unwrapArgc++;
        }
        return true;
    }

    napi_value OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "Call");
        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [weak = context_, innerErrCode]() {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::UISERVC_EXT, "null context");
                *innerErrCode = static_cast<int>(ERROR_CODE_ONE);
                return;
            }
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "JSUIServiceExtensionContext OnTerminateSelf");
            *innerErrCode= context->TerminateSelf();
        };
        NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                task.Resolve(env, CreateJsUndefined(env));
            } else if (*innerErrCode == ERROR_CODE_ONE) {
                task.Reject(env, CreateJsError(env, *innerErrCode, "Context is released"));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };
        napi_value lastParam = nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSUIServiceExtensionContext::OnTerminateSelf",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }

    napi_value OnStartAbilityByType(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "Call");
        if (info.argc < ARGC_THREE) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string type;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type)) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "parse type failed");
            ThrowError(env, INVALID_PARAM, "Incorrect parameter types, param type must be a string");
            return CreateJsUndefined(env);
        }

        AAFwk::WantParams wantParam;
        if (!AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ONE], wantParam)) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "parse wantParam failed");
            ThrowError(env, INVALID_PARAM, "Parameter error. The type of \"WantParams\" must be array");
            return CreateJsUndefined(env);
        }
        std::shared_ptr<JsUIExtensionCallback> callback = std::make_shared<JsUIExtensionCallback>(env);
        callback->SetJsCallbackObject(info.argv[INDEX_TWO]);
        napi_value completionHandler = AppExecFwk::GetPropertyValueByPropertyName(
            env, info.argv[INDEX_TWO], "completionHandler", napi_object);
        if (completionHandler != nullptr) {
            callback->SetCompletionHandler(env, completionHandler);
        }
        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [weak = context_, type, wantParam, callback, innerErrCode]() mutable {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::UISERVC_EXT, "null context");
                *innerErrCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
                return;
            }
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "JSUIServiceExtensionContext OnStartAbilityByType");
            *innerErrCode = context->StartAbilityByType(type, wantParam, callback);
        };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
                HandleScope handleScope(env);
                if (*innerErrCode == ERR_OK) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else if (*innerErrCode == static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)) {
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
                }
            };

        napi_value lastParam = nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSUIServiceExtensionContext::OnStartAbilityByType",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }

    bool CheckConnectionParam(napi_env env, napi_value value,
        sptr<JSUIServiceExtensionConnection>& connection, AAFwk::Want& want, int32_t accountId = -1) const
    {
        if (!CheckTypeForNapiValue(env, value, napi_object)) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "get object failed");
            return false;
        }

        if (connection == nullptr) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "null connection");
            return false;
        }
        connection->SetJsConnectionObject(value);
        ConnectionKey key;
        {
            std::lock_guard guard(g_connectsMutex);
            key.id = g_serialNumber;
            key.want = want;
            key.accountId = accountId;
            connection->SetConnectionId(key.id);
            g_connects.emplace(key, connection);
            if (g_serialNumber < INT32_MAX) {
                g_serialNumber++;
            } else {
                g_serialNumber = 0;
            }
        }
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Unable to find connection, make new one");
        return true;
    }

    NapiAsyncTask::ExecuteCallback GetConnectAbilityExecFunc(const AAFwk::Want &want,
        sptr<JSUIServiceExtensionConnection> connection, int64_t connectId, std::shared_ptr<int> innerErrorCode)
    {
        return [weak = context_, want, connection, connectId, innerErrorCode]() {
            if (innerErrorCode == nullptr) {
                TAG_LOGE(AAFwkTag::UISERVC_EXT, "null innerErrorCode");
                return;
            }

            auto context = weak.lock();
            if (!context) {
                TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
                *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
                return;
            }

            *innerErrorCode = context->ConnectServiceExtensionAbility(want, connection);
        };
    }

    void FindConnection(AAFwk::Want& want, sptr<JSUIServiceExtensionConnection>& connection, int64_t& connectId,
        int32_t &accountId) const
    {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "connection:%{public}d", static_cast<int32_t>(connectId));
        std::lock_guard guard(g_connectsMutex);
        auto item = std::find_if(g_connects.begin(),
            g_connects.end(),
            [&connectId](const auto &obj) {
                return connectId == obj.first.id;
            });
        if (item != g_connects.end()) {
            // match id
            want = item->first.want;
            connection = item->second;
            accountId = item->first.accountId;
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "find conn ability exist");
        }
        return;
    }

    void RemoveConnection(int64_t connectId)
    {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "enter");
        std::lock_guard guard(g_connectsMutex);
        auto item = std::find_if(g_connects.begin(), g_connects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
        if (item != g_connects.end()) {
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "remove conn ability exist.");
            if (item->second) {
                item->second->RemoveConnectionObject();
            }
            g_connects.erase(item);
        } else {
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "remove conn ability not exist.");
        }
    }

    napi_value OnConnectServiceExtensionAbility(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Connect ServiceExtensionAbility called.");
        // Check params count
        if (info.argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        // Unwrap want and connection
        AAFwk::Want want;
        sptr<JSUIServiceExtensionConnection> connection = new JSUIServiceExtensionConnection(env);
        if (!AppExecFwk::UnwrapWant(env, info.argv[0], want)) {
            ThrowInvalidParamError(env, "Parse param want failed, must be a Want.");
            return CreateJsUndefined(env);
        }
        if (!CheckConnectionParam(env, info.argv[1], connection, want)) {
            ThrowInvalidParamError(env, "Parse param options failed, must be a ConnectOptions.");
            return CreateJsUndefined(env);
        }
        int64_t connectId = connection->GetConnectionId();
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        auto execute = GetConnectAbilityExecFunc(want, connection, connectId, innerErrorCode);
        NapiAsyncTask::CompleteCallback complete = [this, connection, connectId, innerErrorCode](napi_env env,
            NapiAsyncTask& task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrorCode == 0) {
                TAG_LOGI(AAFwkTag::UISERVC_EXT, "connect ability success");
                task.ResolveWithNoError(env, CreateJsUndefined(env));
                return;
            }

            TAG_LOGE(AAFwkTag::UISERVC_EXT, "connect ability failed");
            int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(*innerErrorCode));
            if (errcode) {
                connection->CallJsFailed(errcode);
                this->RemoveConnection(connectId);
            }
        };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSUIServiceExtensionContext::OnConnectServiceExtensionAbility",
            env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return CreateJsValue(env, connectId);
    }

    napi_value OnDisConnectServiceExtensionAbility(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "DisConnect ServiceExtensionAbility start");
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int64_t connectId = -1;
        if (!AppExecFwk::UnwrapInt64FromJS2(env, info.argv[INDEX_ZERO], connectId)) {
            ThrowInvalidParamError(env, "Parse param connection failed, must be a number.");
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        sptr<JSUIServiceExtensionConnection> connection = nullptr;
        int32_t accountId = -1;
        FindConnection(want, connection, connectId, accountId);
        // begin disconnect
        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, connection, accountId, innerErrCode]() {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::UISERVC_EXT, "null context");
                *innerErrCode = ERROR_CODE_ONE;
                return;
            }
            if (!connection) {
                TAG_LOGW(AAFwkTag::UISERVC_EXT, "null connection");
                *innerErrCode = ERROR_CODE_TWO;
                return;
            }
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "context->DisconnectServiceExtensionAbility");
            *innerErrCode = context->DisConnectServiceExtensionAbility(want, connection, accountId);
        };
        NapiAsyncTask::CompleteCallback complete = [innerErrCode](
            napi_env env, NapiAsyncTask& task, int32_t status) {
                HandleScope handleScope(env);
                if (*innerErrCode == ERROR_CODE_ONE) {
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                    return;
                }
                if (*innerErrCode == ERROR_CODE_TWO) {
                    task.Reject(env, CreateJsError(env, ERROR_CODE_TWO, "not found connection"));
                    return;
                }
                if (*innerErrCode == ERR_OK) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
                }
            };
        napi_value lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSUIServiceExtensionContext::OnDisConnectServiceExtensionAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }
};
} // namespace

napi_value CreateJsUIServiceExtensionContext(napi_env env, std::shared_ptr<UIServiceExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Call");
    HandleEscape handleEscape(env);
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value object = CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JSUIServiceExtensionContext> jsUIContext =
        std::make_unique<JSUIServiceExtensionContext>(context);
    napi_wrap(env, object, jsUIContext.release(), JSUIServiceExtensionContext::Finalizer, nullptr, nullptr);

    std::string type = "UIServiceExtensionContext";
    napi_set_named_property(env, object, "contextType", CreateJsValue(env, type));

    const char *moduleName = "JsUIServiceExtensionContext";
    BindNativeFunction(env, object, "startAbility", moduleName, JSUIServiceExtensionContext::StartAbility);
    BindNativeFunction(env, object, "terminateSelf", moduleName, JSUIServiceExtensionContext::TerminateSelf);
    BindNativeFunction(env, object, "startAbilityByType", moduleName,
        JSUIServiceExtensionContext::StartAbilityByType);
    BindNativeFunction(env, object, "connectServiceExtensionAbility", moduleName,
        JSUIServiceExtensionContext::ConnectServiceExtensionAbility);
    BindNativeFunction(env, object, "disconnectServiceExtensionAbility", moduleName,
        JSUIServiceExtensionContext::DisConnectServiceExtensionAbility);
    return handleEscape.Escape(object);
}

JSUIServiceExtensionConnection::JSUIServiceExtensionConnection(napi_env env) : env_(env) {}

JSUIServiceExtensionConnection::~JSUIServiceExtensionConnection()
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

void JSUIServiceExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

int64_t JSUIServiceExtensionConnection::GetConnectionId()
{
    return connectionId_;
}

void JSUIServiceExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    wptr<JSUIServiceExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIServiceExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGE(AAFwkTag::UISERVC_EXT, "null connectionSptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSUIServiceExtensionConnection::OnAbilityConnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIServiceExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "resultCode:%{public}d", resultCode);
    HandleScope handleScope(env_);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);

    // wrap RemoteObject
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = {napiElementName, napiRemoteObject};
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null jsConnectionObject_");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get object error");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null methodOnConnect");
        return;
    }
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
}

void JSUIServiceExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    wptr<JSUIServiceExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIServiceExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGI(AAFwkTag::UISERVC_EXT, "null connectionSptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSUIServiceExtensionConnection::OnAbilityDisconnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIServiceExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "HandleOnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    HandleScope handleScope(env_);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = {napiElementName};
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null jsConnectionObject_");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get object error");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null method");
        return;
    }

    // release connect
    {
        std::lock_guard guard(g_connectsMutex);
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone g_connects.size:%{public}zu", g_connects.size());
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        auto item = std::find_if(g_connects.begin(),
            g_connects.end(),
            [bundleName, abilityName, connectionId = connectionId_](
                const auto &obj) {
                return (bundleName == obj.first.want.GetBundle()) &&
                    (abilityName == obj.first.want.GetElement().GetAbilityName()) &&
                    connectionId == obj.first.id;
            });
        if (item != g_connects.end()) {
            // match bundlename && abilityname
            if (item->second) {
                item->second->RemoveConnectionObject();
            }
            g_connects.erase(item);
            TAG_LOGD(
                AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone erase g_connects.size:%{public}zu", g_connects.size());
        }
    }
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}

void JSUIServiceExtensionConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &ref);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

void JSUIServiceExtensionConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

void JSUIServiceExtensionConnection::CallJsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "CallJsFailed begin");
    HandleScope handleScope(env_);
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null jsConnectionObject_");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get object wrong");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null method");
        return;
    }
    napi_value argv[] = {CreateJsValue(env_, errorCode)};
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "CallJsFailed end");
}
} // namespace AbilityRuntime
}  // namespace OHOS
