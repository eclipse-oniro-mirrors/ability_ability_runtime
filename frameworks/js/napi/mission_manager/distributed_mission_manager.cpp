/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <string>

#include "distributed_mission_manager.h"

#include "ability_manager_client.h"
#include "dms_sa_client.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "napi_common_data.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
using AbilityManagerClient = AAFwk::AbilityManagerClient;
const std::string TAG = "NAPIMissionRegistration";
constexpr size_t VALUE_BUFFER_SIZE = 128;
const std::string CODE_KEY_NAME = "code";
constexpr size_t INT32_SHORT_ID_LEN = 20;
constexpr size_t INT32_MIN_ID_LEN = 6;
constexpr size_t INT32_PLAINTEXT_LEN = 4;

napi_value GenerateBusinessError(const napi_env &env, int32_t errCode, const std::string &errMsg)
{
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);
    napi_value msg = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &msg);
    napi_value businessError = nullptr;
    napi_create_error(env, nullptr, msg, &businessError);
    napi_set_named_property(env, businessError, CODE_KEY_NAME.c_str(), code);
    return handleEscape.Escape(businessError);
}

static int32_t ErrorCodeReturn(int32_t code)
{
    switch (code) {
        case NO_ERROR:
            return NO_ERROR;
        case CHECK_PERMISSION_FAILED:
            return PERMISSION_DENIED;
        case DMS_PERMISSION_DENIED:
            return PERMISSION_DENIED;
        case ERR_INVALID_VALUE:
            return PARAMETER_CHECK_FAILED;
        case INVALID_PARAMETERS_ERR:
            return PARAMETER_CHECK_FAILED;
        case REGISTER_REMOTE_MISSION_LISTENER_FAIL:
            return PARAMETER_CHECK_FAILED;
        case NO_MISSION_INFO_FOR_MISSION_ID:
            return NO_MISSION_INFO_FOR_MISSION_ID;
        case CONTINUE_REMOTE_UNINSTALLED_UNSUPPORT_FREEINSTALL:
            return REMOTE_UNINSTALLED_AND_UNSUPPORT_FREEINSTALL_FOR_CONTINUE;
        case CONTINUE_REMOTE_UNINSTALLED_SUPPORT_FREEINSTALL:
            return CONTINUE_WITHOUT_FREEINSTALL_FLAG;
        case OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET:
            return OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET;
        case CONTINUE_ALREADY_IN_PROGRESS:
            return CONTINUE_ALREADY_IN_PROGRESS;
        case MISSION_FOR_CONTINUING_IS_NOT_ALIVE:
            return MISSION_FOR_CONTINUING_IS_NOT_ALIVE;
        case ERR_NOT_SYSTEM_APP:
            return NOT_SYSTEM_APP;
        default:
            return SYSTEM_WORK_ABNORMALLY;
    };
}

static std::string ErrorMessageReturn(int32_t code)
{
    switch (code) {
        case NO_ERROR:
            return std::string();
        case PERMISSION_DENIED:
            return std::string("permission denied");
        case PARAMETER_CHECK_FAILED:
            return std::string("parameter check failed.");
        case SYSTEM_WORK_ABNORMALLY:
            return std::string("the system ability work abnormally.");
        case NO_MISSION_INFO_FOR_MISSION_ID:
            return std::string("failed to get the missionInfo of the specified missionId.");
        case REMOTE_UNINSTALLED_AND_UNSUPPORT_FREEINSTALL_FOR_CONTINUE:
            return std::string("the application is not installed on the "
                "remote end and installation-free is not supported.");
        case CONTINUE_WITHOUT_FREEINSTALL_FLAG:
            return std::string("The application is not installed on the remote end and "
                "installation-free is supported. Try again with the freeInstall flag.");
        case OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET:
            return std::string("The operation device must be the device where the "
                "application to be continued is currently located or the target device.");
        case ERR_CONTINUE_ALREADY_IN_PROGRESS:
        case CONTINUE_ALREADY_IN_PROGRESS:
            return std::string("the local continuation task is already in progress.");
        case MISSION_FOR_CONTINUING_IS_NOT_ALIVE:
            return std::string("the mission for continuing is not alive, "
                "try again after restart this mission.");
        case ERR_GET_MISSION_INFO_OF_BUNDLE_NAME:
            return std::string("Failed to get the missionInfo of the specified bundle name.");
        case ERR_BIND_REMOTE_HOTSPOT_ENABLE_STATE:
            return std::string("bind error due to the remote device hotspot enable, try again after disable "
                "the remote device hotspot.");
        case ERR_BIND_REMOTE_IN_BUSY_LINK:
            return std::string("the remote device has been linked with other devices, try again when "
                "the remote device is idle.");
        case NOT_SYSTEM_APP:
            return std::string("The app is not system-app.");
        default:
            return std::string("the system ability work abnormally.");
    };
}

napi_value GetUndefined(const napi_env &env)
{
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value nullResult = nullptr;
    napi_get_undefined(env, &nullResult);
    return handleEscape.Escape(nullResult);
}

static std::string GetAnonymStr(const std::string &value)
{
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LEN) {
        return tmpStr;
    }

    if (strLen <= INT32_SHORT_ID_LEN) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LEN);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LEN, INT32_PLAINTEXT_LEN);
    }
    return res;
}

bool SetStartSyncMissionsContext(const napi_env &env, const napi_value &value,
    SyncRemoteMissionsContext* context, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "call");
    AbilityRuntime::HandleScope handleScope(env);
    bool isFixConflict = false;
    napi_has_named_property(env, value, "fixConflict", &isFixConflict);
    if (!isFixConflict) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong fixConflict argument name");
        errInfo = "Parameter error. The key of \"MissionParameter\" must be fixConflict";
        return false;
    }
    napi_value fixConflictValue = nullptr;
    napi_get_named_property(env, value, "fixConflict", &fixConflictValue);
    if (fixConflictValue == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "not find fixConflict");
        errInfo = "Parameter error. The value of \"fixConflict\" must not be undefined";
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, fixConflictValue, &valueType);
    if (valueType != napi_boolean) {
        TAG_LOGE(AAFwkTag::MISSION, "fixConflict error type");
        errInfo = "Parameter error. The type of \"fixConflict\" must be boolean";
        return false;
    }
    napi_get_value_bool(env, fixConflictValue, &context->fixConflict);
    bool isTag = false;
    napi_has_named_property(env, value, "tag", &isTag);
    if (!isTag) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong tag argument name");
        errInfo = "Parameter error. The key of \"MissionParameter\" must be tag";
        return false;
    }
    napi_value tagValue = nullptr;
    napi_get_named_property(env, value, "tag", &tagValue);
    if (tagValue == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "not find tag");
        errInfo = "Parameter error. The value of \"tag\" must not be undefined";
        return false;
    }
    napi_typeof(env, tagValue, &valueType);
    if (valueType != napi_number) {
        TAG_LOGE(AAFwkTag::MISSION, "tag error type");
        errInfo = "Parameter error. The type of \"tag\" must be number";
        return false;
    }
    napi_get_value_int64(env, tagValue, &context->tag);
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return true;
}

bool SetSyncRemoteMissionsContext(const napi_env &env, const napi_value &value,
    bool isStart, SyncRemoteMissionsContext* context, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "call");
    AbilityRuntime::HandleScope handleScope(env);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument type");
        errInfo = "Parameter error. The type of \"parameter\" must be MissionParameter";
        return false;
    }
    napi_value deviceIdValue = nullptr;
    bool isDeviceId = false;
    napi_has_named_property(env, value, "deviceId", &isDeviceId);
    if (!isDeviceId) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong deviceId argument name");
        errInfo = "Parameter error. The key of \"parameter\" must be deviceId";
        return false;
    }
    napi_get_named_property(env, value, "deviceId", &deviceIdValue);
    if (deviceIdValue == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "not find deviceId");
        errInfo = "Parameter error. The value of \"deviceId\" must not be undefined";
        return false;
    }
    napi_typeof(env, deviceIdValue, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId error type");
        errInfo = "Parameter error. The type of \"deviceId\" must be string";
        return false;
    }

    char deviceId[VALUE_BUFFER_SIZE + 1] = {0};
    napi_get_value_string_utf8(env, deviceIdValue, deviceId, VALUE_BUFFER_SIZE + 1, &context->valueLen);
    if (context->valueLen > VALUE_BUFFER_SIZE) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId length not correct");
        errInfo = "Parameter error. The length of \"deviceId\" must be less than " +
            std::to_string(VALUE_BUFFER_SIZE);
        return false;
    }
    context->deviceId = deviceId;

    if (isStart) {
        if (!SetStartSyncMissionsContext (env, value, context, errInfo)) {
            TAG_LOGE(AAFwkTag::MISSION, "Wrong start sync argument");
            return false;
        }
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return true;
}

bool ProcessSyncInput(napi_env &env, napi_callback_info info, bool isStart,
    SyncRemoteMissionsContext* syncContext, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleScope handleScope(env);
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGS_ONE && argc != ARGS_TWO) {
        TAG_LOGE(AAFwkTag::MISSION, "argument size error");
        errInfo = "Parameter error. The type of \"number of parameters\" must be 1 or 2";
        return false;
    }
    syncContext->env = env;
    if (!SetSyncRemoteMissionsContext(env, argv[0], isStart, syncContext, errInfo)) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument");
        return false;
    }
    if (argc == ARGS_TWO) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[1], &valueType);
        if (valueType != napi_function) {
            TAG_LOGE(AAFwkTag::MISSION, "callback error type");
            errInfo = "Parameter error. The type of \"callback\" must be AsynCallback<void>: void";
            return false;
        }
        napi_create_reference(env, argv[1], 1, &syncContext->callbackRef);
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return true;
}

void StartSyncRemoteMissionsAsyncWork(napi_env &env, const napi_value resourceName,
    SyncRemoteMissionsContext* syncContext)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            syncContext->result = AbilityManagerClient::GetInstance()->
                StartSyncRemoteMissions(syncContext->deviceId,
                syncContext->fixConflict, syncContext->tag);
        },
        [](napi_env env, napi_status status, void* data) {
            AbilityRuntime::HandleScope handleScope(env);
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            // set result
            napi_value result[2] = { nullptr };
            napi_get_undefined(env, &result[1]);
            if (syncContext->result == 0) {
                napi_get_undefined(env, &result[0]);
            } else {
                int32_t errCode = ErrorCodeReturn(syncContext->result);
                result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
            }

            if (syncContext->callbackRef == nullptr) { // promise
                if (syncContext->result == 0) {
                    napi_resolve_deferred(env, syncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, syncContext->deferred, result[0]);
                }
            } else { // AsyncCallback
                napi_value callback = nullptr;
                napi_get_reference_value(env, syncContext->callbackRef, &callback);
                napi_value callResult;
                napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
                napi_delete_reference(env, syncContext->callbackRef);
            }
            napi_delete_async_work(env, syncContext->work);
            delete syncContext;
            syncContext = nullptr;
        },
        static_cast<void *>(syncContext),
        &syncContext->work);
        napi_queue_async_work_with_qos(env, syncContext->work, napi_qos_user_initiated);
    TAG_LOGI(AAFwkTag::MISSION, "end");
}

napi_value NAPI_StartSyncRemoteMissions(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    std::string errInfo = "Parameter error";
    auto syncContext = new SyncRemoteMissionsContext();
    if (!ProcessSyncInput(env, info, true, syncContext, errInfo)) {
        delete syncContext;
        syncContext = nullptr;
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument");
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    napi_value result = nullptr;
    if (syncContext->callbackRef == nullptr) {
        napi_create_promise(env, &syncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    StartSyncRemoteMissionsAsyncWork(env, resourceName, syncContext);
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return handleEscape.Escape(result);
}

void StopSyncRemoteMissionsAsyncWork(napi_env &env, napi_value resourceName,
    SyncRemoteMissionsContext* syncContext)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            syncContext->result = AbilityManagerClient::GetInstance()->
                StopSyncRemoteMissions(syncContext->deviceId);
        },
        [](napi_env env, napi_status status, void* data) {
            AbilityRuntime::HandleScope handleScope(env);
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            // set result
            napi_value result[2] = { nullptr };
            napi_get_undefined(env, &result[1]);
            if (syncContext->result == 0) {
                napi_get_undefined(env, &result[0]);
            } else {
                int32_t errCode = ErrorCodeReturn(syncContext->result);
                result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
            }

            if (syncContext->callbackRef == nullptr) { // promise
                if (syncContext->result == 0) {
                    napi_resolve_deferred(env, syncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, syncContext->deferred, result[0]);
                }
            } else { // AsyncCallback
                napi_value callback = nullptr;
                napi_get_reference_value(env, syncContext->callbackRef, &callback);
                napi_value callResult;
                napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
                napi_delete_reference(env, syncContext->callbackRef);
            }
            napi_delete_async_work(env, syncContext->work);
            delete syncContext;
            syncContext = nullptr;
        },
        static_cast<void *>(syncContext),
        &syncContext->work);
        napi_queue_async_work_with_qos(env, syncContext->work, napi_qos_user_initiated);
    TAG_LOGI(AAFwkTag::MISSION, "end");
}

napi_value NAPI_StopSyncRemoteMissions(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    std::string errInfo = "Parameter error";
    auto syncContext = new SyncRemoteMissionsContext();
    if (!ProcessSyncInput(env, info, false, syncContext, errInfo)) {
        delete syncContext;
        syncContext = nullptr;
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument");
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    napi_value result = nullptr;
    if (syncContext->callbackRef == nullptr) {
        napi_create_promise(env, &syncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    StopSyncRemoteMissionsAsyncWork(env, resourceName, syncContext);
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return handleEscape.Escape(result);
}

RegisterMissionCB *CreateRegisterMissionCBCBInfo(napi_env &env)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    auto registerMissionCB = new (std::nothrow) RegisterMissionCB;
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        return nullptr;
    }
    registerMissionCB->cbBase.cbInfo.env = env;
    registerMissionCB->cbBase.asyncWork = nullptr;
    registerMissionCB->cbBase.deferred = nullptr;
    registerMissionCB->callbackRef = nullptr;
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return registerMissionCB;
}

OnCB *CreateOnCBCBInfo(napi_env &env)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    auto onCB = new (std::nothrow) OnCB;
    if (onCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null onCB");
        return nullptr;
    }
    onCB->cbBase.cbInfo.env = env;
    onCB->cbBase.asyncWork = nullptr;
    onCB->cbBase.deferred = nullptr;
    onCB->callbackRef = nullptr;
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return onCB;
}

void RegisterMissionExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    auto registerMissionCB = (RegisterMissionCB*)data;

    std::lock_guard<std::mutex> autoLock(registrationLock_);
    sptr<NAPIRemoteMissionListener> registration;
    auto item = registration_.find(registerMissionCB->deviceId);
    if (item != registration_.end()) {
        TAG_LOGI(AAFwkTag::MISSION, "registration exits");
        registration = registration_[registerMissionCB->deviceId];
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "registration not exits");
        registration = new (std::nothrow) NAPIRemoteMissionListener();
    }
    registerMissionCB->missionRegistration = registration;
    if (registerMissionCB->missionRegistration == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null missionRegistration");
        registerMissionCB->result = -1;
        return;
    }
    registerMissionCB->missionRegistration->SetEnv(env);
    registerMissionCB->missionRegistration->
        SetNotifyMissionsChangedCBRef(registerMissionCB->missionRegistrationCB.callback[0]);
    registerMissionCB->missionRegistration->
        SetNotifySnapshotCBRef(registerMissionCB->missionRegistrationCB.callback[1]);
    registerMissionCB->missionRegistration->
        SetNotifyNetDisconnectCBRef(registerMissionCB->
            missionRegistrationCB.callback[2]); // 2 refers the second argument
    TAG_LOGI(AAFwkTag::MISSION, "set callback success");

    registerMissionCB->result =
        AbilityManagerClient::GetInstance()->
        RegisterMissionListener(registerMissionCB->deviceId,
        registerMissionCB->missionRegistration);
    if (registerMissionCB->result == NO_ERROR) {
        TAG_LOGI(AAFwkTag::MISSION, "add registration");
        registration_[registerMissionCB->deviceId] = registration;
    }
    TAG_LOGD(AAFwkTag::MISSION, "end.deviceId:%{public}d", registerMissionCB->result);
}

void RegisterMissionCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleScope handleScope(env);
    auto registerMissionCB = static_cast<RegisterMissionCB *>(data);
    // set result
    napi_value result[2] = { nullptr };
    napi_get_undefined(env, &result[1]);
    if (registerMissionCB->result == 0) {
        napi_get_undefined(env, &result[0]);
    } else {
        int32_t errCode = ErrorCodeReturn(registerMissionCB->result);
        result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
    }

    ReturnValueToApplication(env, &result[0], registerMissionCB);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    TAG_LOGI(AAFwkTag::MISSION, "end");
}

void ReturnValueToApplication(napi_env &env, napi_value *result, RegisterMissionCB *registerMissionCB)
{
    AbilityRuntime::HandleScope handleScope(env);
    if (registerMissionCB->callbackRef == nullptr) { // promise
        if (registerMissionCB->result == 0) {
            napi_resolve_deferred(env, registerMissionCB->cbBase.deferred, result[1]);
        } else {
            napi_reject_deferred(env, registerMissionCB->cbBase.deferred, result[0]);
        }
    } else { // AsyncCallback
        napi_value callback = nullptr;
        napi_get_reference_value(env, registerMissionCB->callbackRef, &callback);
        napi_value callResult;
        napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env, registerMissionCB->callbackRef);
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, registerMissionCB->cbBase.asyncWork));
}

napi_value RegisterMissionAsync(napi_env env, RegisterMissionCB *registerMissionCB)
{
    TAG_LOGI(AAFwkTag::MISSION, "asyncCallback");
    AbilityRuntime::HandleEscape handleEscape(env);
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return nullptr;
    }
    napi_value result = nullptr;
    if (registerMissionCB->callbackRef == nullptr) {
        napi_create_promise(env, &registerMissionCB->cbBase.deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        RegisterMissionExecuteCB,
        RegisterMissionCallbackCompletedCB,
        static_cast<void *>(registerMissionCB),
        &registerMissionCB->cbBase.asyncWork);
    napi_queue_async_work(env, registerMissionCB->cbBase.asyncWork);
    TAG_LOGI(AAFwkTag::MISSION, "asyncCallback end");
    return handleEscape.Escape(result);
}

bool CheckMissionCallbackProperty(napi_env &env, const napi_value &value, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    bool isFirstCallback = false;
    napi_has_named_property(env, value, "notifyMissionsChanged", &isFirstCallback);
    bool isSecondCallback = false;
    napi_has_named_property(env, value, "notifySnapshot", &isSecondCallback);
    bool isThirdCallback = false;
    napi_has_named_property(env, value, "notifyNetDisconnect", &isThirdCallback);
    if (!isFirstCallback || !isSecondCallback || !isThirdCallback) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong callback argument name");
        errInfo = "Parameter error. The type of \"options\" must be MissionCallback";
        return false;
    }
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return true;
}

bool SetCallbackReference(napi_env &env, const napi_value &value,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleScope handleScope(env);
    if (!CheckMissionCallbackProperty(env, value, errInfo)) {
        return false;
    }
    napi_value jsMethod = nullptr;
    napi_get_named_property(env, value, "notifyMissionsChanged", &jsMethod);
    if (jsMethod == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "not find notifyMissionsChanged");
        errInfo = "Parameter error. The value of \"notifyMissionsChanged\" must not be undefined";
        return false;
    }
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        TAG_LOGE(AAFwkTag::MISSION, "notifyMissionsChanged error type");
        errInfo = "Parameter error. The type of \"notifyMissionsChanged\" must be function";
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &registerMissionCB->missionRegistrationCB.callback[0]);
    napi_get_named_property(env, value, "notifySnapshot", &jsMethod);
    if (jsMethod == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "not find notifySnapshot");
        errInfo = "Parameter error. The value of \"notifySnapshot\" must not be undefined";
        return false;
    }
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        TAG_LOGE(AAFwkTag::MISSION, "notifySnapshot error type");
        errInfo = "Parameter error. The type of \"notifySnapshot\" must be function";
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &registerMissionCB->missionRegistrationCB.callback[1]);
    napi_get_named_property(env, value, "notifyNetDisconnect", &jsMethod);
    if (jsMethod == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "not find notifyNetDisconnect");
        errInfo = "Parameter error. The value of \"notifyNetDisconnect\" must not be undefined";
        return false;
    }
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        TAG_LOGE(AAFwkTag::MISSION, "notifyNetDisconnect error type");
        errInfo = "Parameter error. The type of \"notifyNetDisconnect\" must be function";
        return false;
    }
    // 2 refers the second argument
    napi_create_reference(env, jsMethod, 1, &registerMissionCB->missionRegistrationCB.callback[2]);
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return true;
}

bool CreateCallbackReference(napi_env &env, const napi_value &value,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, value, &valuetype);
    if (valuetype == napi_object) {
        if (!SetCallbackReference(env, value, registerMissionCB, errInfo)) {
            TAG_LOGE(AAFwkTag::MISSION, "Wrong callback.");
            return false;
        }
    } else {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument type");
        errInfo = "Parameter error. The type of \"options\" must be MissionCallback";
        return false;
    }
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return true;
}

bool CreateOnCallbackReference(napi_env &env, const napi_value &jsMethod,
    OnCB *onCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        TAG_LOGE(AAFwkTag::MISSION, "onCallback error type");
        errInfo = "Parameter error. The type of \"onCallback\" must be function";
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &onCB->onCallbackCB.callback);
    napi_create_reference(env, jsMethod, 1, &onCB->callbackRef);
    onCB->onCallbackCB.napiCallback =
        std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(onCB->onCallbackCB.callback));
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return true;
}

bool RegisterMissionWrapDeviceId(napi_env &env, napi_value &argc,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    AbilityRuntime::HandleScope handleScope(env);
    napi_valuetype valueType = napi_undefined;
    bool isDeviceId = false;
    napi_has_named_property(env, argc, "deviceId", &isDeviceId);
    napi_typeof(env, argc, &valueType);
    if (!isDeviceId || valueType != napi_object) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong deviceId argument name");
        errInfo = "Parameter error. The key of \"MissionDeviceInfo\" must be deviceId";
        return false;
    }

    napi_value napiDeviceId = nullptr;
    napi_get_named_property(env, argc, "deviceId", &napiDeviceId);
    if (napiDeviceId == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "not find deviceId.");
        errInfo = "Parameter error. The value of \"deviceId\" must not be undefined";
        return false;
    }
    napi_typeof(env, napiDeviceId, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId error type");
        errInfo = "Parameter error. The type of \"deviceId\" must be string";
        return false;
    }
    char deviceId[VALUE_BUFFER_SIZE + 1] = {0};
    size_t valueLen = 0;
    napi_get_value_string_utf8(env, napiDeviceId, deviceId, VALUE_BUFFER_SIZE + 1, &valueLen);
    if (valueLen > VALUE_BUFFER_SIZE) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId length not correct");
        errInfo = "Parameter error. The length of \"deviceId\" must be less than " +
            std::to_string(VALUE_BUFFER_SIZE);
        return false;
    }
    registerMissionCB->deviceId = std::string(deviceId);
    return true;
}

bool OnWrapType(napi_env &env, napi_value &argc,
    OnCB *onCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argc, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong type argument name");
        errInfo = "Parameter error. The type of \"type\" must be string";
        return false;
    }
    std::string type = AppExecFwk::UnwrapStringFromJS(env, argc, "");
    if (type != "continueStateChange") {
        TAG_LOGE(AAFwkTag::MISSION, "not find type");
        errInfo = "Parameter error. The value of \"type\" must not be continueStateChange";
        return false;
    }
    onCB->type = type;
    return true;
}

napi_value RegisterMissionWrap(napi_env &env, napi_callback_info info,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    size_t argcAsync = 3;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    if (argcAsync != ARGS_TWO && argcAsync != ARGS_THREE) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument count");
        errInfo = "Parameter error. The type of \"number of parameters\" must be 2 or 3";
        return nullptr;
    }

    if (!RegisterMissionWrapDeviceId(env, args[0], registerMissionCB, errInfo)) {
        TAG_LOGI(AAFwkTag::MISSION, "RegisterMissionWrapDeviceId failed");
        return nullptr;
    }
    if (argcAsync > 1 && !CreateCallbackReference(env, args[1], registerMissionCB, errInfo)) {
        return nullptr;
    }
    napi_valuetype valueType = napi_undefined;
    if (argcAsync == ARGS_THREE) {
        napi_typeof(env, args[ARGS_TWO], &valueType);
        if (valueType != napi_function) {
            TAG_LOGE(AAFwkTag::MISSION, "callback error type");
            errInfo = "Parameter error. The type of \"options\" must be MissionCallback";
            return nullptr;
        }
        napi_create_reference(env, args[ARGS_TWO], 1, &registerMissionCB->callbackRef);
    }

    napi_value ret = RegisterMissionAsync(env, registerMissionCB);
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return handleEscape.Escape(ret);
}

void OnExecuteCB(napi_env &env, OnCB *onCB)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    std::lock_guard<std::mutex> autoLock(onLock_);
    sptr<NAPIRemoteOnListener> registrationOfOn;
    auto item = registrationOfOn_.find(onCB->type);
    if (item != registrationOfOn_.end()) {
        TAG_LOGI(AAFwkTag::MISSION, "registrationOfOn exits");
        registrationOfOn = registrationOfOn_[onCB->type];
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "registrationOfOn not exits");
        registrationOfOn = new (std::nothrow) NAPIRemoteOnListener();
    }
    onCB->onRegistration = registrationOfOn;
    if (onCB->onRegistration == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null onRegistration");
        onCB->result = -1;
        int32_t errCode = ErrorCodeReturn(onCB->result);
        napi_throw(env, GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode)));
        return;
    }
    onCB->onRegistration->SetEnv(env);
    std::vector<std::shared_ptr<NativeReference>> vecCallback = onCB->onRegistration->GetOnCallbackCBRef();
    bool result = false;
    for (auto ele = vecCallback.begin(); ele != vecCallback.end(); ++ele) {
        napi_strict_equals(env, (*ele)->GetNapiValue(), onCB->onCallbackCB.napiCallback->GetNapiValue(), &result);
        if (result) {
            TAG_LOGE(AAFwkTag::MISSION, "Object does match value");
            return;
        }
    }
    onCB->onRegistration->
        SetOnCallbackCBRef(onCB->onCallbackCB.napiCallback);
    TAG_LOGI(AAFwkTag::MISSION, "set callback success");
    onCB->result = DmsSaClient::GetInstance().AddListener(onCB->type, onCB->onRegistration);
    if (onCB->result == NO_ERROR) {
        TAG_LOGI(AAFwkTag::MISSION, "add registrationOfOn success");
        registrationOfOn_[onCB->type] = registrationOfOn;
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "add registrationOfOn failed");
    }
    TAG_LOGI(AAFwkTag::MISSION, "called end");
}

napi_value OnWrap(napi_env &env, napi_callback_info info,
    OnCB *onCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    size_t argcAsync = 2;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    if (argcAsync != ARGS_TWO) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument count");
        errInfo = "Parameter error. The type of \"number of parameters\" must be 2";
        return nullptr;
    }
    if (!OnWrapType(env, args[0], onCB, errInfo)) {
        TAG_LOGI(AAFwkTag::MISSION, "OnWrapType failed");
        return nullptr;
    }
    if (!CreateOnCallbackReference(env, args[1], onCB, errInfo)) {
        return nullptr;
    }
    OnExecuteCB(env, onCB);
    if (onCB->result != 0) {
        int32_t errCode = ErrorCodeReturn(onCB->result);
        napi_throw(env, GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode)));
    }
    onCB->onCallbackCB.napiCallback = nullptr;
    if (onCB->callbackRef != nullptr) {
        napi_delete_reference(env, onCB->callbackRef);
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return handleEscape.Escape(result);
}

void OffExecuteCB(napi_env env, OnCB *onCB)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    std::lock_guard<std::mutex> autoLock(onLock_);
    sptr<NAPIRemoteOnListener> registrationOfOn;
    auto item = registrationOfOn_.find(onCB->type);
    if (item != registrationOfOn_.end()) {
        TAG_LOGI(AAFwkTag::MISSION, "registrationOfOff exits");
        registrationOfOn = registrationOfOn_[onCB->type];
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "registrationOfOff not exits");
        onCB->result = -1;
        return;
    }
    onCB->onRegistration = registrationOfOn;
    onCB->onRegistration->DelOnCallbackCBRef(env, onCB->onCallbackCB.napiCallback);
    if (!onCB->onRegistration->GetOnCallbackCBRef().empty()) {
        TAG_LOGI(AAFwkTag::MISSION, "callback remained");
    }
    DmsSaClient::GetInstance().DelListener(onCB->type, onCB->onRegistration);
    if (onCB->result == NO_ERROR) {
        TAG_LOGI(AAFwkTag::MISSION, "remove registration");
        registrationOfOn_.erase(onCB->type);
    }
    TAG_LOGD(AAFwkTag::MISSION, "end.type:%{public}d", onCB->result);
}

napi_value OffWrap(napi_env &env, napi_callback_info info,
    OnCB *onCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    size_t argcAsync = 2;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    if (argcAsync != ARGS_ONE && argcAsync != ARGS_TWO) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument count");
        errInfo = "Parameter error. The type of \"number of parameters\" must be 1 or 2";
        return nullptr;
    }
    if (!OnWrapType(env, args[0], onCB, errInfo)) {
        TAG_LOGI(AAFwkTag::MISSION, "OffWrapType failed");
        return nullptr;
    }
    if (argcAsync == ARGS_TWO && !CreateOnCallbackReference(env, args[1], onCB, errInfo)) {
        return nullptr;
    }
    OffExecuteCB(env, onCB);
    if (onCB->result != 0) {
        int32_t errCode = ErrorCodeReturn(onCB->result);
        napi_throw(env, GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode)));
    }
    if (onCB->callbackRef != nullptr) {
        napi_delete_reference(env, onCB->callbackRef);
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return handleEscape.Escape(result);
}

napi_value NAPI_RegisterMissionListener(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    std::string errInfo = "Parameter error";
    RegisterMissionCB *registerMissionCB = CreateRegisterMissionCBCBInfo(env);
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = RegisterMissionWrap(env, info, registerMissionCB, errInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null ret");
        delete registerMissionCB;
        registerMissionCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return handleEscape.Escape(ret);
}

napi_value NAPI_NotifyToOn(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    std::string errInfo = "Parameter error";
    OnCB *onCB = CreateOnCBCBInfo(env);
    if (onCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null onCB");
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = OnWrap(env, info, onCB, errInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null ret");
        delete onCB;
        onCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return handleEscape.Escape(ret);
}

napi_value NAPI_NotifyToOff(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    std::string errInfo = "Parameter error";
    OnCB *onCB = CreateOnCBCBInfo(env);
    if (onCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null onCB");
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = OffWrap(env, info, onCB, errInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null ret");
        delete onCB;
        onCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return handleEscape.Escape(ret);
}

NAPIRemoteMissionListener::~NAPIRemoteMissionListener()
{
    if (env_ == nullptr) {
        return;
    }
    if (notifyMissionsChangedRef_ != nullptr) {
        napi_delete_reference(env_, notifyMissionsChangedRef_);
        notifyMissionsChangedRef_ = nullptr;
    }
    if (notifySnapshotRef_ != nullptr) {
        napi_delete_reference(env_, notifySnapshotRef_);
        notifySnapshotRef_ = nullptr;
    }
    if (notifyNetDisconnectRef_ != nullptr) {
        napi_delete_reference(env_, notifyNetDisconnectRef_);
        notifyNetDisconnectRef_ = nullptr;
    }
}

void NAPIRemoteMissionListener::SetEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIRemoteOnListener::SetEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIRemoteMissionListener::SetNotifyMissionsChangedCBRef(const napi_ref &ref)
{
    notifyMissionsChangedRef_ = ref;
}

void NAPIRemoteOnListener::SetOnCallbackCBRef(std::shared_ptr<NativeReference> &ref)
{
    callbacks_.push_back(ref);
}

std::vector<std::shared_ptr<NativeReference>> NAPIRemoteOnListener::GetOnCallbackCBRef()
{
    return callbacks_;
}

bool NAPIRemoteOnListener::DelOnCallbackCBRef(napi_env env, std::shared_ptr<NativeReference> &ref)
{
    bool result = false;
    for (auto ele = callbacks_.begin(); ele != callbacks_.end(); ++ele) {
        napi_strict_equals(env, (*ele)->GetNapiValue(), ref->GetNapiValue(), &result);
        if (result) {
            TAG_LOGE(AAFwkTag::MISSION, "Object does match value, del callback");
            callbacks_.erase(ele);
            return result;
        }
    }

    return result;
}

void NAPIRemoteMissionListener::SetNotifySnapshotCBRef(const napi_ref &ref)
{
    notifySnapshotRef_ = ref;
}

void NAPIRemoteMissionListener::SetNotifyNetDisconnectCBRef(const napi_ref &ref)
{
    notifyNetDisconnectRef_ = ref;
}

void UvWorkNotifyMissionChanged(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::MISSION, "start, uv_queue_work");
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null work");
        return;
    }
    RegisterMissionCB *registerMissionCB = static_cast<RegisterMissionCB *>(work->data);
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        delete work;
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_status scopeStatus = napi_open_handle_scope(registerMissionCB->cbBase.cbInfo.env, &scope);
    if (scopeStatus != napi_ok || scope == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "napi_open_handle_scope failed");
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
        return;
    }

    napi_value result = nullptr;
    result =
        WrapString(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->deviceId.c_str(), "deviceId");

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(registerMissionCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(
        registerMissionCB->cbBase.cbInfo.env, registerMissionCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(registerMissionCB->cbBase.cbInfo.env, undefined, callback, 1, &result, &callResult);

    napi_close_handle_scope(registerMissionCB->cbBase.cbInfo.env, scope);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    delete work;
    TAG_LOGI(AAFwkTag::MISSION, "uv_queue_work end");
}

static void BuildCallbackResult(OnCB *onCB, napi_value result[3])
{
    TAG_LOGI(AAFwkTag::MISSION, "BuildCallbackResult");
    napi_create_int32(onCB->cbBase.cbInfo.env, onCB->continueState, &result[0]);
    napi_create_object(onCB->cbBase.cbInfo.env, &result[1]);
    napi_create_object(onCB->cbBase.cbInfo.env, &result[ARGS_TWO]);

    std::string napiValue1 = onCB->srcDeviceId;
    std::string napiValue2 = onCB->bundleName;
    std::string napiValue3 = onCB->continueType;
    std::string napiValue4 = onCB->srcBundleName;
    napi_value jsValueArr[PARAM4] = {nullptr};
    napi_create_string_utf8(onCB->cbBase.cbInfo.env, napiValue1.c_str(), NAPI_AUTO_LENGTH, &jsValueArr[0]);
    napi_create_string_utf8(onCB->cbBase.cbInfo.env, napiValue2.c_str(), NAPI_AUTO_LENGTH, &jsValueArr[1]);
    napi_create_string_utf8(onCB->cbBase.cbInfo.env, napiValue3.c_str(), NAPI_AUTO_LENGTH, &jsValueArr[ARGS_TWO]);
    napi_create_string_utf8(onCB->cbBase.cbInfo.env, napiValue4.c_str(), NAPI_AUTO_LENGTH, &jsValueArr[ARGS_THREE]);

    std::string paramName1 = "srcDeviceId";
    std::string paramName2 = "bundleName";
    std::string paramName3 = "continueType";
    std::string paramName4 = "srcBundleName";
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[1], paramName1.c_str(), jsValueArr[0]);
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[1], paramName2.c_str(), jsValueArr[1]);
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[1], paramName3.c_str(), jsValueArr[ARGS_TWO]);
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[1], paramName4.c_str(), jsValueArr[ARGS_THREE]);

    TAG_LOGI(AAFwkTag::MISSION, "BuildCallbackResult: continueState=%{public}u, srcDeviceId=%{public}s, "
                                "bundleName=%{public}s, continueType=%{public}s, srcBundleName=%{public}s",
             onCB->continueState, onCB->srcDeviceId.c_str(), onCB->bundleName.c_str(),
             onCB->continueType.c_str(), onCB->srcBundleName.c_str());

    napi_value jsAppIdsArray = nullptr;
    napi_create_array(onCB->cbBase.cbInfo.env, &jsAppIdsArray);
    for (size_t i = 0; i < onCB->appIdentifiers.size(); ++i) {
        napi_value jsAppId = nullptr;
        napi_create_string_utf8(onCB->cbBase.cbInfo.env, onCB->appIdentifiers[i].c_str(),
            NAPI_AUTO_LENGTH, &jsAppId);
        napi_set_element(onCB->cbBase.cbInfo.env, jsAppIdsArray, i, jsAppId);
        TAG_LOGI(AAFwkTag::MISSION, "BuildCallbackResult: appIdentifiers[%{public}zu]=%{public}s",
                 i, GetAnonymStr(onCB->appIdentifiers[i]).c_str());
    }
    // Always set appIds property, even if empty
    std::string paramName5 = "targetAppIds";
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[1], paramName5.c_str(), jsAppIdsArray);
    std::string napiState = "state";
    std::string napiInfo = "info";
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[ARGS_TWO], napiState.c_str(), result[0]);
    napi_set_named_property(onCB->cbBase.cbInfo.env, result[ARGS_TWO], napiInfo.c_str(), result[1]);
}

void UvWorkOnCallback(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::MISSION, "uv_queue_work");
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null work");
        return;
    }
    OnCB *onCB = static_cast<OnCB *>(work->data);
    if (onCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null onCB");
        delete work;
        return;
    }

    AbilityRuntime::HandleScope handleScope(onCB->cbBase.cbInfo.env);
    napi_value result[3] = {nullptr};
    BuildCallbackResult(onCB, result);

    for (auto ele = onCB->cbBase.cbInfo.vecCallbacks.begin(); ele != onCB->cbBase.cbInfo.vecCallbacks.end(); ++ele) {
        napi_value undefined = nullptr;
        napi_get_undefined(onCB->cbBase.cbInfo.env, &undefined);
        napi_value callResult = nullptr;
        napi_call_function(onCB->cbBase.cbInfo.env, undefined,
            (*ele)->GetNapiValue(), ARGS_ONE, &result[ARGS_TWO], &callResult);
    }
    delete onCB;
    onCB = nullptr;
    delete work;
    TAG_LOGI(AAFwkTag::MISSION, "uv_queue_work end");
}

void NAPIRemoteMissionListener::NotifyMissionsChanged(const std::string &deviceId)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null loop");
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto registerMissionCB = new (std::nothrow) RegisterMissionCB;
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        delete work;
        return;
    }
    registerMissionCB->cbBase.cbInfo.env = env_;
    registerMissionCB->cbBase.cbInfo.callback = notifyMissionsChangedRef_;
    registerMissionCB->deviceId = deviceId;
    work->data = static_cast<void *>(registerMissionCB);

    int rev = uv_queue_work_with_qos_internal(
        loop, work, [](uv_work_t *work) {}, UvWorkNotifyMissionChanged, uv_qos_user_initiated,
        "distributedsched:NotifyMissionsChanged");
    if (rev != 0) {
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
}

void NAPIRemoteOnListener::OnCallback(const OnCallbackInfo &info)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null loop");
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto onCB = new (std::nothrow) OnCB;
    if (onCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null onCB");
        delete work;
        return;
    }
    for (auto ele = callbacks_.begin(); ele != callbacks_.end(); ++ele) {
        onCB->cbBase.cbInfo.vecCallbacks.push_back(*ele);
    }
    onCB->cbBase.cbInfo.env = env_;
    onCB->continueState = info.continueState;
    onCB->srcDeviceId = info.srcDeviceId;
    onCB->bundleName = info.bundleName;
    onCB->continueType = info.continueType;
    onCB->srcBundleName = info.srcBundleName;
    onCB->appIdentifiers = info.appIdentifiers;
    work->data = static_cast<void *>(onCB);

    int rev = uv_queue_work_with_qos_internal(
        loop, work, [](uv_work_t *work) {}, UvWorkOnCallback, uv_qos_user_initiated, "distributedsched:OnCallback");
    if (rev != 0) {
        delete onCB;
        onCB = nullptr;
        delete work;
    }
    TAG_LOGI(AAFwkTag::MISSION, "OnCallback end");
}

void UvWorkNotifySnapshot(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null work");
        return;
    }
    RegisterMissionCB *registerMissionCB = static_cast<RegisterMissionCB *>(work->data);
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        delete work;
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_status scopeStatus = napi_open_handle_scope(registerMissionCB->cbBase.cbInfo.env, &scope);
    if (scopeStatus != napi_ok || scope == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "napi_open_handle_scope failed");
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
        return;
    }

    napi_value result[2] = {nullptr};
    result[0] =
        WrapString(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->deviceId.c_str(), "deviceId");
    result[1] =
        CreateInt32(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->missionId, "missionId");
    CallbackReturn(&result[0], registerMissionCB);

    napi_close_handle_scope(registerMissionCB->cbBase.cbInfo.env, scope);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    delete work;
    TAG_LOGI(AAFwkTag::MISSION, "uv_queue_work end");
}

void CallbackReturn(napi_value *result, RegisterMissionCB *registerMissionCB)
{
    AbilityRuntime::HandleScope handleScope(registerMissionCB->cbBase.cbInfo.env);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(registerMissionCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(
        registerMissionCB->cbBase.cbInfo.env, registerMissionCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(registerMissionCB->cbBase.cbInfo.env, undefined, callback, ARGS_TWO, &result[0], &callResult);
}

void NAPIRemoteMissionListener::NotifySnapshot(const std::string &deviceId, int32_t missionId)
{
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null loop");
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto registerMissionCB = new (std::nothrow) RegisterMissionCB;
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        delete work;
        return;
    }
    registerMissionCB->cbBase.cbInfo.env = env_;
    registerMissionCB->cbBase.cbInfo.callback = notifySnapshotRef_;
    registerMissionCB->deviceId = deviceId;
    registerMissionCB->missionId = missionId;
    work->data = static_cast<void *>(registerMissionCB);

    int rev = uv_queue_work_internal(
        loop, work, [](uv_work_t *work) {}, UvWorkNotifySnapshot, "distributedsched:NotifySnapshot");
    if (rev != 0) {
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
    }
    TAG_LOGI(AAFwkTag::MISSION, "NotifySnapshot end");
}

void UvWorkNotifyNetDisconnect(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::MISSION, "begin, uv_queue_work");
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null work");
        return;
    }
    RegisterMissionCB *registerMissionCB = static_cast<RegisterMissionCB *>(work->data);
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        delete work;
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_status scopeStatus = napi_open_handle_scope(registerMissionCB->cbBase.cbInfo.env, &scope);
    if (scopeStatus != napi_ok || scope == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "napi_open_handle_scope failed");
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
        return;
    }

    napi_value result[2] = {nullptr};
    result[0] =
        WrapString(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->deviceId.c_str(), "deviceId");
    TAG_LOGI(AAFwkTag::MISSION, "state: %{public}d", registerMissionCB->state);
    result[1] =
        CreateInt32(registerMissionCB->cbBase.cbInfo.env, registerMissionCB->state, "state");

    CallbackReturn(&result[0], registerMissionCB);

    napi_close_handle_scope(registerMissionCB->cbBase.cbInfo.env, scope);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    delete work;
    TAG_LOGI(AAFwkTag::MISSION, "uv_queue_work end");
}

void NAPIRemoteMissionListener::NotifyNetDisconnect(const std::string &deviceId, int32_t state)
{
    TAG_LOGI(AAFwkTag::MISSION, "called. state: %{public}d", state);
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null loop");
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto registerMissionCB = new (std::nothrow) RegisterMissionCB;
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        delete work;
        return;
    }
    registerMissionCB->cbBase.cbInfo.env = env_;
    registerMissionCB->cbBase.cbInfo.callback = notifyNetDisconnectRef_;
    registerMissionCB->deviceId = deviceId;
    registerMissionCB->state = state;
    work->data = static_cast<void *>(registerMissionCB);

    int rev = uv_queue_work_internal(
        loop, work, [](uv_work_t *work) {}, UvWorkNotifyNetDisconnect, "distributedsched:NotifyNetDisconnect");
    if (rev != 0) {
        delete registerMissionCB;
        registerMissionCB = nullptr;
        delete work;
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
}

void UnRegisterMissionExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    auto registerMissionCB = (RegisterMissionCB*)data;

    std::lock_guard<std::mutex> autoLock(registrationLock_);
    sptr<NAPIRemoteMissionListener> registration;
    auto item = registration_.find(registerMissionCB->deviceId);
    if (item != registration_.end()) {
        TAG_LOGI(AAFwkTag::MISSION, "registration exits");
        registration = registration_[registerMissionCB->deviceId];
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "registration not exits");
        registerMissionCB->result = INVALID_PARAMETERS_ERR;
        return;
    }
    registerMissionCB->missionRegistration = registration;

    registerMissionCB->result =
        AbilityManagerClient::GetInstance()->
        UnRegisterMissionListener(registerMissionCB->deviceId,
        registerMissionCB->missionRegistration);
    if (registerMissionCB->result == NO_ERROR) {
        TAG_LOGI(AAFwkTag::MISSION, "remove registration");
        registration_.erase(registerMissionCB->deviceId);
    }
    TAG_LOGD(AAFwkTag::MISSION, "end.deviceId:%{public}d", registerMissionCB->result);
}

void UnRegisterMissionPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleScope handleScope(env);
    auto registerMissionCB = (RegisterMissionCB*)data;
    // set result
    napi_value result[2] = { nullptr };
    napi_get_undefined(env, &result[1]);
    if (registerMissionCB->result == 0) {
        napi_get_undefined(env, &result[0]);
    } else {
        int32_t errCode = ErrorCodeReturn(registerMissionCB->result);
        result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
    }

    ReturnValueToApplication(env, &result[0], registerMissionCB);
    delete registerMissionCB;
    registerMissionCB = nullptr;
    TAG_LOGI(AAFwkTag::MISSION, "end");
}

napi_value UnRegisterMissionPromise(napi_env env, RegisterMissionCB *registerMissionCB)
{
    TAG_LOGI(AAFwkTag::MISSION, "asyncCallback");
    AbilityRuntime::HandleEscape handleEscape(env);
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null param");
        return nullptr;
    }
    napi_value promise = nullptr;
    if (registerMissionCB->callbackRef == nullptr) {
        napi_create_promise(env, &registerMissionCB->cbBase.deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        UnRegisterMissionExecuteCB,
        UnRegisterMissionPromiseCompletedCB,
        static_cast<void *>(registerMissionCB),
        &registerMissionCB->cbBase.asyncWork);
    napi_queue_async_work(env, registerMissionCB->cbBase.asyncWork);
    TAG_LOGI(AAFwkTag::MISSION, "asyncCallback end");
    return handleEscape.Escape(promise);
}

bool GetUnRegisterMissionDeviceId(napi_env &env, const napi_value &value,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleScope handleScope(env);
    napi_value napiDeviceId = nullptr;
    napi_valuetype valueType = napi_undefined;
    bool isDeviceId = false;
    napi_has_named_property(env, value, "deviceId", &isDeviceId);
    napi_typeof(env, value, &valueType);
    if (isDeviceId && valueType == napi_object) {
        napi_get_named_property(env, value, "deviceId", &napiDeviceId);
    } else {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong deviceId argument name");
        errInfo = "Parameter error. The key of \"MissionDeviceInfo\" must be deviceId";
        return false;
    }
    if (napiDeviceId == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "not find deviceId");
        errInfo = "Parameter error. The value of \"deviceId\" must not be undefined";
        return false;
    }

    size_t valueLen = 0;
    napi_typeof(env, napiDeviceId, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::MISSION, " Wrong argument type");
        errInfo = "Parameter error. The type of \"deviceId\" must be string";
        return false;
    }
    char deviceId[VALUE_BUFFER_SIZE + 1] = {0};
    napi_get_value_string_utf8(env, napiDeviceId, deviceId, VALUE_BUFFER_SIZE + 1, &valueLen);
    if (valueLen > VALUE_BUFFER_SIZE) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId length not correct");
        errInfo = "Parameter error. The length of \"deviceId\" must be less than " +
            std::to_string(VALUE_BUFFER_SIZE);
        return false;
    }
    registerMissionCB->deviceId = std::string(deviceId);
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return true;
}

napi_value UnRegisterMissionWrap(napi_env &env, napi_callback_info info,
    RegisterMissionCB *registerMissionCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    size_t argc = 2;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    TAG_LOGI(AAFwkTag::MISSION, "argc is %{public}zu", argc);
    if (argc != ARGS_ONE && argc != ARGS_TWO) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument count");
        errInfo = "Parameter error. The type of \"number of parameters\" must be 1 or 2";
        return nullptr;
    }

    if (!GetUnRegisterMissionDeviceId(env, args[0], registerMissionCB, errInfo)) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument");
        return nullptr;
    }

    if (argc == ARGS_TWO) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[1], &valueType);
        if (valueType != napi_function) {
            TAG_LOGE(AAFwkTag::MISSION, "callback error type");
            errInfo = "Parameter error. The type of \"callback\" must be AsynCallback<void>: void";
            return nullptr;
        }
        napi_create_reference(env, args[1], 1, &registerMissionCB->callbackRef);
    }
    ret = UnRegisterMissionPromise(env, registerMissionCB);
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return handleEscape.Escape(ret);
}

napi_value NAPI_UnRegisterMissionListener(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    std::string errInfo = "Parameter error";
    RegisterMissionCB *registerMissionCB = CreateRegisterMissionCBCBInfo(env);
    if (registerMissionCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null registerMissionCB");
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = UnRegisterMissionWrap(env, info, registerMissionCB, errInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null ret");
        delete registerMissionCB;
        registerMissionCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return handleEscape.Escape(ret);
}

napi_value WrapString(napi_env &env, const std::string &param, const std::string &paramName)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");

    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value jsValue = nullptr;
    TAG_LOGD(AAFwkTag::MISSION, "called. %{public}s = %{public}s",
        paramName.c_str(), param.c_str());
    napi_create_string_utf8(env, param.c_str(), NAPI_AUTO_LENGTH, &jsValue);

    return handleEscape.Escape(jsValue);
}

napi_value WrapInt32(napi_env &env, int32_t num, const std::string &paramName)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");

    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);

    napi_value jsValue = nullptr;
    TAG_LOGD(AAFwkTag::MISSION, "called. %{public}s = %{public}d", paramName.c_str(), num);
    napi_create_int32(env, num, &jsValue);
    napi_set_named_property(env, jsObject, paramName.c_str(), jsValue);

    return handleEscape.Escape(jsObject);
}

napi_value CreateInt32(napi_env &env, int32_t num, const std::string &paramName)
{
    TAG_LOGD(AAFwkTag::MISSION, "called. %{public}s = %{public}d", paramName.c_str(), num);

    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value jsValue = nullptr;
    napi_create_int32(env, num, &jsValue);

    return handleEscape.Escape(jsValue);
}

ContinueAbilityCB *CreateContinueAbilityCBCBInfo(napi_env &env)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    auto continueAbilityCB = new (std::nothrow) ContinueAbilityCB;
    if (continueAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null continueAbilityCB");
        return nullptr;
    }
    continueAbilityCB->cbBase.cbInfo.env = env;
    continueAbilityCB->cbBase.asyncWork = nullptr;
    continueAbilityCB->cbBase.deferred = nullptr;
    continueAbilityCB->callbackRef = nullptr;
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return continueAbilityCB;
}

void ContinueAbilityExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    auto continueAbilityCB = static_cast<ContinueAbilityCB *>(data);
    TAG_LOGI(AAFwkTag::MISSION, "create continueAbilityCB success.");
    sptr<NAPIMissionContinue> continuation(new (std::nothrow) NAPIMissionContinue());
    continueAbilityCB->abilityContinuation = continuation;
    if (continueAbilityCB->abilityContinuation == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null abilityContinuation");
        return;
    }
    continueAbilityCB->abilityContinuation->SetContinueAbilityEnv(env);
    TAG_LOGI(AAFwkTag::MISSION, "set env success");
    if (continueAbilityCB->abilityContinuationCB.callback[0] != nullptr) {
        continueAbilityCB->abilityContinuation->
            SetContinueAbilityCBRef(continueAbilityCB->abilityContinuationCB.callback[0]);
        TAG_LOGI(AAFwkTag::MISSION, "set callback success");
    } else {
        continueAbilityCB->abilityContinuation->
            SetContinueAbilityPromiseRef(continueAbilityCB->cbBase.deferred);
        TAG_LOGI(AAFwkTag::MISSION, "set promise success");
    }

    continueAbilityCB->result = -1;
    continueAbilityCB->abilityContinuation->SetContinueAbilityHasBundleName(continueAbilityCB->hasArgsWithBundleName);
    if (continueAbilityCB->hasArgsWithBundleName) {
        ContinueMissionInfo continueMissionInfo;
        continueMissionInfo.dstDeviceId = continueAbilityCB->dstDeviceId;
        continueMissionInfo.srcDeviceId = continueAbilityCB->srcDeviceId;
        continueMissionInfo.bundleName = continueAbilityCB->bundleName;
        continueMissionInfo.srcBundleName = continueAbilityCB->srcBundleName;
        continueMissionInfo.continueType = continueAbilityCB->continueType;
        continueMissionInfo.wantParams = continueAbilityCB->wantParams;
        continueAbilityCB->result = AAFwk::AbilityManagerClient::GetInstance()->
        ContinueMission(continueMissionInfo, continueAbilityCB->abilityContinuation);
    } else {
        continueAbilityCB->result = AAFwk::AbilityManagerClient::GetInstance()->
        ContinueMission(continueAbilityCB->srcDeviceId, continueAbilityCB->dstDeviceId,
        continueAbilityCB->missionId, continueAbilityCB->abilityContinuation,
        continueAbilityCB->wantParams);
    }
    TAG_LOGI(AAFwkTag::MISSION, "end. error:%{public}d ", continueAbilityCB->result);
}

void ContinueAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleScope handleScope(env);
    auto continueAbilityCB = static_cast<ContinueAbilityCB *>(data);
    // set result
    napi_value result[2] = { nullptr };
    napi_get_undefined(env, &result[1]);
    if (continueAbilityCB->result == 0) {
        napi_get_undefined(env, &result[0]);
    } else {
        int32_t errCode = ErrorCodeReturn(continueAbilityCB->result);
        result[0] = GenerateBusinessError(env, errCode, ErrorMessageReturn(errCode));
    }
    if (!continueAbilityCB->hasArgsWithBundleName) {
        if (continueAbilityCB->callbackRef == nullptr) { // promise
            if (continueAbilityCB->result == 0) {
                napi_resolve_deferred(env, continueAbilityCB->cbBase.deferred, result[1]);
            } else {
                napi_reject_deferred(env, continueAbilityCB->cbBase.deferred, result[0]);
            }
        } else { // AsyncCallback
            napi_value callback = nullptr;
            napi_get_reference_value(env, continueAbilityCB->callbackRef, &callback);
            napi_value callResult;
            napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
            napi_delete_reference(env, continueAbilityCB->callbackRef);
        }
    } else {
        if (continueAbilityCB->callbackRef == nullptr && continueAbilityCB->result != 0) { // promise
            napi_reject_deferred(env, continueAbilityCB->cbBase.deferred, result[0]);
        } else if (continueAbilityCB->callbackRef != nullptr && continueAbilityCB->result != 0) { // AsyncCallback
            napi_value callback = nullptr;
            napi_get_reference_value(env, continueAbilityCB->callbackRef, &callback);
            napi_value callResult;
            napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
            napi_delete_reference(env, continueAbilityCB->callbackRef);
        }
    }
    napi_delete_async_work(env, continueAbilityCB->cbBase.asyncWork);
    delete continueAbilityCB;
    continueAbilityCB = nullptr;
    TAG_LOGI(AAFwkTag::MISSION, "end");
}

napi_value ContinueAbilityAsync(napi_env env, ContinueAbilityCB *continueAbilityCB)
{
    TAG_LOGI(AAFwkTag::MISSION, "asyncCallback");
    AbilityRuntime::HandleEscape handleEscape(env);
    if (continueAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null param");
        return nullptr;
    }

    napi_value result = nullptr;
    if (continueAbilityCB->callbackRef == nullptr) {
        napi_create_promise(env, &continueAbilityCB->cbBase.deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "ContinueAbilityAsyncForLauncher", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        ContinueAbilityExecuteCB,
        ContinueAbilityCallbackCompletedCB,
        static_cast<void *>(continueAbilityCB),
        &continueAbilityCB->cbBase.asyncWork);
    napi_queue_async_work_with_qos(env, continueAbilityCB->cbBase.asyncWork, napi_qos_user_initiated);
    TAG_LOGI(AAFwkTag::MISSION, "asyncCallback end");
    return handleEscape.Escape(result);
}

bool CheckContinueDeviceInfoSrcDeviceId(napi_env &env, napi_value &napiSrcDeviceId,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiSrcDeviceId, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::MISSION, "srcDeviceId invalid type");
        errInfo = "Parameter error. The type of \"srcDeviceId\" must be string";
        return false;
    }
    continueAbilityCB->srcDeviceId = AppExecFwk::UnwrapStringFromJS(env, napiSrcDeviceId, "");
    return true;
}

bool CheckContinueDeviceInfoDstDeviceId(napi_env &env, napi_value &napiDstDeviceId,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiDstDeviceId, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::MISSION, "dstDeviceId invalid type");
        errInfo = "Parameter error. The type of \"dstDeviceId\" must be string";
        return false;
    }
    continueAbilityCB->dstDeviceId = AppExecFwk::UnwrapStringFromJS(env, napiDstDeviceId, "");
    return true;
}

bool CheckContinueDeviceInfoMissionId(napi_env &env, napi_value &napiMissionId,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiMissionId, &valueType);
    if (valueType != napi_number) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId invalid type");
        errInfo = "Parameter error. The type of \"missionId\" must be number";
        return false;
    }
    continueAbilityCB->missionId = AppExecFwk::UnwrapInt32FromJS(env, napiMissionId, -1);
    return true;
}

bool CheckContinueDeviceInfoBundleName(napi_env &env, napi_value &napiBundleName,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiBundleName, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId invalid type");
        errInfo = "Parameter error. The type of \"bundleName\" must be string";
        return false;
    }
    continueAbilityCB->bundleName = AppExecFwk::UnwrapStringFromJS(env, napiBundleName, "");
    return true;
}

bool CheckContinueDeviceInfoSrcBundleName(napi_env &env, napi_value &napiSrcBundleName,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiSrcBundleName, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId invalid type");
        errInfo = "Parameter error. The type of \"bundleName\" must be string";
        return false;
    }
    continueAbilityCB->srcBundleName = AppExecFwk::UnwrapStringFromJS(env, napiSrcBundleName, "");
    return true;
}

bool CheckContinueDeviceInfoContinueType(napi_env &env, napi_value &napiContinueType,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiContinueType, &valueType);
    if (valueType != napi_string) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId invalid type");
        errInfo = "Parameter error. The type of \"bundleName\" must be string";
        return false;
    }
    continueAbilityCB->continueType = AppExecFwk::UnwrapStringFromJS(env, napiContinueType, "");
    return true;
}

bool CheckContinueDeviceInfoWantParam(napi_env &env, napi_value &napiWantParam,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, napiWantParam, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::MISSION, "wantParam invalid type");
        errInfo = "Parameter error. The type of \"wantParams\" must be object";
        return false;
    }
    if (!AppExecFwk::UnwrapWantParams(env, napiWantParam, continueAbilityCB->wantParams)) {
        TAG_LOGE(AAFwkTag::MISSION, "wantParam invalid type");
        errInfo = "Parameter error. The type of \"wantParams\" must be array";
        return false;
    }
    return true;
}

bool CheckContinueFirstArgs(napi_env &env, const napi_value &value,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleScope handleScope(env);
    if (!CheckContinueKeyExist(env, value)) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument key");
        errInfo = "Parameter error. The type of \"parameter\" must be ContinueMission";
        return false;
    }
    napi_value napiSrcDeviceId = nullptr;
    napi_value napiDstDeviceId = nullptr;
    napi_value napiMissionId = nullptr;
    napi_value napiWantParam = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument type");
        errInfo = "Parameter error. The type of \"parameter\" must be ContinueMission";
        return false;
    }
    napi_get_named_property(env, value, "srcDeviceId", &napiSrcDeviceId);
    napi_get_named_property(env, value, "dstDeviceId", &napiDstDeviceId);
    napi_get_named_property(env, value, "missionId", &napiMissionId);
    napi_get_named_property(env, value, "wantParam", &napiWantParam);
    if (napiSrcDeviceId == nullptr || napiDstDeviceId == nullptr ||
        napiMissionId == nullptr || napiWantParam == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "miss required parameters");
        errInfo = "Parameter error. The number of \"ContinueMission\" must be 4";
        return false;
    }
    if (!CheckContinueDeviceInfoSrcDeviceId(env, napiSrcDeviceId, continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoDstDeviceId(env, napiDstDeviceId, continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoMissionId(env, napiMissionId, continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoWantParam(env, napiWantParam, continueAbilityCB, errInfo)) {
        TAG_LOGE(AAFwkTag::MISSION, "continueMission check ContinueDeviceInfo failed");
        return false;
    }
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return true;
}

bool CheckArgsWithBundleName(napi_env &env, const napi_value &value,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleScope handleScope(env);
    if (!CheckBundleNameExist(env, value)) {
        TAG_LOGE(AAFwkTag::MISSION, "Args without bundleName");
        return false;
    }
    napi_value napiValue[ARGS_SIX] = {nullptr};
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::MISSION, "Args without bundleName");
        return false;
    }
    napi_get_named_property(env, value, "srcDeviceId", &napiValue[ARGS_ZERO]);
    napi_get_named_property(env, value, "dstDeviceId", &napiValue[ARGS_ONE]);
    napi_get_named_property(env, value, "bundleName", &napiValue[ARGS_TWO]);
    napi_get_named_property(env, value, "wantParam", &napiValue[ARGS_THREE]);
    napi_get_named_property(env, value, "srcBundleName", &napiValue[ARGS_FOUR]);
    napi_get_named_property(env, value, "continueType", &napiValue[ARGS_FIVE]);
    if (napiValue[ARGS_ZERO] == nullptr || napiValue[ARGS_ONE] == nullptr ||
        napiValue[ARGS_TWO] == nullptr || napiValue[ARGS_THREE] == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "miss required parameters");
        return false;
    }
    CheckContinueDeviceInfoContinueType(env, napiValue[ARGS_FIVE], continueAbilityCB, errInfo);
    CheckContinueDeviceInfoSrcBundleName(env, napiValue[ARGS_FOUR], continueAbilityCB, errInfo);
    if (!CheckContinueDeviceInfoSrcDeviceId(env, napiValue[ARGS_ZERO], continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoDstDeviceId(env, napiValue[ARGS_ONE], continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoBundleName(env, napiValue[ARGS_TWO], continueAbilityCB, errInfo) ||
        !CheckContinueDeviceInfoWantParam(env, napiValue[ARGS_THREE], continueAbilityCB, errInfo)) {
        TAG_LOGE(AAFwkTag::MISSION, "continueMission check ContinueDeviceInfo failed");
        return false;
    }
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return true;
}

bool CheckContinueCallback(napi_env &env, const napi_value &value,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleScope handleScope(env);
    napi_value jsMethod = nullptr;
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, value, &valuetype);
    if (valuetype != napi_object) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument type");
        errInfo = "Parameter error. The type of \"options\" must be ContinueCallback";
        return false;
    }
    bool isFirstCallback = false;
    napi_has_named_property(env, value, "onContinueDone", &isFirstCallback);
    if (!isFirstCallback) {
        TAG_LOGE(AAFwkTag::MISSION, "invalid onContinueDone name");
        errInfo = "Parameter error. The key of \"ContinueCallback\" must be onContinueDone";
        return false;
    }
    napi_get_named_property(env, value, "onContinueDone", &jsMethod);
    if (jsMethod == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "not find onContinueDone");
        errInfo = "Parameter error. The value of \"onContinueDone\" must not be undefined";
        return false;
    }
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        TAG_LOGE(AAFwkTag::MISSION, "onContinueDone error type");
        errInfo = "Parameter error. The type of \"onContinueDone\" must be function";
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &continueAbilityCB->abilityContinuationCB.callback[0]);
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return true;
}

bool CheckContinueCallbackWithBundleName(napi_env &env, const napi_value &value,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_function) {
        TAG_LOGE(AAFwkTag::MISSION, "Wrong argument type");
        return false;
    }
    napi_create_reference(env, value, 1, &continueAbilityCB->abilityContinuationCB.callback[0]);
    napi_create_reference(env, value, 1, &continueAbilityCB->callbackRef);
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return true;
}

napi_value ContinueAbilityWrap(napi_env &env, napi_callback_info info,
    ContinueAbilityCB *continueAbilityCB, std::string &errInfo)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    size_t argcAsync = 3;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    TAG_LOGI(AAFwkTag::MISSION, "argcAsync is %{public}zu", argcAsync);

    if (argcAsync != ARGS_ONE && argcAsync != ARGS_TWO && argcAsync != ARGS_THREE) {
        TAG_LOGE(AAFwkTag::MISSION, "invalid argc");
        errInfo = "Parameter error. The type of \"number of parameters\" must be 1 or 2 or 3";
        return nullptr;
    }

    if (CheckArgsWithBundleName(env, args[0], continueAbilityCB, errInfo)) {
        continueAbilityCB->hasArgsWithBundleName = true;
        if (argcAsync == ARGS_TWO && CheckContinueCallbackWithBundleName(env, args[1], continueAbilityCB, errInfo)) {
            ret = ContinueAbilityAsync(env, continueAbilityCB);
            TAG_LOGI(AAFwkTag::MISSION, "called end");
            return ret;
        }
    }

    if (!continueAbilityCB->hasArgsWithBundleName) {
        if (!CheckContinueFirstArgs(env, args[0], continueAbilityCB, errInfo)) {
            TAG_LOGE(AAFwkTag::MISSION, "check the first argument failed");
            return nullptr;
        }

        if (argcAsync > 1) {
            if (!CheckContinueCallback(env, args[1], continueAbilityCB, errInfo)) {
                TAG_LOGE(AAFwkTag::MISSION, "check callback failed");
                return nullptr;
            }
        }

        if (argcAsync == ARGS_THREE) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, args[ARGS_TWO], &valueType);
            if (valueType != napi_function) {
                TAG_LOGE(AAFwkTag::MISSION, "callback error type");
                errInfo = "Parameter error. The type of \"callback\" must be AsynCallback<void>: void";
                return nullptr;
            }
            napi_create_reference(env, args[ARGS_TWO], 1, &continueAbilityCB->callbackRef);
        }
    }

    ret = ContinueAbilityAsync(env, continueAbilityCB);
    TAG_LOGI(AAFwkTag::MISSION, "called end");
    return handleEscape.Escape(ret);
}

napi_value NAPI_ContinueAbility(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    std::string errInfo = "Parameter error";
    ContinueAbilityCB *continueAbilityCB = CreateContinueAbilityCBCBInfo(env);
    if (continueAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null continueAbilityCB");
        napi_throw(env, GenerateBusinessError(env, SYSTEM_WORK_ABNORMALLY, ErrorMessageReturn(SYSTEM_WORK_ABNORMALLY)));
        return GetUndefined(env);
    }

    napi_value ret = ContinueAbilityWrap(env, info, continueAbilityCB, errInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null ret");
        delete continueAbilityCB;
        continueAbilityCB = nullptr;
        napi_throw(env, GenerateBusinessError(env, PARAMETER_CHECK_FAILED, errInfo));
        return GetUndefined(env);
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
    return handleEscape.Escape(ret);
}

ContinueAbilityCB *CheckAndGetParameters(uv_work_t *work, napi_handle_scope *scope)
{
    TAG_LOGI(AAFwkTag::MISSION, "start");
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null work");
        return nullptr;
    }
    ContinueAbilityCB *continueAbilityCB = static_cast<ContinueAbilityCB *>(work->data);
    if (continueAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null continueAbilityCB");
        delete work;
        return nullptr;
    }
    napi_status scopeStatus = napi_open_handle_scope(continueAbilityCB->cbBase.cbInfo.env, scope);
    if (scopeStatus != napi_ok || scope == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "napi_open_handle_scope failed");
        delete continueAbilityCB;
        continueAbilityCB = nullptr;
        delete work;
        return nullptr;
    }
    return continueAbilityCB;
}

void UvWorkOnContinueDone(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::MISSION, "uv_queue_work");
    napi_handle_scope scope = nullptr;
    ContinueAbilityCB *continueAbilityCB = CheckAndGetParameters(work, &scope);
    if (continueAbilityCB == nullptr) {
        return;
    }
    TAG_LOGI(AAFwkTag::MISSION, "resultCode: %{public}d", continueAbilityCB->resultCode);
    napi_value result = WrapInt32(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->resultCode, "resultCode");
    if (continueAbilityCB->hasArgsWithBundleName) {
        result = WrapInt32(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->resultCode, "code");
    }
    if (continueAbilityCB->cbBase.deferred == nullptr) {
        std::lock_guard<std::mutex> autoLock(registrationLock_);
        napi_value callback = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(continueAbilityCB->cbBase.cbInfo.env, &undefined);
        napi_value callResult = nullptr;
        napi_get_reference_value(continueAbilityCB->cbBase.cbInfo.env,
            continueAbilityCB->cbBase.cbInfo.callback, &callback);
        napi_call_function(continueAbilityCB->cbBase.cbInfo.env, undefined, callback, 1, &result, &callResult);
        if (continueAbilityCB->cbBase.cbInfo.callback != nullptr) {
            napi_delete_reference(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->cbBase.cbInfo.callback);
            continueAbilityCB->cbBase.cbInfo.callback = nullptr;
        }
    } else {
        napi_value result[2] = { nullptr };
        napi_get_undefined(continueAbilityCB->cbBase.cbInfo.env, &result[1]);
        if (continueAbilityCB->resultCode == 0) {
            napi_resolve_deferred(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->cbBase.deferred, result[1]);
        } else {
            result[0] = GenerateBusinessError(continueAbilityCB->cbBase.cbInfo.env,
                continueAbilityCB->resultCode, ErrorMessageReturn(continueAbilityCB->resultCode));
            napi_reject_deferred(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->cbBase.deferred, result[0]);
        }
    }
    napi_close_handle_scope(continueAbilityCB->cbBase.cbInfo.env, scope);
    delete continueAbilityCB;
    continueAbilityCB = nullptr;
    delete work;
    TAG_LOGI(AAFwkTag::MISSION, "uv_queue_work end");
}

void NAPIMissionContinue::OnContinueDone(int32_t result)
{
    TAG_LOGI(AAFwkTag::MISSION, "called. result = %{public}d", result);
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null loop");
        return;
    }

    uv_work_t *work = new uv_work_t;

    auto continueAbilityCB = new (std::nothrow) ContinueAbilityCB;
    if (continueAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null continueAbilityCB");
        delete work;
        return;
    }
    continueAbilityCB->cbBase.cbInfo.env = env_;
    continueAbilityCB->hasArgsWithBundleName = onContinueDoneHasBundleName_;
    if (onContinueDoneRef_ != nullptr) {
        continueAbilityCB->cbBase.cbInfo.callback = onContinueDoneRef_;
    } else {
        continueAbilityCB->cbBase.deferred = promiseDeferred_;
    }
    continueAbilityCB->resultCode = result;
    work->data = static_cast<void *>(continueAbilityCB);

    int rev = uv_queue_work_with_qos_internal(
        loop, work, [](uv_work_t *work) {}, UvWorkOnContinueDone, uv_qos_user_initiated,
        "distributedsched:OnContinueDone");
    if (rev != 0) {
        delete continueAbilityCB;
        continueAbilityCB = nullptr;
        delete work;
    }
    TAG_LOGI(AAFwkTag::MISSION, "end");
}

napi_value DistributedMissionManagerExport(napi_env env, napi_value exports)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("startSyncRemoteMissions", NAPI_StartSyncRemoteMissions),
        DECLARE_NAPI_FUNCTION("stopSyncRemoteMissions", NAPI_StopSyncRemoteMissions),
        DECLARE_NAPI_FUNCTION("registerMissionListener", NAPI_RegisterMissionListener),
        DECLARE_NAPI_FUNCTION("unRegisterMissionListener", NAPI_UnRegisterMissionListener),
        DECLARE_NAPI_FUNCTION("continueMission", NAPI_ContinueAbility),
        DECLARE_NAPI_FUNCTION("on", NAPI_NotifyToOn),
        DECLARE_NAPI_FUNCTION("off", NAPI_NotifyToOff),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));
    return exports;
}

static napi_module missionModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DistributedMissionManagerExport,
    .nm_modname = "distributedMissionManager",
    .nm_priv = (static_cast<void*>(nullptr)),
    .reserved = {nullptr}
};

extern "C" __attribute__((constructor)) void AbilityRegister()
{
    napi_module_register(&missionModule);
}
}
}
