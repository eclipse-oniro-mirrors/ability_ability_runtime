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

#ifndef OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_REQUEST_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_REQUEST_CALLBACK_H

#include "fill_request_callback_interface.h"
#include "native_engine/native_value.h"

class NativeReference;
namespace OHOS {
namespace AbilityRuntime {
using AutoFillManagerFunc = std::function<void(int32_t)>;
class JsAutoFillRequestCallback : public IFillRequestCallback,
                                  public std::enable_shared_from_this<JsAutoFillRequestCallback> {
public:
    JsAutoFillRequestCallback(napi_env env, int32_t instanceId, AutoFillManagerFunc autoFillManagerFunc);
    virtual ~JsAutoFillRequestCallback();

    void Register(napi_value value);
    void OnFillRequestSuccess(const AbilityBase::ViewData &viewData) override;
    void OnFillRequestFailed(int32_t errCode, const std::string &fillContent = "", bool isPopup = false) override;

private:
    void JSCallFunction(const std::string &methodName, napi_value const *argv, size_t argc);
    void JSCallFunctionWorker(const std::string &methodName, napi_value const *argv, size_t argc);
    bool IsJsCallbackEquals(std::shared_ptr<NativeReference> callback, napi_value value);

    napi_env env_ = nullptr;
    std::shared_ptr<NativeReference> callback_;
    int32_t instanceId_ = -1;
    AutoFillManagerFunc autoFillManagerFunc_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_REQUEST_CALLBACK_H
