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

#include "js_memory_optimizer.h"

#include "ability_business_error.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "js_error_utils.h"
#include "madvise_utils.h"
#include "napi_common_util.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr int32_t INDEX_ZERO = 0;
constexpr const char *MODULE_NAME = "JsMemoryOptimizer";
} // namespace

void JsMemoryOptimizer::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    std::unique_ptr<JsMemoryOptimizer>(static_cast<JsMemoryOptimizer *>(data));
}

napi_value JsMemoryOptimizer::EvictFilePages(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsMemoryOptimizer, OnEvictFilePages);
}

napi_value JsMemoryOptimizer::EvictModuleFilePages(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsMemoryOptimizer, OnEvictModuleFilePages);
}

bool JsMemoryOptimizer::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::ABILITY, "not system app");
        return false;
    }
    return true;
}

napi_value JsMemoryOptimizer::OnEvictFilePages(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null env");
        return nullptr;
    }
    AbilityRuntime::HandleEscape handleEscape(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::ABILITY, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::ABILITY, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    std::vector<std::string> fileNames;
    if (!AppExecFwk::UnwrapArrayStringFromJS(env, info.argv[INDEX_ZERO], fileNames)) {
        TAG_LOGE(AAFwkTag::ABILITY, "parse fileNames failed");
        ThrowInvalidParamError(env, "Parameter error. Parse fileNames failed.");
        return CreateJsUndefined(env);
    }
    if (fileNames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "empty fileNames array");
        ThrowInvalidParamError(env, "Parameter error. Empty fileNames array.");
        return CreateJsUndefined(env);
    }
    for (const auto &name : fileNames) {
        if (!MadviseUtil::IsValidEvictFileName(name)) {
            TAG_LOGE(AAFwkTag::ABILITY, "invalid file type: %{public}s", name.c_str());
            ThrowError(env, AbilityErrorCode::ERROR_CODE_FILE_TYPE_ERROR);
            return CreateJsUndefined(env);
        }
    }

    NapiAsyncTask::ExecuteCallback execute = [fileNames]() {
        MadviseUtil::EvictFilePages(fileNames);
    };
    NapiAsyncTask::CompleteCallback complete = [](napi_env env, NapiAsyncTask &task, int32_t status) {
        AbilityRuntime::HandleScope handleScope(env);
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsMemoryOptimizer::OnEvictFilePages", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsMemoryOptimizer::OnEvictModuleFilePages(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null env");
        return nullptr;
    }
    AbilityRuntime::HandleEscape handleEscape(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::ABILITY, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::ABILITY, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    std::vector<std::string> moduleNames;
    if (!AppExecFwk::UnwrapArrayStringFromJS(env, info.argv[INDEX_ZERO], moduleNames)) {
        TAG_LOGE(AAFwkTag::ABILITY, "parse moduleNames failed");
        ThrowInvalidParamError(env, "Parameter error. Parse moduleNames failed.");
        return CreateJsUndefined(env);
    }
    if (moduleNames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "empty moduleNames array");
        ThrowInvalidParamError(env, "Parameter error. Empty moduleNames array.");
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<ErrCode>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [moduleNames, ret = retVal]() {
        *ret = MadviseUtil::EvictModuleFilePages(moduleNames);
    };
    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        AbilityRuntime::HandleScope handleScope(env);
        if (*ret != ERR_OK) {
            task.Reject(env, CreateJsErrorByNativeErr(env, *ret));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsMemoryOptimizer::OnEvictModuleFilePages", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsMemoryOptimizerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null env or exportObj");
        return nullptr;
    }

    auto jsMemoryOptimizer = std::make_unique<JsMemoryOptimizer>();
    napi_wrap(env, exportObj, jsMemoryOptimizer.release(),
        JsMemoryOptimizer::Finalizer, nullptr, nullptr);

    BindNativeFunction(env, exportObj, "evictFilePages", MODULE_NAME, JsMemoryOptimizer::EvictFilePages);
    BindNativeFunction(env, exportObj, "evictModuleFilePages", MODULE_NAME, JsMemoryOptimizer::EvictModuleFilePages);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
