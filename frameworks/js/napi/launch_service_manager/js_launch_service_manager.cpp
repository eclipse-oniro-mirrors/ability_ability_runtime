/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "js_launch_service_manager.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unistd.h>

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "daemon_service_manager_client.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "napi_common_util.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_FOUR = 4;
constexpr size_t ARGC_FIVE = 5;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t INDEX_THREE = 3;
constexpr size_t INDEX_FOUR = 4;
constexpr int32_t CONNECTION_ERROR = 16000050;
constexpr int32_t USER_ID_RANGE = 200000;
constexpr int32_t ALL_USER_ID = -3;
constexpr int32_t MAX_APP_INDEX = 1000;
constexpr const char *REGISTER_DAEMON_PERMISSION = "ohos.permission.REGISTER_DAEMON_SERVICE";
constexpr const char *USE_DAEMON_PERMISSION = "ohos.permission.USE_DAEMON_SERVICE";
constexpr const char *MANAGE_DAEMON_PERMISSION = "ohos.permission.MANAGE_DAEMON_SERVICE";

napi_value CreateDescriptor(napi_env env, const LaunchServiceDescriptor &descriptor);

std::atomic<uint32_t> g_nextObserverId{0};

bool ConvertUint8Array(napi_env env, napi_value value, std::vector<uint8_t> &result)
{
    bool isTypedArray = false;
    if (napi_is_typedarray(env, value, &isTypedArray) != napi_ok || !isTypedArray) {
        return false;
    }
    napi_typedarray_type type;
    size_t length = 0;
    void *data = nullptr;
    if (napi_get_typedarray_info(env, value, &type, &length, &data, nullptr, nullptr) != napi_ok ||
        type != napi_uint8_array || (length > 0 && data == nullptr)) {
        return false;
    }
    if (length == 0) {
        result.clear();
        return true;
    }
    auto *bytes = static_cast<uint8_t *>(data);
    result.assign(bytes, bytes + length);
    return true;
}

bool IsUndefined(napi_env env, napi_value value)
{
    napi_valuetype type = napi_undefined;
    return value != nullptr && napi_typeof(env, value, &type) == napi_ok && type == napi_undefined;
}

class JsLaunchServiceObserver final : public LaunchServiceObserverStub {
public:
    explicit JsLaunchServiceObserver(napi_env env) : env_(env)
    {
    }

    int32_t AddObserver(napi_value observer)
    {
        napi_ref ref = nullptr;
        if (napi_create_reference(env_, observer, 1, &ref) != napi_ok) {
            return 0;
        }
        std::lock_guard<std::mutex> lock(observerMutex_);
        const int32_t observerId = AllocateObserverIdLocked();
        if (observerId <= 0) {
            napi_delete_reference(env_, ref);
            return 0;
        }
        observers_.emplace(observerId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref)));
        return observerId;
    }

    bool HasObserver(int32_t observerId)
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        return observers_.find(observerId) != observers_.end();
    }

    size_t GetObserverCount()
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        return observers_.size();
    }

    void RemoveObserver(int32_t observerId)
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.erase(observerId);
    }

    void Disable()
    {
        active_.store(false);
        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.clear();
    }

    void OnLaunchServiceStatusChanged(const LaunchServiceStateData &stateData) override
    {
        Schedule(
            "onLaunchServiceRegisteredChanged", stateData.descriptor, static_cast<int32_t>(stateData.state), 0, false);
    }

    void OnLaunchServiceProcessStateChanged(const LaunchServiceProcessStateData &processStateData) override
    {
        Schedule("onLaunchServiceProcessStateChanged", processStateData.descriptor,
            static_cast<int32_t>(processStateData.state), processStateData.pid, true);
    }

private:
    int32_t AllocateObserverIdLocked()
    {
        for (size_t count = 0; count <= observers_.size(); ++count) {
            const uint32_t serial = g_nextObserverId.fetch_add(1, std::memory_order_relaxed);
            const int32_t candidate =
                static_cast<int32_t>(serial % static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) + 1;
            if (observers_.find(candidate) == observers_.end()) {
                return candidate;
            }
        }
        return 0;
    }

    void Schedule(
        const char *method, const LaunchServiceDescriptor &descriptor, int32_t state, int32_t pid, bool processData)
    {
        if (!active_.load()) {
            return;
        }
        wptr<JsLaunchServiceObserver> weak = this;
        auto complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
            [weak, methodName = std::string(method), descriptor, state, pid, processData](
                napi_env env, NapiAsyncTask &, int32_t) {
                auto self = weak.promote();
                if (self == nullptr || !self->active_.load()) {
                    return;
                }
                self->NotifyObservers(env, methodName, descriptor, state, pid, processData);
            });
        napi_ref callback = nullptr;
        NapiAsyncTask::Schedule("JsLaunchServiceObserver::Notify", env_,
            std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
    }

    void NotifyObservers(napi_env env, const std::string &methodName, const LaunchServiceDescriptor &descriptor,
        int32_t state, int32_t pid, bool processData)
    {
        std::vector<std::shared_ptr<NativeReference>> observers;
        {
            std::lock_guard<std::mutex> lock(observerMutex_);
            for (const auto &item : observers_) {
                observers.emplace_back(item.second);
            }
        }
        napi_value data = nullptr;
        napi_create_object(env, &data);
        napi_set_named_property(env, data, "descriptor", CreateDescriptor(env, descriptor));
        napi_value argument = data;
        if (processData) {
            napi_set_named_property(env, data, "pid", CreateJsValue(env, pid));
            napi_set_named_property(env, data, "state", CreateJsValue(env, state));
        } else {
            napi_set_named_property(env, data, "registered",
                CreateJsValue(env, state != static_cast<int32_t>(LaunchServiceState::NOT_REGISTERED)));
            napi_create_array_with_length(env, 1, &argument);
            napi_set_element(env, argument, 0, data);
        }
        for (const auto &observer : observers) {
            napi_value object = observer->GetNapiValue();
            napi_value callback = nullptr;
            if (object == nullptr || napi_get_named_property(env, object, methodName.c_str(), &callback) != napi_ok) {
                continue;
            }
            napi_value result = nullptr;
            napi_call_function(env, object, callback, 1, &argument, &result);
        }
    }

    napi_env env_ = nullptr;
    std::atomic_bool active_ = true;
    std::mutex observerMutex_;
    std::map<int32_t, std::shared_ptr<NativeReference>> observers_;
};

std::mutex g_observerMutex;
std::map<napi_env, sptr<JsLaunchServiceObserver>> g_observers;

void CleanupEnableRequests(napi_env env);

void CleanupObservers(void *data)
{
    napi_env env = static_cast<napi_env>(data);
    CleanupEnableRequests(env);
    sptr<JsLaunchServiceObserver> observer;
    {
        std::lock_guard<std::mutex> lock(g_observerMutex);
        auto it = g_observers.find(env);
        if (it != g_observers.end()) {
            observer = it->second;
            g_observers.erase(it);
        }
    }
    if (observer != nullptr) {
        const size_t count = observer->GetObserverCount();
        for (size_t index = 0; index < count; ++index) {
            DaemonServiceManagerClient::GetInstance().UnregisterObserver(observer);
        }
        observer->Disable();
    }
}

bool HasFunction(napi_env env, napi_value object, const char *name)
{
    napi_value method = nullptr;
    napi_valuetype type = napi_undefined;
    return napi_get_named_property(env, object, name, &method) == napi_ok && method != nullptr &&
        napi_typeof(env, method, &type) == napi_ok && type == napi_function;
}

bool GetNonEmptyString(napi_env env, napi_value property, const std::string &name, std::string &value)
{
    if (!ConvertFromJsValue(env, property, value)) {
        ThrowInvalidParamError(env, "Parameter error. " + name + " must be a string.");
        return false;
    }
    if (value.empty()) {
        ThrowInvalidParamError(env, "Parameter error. " + name + " must not be empty.");
        return false;
    }
    return true;
}

bool GetRequiredString(napi_env env, napi_value object, const char *name, std::string &value)
{
    napi_value property = nullptr;
    const std::string fieldName = std::string("descriptor.") + name;
    if (napi_get_named_property(env, object, name, &property) != napi_ok || property == nullptr) {
        ThrowInvalidParamError(env, "Parameter error. Failed to read " + fieldName + ".");
        return false;
    }
    return GetNonEmptyString(env, property, fieldName, value);
}

bool GetNonNegativeInt(napi_env env, napi_value property, const std::string &name, int32_t &value)
{
    double number = 0;
    if (napi_get_value_double(env, property, &number) != napi_ok) {
        ThrowInvalidParamError(env, "Parameter error. " + name + " must be a number.");
        return false;
    }
    if (!std::isfinite(number) || std::trunc(number) != number) {
        ThrowInvalidParamError(env, "Parameter error. " + name + " must be a finite integer.");
        return false;
    }
    if (number < 0) {
        ThrowInvalidParamError(env, "Parameter error. " + name + " must not be negative.");
        return false;
    }
    if (number > std::numeric_limits<int32_t>::max()) {
        ThrowInvalidParamError(env, "Parameter error. " + name + " must not exceed 2147483647.");
        return false;
    }
    value = static_cast<int32_t>(number);
    return true;
}

bool CheckAppIndex(napi_env env, int32_t appIndex, const std::string &name)
{
    if (appIndex > MAX_APP_INDEX) {
        ThrowInvalidParamError(
            env, "Parameter error. " + name + " must not exceed " + std::to_string(MAX_APP_INDEX) + ".");
        return false;
    }
    return true;
}

bool GetOptionalInt(napi_env env, napi_value object, const char *name, bool &hasValue, int32_t &value)
{
    bool hasProperty = false;
    const std::string fieldName = std::string("descriptor.") + name;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok) {
        ThrowInvalidParamError(env, "Parameter error. Failed to check " + fieldName + ".");
        return false;
    }
    hasValue = hasProperty;
    if (!hasProperty) {
        return true;
    }
    napi_value property = nullptr;
    if (napi_get_named_property(env, object, name, &property) != napi_ok || property == nullptr) {
        ThrowInvalidParamError(env, "Parameter error. Failed to read " + fieldName + ".");
        return false;
    }
    hasValue = !IsUndefined(env, property);
    if (!hasValue) {
        return true;
    }
    return GetNonNegativeInt(env, property, fieldName, value);
}

bool UnwrapDescriptor(napi_env env, napi_value value, LaunchServiceDescriptor &descriptor)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_object) {
        ThrowInvalidParamError(env, "Parameter error. descriptor must be an object.");
        return false;
    }
    if (!GetRequiredString(env, value, "bundleName", descriptor.bundleName)) {
        return false;
    }
    if (!GetRequiredString(env, value, "moduleName", descriptor.moduleName)) {
        return false;
    }
    if (!GetRequiredString(env, value, "serviceName", descriptor.serviceName)) {
        return false;
    }
    descriptor.userId = getuid() / USER_ID_RANGE;
    descriptor.appIndex = 0;
    bool hasUserId = false;
    bool hasAppIndex = false;
    if (!GetOptionalInt(env, value, "userId", hasUserId, descriptor.userId)) {
        return false;
    }
    if (!GetOptionalInt(env, value, "appIndex", hasAppIndex, descriptor.appIndex)) {
        return false;
    }
    if (hasAppIndex && !CheckAppIndex(env, descriptor.appIndex, "descriptor.appIndex")) {
        return false;
    }
    return true;
}

napi_value CreateDescriptor(napi_env env, const LaunchServiceDescriptor &descriptor)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, descriptor.bundleName));
    napi_set_named_property(env, object, "moduleName", CreateJsValue(env, descriptor.moduleName));
    napi_set_named_property(env, object, "serviceName", CreateJsValue(env, descriptor.serviceName));
    napi_set_named_property(env, object, "userId", CreateJsValue(env, descriptor.userId));
    napi_set_named_property(env, object, "appIndex", CreateJsValue(env, descriptor.appIndex));
    return object;
}

napi_value CreateInformation(napi_env env, const LaunchServiceInformation &information)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_set_named_property(env, object, "descriptor", CreateDescriptor(env, information.descriptor));
    napi_set_named_property(env, object, "state", CreateJsValue(env, static_cast<int32_t>(information.state)));
    napi_set_named_property(env, object, "pid", CreateJsValue(env, information.pid));
    return object;
}

napi_value CreateInformationArray(napi_env env, const std::vector<LaunchServiceInformation> &informations)
{
    napi_value array = nullptr;
    napi_create_array_with_length(env, informations.size(), &array);
    for (size_t index = 0; index < informations.size(); ++index) {
        napi_set_element(env, array, index, CreateInformation(env, informations[index]));
    }
    return array;
}

napi_value CreateLaunchServiceError(napi_env env, int32_t errorCode, const char *permission)
{
    if (errorCode == AAFwk::ABILITY_SERVICE_NOT_CONNECTED) {
        return CreateJsError(env, CONNECTION_ERROR, "Failed to connect to the system service.");
    }
    return CreateJsErrorByNativeErr(env, errorCode, permission);
}

using LaunchServiceClientCall = int32_t (DaemonServiceManagerClient::*)(const LaunchServiceDescriptor &);

napi_value InvokeLaunchServiceCall(
    napi_env env, napi_callback_info info, const char *taskName, const char *permission, LaunchServiceClientCall call)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        ThrowInvalidNumParametersError(env);
        return CreateJsUndefined(env);
    }
    auto descriptor = std::make_shared<LaunchServiceDescriptor>();
    if (!UnwrapDescriptor(env, argv[INDEX_ZERO], *descriptor)) {
        return CreateJsUndefined(env);
    }
    auto errorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [descriptor, errorCode, call]() {
        *errorCode = (DaemonServiceManagerClient::GetInstance().*call)(*descriptor);
    };
    NapiAsyncTask::CompleteCallback complete = [errorCode, permission](napi_env env, NapiAsyncTask &task, int32_t) {
        if (*errorCode != ERR_OK) {
            task.Reject(env, CreateLaunchServiceError(env, *errorCode, permission));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value promise = nullptr;
    NapiAsyncTask::Schedule(
        taskName, env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &promise));
    return promise;
}

napi_value Register(napi_env env, napi_callback_info info)
{
    return InvokeLaunchServiceCall(env, info, "JsLaunchServiceManager::Register", REGISTER_DAEMON_PERMISSION,
        &DaemonServiceManagerClient::Register);
}

napi_value Unregister(napi_env env, napi_callback_info info)
{
    return InvokeLaunchServiceCall(env, info, "JsLaunchServiceManager::Unregister", REGISTER_DAEMON_PERMISSION,
        &DaemonServiceManagerClient::Unregister);
}

napi_value Start(napi_env env, napi_callback_info info)
{
    return InvokeLaunchServiceCall(
        env, info, "JsLaunchServiceManager::Start", USE_DAEMON_PERMISSION, &DaemonServiceManagerClient::Start);
}

napi_value Stop(napi_env env, napi_callback_info info)
{
    return InvokeLaunchServiceCall(
        env, info, "JsLaunchServiceManager::Stop", USE_DAEMON_PERMISSION, &DaemonServiceManagerClient::Stop);
}

napi_value GetInformation(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        ThrowInvalidNumParametersError(env);
        return CreateJsUndefined(env);
    }
    auto descriptor = std::make_shared<LaunchServiceDescriptor>();
    if (!UnwrapDescriptor(env, argv[INDEX_ZERO], *descriptor)) {
        return CreateJsUndefined(env);
    }
    auto result = std::make_shared<LaunchServiceInformation>();
    auto errorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [descriptor, result, errorCode]() {
        *errorCode = DaemonServiceManagerClient::GetInstance().GetInformation(*descriptor, *result);
    };
    NapiAsyncTask::CompleteCallback complete = [result, errorCode](napi_env env, NapiAsyncTask &task, int32_t) {
        if (*errorCode != ERR_OK) {
            task.Reject(env, CreateLaunchServiceError(env, *errorCode, USE_DAEMON_PERMISSION));
            return;
        }
        task.ResolveWithNoError(env, CreateInformation(env, *result));
    };
    napi_value promise = nullptr;
    NapiAsyncTask::Schedule("JsLaunchServiceManager::GetInformation", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &promise));
    return promise;
}

napi_value GetAllRegisteredInformation(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    int32_t userId = ALL_USER_ID;
    if (argc == ARGC_ONE && !IsUndefined(env, argv[INDEX_ZERO])) {
        if (!GetNonNegativeInt(env, argv[INDEX_ZERO], "userId", userId)) {
            return CreateJsUndefined(env);
        }
    }
    auto result = std::make_shared<std::vector<LaunchServiceInformation>>();
    auto errorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [userId, result, errorCode]() {
        *errorCode = DaemonServiceManagerClient::GetInstance().GetAllRegisteredInformation(userId, *result);
    };
    NapiAsyncTask::CompleteCallback complete = [result, errorCode](napi_env env, NapiAsyncTask &task, int32_t) {
        if (*errorCode != ERR_OK) {
            task.Reject(env, CreateLaunchServiceError(env, *errorCode, MANAGE_DAEMON_PERMISSION));
            return;
        }
        task.ResolveWithNoError(env, CreateInformationArray(env, *result));
    };
    napi_value promise = nullptr;
    NapiAsyncTask::Schedule("JsLaunchServiceManager::GetAllRegisteredInformation", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &promise));
    return promise;
}

struct PendingEnableRequest {
    napi_env env = nullptr;
    std::unique_ptr<NapiAsyncTask> task;
};

std::mutex g_enableMutex;
std::map<napi_env, std::vector<std::shared_ptr<PendingEnableRequest>>> g_enableRequests;

void CleanupEnableRequests(napi_env env)
{
    std::lock_guard<std::mutex> lock(g_enableMutex);
    auto it = g_enableRequests.find(env);
    if (it == g_enableRequests.end()) {
        return;
    }
    for (const auto &request : it->second) {
        request->env = nullptr;
        request->task.reset();
    }
    g_enableRequests.erase(it);
}

void HandleEnableResult(const std::shared_ptr<PendingEnableRequest> &request, int32_t result)
{
    napi_env env = nullptr;
    std::unique_ptr<NapiAsyncTask> task;
    {
        std::lock_guard<std::mutex> lock(g_enableMutex);
        env = request->env;
        if (env == nullptr) {
            return;
        }
        task = std::move(request->task);
        request->env = nullptr;
        auto it = g_enableRequests.find(env);
        if (it != g_enableRequests.end()) {
            auto &requests = it->second;
            requests.erase(std::remove(requests.begin(), requests.end(), request), requests.end());
            if (requests.empty()) {
                g_enableRequests.erase(it);
            }
        }
    }
    HandleScope scope(env);
    if (result != ERR_OK) {
        task->Reject(env, CreateLaunchServiceError(env, result, MANAGE_DAEMON_PERMISSION));
    } else {
        task->ResolveWithNoError(env, CreateJsUndefined(env));
    }
}

void CompleteEnableRequest(const std::shared_ptr<PendingEnableRequest> &request, int32_t result)
{
    std::lock_guard<std::mutex> lock(g_enableMutex);
    if (request->env == nullptr) {
        return;
    }
    auto complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [request, result](napi_env env, NapiAsyncTask &task, int32_t status) { HandleEnableResult(request, result); });
    NapiAsyncTask::Schedule("JsLaunchServiceManager::SetLaunchServicesEnabled", request->env,
        std::make_unique<NapiAsyncTask>(static_cast<napi_ref>(nullptr), nullptr, std::move(complete)));
}

napi_value SubmitLaunchServicesEnabled(napi_env env, const std::string &bundleName, int32_t userId, int32_t appIndex,
    const std::vector<uint8_t> &token, bool enable)
{
    napi_value promise = nullptr;
    auto request = std::make_shared<PendingEnableRequest>();
    request->env = env;
    request->task = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &promise);
    {
        std::lock_guard<std::mutex> lock(g_enableMutex);
        g_enableRequests[env].emplace_back(request);
    }
    DaemonServiceManagerClient::GetInstance().SetLaunchServicesEnabled(bundleName, userId, appIndex, token, enable,
        [request](int32_t result) { CompleteEnableRequest(request, result); });
    return promise;
}

napi_value SetLaunchServicesEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FIVE;
    napi_value argv[ARGC_FIVE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_FOUR && argc != ARGC_FIVE) {
        ThrowInvalidNumParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    int32_t userId = 0;
    int32_t appIndex = 0;
    std::vector<uint8_t> token;
    bool enable = false;
    if (!GetNonEmptyString(env, argv[INDEX_ZERO], "bundleName", bundleName)) {
        return CreateJsUndefined(env);
    }
    if (!GetNonNegativeInt(env, argv[INDEX_ONE], "userId", userId)) {
        return CreateJsUndefined(env);
    }
    if (!GetNonNegativeInt(env, argv[INDEX_TWO], "appIndex", appIndex)) {
        return CreateJsUndefined(env);
    }
    if (!CheckAppIndex(env, appIndex, "appIndex")) {
        return CreateJsUndefined(env);
    }
    if (!ConvertFromJsValue(env, argv[INDEX_THREE], enable)) {
        ThrowInvalidParamError(env, "Parameter error. enable must be a boolean.");
        return CreateJsUndefined(env);
    }
    if (argc == ARGC_FIVE && !IsUndefined(env, argv[INDEX_FOUR])) {
        if (!ConvertUint8Array(env, argv[INDEX_FOUR], token)) {
            ThrowInvalidParamError(env, "Parameter error. token must be a Uint8Array.");
            return CreateJsUndefined(env);
        }
    }

    return SubmitLaunchServicesEnabled(env, bundleName, userId, appIndex, token, enable);
}

bool CheckLaunchServiceObserver(napi_env env, napi_value observerObject)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, observerObject, &type) != napi_ok || type != napi_object) {
        ThrowInvalidParamError(env, "Parameter error. observer must be an object.");
        return false;
    }
    if (!HasFunction(env, observerObject, "onLaunchServiceRegisteredChanged")) {
        ThrowInvalidParamError(env, "Parameter error. observer.onLaunchServiceRegisteredChanged must be a function.");
        return false;
    }
    if (!HasFunction(env, observerObject, "onLaunchServiceProcessStateChanged")) {
        ThrowInvalidParamError(env, "Parameter error. observer.onLaunchServiceProcessStateChanged must be a function.");
        return false;
    }
    return true;
}

napi_value OnLaunchServiceObserver(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        ThrowInvalidNumParametersError(env);
        return CreateJsUndefined(env);
    }
    if (!CheckLaunchServiceObserver(env, argv[INDEX_ZERO])) {
        return CreateJsUndefined(env);
    }
    sptr<JsLaunchServiceObserver> observer;
    {
        std::lock_guard<std::mutex> lock(g_observerMutex);
        auto it = g_observers.find(env);
        if (it != g_observers.end()) {
            observer = it->second;
        } else {
            observer = new (std::nothrow) JsLaunchServiceObserver(env);
            if (observer != nullptr) {
                g_observers.emplace(env, observer);
            }
        }
    }
    if (observer == nullptr) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    const int32_t observerId = observer->AddObserver(argv[INDEX_ZERO]);
    if (observerId <= 0) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    int32_t ret = DaemonServiceManagerClient::GetInstance().RegisterObserver(observer);
    if (ret != ERR_OK) {
        observer->RemoveObserver(observerId);
        napi_throw(env, CreateLaunchServiceError(env, ret, MANAGE_DAEMON_PERMISSION));
        return CreateJsUndefined(env);
    }
    return CreateJsValue(env, observerId);
}

sptr<JsLaunchServiceObserver> FindLaunchServiceObserver(napi_env env)
{
    sptr<JsLaunchServiceObserver> observer;
    {
        std::lock_guard<std::mutex> lock(g_observerMutex);
        auto it = g_observers.find(env);
        if (it != g_observers.end()) {
            observer = it->second;
        }
    }
    return observer;
}

napi_value OffLaunchServiceObserver(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    int32_t observerId = 0;
    if (argc != ARGC_ONE) {
        ThrowInvalidNumParametersError(env);
        return CreateJsUndefined(env);
    }
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], observerId)) {
        ThrowInvalidParamError(env, "Parameter error. observerId must be a number.");
        return CreateJsUndefined(env);
    }
    auto observer = FindLaunchServiceObserver(env);
    auto invalidObserverId = std::make_shared<bool>(false);
    auto errorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [observer, observerId, invalidObserverId, errorCode]() {
        if (observer == nullptr || !observer->HasObserver(observerId)) {
            *invalidObserverId = true;
            return;
        }
        *errorCode = DaemonServiceManagerClient::GetInstance().UnregisterObserver(observer);
    };
    NapiAsyncTask::CompleteCallback complete = [observer, observerId, invalidObserverId, errorCode](
        napi_env env, NapiAsyncTask &task, int32_t) {
        if (*invalidObserverId) {
            task.Reject(env,
                CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_LAUNCH_SERVICE_INVALID_ID),
                    "The specified observerId is invalid."));
            return;
        }
        if (*errorCode != ERR_OK) {
            task.Reject(env, CreateLaunchServiceError(env, *errorCode, MANAGE_DAEMON_PERMISSION));
            return;
        }
        observer->RemoveObserver(observerId);
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value promise = nullptr;
    NapiAsyncTask::Schedule("JsLaunchServiceManager::OffLaunchServiceObserver", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &promise));
    return promise;
}
} // namespace

napi_value JsLaunchServiceManagerInit(napi_env env, napi_value exports)
{
    napi_add_env_cleanup_hook(env, CleanupObservers, env);
    BindNativeFunction(env, exports, "register", "JsLaunchServiceManager", Register);
    BindNativeFunction(env, exports, "unregister", "JsLaunchServiceManager", Unregister);
    BindNativeFunction(env, exports, "start", "JsLaunchServiceManager", Start);
    BindNativeFunction(env, exports, "stop", "JsLaunchServiceManager", Stop);
    BindNativeFunction(env, exports, "getInformation", "JsLaunchServiceManager", GetInformation);
    BindNativeFunction(
        env, exports, "getAllRegisteredInformation", "JsLaunchServiceManager", GetAllRegisteredInformation);
    BindNativeFunction(env, exports, "setLaunchServicesEnabled", "JsLaunchServiceManager", SetLaunchServicesEnabled);
    BindNativeFunction(env, exports, "onLaunchServiceObserver", "JsLaunchServiceManager", OnLaunchServiceObserver);
    BindNativeFunction(env, exports, "offLaunchServiceObserver", "JsLaunchServiceManager", OffLaunchServiceObserver);

    napi_value state = nullptr;
    napi_create_object(env, &state);
    napi_set_named_property(
        env, state, "NOT_REGISTERED", CreateJsValue(env, static_cast<int32_t>(LaunchServiceState::NOT_REGISTERED)));
    napi_set_named_property(
        env, state, "ENABLED", CreateJsValue(env, static_cast<int32_t>(LaunchServiceState::ENABLED)));
    napi_set_named_property(
        env, state, "DISABLED", CreateJsValue(env, static_cast<int32_t>(LaunchServiceState::DISABLED)));
    napi_set_named_property(env, exports, "LaunchServiceState", state);
    napi_value processState = nullptr;
    napi_create_object(env, &processState);
    napi_set_named_property(
        env, processState, "CREATED", CreateJsValue(env, static_cast<int32_t>(LaunchServiceProcessState::CREATED)));
    napi_set_named_property(env, processState, "TERMINATED",
        CreateJsValue(env, static_cast<int32_t>(LaunchServiceProcessState::TERMINATED)));
    napi_set_named_property(env, exports, "LaunchServiceProcessState", processState);
    return exports;
}
} // namespace AbilityRuntime
} // namespace OHOS
