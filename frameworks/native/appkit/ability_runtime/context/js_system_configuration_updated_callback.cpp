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

#include "js_system_configuration_updated_callback.h"

#include "configuration_convertor.h"
#include "hilog_tag_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace {
bool IsValidValue(const char *end, const std::string &str)
{
    if (!end) {
        return false;
    }

    if (end == str.c_str() || errno == ERANGE || *end != '\0') {
        return false;
    }
    return true;
}

bool ConvertToDouble(const std::string &str, double &outValue)
{
    if (str.empty()) {
        TAG_LOGW(AAFwkTag::JSNAPI, "ConvertToDouble failed str is null");
        return false;
    }
    char *end = nullptr;
    errno = 0;
    double value = std::strtod(str.c_str(), &end);
    if (!IsValidValue(end, str)) {
        TAG_LOGW(AAFwkTag::JSNAPI, "ConvertToDouble failed for: %{public}s", str.c_str());
        return false;
    }
    outValue = value;
    return true;
}
}  // namespace
namespace OHOS {
namespace AbilityRuntime {
JsSystemConfigurationUpdatedCallback::JsSystemConfigurationUpdatedCallback(napi_env env) : env_(env)
{}

JsSystemConfigurationUpdatedCallback::~JsSystemConfigurationUpdatedCallback()
{
    std::lock_guard lock(mutex_);
    for (auto& callback:callbacks_) {
        FreeNativeReference(std::move(callback));
    }
}

void JsSystemConfigurationUpdatedCallback::CallJsMethodInnerCommon(
    const std::string &methodName, const std::shared_ptr<NativeReference> &callback, const napi_value &value)
{
    if (!callback) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid jsCallback");
        return;
    }

    auto obj = callback->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, methodName.data(), &method);
    if (method == nullptr || CheckTypeForNapiValue(env_, method, napi_undefined) ||
        CheckTypeForNapiValue(env_, method, napi_null)) {
        TAG_LOGE(AAFwkTag::APPKIT, "null method %{public}s", methodName.data());
        return;
    }

    napi_value argv[] = {value};
    napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
}

template <class T>
void JsSystemConfigurationUpdatedCallback::CallJsMethod(
    std::shared_ptr<NativeReference> callback, const std::string &methodName, const T &value)
{
    TAG_LOGD(AAFwkTag::APPKIT, "MethodName = %{public}s", methodName.c_str());
    std::weak_ptr<JsSystemConfigurationUpdatedCallback> thisWeakPtr(shared_from_this());

    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [thisWeakPtr, methodName, jsCallback = callback, value](napi_env env, NapiAsyncTask &task, int32_t status) {
            std::shared_ptr<JsSystemConfigurationUpdatedCallback> configUpdatedPtr = thisWeakPtr.lock();
            if (!configUpdatedPtr) {
                TAG_LOGE(AAFwkTag::APPKIT, "configUpdatedPtr null");
                return;
            }
            HandleScope handleScope(env);
            napi_value jsValue = CreateJsValue(env, value);
            if (jsValue == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "create napi_value failed");
            }
            configUpdatedPtr->CallJsMethodInnerCommon(methodName, jsCallback, jsValue);
        });

    napi_ref callback1 = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsApplicationStateChangeCallback::CallJsMethod:" + methodName,
        env_,
        std::make_unique<NapiAsyncTask>(callback1, std::move(execute), std::move(complete)));
}

bool JsSystemConfigurationUpdatedCallback::HasJsMethodExist(napi_env env, std::shared_ptr<NativeReference> callback,
    const char *methodName)
{
    if (env == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env or callback null");
        return false;
    }
    napi_value jsCallback = callback->GetNapiValue();
    bool isExist = false;
    napi_has_named_property(env, jsCallback, methodName, &isExist);
    return isExist;
}

void JsSystemConfigurationUpdatedCallback::NotifySystemConfigurationUpdated(
    const OHOS::AppExecFwk::Configuration &configuration)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifySystemConfig:%{public}s", configuration.GetName().c_str());

    std::lock_guard lock(mutex_);
    for (auto &callback : callbacks_) {
        auto colorMode = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        if (!colorMode.empty() &&
            HasJsMethodExist(
                env_, callback, SystemConfigurationUpdatedFunctionName::SYSTEM_COLOR_MODE_UPDATED_FUNCTION_NAME)) {
            NotifyColorModeUpdated(callback, colorMode);
        }

        auto fontSizeScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
        if (!fontSizeScale.empty() &&
            HasJsMethodExist(
                env_, callback, SystemConfigurationUpdatedFunctionName::SYSTEM_FONT_SIZE_SCALE_UPDATED_FUNCTION_NAME)) {
            NotifyFontSizeScaleUpdated(callback, fontSizeScale);
        }

        auto fontWeightScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE);
        if (!fontWeightScale.empty() &&
            HasJsMethodExist(env_,
                callback,
                SystemConfigurationUpdatedFunctionName::SYSTEM_FONT_WEIGHT_SCALE_UPDATED_FUNCTION_NAME)) {
            NotifyFontWeightScaleUpdated(callback, fontWeightScale);
        }

        auto language = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        if (!language.empty() &&
            HasJsMethodExist(
                env_, callback, SystemConfigurationUpdatedFunctionName::SYSTEM_LANGUAGE_UPDATED_FUNCTION_NAME)) {
            NotifyLanguageUpdated(callback, language);
        }

        auto mcc = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC);
        if (!mcc.empty() &&
            HasJsMethodExist(
                env_, callback, SystemConfigurationUpdatedFunctionName::SYSTEM_MCC_UPDATED_FUNCTION_NAME)) {
            NotifyMCCUpdated(callback, mcc);
        }

        auto mnc = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC);
        if (!mnc.empty() &&
            HasJsMethodExist(
                env_, callback, SystemConfigurationUpdatedFunctionName::SYSTEM_MNC_UPDATED_FUNCTION_NAME)) {
            NotifyMNCUpdated(callback, mnc);
        }

        auto locale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE);
        if (!locale.empty() &&
            HasJsMethodExist(
                env_, callback, SystemConfigurationUpdatedFunctionName::SYSTEM_LOCALE_UPDATED_FUNCTION_NAME)) {
            NotifyLocaleUpdated(callback, locale);
        }

        auto hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        if (!hasPointerDevice.empty() &&
            HasJsMethodExist(env_,
                callback,
                SystemConfigurationUpdatedFunctionName::SYSTEM_HAS_POINTER_DEVICE_UPDATED_FUNCTION_NAME)) {
            NotifyHasPointerDeviceUpdated(callback, hasPointerDevice);
        }

        auto fontId = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_ID);
        if (!fontId.empty() &&
            HasJsMethodExist(
                env_, callback, SystemConfigurationUpdatedFunctionName::SYSTEM_FONTID_UPDATED_FUNCTION_NAME)) {
            NotifyFontIdUpdated(callback, fontId);
        }
    }
}

void JsSystemConfigurationUpdatedCallback::NotifyColorModeUpdated(
    std::shared_ptr<NativeReference> callback, const std::string &colorMode)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifyColorModeUpdated");
    int32_t colorModeValue = AppExecFwk::ConvertColorMode(colorMode);
    CallJsMethod(
        callback, SystemConfigurationUpdatedFunctionName::SYSTEM_COLOR_MODE_UPDATED_FUNCTION_NAME, colorModeValue);
}

void JsSystemConfigurationUpdatedCallback::NotifyFontSizeScaleUpdated(
    std::shared_ptr<NativeReference> callback, const std::string &fontSizeScale)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifyFontSizeScaleUpdated");
    double fontSizeScaleDouble = 1.0;
    ConvertToDouble(fontSizeScale, fontSizeScaleDouble);
    CallJsMethod(callback,
        SystemConfigurationUpdatedFunctionName::SYSTEM_FONT_SIZE_SCALE_UPDATED_FUNCTION_NAME,
        fontSizeScaleDouble);
}
void JsSystemConfigurationUpdatedCallback::NotifyFontWeightScaleUpdated(
    std::shared_ptr<NativeReference> callback, const std::string &fontWeightScale)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifyFontWeightScaleUpdated");
    double fontWeightScaleDouble = 1.0;
    ConvertToDouble(fontWeightScale, fontWeightScaleDouble);
    CallJsMethod(callback,
        SystemConfigurationUpdatedFunctionName::SYSTEM_FONT_WEIGHT_SCALE_UPDATED_FUNCTION_NAME,
        fontWeightScaleDouble);
}
void JsSystemConfigurationUpdatedCallback::NotifyLanguageUpdated(
    std::shared_ptr<NativeReference> callback, const std::string &language)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifyLanguageUpdated");
    CallJsMethod(callback, SystemConfigurationUpdatedFunctionName::SYSTEM_LANGUAGE_UPDATED_FUNCTION_NAME, language);
}

void JsSystemConfigurationUpdatedCallback::NotifyFontIdUpdated(
    std::shared_ptr<NativeReference> callback, const std::string &fontId)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifyFontIdUpdated");
    CallJsMethod(callback, SystemConfigurationUpdatedFunctionName::SYSTEM_FONTID_UPDATED_FUNCTION_NAME, fontId);
}
void JsSystemConfigurationUpdatedCallback::NotifyMCCUpdated(
    std::shared_ptr<NativeReference> callback, const std::string &mcc)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifyMCCUpdated");
    CallJsMethod(callback, SystemConfigurationUpdatedFunctionName::SYSTEM_MCC_UPDATED_FUNCTION_NAME, mcc);
}
void JsSystemConfigurationUpdatedCallback::NotifyMNCUpdated(
    std::shared_ptr<NativeReference> callback, const std::string &mnc)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifyMNCUpdated");
    CallJsMethod(callback, SystemConfigurationUpdatedFunctionName::SYSTEM_MNC_UPDATED_FUNCTION_NAME, mnc);
}
void JsSystemConfigurationUpdatedCallback::NotifyLocaleUpdated(
    std::shared_ptr<NativeReference> callback, const std::string &locale)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifyLocaleUpdated");
    CallJsMethod(callback, SystemConfigurationUpdatedFunctionName::SYSTEM_LOCALE_UPDATED_FUNCTION_NAME, locale);
}
void JsSystemConfigurationUpdatedCallback::NotifyHasPointerDeviceUpdated(
    std::shared_ptr<NativeReference> callback, const std::string &hasPointerDevice)
{
    TAG_LOGI(AAFwkTag::APPKIT, "NotifyHasPointerDeviceUpdated");
    CallJsMethod(callback,
        SystemConfigurationUpdatedFunctionName::SYSTEM_HAS_POINTER_DEVICE_UPDATED_FUNCTION_NAME,
        hasPointerDevice == "true" ? true : false);
}

void JsSystemConfigurationUpdatedCallback::Register(napi_value jsCallback)
{
    if (env_ == nullptr || jsCallback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or jsCallback");
        return;
    }
    {
        std::lock_guard lock(mutex_);
        for (auto &callback : callbacks_) {
            if (IsEqual(callback, jsCallback)) {
                TAG_LOGW(AAFwkTag::APPKIT, "callback exist");
                return;
            }
        }
    }

    napi_ref ref = nullptr;
    napi_create_reference(env_, jsCallback, 1, &ref);
    std::lock_guard lock(mutex_);
    callbacks_.push_back(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref)));
}

bool JsSystemConfigurationUpdatedCallback::UnRegister(napi_value jsCallback)
{
    if (jsCallback == nullptr) {
        TAG_LOGI(AAFwkTag::APPKIT, "null jsCallback");
        std::lock_guard lock(mutex_);
        callbacks_.clear();
        return true;
    }

    std::lock_guard lock(mutex_);
    for (auto it = callbacks_.begin(); it != callbacks_.end(); it++) {
        if (IsEqual(*it, jsCallback)) {
            std::shared_ptr<NativeReference> nativeRef = *it;
            callbacks_.erase(it);
            FreeNativeReference(std::move(nativeRef));
            return true;
        }
    }

    return false;
}

bool JsSystemConfigurationUpdatedCallback::IsEmpty() const
{
    std::lock_guard lock(mutex_);
    return callbacks_.empty();
}

bool JsSystemConfigurationUpdatedCallback::IsEqual(
    std::shared_ptr<NativeReference> callbackRef, const napi_value &jsCallback) const
{
    if (env_ == nullptr || jsCallback == nullptr || callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or jsCallback");
        return false;
    }

    napi_value value = callbackRef->GetNapiValue();
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null value");
        return false;
    }

    bool isEqual = false;
    napi_strict_equals(env_, value, jsCallback, &isEqual);
    return isEqual;
}

struct JsNativeReferenceDeleterObject {
    std::shared_ptr<NativeReference> sharedNativeRef_ = nullptr;
};
void JsSystemConfigurationUpdatedCallback::FreeNativeReference(std::shared_ptr<NativeReference> &&reference)
{
    if (reference == nullptr) {
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

    auto cb = new (std::nothrow) JsNativeReferenceDeleterObject();
    if (cb == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null cb");
        delete work;
        work = nullptr;
        return;
    }
    cb->sharedNativeRef_ = std::move(reference);

    work->data = reinterpret_cast<void *>(cb);
    int ret = uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            if (work != nullptr) {
                if (work->data != nullptr) {
                    delete reinterpret_cast<JsNativeReferenceDeleterObject *>(work->data);
                    work->data = nullptr;
                }
                delete work;
                work = nullptr;
            }
        });
    if (ret != 0) {
        delete reinterpret_cast<JsNativeReferenceDeleterObject *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
