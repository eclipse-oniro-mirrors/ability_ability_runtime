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

#include "js_preload_ui_extension_callback_client.h"

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
JsPreloadUIExtensionCallbackClient::~JsPreloadUIExtensionCallbackClient()
{
    if (callbackRef_ == nullptr || env_ == nullptr) {
        return;
    }
    napi_env env = env_;
    napi_ref ref = callbackRef_;
    callbackRef_ = nullptr;
    auto deleteTask = [env, ref]() {
        napi_status status = napi_delete_reference(env, ref);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "napi_delete_reference failed: %{public}d", status);
        }
    };
    if (napi_send_event(env, deleteTask, napi_eprio_high) != napi_ok) {
        // Never call napi_delete_reference on the current (Binder) thread; if the JS event loop is
        // unavailable, leak the reference rather than risk a cross-thread N-API call.
        TAG_LOGE(AAFwkTag::ABILITYMGR, "napi_send_event failed, skip cross-thread reference release");
    }
}

void JsPreloadUIExtensionCallbackClient::ProcessOnLoadedDone(int32_t extensionAbilityId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ProcessOnLoadedDone call, extensionAbilityId: %{public}d", extensionAbilityId);
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env_");
        return;
    }
    std::shared_ptr<JsPreloadUIExtensionCallbackClient> jsPreloadUIExtensionLoadedCallbackClient =
        shared_from_this();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [jsPreloadUIExtensionLoadedCallbackClient, extensionAbilityId](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            if (jsPreloadUIExtensionLoadedCallbackClient != nullptr) {
                jsPreloadUIExtensionLoadedCallbackClient->CallJsPreloadedUIExtensionAbility(extensionAbilityId);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsPreloadUIExtensionCallbackClient::OnLoadedDone:", env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsPreloadUIExtensionCallbackClient::ProcessOnDestroyDone(int32_t extensionAbilityId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ProcessOnDestroyDone call, extensionAbilityId: %{public}d", extensionAbilityId);
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env_");
        return;
    }
    std::shared_ptr<JsPreloadUIExtensionCallbackClient> jsPreloadUIExtensionDestroyCallbackClient =
        shared_from_this();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [jsPreloadUIExtensionDestroyCallbackClient, extensionAbilityId](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            if (jsPreloadUIExtensionDestroyCallbackClient != nullptr) {
                jsPreloadUIExtensionDestroyCallbackClient->CallJsPreloadedUIExtensionAbility(extensionAbilityId);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsPreloadUIExtensionCallbackClient::OnDestroyDone:", env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsPreloadUIExtensionCallbackClient::CallJsPreloadedUIExtensionAbility(int32_t preloadId)
{
    HandleScope handleScope(env_);
    if (callbackRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null callbackRef_");
        return;
    }
    napi_value jsFunc = nullptr;
    napi_status status = napi_get_reference_value(env_, callbackRef_, &jsFunc);
    if (status != napi_ok || jsFunc == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get_reference_value failed or null jsFunc, status: %{public}d", status);
        return;
    }
    napi_value global = nullptr;
    status = napi_get_global(env_, &global);
    if (status != napi_ok || global == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get_global failed, status: %{public}d", status);
        return;
    }
    napi_value preloadIdValue = nullptr;
    status = napi_create_int32(env_, preloadId, &preloadIdValue);
    if (status != napi_ok || preloadIdValue == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create_int32 failed, status: %{public}d", status);
        return;
    }
    napi_value argv[] = { preloadIdValue };
    status = napi_call_function(env_, global, jsFunc, 1, argv, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "napi_call_function failed: %{public}d", status);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS