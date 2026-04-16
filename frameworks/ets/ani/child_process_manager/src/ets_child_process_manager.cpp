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

#include "ets_child_process_manager.h"

#include "ani_common_child_process_param.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "ani_task.h"
#include "child_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "ets_error_utils.h"

namespace OHOS {
namespace ChildProcessManagerEts {
namespace {
constexpr const char* CHILD_PROCESS_MANAGER_NAME_SPACE = "@ohos.app.ability.childProcessManager.childProcessManager";
constexpr size_t ETS_SUFFIX_LEN = 4;
enum {
    MODE_SELF_FORK = 0,
    MODE_APP_SPAWN_FORK = 1,
};
struct ChildProcessAniParam {
    std::string srcEntry;
    bool isStaticChildProcess = false;
    AppExecFwk::ChildProcessArgs args;
    AppExecFwk::ChildProcessOptions options;
    int32_t childProcessType;
};

void ProcessSrcEntry(std::string &srcEntry, bool &isStaticChildProcess)
{
    if (srcEntry.size() >= ETS_SUFFIX_LEN && srcEntry.substr(srcEntry.size() - ETS_SUFFIX_LEN) != ".ets") {
        isStaticChildProcess = true;
    } else {
        isStaticChildProcess = false;
    }
}
}

class EtsChildProcessManager {
public:
    EtsChildProcessManager() = default;
    ~EtsChildProcessManager() = default;

    static EtsChildProcessManager &GetInstance()
    {
        static EtsChildProcessManager instance;
        return instance;
    }

    static void StartChildProcess(ani_env *env, ani_string etsSrcEntry, ani_enum_item etsStartMode,
        ani_object callback)
    {
        GetInstance().OnStartChildProcess(env, etsSrcEntry, etsStartMode, callback);
    }

    static void StartArkChildProcess(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object callback)
    {
        GetInstance().OnStartArkChildProcess(env, etsSrcEntry, ChildProcessArgs, nullptr, callback);
    }

    static void StartArkChildProcessWithOptions(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object ChildProcessOptions, ani_object callback)
    {
        GetInstance().OnStartArkChildProcess(env, etsSrcEntry, ChildProcessArgs, ChildProcessOptions, callback);
    }

    static void StartNativeChildProcess(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object callback)
    {
        GetInstance().OnStartNativeChildProcess(env, etsSrcEntry, ChildProcessArgs, nullptr, callback);
    }

    static void StartNativeChildProcessWithOptions(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object ChildProcessOptions, ani_object callback)
    {
        GetInstance().OnStartNativeChildProcess(env, etsSrcEntry, ChildProcessArgs, ChildProcessOptions, callback);
    }

    static void StartChildProcessCheck(ani_env *env, ani_string etsSrcEntry, ani_enum_item etsStartMode)
    {
        GetInstance().OnStartChildProcessCheck(env, etsSrcEntry, etsStartMode);
    }

    static void StartArkChildProcessCheck(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object ChildProcessOptions)
    {
        GetInstance().OnStartArkChildProcessCheck(env, etsSrcEntry, ChildProcessArgs, ChildProcessOptions);
    }

    static void StartNativeChildProcessCheck(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object ChildProcessOptions)
    {
        GetInstance().OnStartNativeChildProcessCheck(env, etsSrcEntry, ChildProcessArgs, ChildProcessOptions);
    }

private:
    void OnStartChildProcess(ani_env *env, ani_string etsSrcEntry, ani_enum_item etsStartMode, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return;
        }
        if (AbilityRuntime::ChildProcessManager::GetInstance().IsChildProcess()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            AbilityRuntime::EtsErrorUtil::ThrowError(env,
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }
        std::string srcEntry;
        if (!AppExecFwk::GetStdString(env, etsSrcEntry, srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse srcEntry failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param srcEntry failed, must be a valid string.");
            return;
        }
        bool isStaticChildProcess = false;
        ProcessSrcEntry(srcEntry, isStaticChildProcess);
        TAG_LOGD(AAFwkTag::PROCESSMGR, "srcEntry %{public}s, isStaticChildProcess %{public}d", srcEntry.c_str(),
            isStaticChildProcess);
        int32_t startMode;
        if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, etsStartMode, startMode)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse startMode failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Unsupported startMode, must be StartMode.SELF_FORK or StartMode.APP_SPAWN_FORK.");
            return;
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "startMode %{public}d", startMode);
        if (startMode != MODE_SELF_FORK && startMode != MODE_APP_SPAWN_FORK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid StartMode");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Unsupported startMode, must be StartMode.SELF_FORK or StartMode.APP_SPAWN_FORK.");
            return;
        }
        if (startMode == MODE_SELF_FORK) {
            StartChildProcessSelfForkTask(env, srcEntry, callback, isStaticChildProcess);
        } else {
            StartChildProcessAppSpawnForkTask(env, srcEntry, callback, isStaticChildProcess);
        }
    }

    void StartChildProcessSelfForkTask(ani_env *env, std::string &etsSrcEntry, ani_object callback,
        bool isStaticChildProcess)
    {
        ani_status status = ANI_ERROR;
        ani_vm *aniVM = nullptr;
        if (env->GetVM(&aniVM) != ANI_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "get aniVM failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "get aniVM failed.");
            return;
        }
        ani_ref callbackRef = nullptr;
        if ((status = env->GlobalReference_Create(callback, &callbackRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "GlobalReference_Create callbackRef failed status: %{public}d", status);
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "GlobalReference_Create failed.");
            return;
        }
        auto task = [aniVM, etsSrcEntry, isStaticChildProcess, callbackRef]() {
            if (aniVM == nullptr || callbackRef == nullptr) {
                TAG_LOGE(AAFwkTag::PROCESSMGR, "aniVM or callbackRef is null");
                return;
            }
            bool isAttachThread = false;
            ani_env *aniEnv = AppExecFwk::AttachAniEnv(aniVM, isAttachThread);
            if (aniEnv == nullptr) {
                TAG_LOGE(AAFwkTag::PROCESSMGR, "null aniEnv");
                return;
            }
            pid_t pid = 0;
            auto errorCode = AbilityRuntime::ChildProcessManager::GetInstance().StartChildProcessBySelfFork(
                etsSrcEntry, pid, isStaticChildProcess);
            TAG_LOGD(AAFwkTag::PROCESSMGR, "StartChildProcessBySelfFork errorCode: %{public}d, pid: %{public}d",
                errorCode, pid);
            if (errorCode != AbilityRuntime::ChildProcessManagerErrorCode::ERR_OK) {
                TAG_LOGE(AAFwkTag::PROCESSMGR, "StartChildProcessBySelfFork failed, errorCode: %{public}d", errorCode);
                AppExecFwk::AsyncCallback(aniEnv, static_cast<ani_object>(callbackRef),
                    AbilityRuntime::EtsErrorUtil::CreateError(aniEnv,
                        AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(errorCode)), nullptr);
                aniEnv->GlobalReference_Delete(callbackRef);
                AppExecFwk::DetachAniEnv(aniVM, isAttachThread);
                return;
            }
            AppExecFwk::AsyncCallback(aniEnv, static_cast<ani_object>(callbackRef),
                AbilityRuntime::EtsErrorUtil::CreateError(aniEnv,
                    AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(errorCode)),
                AppExecFwk::CreateInt(aniEnv, static_cast<ani_int>(pid)));
            aniEnv->GlobalReference_Delete(callbackRef);
            AppExecFwk::DetachAniEnv(aniVM, isAttachThread);
        };
        if (AbilityRuntime::AniTask::AniSendEvent(task) != ANI_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to aniSendEvent");
        }
    }

    void StartChildProcessAppSpawnForkTask(ani_env *env, std::string &etsSrcEntry, ani_object callback,
        bool isStaticChildProcess)
    {
        pid_t pid = ERR_INVALID_VALUE;
        AbilityRuntime::ChildProcessManagerErrorCode innerErrorCode =
            AbilityRuntime::ChildProcessManager::GetInstance().StartChildProcessByAppSpawnFork(etsSrcEntry, pid,
            isStaticChildProcess);
        TAG_LOGD(AAFwkTag::PROCESSMGR, "StartChildProcessByAppSpawnFork innerErrorCode: %{public}d, pid: %{public}d",
            innerErrorCode, pid);
        if (innerErrorCode != AbilityRuntime::ChildProcessManagerErrorCode::ERR_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "StartChildProcessByAppSpawnFork failed, innerErrorCode is %{public}d",
                innerErrorCode);
            AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
                AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(innerErrorCode)), nullptr);
            return;
        }
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(innerErrorCode)),
            AppExecFwk::CreateInt(env, static_cast<ani_int>(pid)));
    }

    void OnStartArkChildProcess(ani_env *env, ani_string etsSrcEntry, ani_object childProcessArgs,
        ani_object childProcessOptions, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "OnStartArkChildProcess Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return;
        }
        if (AbilityRuntime::ChildProcessManager::GetInstance().IsChildProcessBySelfFork()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            AbilityRuntime::EtsErrorUtil::ThrowError(env,
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }
        std::string srcEntry;
        if (!AppExecFwk::GetStdString(env, etsSrcEntry, srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse srcEntry failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param srcEntry failed, must be a valid string.");
            return;
        }
        if (srcEntry.empty()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "param srcEntry cannot be empty");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Param srcEntry cannot be empty.");
            return;
        }
        bool isStaticChildProcess = false;
        ProcessSrcEntry(srcEntry, isStaticChildProcess);
        TAG_LOGD(AAFwkTag::PROCESSMGR, "srcEntry: %{public}s, isStaticChildProcess: %{public}d", srcEntry.c_str(),
            isStaticChildProcess);
        AppExecFwk::ChildProcessArgs args;
        AppExecFwk::ChildProcessOptions options;
        if (!ParseArgsAndOptions(env, childProcessArgs, childProcessOptions, args, options)) {
            return;
        }
        ChildProcessAniParam param;
        param.srcEntry = srcEntry;
        param.isStaticChildProcess = isStaticChildProcess;
        param.args = args;
        param.options = options;
        param.childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_ARK;
        StartChildProcessWithArgsTask(env, param, callback);
    }

    void OnStartNativeChildProcess(ani_env *env, ani_string etsEntryPoint, ani_object childProcessArgs,
        ani_object childProcessOptions, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "OnStartNativeChildProcess Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return;
        }
        if (AbilityRuntime::ChildProcessManager::GetInstance().IsChildProcessBySelfFork()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            AbilityRuntime::EtsErrorUtil::ThrowError(env,
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }
        std::string entryPoint;
        if (!AppExecFwk::GetStdString(env, etsEntryPoint, entryPoint)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse param entryPoint failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param entryPoint failed, must be a valid string.");
            return;
        }
        if (entryPoint.empty()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Param entryPoint cannot be empty");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Param entryPoint cannot be empty.");
            return;
        }
        if (entryPoint.find(":") == std::string::npos) {
            TAG_LOGE(AAFwkTag::PROCESSMGR,
                "Param entryPoint must contain a colon to separate library name and entry function.");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Param entryPoint must contain a colon to separate library name and entry function.");
            return;
        }
        AppExecFwk::ChildProcessArgs args;
        AppExecFwk::ChildProcessOptions options;
        if (!ParseArgsAndOptions(env, childProcessArgs, childProcessOptions, args, options)) {
            return;
        }
        ChildProcessAniParam param;
        param.srcEntry = entryPoint;
        param.args = args;
        param.options = options;
        param.childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_NATIVE_ARGS;
        StartChildProcessWithArgsTask(env, param, callback);
    }

    bool ParseArgsAndOptions(ani_env *env, ani_object childProcessArgs, ani_object childProcessOptions,
        AppExecFwk::ChildProcessArgs &args, AppExecFwk::ChildProcessOptions &options)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return false;
        }
        std::string errorMsg;
        if (!AppExecFwk::UnwrapChildProcessArgs(env, childProcessArgs, args, errorMsg)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "parse param args failed: %{private}s", errorMsg.c_str());
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, errorMsg);
            return false;
        }
        if (childProcessOptions &&
            !AppExecFwk::UnwrapChildProcessOptions(env, childProcessOptions, options, errorMsg)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "parse param options failed: %{private}s", errorMsg.c_str());
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, errorMsg);
            return false;
        }
        return true;
    }

    void StartChildProcessWithArgsTask(ani_env *env, const ChildProcessAniParam &param, ani_object callback)
    {
        pid_t pid = 0;
        TAG_LOGD(AAFwkTag::PROCESSMGR,
            "StartChildProcessWithArgs, childProcessType:%{public}d, srcEntry:%{private}s,"
            "isStaticChildProcess:%{public}d, args.entryParams size:%{public}zu, args.fds size:%{public}zu, "
            "options.isolationMode:%{public}d", param.childProcessType, param.srcEntry.c_str(),
            param.isStaticChildProcess, param.args.entryParams.length(), param.args.fds.size(),
            param.options.isolationMode);
        AbilityRuntime::ChildProcessManagerErrorCode errorCode =
            AbilityRuntime::ChildProcessManager::GetInstance().StartChildProcessWithArgs(
                param.srcEntry, pid, param.childProcessType, param.args, param.options, param.isStaticChildProcess);
        if (errorCode != AbilityRuntime::ChildProcessManagerErrorCode::ERR_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "StartChildProcessWithArgs failed, errorCode: %{public}d", errorCode);
            AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
                AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(errorCode)), nullptr);
            return;
        }
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_OK), AppExecFwk::CreateInt(env, static_cast<ani_int>(pid)));
    }

    void OnStartChildProcessCheck(ani_env *env, ani_string etsSrcEntry, ani_enum_item etsStartMode)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return;
        }
        if (AbilityRuntime::ChildProcessManager::GetInstance().IsChildProcess()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            AbilityRuntime::EtsErrorUtil::ThrowError(env,
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }
        std::string srcEntry;
        if (!AppExecFwk::GetStdString(env, etsSrcEntry, srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse srcEntry failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param srcEntry failed, must be a valid string.");
            return;
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "srcEntry %{public}s", srcEntry.c_str());
        int32_t startMode;
        if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, etsStartMode, startMode)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse startMode failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Unsupported startMode, must be StartMode.SELF_FORK or StartMode.APP_SPAWN_FORK.");
            return;
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "startMode %{public}d", startMode);
        if (startMode != MODE_SELF_FORK && startMode != MODE_APP_SPAWN_FORK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid StartMode");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Unsupported startMode, must be StartMode.SELF_FORK or StartMode.APP_SPAWN_FORK.");
        }
    }

    void OnStartArkChildProcessCheck(ani_env *env, ani_string etsSrcEntry, ani_object childProcessArgs,
        ani_object childProcessOptions)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "OnStartArkChildProcessCheck Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return;
        }
        if (AbilityRuntime::ChildProcessManager::GetInstance().IsChildProcessBySelfFork()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            AbilityRuntime::EtsErrorUtil::ThrowError(env,
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }
        std::string srcEntry;
        if (!AppExecFwk::GetStdString(env, etsSrcEntry, srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse srcEntry failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param srcEntry failed, must be a valid string.");
            return;
        }
        if (srcEntry.empty()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "param srcEntry cannot be empty");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Param srcEntry cannot be empty.");
            return;
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "srcEntry: %{public}s", srcEntry.c_str());
        if (!ParseArgsAndOptionsCheck(env, childProcessArgs, childProcessOptions)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "ParseArgsAndOptionsCheck false");
        }
    }

    void OnStartNativeChildProcessCheck(ani_env *env, ani_string etsEntryPoint, ani_object childProcessArgs,
        ani_object childProcessOptions)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "OnStartNativeChildProcessCheck Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return;
        }
        if (AbilityRuntime::ChildProcessManager::GetInstance().IsChildProcessBySelfFork()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            AbilityRuntime::EtsErrorUtil::ThrowError(env,
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }
        std::string entryPoint;
        if (!AppExecFwk::GetStdString(env, etsEntryPoint, entryPoint)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse param entryPoint failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param entryPoint failed, must be a valid string.");
            return;
        }
        if (entryPoint.empty()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Param entryPoint cannot be empty");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Param entryPoint cannot be empty.");
            return;
        }
        if (entryPoint.find(":") == std::string::npos) {
            TAG_LOGE(AAFwkTag::PROCESSMGR,
                "Param entryPoint must contain a colon to separate library name and entry function.");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Param entryPoint must contain a colon to separate library name and entry function.");
            return;
        }
        if (!ParseArgsAndOptionsCheck(env, childProcessArgs, childProcessOptions)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "ParseArgsAndOptionsCheck false");
        }
    }

    bool ParseArgsAndOptionsCheck(ani_env *env, ani_object childProcessArgs, ani_object childProcessOptions)
    {
        AppExecFwk::ChildProcessArgs args;
        AppExecFwk::ChildProcessOptions options;
        std::string errorMsg;
        if (!AppExecFwk::UnwrapChildProcessArgs(env, childProcessArgs, args, errorMsg)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "parse param args failed: %{private}s", errorMsg.c_str());
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, errorMsg);
            return false;
        }
        ani_status status = ANI_OK;
        ani_boolean isUndefined = false;
        if ((status = env->Reference_IsUndefined(childProcessOptions, &isUndefined)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Reference_IsUndefined status: %{public}d", status);
            return false;
        }
        if (!isUndefined && !AppExecFwk::UnwrapChildProcessOptions(env, childProcessOptions, options, errorMsg)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "parse param options failed: %{private}s", errorMsg.c_str());
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, errorMsg);
            return false;
        }
        return true;
    }
};

void EtsChildProcessManagerInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "EtsChildProcessManagerInit call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null env");
        return;
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "ResetError failed");
    }
    ani_status status = ANI_ERROR;
    ani_namespace ns;
    status = env->FindNamespace(CHILD_PROCESS_MANAGER_NAME_SPACE, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "FindNamespace childProcessManager failed status : %{public}d", status);
        return;
    }
    std::array kitFunctions = {
        ani_native_function {"nativeStartChildProcess",
            "C{std.core.String}C{@ohos.app.ability.childProcessManager.childProcessManager.StartMode}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsChildProcessManager::StartChildProcess)},
        ani_native_function {"nativeStartArkChildProcess",
            "C{std.core.String}C{@ohos.app.ability.ChildProcessArgs.ChildProcessArgs}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsChildProcessManager::StartArkChildProcess)},
        ani_native_function {"nativeStartArkChildProcess",
            "C{std.core.String}C{@ohos.app.ability.ChildProcessArgs.ChildProcessArgs}"
            "C{@ohos.app.ability.ChildProcessOptions.ChildProcessOptions}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsChildProcessManager::StartArkChildProcessWithOptions)},
        ani_native_function {"nativeStartNativeChildProcess",
            "C{std.core.String}C{@ohos.app.ability.ChildProcessArgs.ChildProcessArgs}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsChildProcessManager::StartNativeChildProcess)},
        ani_native_function {"nativeStartNativeChildProcess",
            "C{std.core.String}C{@ohos.app.ability.ChildProcessArgs.ChildProcessArgs}"
            "C{@ohos.app.ability.ChildProcessOptions.ChildProcessOptions}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsChildProcessManager::StartNativeChildProcessWithOptions)},
        ani_native_function {"nativeStartChildProcessCheck",
            "C{std.core.String}C{@ohos.app.ability.childProcessManager.childProcessManager.StartMode}:",
            reinterpret_cast<void *>(EtsChildProcessManager::StartChildProcessCheck)},
        ani_native_function {"nativeStartArkChildProcessCheck",
            "C{std.core.String}C{@ohos.app.ability.ChildProcessArgs.ChildProcessArgs}"
            "C{@ohos.app.ability.ChildProcessOptions.ChildProcessOptions}:",
            reinterpret_cast<void *>(EtsChildProcessManager::StartArkChildProcessCheck)},
        ani_native_function {"nativeStartNativeChildProcessCheck",
            "C{std.core.String}C{@ohos.app.ability.ChildProcessArgs.ChildProcessArgs}"
            "C{@ohos.app.ability.ChildProcessOptions.ChildProcessOptions}:",
            reinterpret_cast<void *>(EtsChildProcessManager::StartNativeChildProcessCheck)}
    };
    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::PROCESSMGR, "EtsChildProcessManagerInit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "in ChildProcessManagerEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsChildProcessManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::PROCESSMGR, "ChildProcessManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
}
} // namespace ChildProcessManagerEts
} // namespace OHOS