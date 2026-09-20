/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CALLBACK_H

#include "native_engine/native_reference.h"
#include "ui_extension_callback.h"

namespace OHOS {
namespace AbilityRuntime {
class JsUIExtensionCallback : public UIExtensionCallback,
                              public std::enable_shared_from_this<JsUIExtensionCallback> {
public:
    explicit JsUIExtensionCallback(napi_env env) : env_(env) {}
    ~JsUIExtensionCallback() override;
    void OnError(int32_t number) override;
    void OnAbilityByTypeResult(int32_t errorCode) override;
    void OnResult(int32_t resultCode, const AAFwk::Want &want) override;
    void CallJsResult(int32_t resultCode, const AAFwk::Want &want);
    void SetJsCallbackObject(napi_value jsCallbackObject);
    void SetDeferred(napi_deferred deferred) { deferred_ = deferred; }
    void ClearAsyncResult();
    void RejectAsyncResult(int32_t jsErrCode, const std::string& innerMsg);
    bool SetCompletionCallback(napi_env env, napi_value callback);
    void CallJsError(int32_t number);
    void SetCompletionHandler(napi_env env, napi_value completionHandler);
    void OnRequestSuccess(const std::string& name) override;
    void OnRequestFailure(const std::string& name, int32_t failureCode, const std::string& failureMessage) override;
private:
    napi_env env_ = nullptr;
    napi_deferred deferred_ = nullptr;
    std::unique_ptr<NativeReference> jsCallbackObject_ = nullptr;
    std::unique_ptr<NativeReference> completionCallback_ = nullptr;
    std::unique_ptr<NativeReference> onRequestSuccess_ = nullptr;
    std::unique_ptr<NativeReference> onRequestFailure_ = nullptr;
    void FreeNativeReference(std::unique_ptr<NativeReference>& reference);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CALLBACK_H
