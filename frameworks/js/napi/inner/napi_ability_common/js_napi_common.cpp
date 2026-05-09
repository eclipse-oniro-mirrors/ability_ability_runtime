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

#include "js_napi_common_ability.h"

#include "ability_manager_client.h"
#include "ability_util.h"

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_common_ability.h"
#include "napi_common_ability_wrap_utils.h"
#include "napi_common_util.h"
#include "napi_context.h"
#include "napi_remote_object.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
static std::map<ConnectionKey, sptr<NAPIAbilityConnection>, key_compare> connects_;
static std::mutex g_connectionsLock_;
static int64_t serialNumber_ = 0;

JsNapiCommon::JsNapiCommon() : ability_(nullptr)
{}

JsNapiCommon::~JsNapiCommon()
{
    RemoveAllCallbacksLocked();
}

napi_value JsNapiCommon::HandleJsConnectAbilityError(napi_env env,
    std::shared_ptr<ConnectionCallback> &connectionCallback, const Want &want, int32_t errorVal)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    // return error code in onFailed async callback
    napi_value callback = nullptr;
    napi_value undefinedVal = nullptr;
    napi_value resultVal = nullptr;
    napi_value callResult = nullptr;
    int errorCode = NO_ERROR;
    switch (errorVal) {
        case NAPI_ERR_ACE_ABILITY:
            errorCode = ABILITY_NOT_FOUND;
            break;
        case NAPI_ERR_PARAM_INVALID:
            errorCode = INVALID_PARAMETER;
            break;
        default:
            break;
    }
    NAPI_CALL_BASE(env, napi_create_int32(env, errorCode, &resultVal), CreateJsUndefined(env));
    NAPI_CALL_BASE(env, napi_get_reference_value(env, connectionCallback->failedCallbackRef, &callback),
        CreateJsUndefined(env));
    NAPI_CALL_BASE(env, napi_call_function(env, undefinedVal, callback, ARGS_ONE, &resultVal, &callResult),
        CreateJsUndefined(env));
    connectionCallback->Reset();
    RemoveConnectionLocked(want);
    return resultVal;
}

napi_value JsNapiCommon::OnFindAbilityConnection(napi_env env, sptr<NAPIAbilityConnection> &abilityConnection,
    std::shared_ptr<ConnectionCallback> &connectionCallback, const Want &want, int64_t id)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "callbackSize: %{public}zu",
        abilityConnection->GetCallbackSize());
    // Add callback to connection
    abilityConnection->AddConnectionCallback(connectionCallback);
    // Judge connection-state
    auto connectionState = abilityConnection->GetConnectionState();
    TAG_LOGI(AAFwkTag::JSNAPI, "connectionState=%{public}d", connectionState);
    if (connectionState == CONNECTION_STATE_CONNECTED) {
        abilityConnection->HandleOnAbilityConnectDone(*connectionCallback, ERR_OK);
        return CreateJsValue(env, id);
    } else if (connectionState == CONNECTION_STATE_CONNECTING) {
        return CreateJsValue(env, id);
    } else {
        RemoveConnectionLocked(want);
        return CreateJsUndefined(env);
    }
}

napi_value JsNapiCommon::JsConnectAbility(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGS_TWO) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(g_connectionsLock_);
    Want want;
    if (!UnwrapWant(env, argv[PARAM0], want)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "unwrapWant failed");
        return CreateJsUndefined(env);
    }

    auto connectionCallback = std::make_shared<ConnectionCallback>(env, argv[PARAM1], this);
    bool result = false;
    int32_t errorVal = static_cast<int32_t>(NAPI_ERR_NO_ERROR);
    int64_t id = 0;
    sptr<NAPIAbilityConnection> abilityConnection = nullptr;
    if (CheckAbilityType(abilityType)) {
        abilityConnection = FindConnectionLocked(want, id);
        if (abilityConnection) {
            return OnFindAbilityConnection(env, abilityConnection, connectionCallback, want, id);
        } else {
            result = CreateConnectionAndConnectAbilityLocked(connectionCallback, want, id);
        }
    } else {
        errorVal = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
    }

    if (errorVal != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || result == false) {
        if (HandleJsConnectAbilityError(env, connectionCallback, want, errorVal) == CreateJsUndefined(env)) {
            return CreateJsUndefined(env);
        };
    }
    // free failed callback here, avoid possible multi-threading problems when disconnect success
    napi_delete_reference(env, connectionCallback->failedCallbackRef);
    connectionCallback->failedCallbackRef = nullptr;
    return CreateJsValue(env, id);
}

void JsNapiCommon::SetJsDisConnectAbilityCallback(std::shared_ptr<int32_t> &errorVal, const AbilityType &abilityType,
    sptr<NAPIAbilityConnection> &abilityConnection, NapiAsyncTask::ExecuteCallback &execute,
    NapiAsyncTask::CompleteCallback &complete)
{
    execute = [obj = this, value = errorVal, abilityType, abilityConnection] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        *value = obj->ability_->DisconnectAbility(abilityConnection);
    };
    complete = [obj = this, value = errorVal]
        (napi_env env, NapiAsyncTask &task, const int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR)) {
            task.Reject(env, CreateJsError(env, *value, "DisconnectAbility failed."));
            return;
        }
        task.Resolve(env, CreateJsValue(env, *value));
    };
}

napi_value JsNapiCommon::JsDisConnectAbility(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == ARGS_ZERO || argc > ARGS_TWO) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(g_connectionsLock_);
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    int64_t id = 0;
    sptr<NAPIAbilityConnection> abilityConnection = nullptr;
    if (!ConvertFromJsValue(env, argv[PARAM0], id)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "params error");
        return CreateJsUndefined(env);
    }
    auto item = std::find_if(connects_.begin(), connects_.end(),
        [&id](const std::map<ConnectionKey, sptr<NAPIAbilityConnection>>::value_type &obj) {
            return id == obj.first.id;
        });
    if (item != connects_.end()) {
        abilityConnection = item->second;
    } else {
        TAG_LOGE(AAFwkTag::JSNAPI, "no ability disconnect");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::ExecuteCallback execute;
    NapiAsyncTask::CompleteCallback complete;
    SetJsDisConnectAbilityCallback(errorVal, abilityType, abilityConnection, execute, complete);
    napi_value lastParam = (argc == ARGS_ONE) ? nullptr : argv[PARAM1];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsNapiCommon::JsDisConnectAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

bool JsNapiCommon::CreateConnectionAndConnectAbilityLocked(
    std::shared_ptr<ConnectionCallback> callback, const Want &want, int64_t &id)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "Create new connection");
    // Create connection
    sptr<NAPIAbilityConnection> connection(new (std::nothrow) NAPIAbilityConnection());
    CHECK_POINTER_AND_RETURN_LOG(connection, false, "null connection");
    ConnectionKey key;
    id = serialNumber_;
    key.id = id;
    key.want = want;
    connects_.emplace(key, connection);
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
    }
    // Set callback
    connection->AddConnectionCallback(callback);

    // connectAbility
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        return false;
    }
    connection->SetConnectionState(CONNECTION_STATE_CONNECTING);
    return ability_->ConnectAbility(want, connection);
}

sptr<NAPIAbilityConnection> JsNapiCommon::FindConnectionLocked(const Want &want, int64_t &id)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "bundle/ability:%{public}s/%{public}s", want.GetElement().GetBundleName().c_str(),
        want.GetElement().GetAbilityName().c_str());
    std::string deviceId = want.GetElement().GetDeviceID();
    std::string bundleName = want.GetBundle();
    std::string abilityName = want.GetElement().GetAbilityName();
    auto iter = std::find_if(connects_.begin(),
        connects_.end(), [&deviceId, &bundleName, &abilityName](const std::map<ConnectionKey,
        sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetElement().GetDeviceID()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (iter != connects_.end()) {
        TAG_LOGD(AAFwkTag::JSNAPI, "find connection exist");
        auto connection = iter->second;
        if (connection == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null connection");
            connects_.erase(iter);
            return nullptr;
        }
        id = iter->first.id;
        return connection;
    }
    return nullptr;
}

void JsNapiCommon::RemoveAllCallbacksLocked()
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    std::lock_guard<std::mutex> lock(g_connectionsLock_);
    for (auto it = connects_.begin(); it != connects_.end();) {
        auto connection = it->second;
        if (!connection) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null connection");
            it = connects_.erase(it);
            continue;
        }
        connection->RemoveAllCallbacks(this);
        if (connection->GetCallbackSize() == 0) {
            it = connects_.erase(it);
        } else {
            ++it;
        }
    }
}

void JsNapiCommon::RemoveConnectionLocked(const Want &want)
{
    std::string deviceId = want.GetElement().GetDeviceID();
    std::string bundleName = want.GetBundle();
    std::string abilityName = want.GetElement().GetAbilityName();
    auto iter = std::find_if(connects_.begin(),
        connects_.end(), [&deviceId, &bundleName, &abilityName](const std::map<ConnectionKey,
        sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetElement().GetDeviceID()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    connects_.erase(iter);
}

napi_value JsNapiCommon::JsGetContext(napi_env env, const napi_callback_info info, const AbilityType abilityType)
{
    if (!CheckAbilityType(abilityType)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "ability type error");
        return CreateJsUndefined(env);
    }

    return CreateNapiJSContext(env);
}

napi_value JsNapiCommon::JsGetFilesDir(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsFilesDir> filesDir = std::make_shared<JsFilesDir>();
    auto execute = [obj = this, dir = filesDir, abilityType, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "null abilityContext");
            return;
        }
        dir->name = context->GetFilesDir();
    };
    auto complete = [obj = this, dir = filesDir, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, dir->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetFilesDir",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsNapiCommon::JsIsUpdatingConfigurations(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsConfigurations> config = std::make_shared<JsConfigurations>();
    auto execute = [obj = this, data = config, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        if (data == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "null data");
            return;
        }
        data->status = obj->ability_->IsUpdatingConfigurations();
    };
    auto complete = [obj = this, info = config, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, info->status));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsNapiCommon::JsIsUpdatingConfigurations",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsPrintDrawnCompleted(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsDrawnCompleted> drawComplete = std::make_shared<JsDrawnCompleted>();
    auto execute = [obj = this, data = drawComplete, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        if (data == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "null data");
            return;
        }
        data->status = obj->ability_->PrintDrawnCompleted();
    };
    auto complete = [obj = this, draw = drawComplete, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || draw == nullptr) {
            auto ecode = draw == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsNull(env));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsNapiCommon::JsPrintDrawnCompleted",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetCacheDir(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsCacheDir> cacheDir = std::make_shared<JsCacheDir>();
    auto execute = [obj = this, dir = cacheDir, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "null context");
            return;
        }
        dir->name = context->GetCacheDir();
    };
    auto complete = [obj = this, dir = cacheDir, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "JsCacheDir is null or errorVal is error");
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, dir->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetCacheDir",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetCtxAppType(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsCtxAppType> type = std::make_shared<JsCtxAppType>();
    auto execute = [obj = this, apptype = type, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        if (apptype == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            return;
        }
        apptype->name = obj->ability_->GetAppType();
    };
    auto complete = [obj = this, apptype = type, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || apptype == nullptr) {
            auto ecode = apptype == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, apptype->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetCtxAppType",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetCtxHapModuleInfo(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsHapModuleInfo> infoData = std::make_shared<JsHapModuleInfo>();
    auto execute = [obj = this, hapMod = infoData, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto getInfo = obj->ability_->GetHapModuleInfo();
        if (getInfo != nullptr && hapMod != nullptr) {
            hapMod->hapModInfo = *getInfo;
        } else {
            TAG_LOGE(AAFwkTag::JSNAPI, "GetHapModuleInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "null info or errorVal==0");
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, obj->CreateHapModuleInfo(env, info));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetCtxHapModuleInfo",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetAppVersionInfo(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input arguments count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsApplicationInfo> infoData = std::make_shared<JsApplicationInfo>();
    auto execute = [obj = this, appInfo = infoData, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto getInfo = obj->ability_->GetApplicationInfo();
        if (getInfo != nullptr && appInfo != nullptr) {
            appInfo->appInfo = *getInfo;
        } else {
            TAG_LOGE(AAFwkTag::JSNAPI, "GetApplicationInfo return null");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
            TAG_LOGD(AAFwkTag::JSNAPI, "JsHapModuleInfo is null or errorVal is 0");
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, obj->CreateAppVersionInfo(env, info));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetAppVersionInfo",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetCtxAbilityInfo(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsAbilityInfoInfo> infoData = std::make_shared<JsAbilityInfoInfo>();
    auto execute = [obj = this, abilityInfo = infoData, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto getInfo = obj->ability_->GetAbilityInfo();
        if (getInfo != nullptr && abilityInfo != nullptr) {
            abilityInfo->abilityInfo = *getInfo;
        } else {
            TAG_LOGE(AAFwkTag::JSNAPI, "GetAbilityInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "null info or errorVal==0");
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, obj->CreateAbilityInfo(env, info));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetCtxAbilityInfo",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetOrCreateDistributedDir(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsOrCreateDistributedDir> orCreateDistributedDir = std::make_shared<JsOrCreateDistributedDir>();
    auto execute = [obj = this, dir = orCreateDistributedDir, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null context");
            return;
        }
        dir->name = context->GetDistributedFilesDir();
    };
    auto complete = [obj = this, dir = orCreateDistributedDir, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "errorVal is error or JsCacheDir is null");
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, dir->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsNapiCommon::JsGetOrCreateDistributedDir",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

#ifdef SUPPORT_GRAPHICS
napi_value JsNapiCommon::JsGetDisplayOrientation(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    auto execute = [obj = this, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        *value = obj->ability_->GetDisplayOrientation();
    };
    auto complete = [value = errorVal] (napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGD(AAFwkTag::JSNAPI, "innerCall value=%{public}d", *value);
        if (*value == NAPI_ERR_ACE_ABILITY) {
            task.Reject(env, CreateJsError(env, NAPI_ERR_ACE_ABILITY, "ability is nullptr"));
        } else if (*value == NAPI_ERR_ABILITY_TYPE_INVALID) {
            task.Reject(env, CreateJsError(env, NAPI_ERR_ABILITY_TYPE_INVALID, "ability type is invalid."));
        } else if (*value == NAPI_ERR_NO_WINDOW) {
            task.Reject(env, CreateJsError(env, NAPI_ERR_NO_WINDOW, "window is nullptr"));
        } else {
            task.Resolve(env, CreateJsValue(env, *value));
        }
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetDisplayOrientation",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}
#endif

napi_value JsNapiCommon::CreateProcessInfo(napi_env env, const std::shared_ptr<JsProcessInfo> &processInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    CHECK_POINTER_AND_RETURN_LOG(processInfo, CreateJsUndefined(env), "input params error");

    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    CHECK_POINTER_AND_RETURN_LOG(objContext, CreateJsUndefined(env), "CreateObject failed");

    napi_set_named_property(env, objContext, "processName", CreateJsValue(env, processInfo->processName));
    napi_set_named_property(env, objContext, "pid", CreateJsValue(env, processInfo->pid));

    return objContext;
}

napi_value JsNapiCommon::CreateElementName(napi_env env, const std::shared_ptr<JsElementName> &elementName)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    CHECK_POINTER_AND_RETURN_LOG(elementName, CreateJsUndefined(env), "input params error");

    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    CHECK_POINTER_AND_RETURN_LOG(objContext, CreateJsUndefined(env), "CreateObject failed");

    napi_set_named_property(env, objContext, "deviceId", CreateJsValue(env, elementName->deviceId));
    napi_set_named_property(env, objContext, "bundleName", CreateJsValue(env, elementName->bundleName));
    napi_set_named_property(env, objContext, "abilityName", CreateJsValue(env, elementName->abilityName));
    napi_set_named_property(env, objContext, "uri", CreateJsValue(env, elementName->uri));
    napi_set_named_property(env, objContext, "shortName", CreateJsValue(env, elementName->shortName));

    return objContext;
}

napi_value JsNapiCommon::CreateHapModuleInfo(napi_env env, const std::shared_ptr<JsHapModuleInfo> &hapModInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    CHECK_POINTER_AND_RETURN_LOG(hapModInfo, CreateJsUndefined(env), "input params error");
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    CHECK_POINTER_AND_RETURN_LOG(objContext, CreateJsUndefined(env), "CreateObject failed");

    napi_set_named_property(env, objContext, "name", CreateJsValue(env, hapModInfo->hapModInfo.name));
    napi_set_named_property(env, objContext, "description", CreateJsValue(env, hapModInfo->hapModInfo.description));
    napi_set_named_property(env, objContext, "icon", CreateJsValue(env, hapModInfo->hapModInfo.iconPath));
    napi_set_named_property(env, objContext, "label", CreateJsValue(env, hapModInfo->hapModInfo.label));
    napi_set_named_property(env, objContext, "backgroundImg",
        CreateJsValue(env, hapModInfo->hapModInfo.backgroundImg));
    napi_set_named_property(env, objContext, "moduleName", CreateJsValue(env, hapModInfo->hapModInfo.moduleName));
    napi_set_named_property(env, objContext, "mainAbilityName",
        CreateJsValue(env, hapModInfo->hapModInfo.mainAbility));
    napi_set_named_property(env, objContext, "supportedModes",
        CreateJsValue(env, hapModInfo->hapModInfo.supportedModes));
    napi_set_named_property(env, objContext, "descriptionId",
        CreateJsValue(env, hapModInfo->hapModInfo.descriptionId));
    napi_set_named_property(env, objContext, "labelId", CreateJsValue(env, hapModInfo->hapModInfo.labelId));
    napi_set_named_property(env, objContext, "iconId", CreateJsValue(env, hapModInfo->hapModInfo.iconId));
    napi_set_named_property(env, objContext, "installationFree",
        CreateJsValue(env, hapModInfo->hapModInfo.installationFree));
    napi_set_named_property(env, objContext, "reqCapabilities",
        CreateNativeArray(env, hapModInfo->hapModInfo.reqCapabilities));
    napi_set_named_property(env, objContext, "deviceTypes",
        CreateNativeArray(env, hapModInfo->hapModInfo.deviceTypes));
    napi_set_named_property(env, objContext, "abilityInfo",
        CreateAbilityInfos(env, hapModInfo->hapModInfo.abilityInfos));

    return objContext;
}

napi_value JsNapiCommon::CreateModuleInfo(napi_env env, const ModuleInfo &modInfo)
{
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    if (objContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateObject error");
        return CreateJsUndefined(env);
    }
    if (!CheckTypeForNapiValue(env, objContext, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "ConvertNativeValueTo object error");
        return CreateJsUndefined(env);
    }

    napi_set_named_property(env, objContext, "moduleName", CreateJsValue(env, modInfo.moduleName));
    napi_set_named_property(env, objContext, "moduleSourceDir", CreateJsValue(env, modInfo.moduleSourceDir));

    return objContext;
}

napi_value JsNapiCommon::CreateModuleInfos(napi_env env, const std::vector<ModuleInfo> &moduleInfos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, moduleInfos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateArray failed");
        return CreateJsUndefined(env);
    }
    for (uint32_t i = 0; i < moduleInfos.size(); i++) {
        napi_set_element(env, arrayValue, i, CreateModuleInfo(env, moduleInfos.at(i)));
    }

    return arrayValue;
}

napi_value JsNapiCommon::CreateAppInfo(napi_env env, const ApplicationInfo &appInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    if (objContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateObject error");
        return CreateJsUndefined(env);
    }
    if (!CheckTypeForNapiValue(env, objContext, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "objContext not object");
        return CreateJsUndefined(env);
    }

    napi_set_named_property(env, objContext, "name", CreateJsValue(env, appInfo.name));
    napi_set_named_property(env, objContext, "description", CreateJsValue(env, appInfo.description));
    napi_set_named_property(env, objContext, "descriptionId", CreateJsValue(env, appInfo.descriptionId));
    napi_set_named_property(env, objContext, "systemApp", CreateJsValue(env, appInfo.isSystemApp));
    napi_set_named_property(env, objContext, "enabled", CreateJsValue(env, appInfo.enabled));
    napi_set_named_property(env, objContext, "label", CreateJsValue(env, appInfo.label));
    napi_set_named_property(env, objContext, "labelId", CreateJsValue(env, std::to_string(appInfo.labelId)));
    napi_set_named_property(env, objContext, "icon", CreateJsValue(env, appInfo.iconPath));
    napi_set_named_property(env, objContext, "iconId", CreateJsValue(env, std::to_string(appInfo.iconId)));
    napi_set_named_property(env, objContext, "process", CreateJsValue(env, appInfo.process));
    napi_set_named_property(env, objContext, "entryDir", CreateJsValue(env, appInfo.entryDir));
    napi_set_named_property(env, objContext, "supportedModes", CreateJsValue(env, appInfo.supportedModes));
    napi_set_named_property(env, objContext, "moduleSourceDirs", CreateNativeArray(env, appInfo.moduleSourceDirs));
    napi_set_named_property(env, objContext, "permissions", CreateNativeArray(env, appInfo.permissions));
    napi_set_named_property(env, objContext, "moduleInfos", CreateModuleInfos(env, appInfo.moduleInfos));

    return objContext;
}

napi_value JsNapiCommon::CreateAppInfo(napi_env env, const std::shared_ptr<JsApplicationInfo> &appInfo)
{
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input param error");
        return CreateJsUndefined(env);
    }

    return CreateAppInfo(env, appInfo->appInfo);
}

napi_value JsNapiCommon::CreateAbilityInfo(napi_env env, const AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    if (objContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null objContext");
        return CreateJsUndefined(env);
    }
    if (!CheckTypeForNapiValue(env, objContext, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "objContext not object");
        return CreateJsUndefined(env);
    }

    napi_set_named_property(env, objContext, "bundleName", CreateJsValue(env, abilityInfo.bundleName));
    napi_set_named_property(env, objContext, "name", CreateJsValue(env, abilityInfo.name));
    napi_set_named_property(env, objContext, "label", CreateJsValue(env, abilityInfo.label));
    napi_set_named_property(env, objContext, "description", CreateJsValue(env, abilityInfo.description));
    napi_set_named_property(env, objContext, "icon", CreateJsValue(env, abilityInfo.iconPath));
    napi_set_named_property(env, objContext, "moduleName", CreateJsValue(env, abilityInfo.moduleName));
    napi_set_named_property(env, objContext, "process", CreateJsValue(env, abilityInfo.process));
    napi_set_named_property(env, objContext, "uri", CreateJsValue(env, abilityInfo.uri));
    napi_set_named_property(env, objContext, "readPermission", CreateJsValue(env, abilityInfo.readPermission));
    napi_set_named_property(env, objContext, "writePermission", CreateJsValue(env, abilityInfo.writePermission));
    napi_set_named_property(env, objContext, "targetAbility", CreateJsValue(env, abilityInfo.targetAbility));
    napi_set_named_property(env, objContext, "type", CreateJsValue(env, static_cast<int32_t>(abilityInfo.type)));
    napi_set_named_property(env, objContext, "orientation",
        CreateJsValue(env, static_cast<int32_t>(abilityInfo.orientation)));
    napi_set_named_property(env, objContext, "launchMode",
        CreateJsValue(env, static_cast<int32_t>(abilityInfo.launchMode)));
    napi_set_named_property(env, objContext, "labelId", CreateJsValue(env, abilityInfo.labelId));
    napi_set_named_property(env, objContext, "descriptionId", CreateJsValue(env, abilityInfo.descriptionId));
    napi_set_named_property(env, objContext, "iconId", CreateJsValue(env, abilityInfo.iconId));
    napi_set_named_property(env, objContext, "formEntity", CreateJsValue(env, abilityInfo.formEntity));
    napi_set_named_property(env, objContext, "minFormHeight", CreateJsValue(env, abilityInfo.minFormHeight));
    napi_set_named_property(env, objContext, "defaultFormHeight", CreateJsValue(env, abilityInfo.defaultFormHeight));
    napi_set_named_property(env, objContext, "minFormWidth", CreateJsValue(env, abilityInfo.minFormWidth));
    napi_set_named_property(env, objContext, "defaultFormWidth", CreateJsValue(env, abilityInfo.defaultFormWidth));
    napi_set_named_property(env, objContext, "backgroundModes", CreateJsValue(env, abilityInfo.backgroundModes));
    napi_set_named_property(env, objContext, "subType", CreateJsValue(env, static_cast<int32_t>(abilityInfo.subType)));
    napi_set_named_property(env, objContext, "isVisible", CreateJsValue(env, abilityInfo.visible));
    napi_set_named_property(env, objContext, "formEnabled", CreateJsValue(env, abilityInfo.formEnabled));
    napi_set_named_property(env, objContext, "permissions", CreateNativeArray(env, abilityInfo.permissions));
    napi_set_named_property(env, objContext, "deviceCapabilities",
        CreateNativeArray(env, abilityInfo.deviceCapabilities));
    napi_set_named_property(env, objContext, "deviceTypes", CreateNativeArray(env, abilityInfo.deviceTypes));
    napi_set_named_property(env, objContext, "applicationInfo", CreateAppInfo(env, abilityInfo.applicationInfo));

    return objContext;
}

napi_value JsNapiCommon::CreateAbilityInfo(
    napi_env env, const std::shared_ptr<JsAbilityInfoInfo> &abilityInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null abilityInfo");
        return CreateJsUndefined(env);
    }

    return CreateAbilityInfo(env, abilityInfo->abilityInfo);
}

napi_value JsNapiCommon::CreateAbilityInfos(napi_env env, const std::vector<AbilityInfo> &abilityInfos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, abilityInfos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateArray failed");
        return CreateJsUndefined(env);
    }
    for (uint32_t i = 0; i < abilityInfos.size(); i++) {
        napi_set_element(env, arrayValue, i, CreateAbilityInfo(env, abilityInfos.at(i)));
    }

    return arrayValue;
}

bool JsNapiCommon::CheckAbilityType(const AbilityType typeWant)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "params error");
        return false;
    }
    const std::shared_ptr<AbilityInfo> info = ability_->GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "get ability info error");
        return false;
    }

    switch (typeWant) {
        case AbilityType::PAGE:
            if (static_cast<AbilityType>(info->type) == AbilityType::PAGE ||
                static_cast<AbilityType>(info->type) == AbilityType::DATA) {
                return true;
            }
            return false;
        default:
            return static_cast<AbilityType>(info->type) != AbilityType::PAGE;
    }
    return false;
}

napi_value JsNapiCommon::CreateAppVersionInfo(napi_env env, const std::shared_ptr<JsApplicationInfo> &appInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "CreateAppVersionInfo called");
    CHECK_POINTER_AND_RETURN_LOG(appInfo, CreateJsUndefined(env), "input params error");
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    CHECK_POINTER_AND_RETURN_LOG(objContext, CreateJsUndefined(env), "CreateObject failed");

    napi_set_named_property(env, objContext, "appName", CreateJsValue(env, appInfo->appInfo.name));
    napi_set_named_property(env, objContext, "versionName", CreateJsValue(env, appInfo->appInfo.versionName));
    napi_set_named_property(env, objContext, "versionCode",
        CreateJsValue(env, static_cast<int32_t>(appInfo->appInfo.versionCode)));

    return objContext;
}

bool JsNapiCommon::UnwrapVerifyPermissionParams(napi_env env, napi_callback_info info, JsPermissionOptions &options)
{
    bool flagCall = true;
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == ARGS_ONE) {
        flagCall = false;
    } else if (argc == ARGS_TWO && !AppExecFwk::IsTypeForNapiValue(env, argv[PARAM1], napi_function)) {
        if (!GetPermissionOptions(env, argv[PARAM1], options)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "argc==2 invalid param");
        }
        flagCall = false;
    } else if (argc == ARGS_THREE) {
        if (!GetPermissionOptions(env, argv[PARAM1], options)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "argc==3 invalid param");
        }
    }

    return flagCall;
}

bool JsNapiCommon::GetStringsValue(napi_env env, napi_value object, std::vector<std::string> &strList)
{
    bool isArray = false;
    napi_is_array(env, object, &isArray);
    if (object == nullptr || !isArray) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params error");
        return false;
    }
    uint32_t length = 0;
    napi_get_array_length(env, object, &length);
    for (uint32_t i = 0; i < length; i++) {
        std::string itemStr("");
        napi_value elementVal = nullptr;
        napi_get_element(env, object, i, &elementVal);
        if (!ConvertFromJsValue(env, elementVal, itemStr)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "GetElement from to array [%{public}u] error", i);
            return false;
        }
        strList.push_back(itemStr);
    }

    return true;
}

bool JsNapiCommon::GetPermissionOptions(napi_env env, napi_value object, JsPermissionOptions &options)
{
    if (object == nullptr || !CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params error");
        return false;
    }
    napi_value uidValue = nullptr;
    napi_get_named_property(env, object, "uid", &uidValue);
    napi_value pidValue = nullptr;
    napi_get_named_property(env, object, "pid", &pidValue);
    options.uidFlag = ConvertFromJsValue(env, uidValue, options.uid);
    options.pidFlag = ConvertFromJsValue(env, pidValue, options.pid);

    return true;
}

std::string JsNapiCommon::ConvertErrorCode(int32_t errCode)
{
    static std::map<int32_t, std::string> errMap = {
        { static_cast<int32_t>(NAPI_ERR_ACE_ABILITY), std::string("get ability error") },
        { static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID), std::string("ability call error") },
        { static_cast<int32_t>(NAPI_ERR_PARAM_INVALID), std::string("input param error") },
        { static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID), std::string("ability type error") }
    };
    auto findECode = errMap.find(errCode);
    if (findECode == errMap.end()) {
        TAG_LOGE(AAFwkTag::JSNAPI, "convert error code failed");
        return std::string("execution failed");
    }

    return findECode->second;
}

napi_value JsNapiCommon::JsGetWant(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<JsWant> pwant = std::make_shared<JsWant>();
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    auto execute = [obj = this, want = pwant, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "abilityType error");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }

        auto wantData = obj->ability_->GetWant();
        if (wantData == nullptr || want == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null wantData or want");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            return;
        }
        want->want = *wantData;
    };

    auto complete = [obj = this, value = errorVal, pwant]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value == NAPI_ERR_NO_ERROR && pwant != nullptr) {
            task.Resolve(env, obj->CreateWant(env, pwant));
        } else {
            auto error = (pwant == nullptr) ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, error, "GetAbilityInfo return nullptr"));
        }
    };

    auto callback = (argc == ARGS_ZERO) ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetWant",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsNapiCommon::CreateWant(napi_env env, const std::shared_ptr<JsWant> &want)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null want");
        return CreateJsUndefined(env);
    }

    return CreateJsWant(env, want->want);
}

napi_value JsNapiCommon::JsTerminateAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    if (info.argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", info.argc);
        return CreateJsUndefined(env);
    }

    auto complete = [obj = this](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (obj->ability_ != nullptr) {
            obj->ability_->TerminateAbility();
        } else {
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        }
        task.Resolve(env, CreateJsNull(env));
    };

    auto callback = (info.argc == ARGS_ZERO) ? nullptr : info.argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsTerminateAbility",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

/**
 * @brief Parse the parameters.
 *
 * @param param Indicates the parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
bool UnwrapParamForWant(napi_env env, napi_value args, AbilityType, CallAbilityParam &param)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    bool ret = false;
    napi_valuetype valueType = napi_undefined;
    param.setting = nullptr;
    NAPI_CALL_BASE(env, napi_typeof(env, args, &valueType), false);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong argument type");
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);

    napi_value jsWant = GetPropertyValueByPropertyName(env, args, "want", napi_object);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null jsWant");
        return false;
    }

    ret = UnwrapWant(env, jsWant, param.want);

    napi_value jsSettingObj = GetPropertyValueByPropertyName(env, args, "abilityStartSettings", napi_object);
    if (jsSettingObj == nullptr) {
        jsSettingObj = GetPropertyValueByPropertyName(env, args, "abilityStartSetting", napi_object);
    }
    if (jsSettingObj != nullptr) {
        param.setting = AbilityStartSetting::GetEmptySetting();
        if (!UnwrapAbilityStartSetting(env, jsSettingObj, *(param.setting))) {
            TAG_LOGE(AAFwkTag::JSNAPI, "unwrap abilityStartSetting failed");
        }
        TAG_LOGI(AAFwkTag::JSNAPI, "abilityStartSetting");
    }

    TAG_LOGI(AAFwkTag::JSNAPI, "end");
    return ret;
}

void JsNapiCommon::SetJsStartAbilityExecuteCallback(std::shared_ptr<int32_t> &errorVal, AbilityType &abilityType,
    std::shared_ptr<CallAbilityParam> &param, NapiAsyncTask::ExecuteCallback &execute)
{
    execute = [obj = this, value = errorVal, abilityType, paramObj = param, &observer = freeInstallObserver_] () {
        if (*value != NAPI_ERR_NO_ERROR) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid param");
            return;
        }

        if (obj->ability_ == nullptr) {
            *value = NAPI_ERR_ACE_ABILITY;
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            return;
        }

        if (!obj->CheckAbilityType(abilityType)) {
            *value = NAPI_ERR_ABILITY_TYPE_INVALID;
            TAG_LOGE(AAFwkTag::JSNAPI, "abilityType error");
            return;
        }
#ifdef SUPPORT_SCREEN
        // inherit split mode
        auto windowMode = obj->ability_->GetCurrentWindowMode();
        if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
            windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY ||
            windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SPLIT) {
            paramObj->want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
        }
        TAG_LOGD(AAFwkTag::JSNAPI, "window mode is %{public}d", windowMode);

        // follow orientation
        paramObj->want.SetParam("ohos.aafwk.Orientation", 0);
        if (windowMode != AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING) {
            auto orientation = obj->ability_->GetDisplayOrientation();
            paramObj->want.SetParam("ohos.aafwk.Orientation", orientation);
            TAG_LOGD(AAFwkTag::JSNAPI, "display orientation is %{public}d", orientation);
        }
#endif
        if (paramObj->setting == nullptr) {
            TAG_LOGI(AAFwkTag::JSNAPI, "null setting");
            *value = obj->ability_->StartAbility(paramObj->want);
        } else {
            TAG_LOGI(AAFwkTag::JSNAPI, "null setting");
            *value = obj->ability_->StartAbility(paramObj->want, *(paramObj->setting));
        }
        if ((paramObj->want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
            *value != 0 && observer != nullptr) {
            std::string bundleName = paramObj->want.GetElement().GetBundleName();
            std::string abilityName = paramObj->want.GetElement().GetAbilityName();
            std::string startTime = paramObj->want.GetStringParam(Want::PARAM_RESV_START_TIME);
            observer->OnInstallFinished(bundleName, abilityName, startTime, *value);
        }
    };
}

napi_value JsNapiCommon::JsStartAbility(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    auto param = std::make_shared<CallAbilityParam>();
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == 0 || argc > ARGS_TWO) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        *errorVal = NAPI_ERR_PARAM_INVALID;
    } else {
        if (!UnwrapParamForWant(env, argv[PARAM0], abilityType, *param)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "unwrapParamForWant failed");
            *errorVal = NAPI_ERR_PARAM_INVALID;
        }
    }

    if ((param->want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        param->want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    }
    NapiAsyncTask::ExecuteCallback execute;
    SetJsStartAbilityExecuteCallback(errorVal, abilityType, param, execute);
    auto complete = [value = errorVal]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*value != NAPI_ERR_NO_ERROR) {
            int32_t errCode = GetStartAbilityErrorCode(*value);
            task.Reject(env, CreateJsError(env, errCode, "StartAbility Failed"));
            return;
        }
        task.Resolve(env, CreateJsValue(env, *value));
    };

    auto callback = (argc == ARGS_ONE) ? nullptr : argv[PARAM1];
    napi_value result = nullptr;
    if ((param->want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        AddFreeInstallObserver(env, param->want, callback, &result);
        NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsStartAbility", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, nullptr));
    } else {
        NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsStartAbility", env,
            CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    }

    return result;
}

napi_value JsNapiCommon::JsGetExternalCacheDir(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc:%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto complete = [obj = this, abilityType](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (obj->ability_ == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
            task.RejectWithCustomize(
                env,
                CreateJsError(env, NAPI_ERR_ACE_ABILITY, "JsGetExternalCacheDir Failed"),
                CreateJsNull(env));
            return;
        }

        if (!obj->CheckAbilityType(abilityType)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "abilityType error");
            task.Reject(env, CreateJsError(env, NAPI_ERR_ABILITY_TYPE_INVALID, "JsGetExternalCacheDir Failed"));
            return;
        }

        std::string result = obj->ability_->GetExternalCacheDir();
        task.Resolve(env, CreateJsValue(env, result));
    };

    auto callback = (argc == ARGS_ZERO) ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetExternalCacheDir",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

void JsNapiCommon::AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback,
    napi_value* result)
{
    // adapter free install async return install and start result
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        return;
    }
    int ret = 0;
    if (freeInstallObserver_ == nullptr) {
        freeInstallObserver_ = new JsFreeInstallObserver(env);
        ret = ability_->AddFreeInstallObserver(freeInstallObserver_);
    }

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "add observer failed");
    } else {
        TAG_LOGD(AAFwkTag::JSNAPI, "called");
        // build a callback observer with last param
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
        freeInstallObserver_->AddJsObserverObject(bundleName, abilityName, startTime, callback, result);
    }
}

void ClearCallbackWork(uv_work_t* req, int)
{
    std::unique_ptr<uv_work_t> work(req);
    if (!req) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null work");
        return;
    }
    std::unique_ptr<ConnectionCallback> callback(reinterpret_cast<ConnectionCallback*>(req->data));
    if (!callback) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null data");
        return;
    }
    callback->Reset();
}

void ConnectionCallback::Reset()
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (engine == nullptr) {
        removeKey = nullptr;
        return;
    }
    if (pthread_self() == engine->GetTid()) {
        TAG_LOGD(AAFwkTag::JSNAPI, "in-js-thread");
        if (connectCallbackRef) {
            napi_delete_reference(env, connectCallbackRef);
            connectCallbackRef = nullptr;
        }
        if (disconnectCallbackRef) {
            napi_delete_reference(env, disconnectCallbackRef);
            disconnectCallbackRef = nullptr;
        }
        if (failedCallbackRef) {
            napi_delete_reference(env, failedCallbackRef);
            failedCallbackRef = nullptr;
        }
        env = nullptr;
        removeKey = nullptr;
        return;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "not in-js-thread");
    auto loop = engine->GetUVLoop();
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null loop");
        env = nullptr;
        removeKey = nullptr;
        return;
    }
    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null work");
        return;
    }
    ConnectionCallback *data = new(std::nothrow) ConnectionCallback(std::move(*this));
    if (data == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null data");
        delete work;
        return;
    }
    work->data = data;
    auto ret = uv_queue_work(loop, work, [](uv_work_t*) {}, ClearCallbackWork);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::JSNAPI, "uv_queue_work failed: %{public}d", ret);
        data->env = nullptr;
        data->removeKey = nullptr;
        delete data;
        delete work;
    }
}

void NAPIAbilityConnection::AddConnectionCallback(std::shared_ptr<ConnectionCallback> callback)
{
    std::lock_guard<std::mutex> guard(lock_);
    callbacks_.emplace_back(callback);
}

int NAPIAbilityConnection::GetConnectionState() const
{
    std::lock_guard<std::mutex> guard(lock_);
    return connectionState_;
}

void NAPIAbilityConnection::SetConnectionState(int connectionState)
{
    std::lock_guard<std::mutex> guard(lock_);
    connectionState_ = connectionState;
}

size_t NAPIAbilityConnection::GetCallbackSize()
{
    std::lock_guard<std::mutex> guard(lock_);
    return callbacks_.size();
}

size_t NAPIAbilityConnection::RemoveAllCallbacks(ConnectRemoveKeyType key)
{
    size_t result = 0;
    std::lock_guard<std::mutex> guard(lock_);
    for (auto it = callbacks_.begin(); it != callbacks_.end();) {
        auto callback = *it;
        if (callback && callback->removeKey == key) {
            it = callbacks_.erase(it);
            result++;
        } else {
            ++it;
        }
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "removed size:%{public}zu, left size:%{public}zu", result,
             callbacks_.size());
    return result;
}

void UvWorkOnAbilityConnectDone(uv_work_t *work, int status)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    std::unique_ptr<uv_work_t> managedWork(work);
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null work");
        return;
    }
    // JS Thread
    std::unique_ptr<ConnectAbilityCB> connectAbilityCB(static_cast<ConnectAbilityCB *>(work->data));
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null connectAbilityCB");
        return;
    }
    CallbackInfo &cbInfo = connectAbilityCB->cbBase.cbInfo;
    napi_handle_scope scope = nullptr;
    napi_status scopeStatus = napi_open_handle_scope(cbInfo.env, &scope);
    if (scopeStatus != napi_ok || scope == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_open_handle_scope failed");
        return;
    }

    napi_value globalValue;
    napi_get_global(cbInfo.env, &globalValue);
    napi_value func;
    napi_get_named_property(cbInfo.env, globalValue, "requireNapi", &func);

    napi_value rpcInfo;
    napi_create_string_utf8(cbInfo.env, "rpc", NAPI_AUTO_LENGTH, &rpcInfo);
    napi_value funcArgv[1] = { rpcInfo };
    napi_value returnValue;
    napi_call_function(cbInfo.env, globalValue, func, 1, funcArgv, &returnValue);

    napi_value result[ARGS_TWO] = {nullptr};
    result[PARAM0] =
        WrapElementName(cbInfo.env, connectAbilityCB->abilityConnectionCB.elementName);
    napi_value jsRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        cbInfo.env, connectAbilityCB->abilityConnectionCB.connection);
    result[PARAM1] = jsRemoteObject;

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(cbInfo.env, cbInfo.callback, &callback);

    napi_call_function(
        cbInfo.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);
    if (cbInfo.callback != nullptr) {
        napi_delete_reference(cbInfo.env, cbInfo.callback);
    }
    napi_close_handle_scope(cbInfo.env, scope);
}

void NAPIAbilityConnection::HandleOnAbilityConnectDone(ConnectionCallback &callback, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callback.env, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null loop");
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null work");
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null connectAbilityCB");
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }
    connectAbilityCB->cbBase.cbInfo.env = callback.env;
    connectAbilityCB->cbBase.cbInfo.callback = callback.connectCallbackRef;
    callback.connectCallbackRef = nullptr;
    connectAbilityCB->abilityConnectionCB.elementName = element_;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    connectAbilityCB->abilityConnectionCB.connection = serviceRemoteObject_;
    work->data = static_cast<void *>(connectAbilityCB);

    int rev = uv_queue_work_with_qos(
        loop, work, [](uv_work_t *work) {}, UvWorkOnAbilityConnectDone, uv_qos_user_initiated);
    if (rev != 0) {
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

void NAPIAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s bundleName:%{public}s abilityName:%{public}s, resultCode:%{public}d",
             __func__, element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, remoteObject == nullptr.", __func__);
        return;
    }
    std::lock_guard<std::mutex> guard(lock_);
    element_ = element;
    serviceRemoteObject_ = remoteObject;
    for (const auto &callback : callbacks_) {
        HandleOnAbilityConnectDone(*callback, resultCode);
    }
    connectionState_ = CONNECTION_STATE_CONNECTED;
}

void UvWorkOnAbilityDisconnectDone(uv_work_t *work, int status)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    std::unique_ptr<uv_work_t> managedWork(work);
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null work");
        return;
    }
    // JS Thread
    std::unique_ptr<ConnectAbilityCB> connectAbilityCB(static_cast<ConnectAbilityCB *>(work->data));
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null connectAbilityCB");
        return;
    }
    CallbackInfo &cbInfo = connectAbilityCB->cbBase.cbInfo;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cbInfo.env, &scope);
    if (scope == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_open_handle_scope failed");
        return;
    }
    napi_value result = WrapElementName(cbInfo.env, connectAbilityCB->abilityConnectionCB.elementName);
    if (cbInfo.callback != nullptr) {
        napi_value callback = nullptr;
        napi_value callResult = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(cbInfo.env, &undefined);
        napi_get_reference_value(cbInfo.env, cbInfo.callback, &callback);
        napi_call_function(cbInfo.env, undefined, callback, ARGS_ONE, &result, &callResult);
        napi_delete_reference(cbInfo.env, cbInfo.callback);
        cbInfo.callback = nullptr;
    }
    napi_close_handle_scope(cbInfo.env, scope);

    // release connect
    std::lock_guard<std::mutex> lock(g_connectionsLock_);
    TAG_LOGI(AAFwkTag::JSNAPI, "connects_.size:%{public}zu", connects_.size());
    std::string deviceId = connectAbilityCB->abilityConnectionCB.elementName.GetDeviceID();
    std::string bundleName = connectAbilityCB->abilityConnectionCB.elementName.GetBundleName();
    std::string abilityName = connectAbilityCB->abilityConnectionCB.elementName.GetAbilityName();
    auto item = std::find_if(connects_.begin(), connects_.end(),
        [deviceId, bundleName, abilityName](const std::map<ConnectionKey,
            sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetDeviceId()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (item != connects_.end()) {
        // match deviceId & bundlename && abilityname
        connects_.erase(item);
        TAG_LOGI(AAFwkTag::JSNAPI, "connects_.size:%{public}zu", connects_.size());
    }
}

void NAPIAbilityConnection::HandleOnAbilityDisconnectDone(ConnectionCallback &callback, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callback.env, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null loop");
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null work");
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null connectAbilityCB");
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }

    connectAbilityCB->cbBase.cbInfo.env = callback.env;
    connectAbilityCB->cbBase.cbInfo.callback = callback.disconnectCallbackRef;
    callback.disconnectCallbackRef = nullptr;
    connectAbilityCB->abilityConnectionCB.elementName = element_;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    work->data = static_cast<void *>(connectAbilityCB);

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkOnAbilityDisconnectDone);
    if (rev != 0) {
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

void NAPIAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s bundleName:%{public}s abilityName:%{public}s, resultCode:%{public}d",
             __func__, element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    std::lock_guard<std::mutex> guard(lock_);
    element_ = element;
    for (const auto &callback : callbacks_) {
        HandleOnAbilityDisconnectDone(*callback, resultCode);
    }
    connectionState_ = CONNECTION_STATE_DISCONNECTED;
}
}  // namespace AppExecFwk
}  // namespace OHOS
