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

#include "js_auto_fill_request_callback.h"

#include "hilog_tag_wrapper.h"
#include "js_auto_fill_manager_util.h"
#include "js_auto_fill_manager.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
const std::string METHOD_ON_FILL_REQUEST_SUCCESS = "onSuccess";
const std::string METHOD_ON_FILL_REQUEST_FAILED = "onFailure";
} // namespace
JsAutoFillRequestCallback::JsAutoFillRequestCallback(
    napi_env env, int32_t instanceId, AutoFillManagerFunc autoFillManagerFunc)
    : env_(env), instanceId_(instanceId), autoFillManagerFunc_(autoFillManagerFunc) {}

JsAutoFillRequestCallback::~JsAutoFillRequestCallback() {}

void JsAutoFillRequestCallback::OnFillRequestSuccess(const AbilityBase::ViewData &viewData)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    napi_value argv[ARGC_ONE] = { WrapViewData(env_, viewData) };
    JSCallFunction(METHOD_ON_FILL_REQUEST_SUCCESS, argv, ARGC_ONE);
    if (autoFillManagerFunc_ != nullptr) {
        autoFillManagerFunc_(instanceId_);
    }
}

void JsAutoFillRequestCallback::OnFillRequestFailed(int32_t errCode, const std::string &fillContent, bool isPopup)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    napi_value argv[ARGC_ONE] = { WrapFillFailureResult(env_, errCode) };
    JSCallFunction(METHOD_ON_FILL_REQUEST_FAILED, argv, ARGC_ONE);
    if (autoFillManagerFunc_ != nullptr) {
        autoFillManagerFunc_(instanceId_);
    }
}

void JsAutoFillRequestCallback::Register(napi_value value)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    if (IsJsCallbackEquals(callback_, value)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "callback exist");
        return;
    }

    napi_ref ref = nullptr;
    napi_status createStatus = napi_create_reference(env_, value, 1, &ref);
    if (createStatus != napi_ok || ref == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "napi_create_reference failed, %{public}d", createStatus);
        return;
    }
    callback_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
}

void JsAutoFillRequestCallback::JSCallFunction(const std::string &methodName, napi_value const *argv, size_t argc)
{
    auto thisPtr = shared_from_this();
    std::vector<napi_value> argvCopy(argv, argv + argc);
    NapiAsyncTask::CompleteCallback complete =
        [thisPtr, methodName, argvCopy](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (thisPtr) {
                thisPtr->JSCallFunctionWorker(methodName, argvCopy.data(), argvCopy.size());
            }
        };

    NapiAsyncTask::Schedule("JsAutoFillRequestCallback::JSCallFunction:" + methodName,
        env_,
        CreateAsyncTaskWithLastParam(env_, nullptr, nullptr, std::move(complete), nullptr));
}

void JsAutoFillRequestCallback::JSCallFunctionWorker(const std::string &methodName, napi_value const *argv, size_t argc)
{
    HandleScope handleScope(env_);
    if (callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null callback_");
        return;
    }

    auto obj = callback_->GetNapiValue();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null obj");
        return;
    }

    napi_value funcObject;
    if (napi_get_named_property(env_, obj, methodName.c_str(), &funcObject) != napi_ok) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Get function failed");
        return;
    }

    napi_call_function(env_, obj, funcObject, argc, argv, nullptr);
}

bool JsAutoFillRequestCallback::IsJsCallbackEquals(std::shared_ptr<NativeReference> callback, napi_value value)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Invalid jsCallback");
        return false;
    }

    auto object = callback->GetNapiValue();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null obj");
        return false;
    }

    bool result = false;
    if (napi_strict_equals(env_, object, value, &result) != napi_ok) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object not match");
        return false;
    }

    return result;
}
} // namespace AbilityRuntime
} // namespace OHOS