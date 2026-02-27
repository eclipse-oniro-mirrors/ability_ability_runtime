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

#ifndef OHOS_ABILITY_RUNTIME_JS_SYSTEM_CONFIGURATION_UPDATED_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_SYSTEM_CONFIGURATION_UPDATED_CALLBACK_H

#include <memory>
#include <mutex>
#include <vector>

#include "system_configuration_updated_callback.h"

class NativeReference;
typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;

namespace OHOS {
namespace AppExecFwk {
class Configuration;
}
namespace AbilityRuntime {

class JsSystemConfigurationUpdatedCallback : public SystemConfigurationUpdatedCallback,
    public std::enable_shared_from_this<JsSystemConfigurationUpdatedCallback> {
public:

    explicit JsSystemConfigurationUpdatedCallback(napi_env env);
    virtual ~JsSystemConfigurationUpdatedCallback();

    void NotifySystemConfigurationUpdated(const  AppExecFwk::Configuration& configuration) override;
    void NotifyColorModeUpdated(std::shared_ptr<NativeReference> callback, const std::string& colorMode);
    void NotifyFontSizeScaleUpdated(std::shared_ptr<NativeReference> callback, const std::string&);
    void NotifyFontWeightScaleUpdated(std::shared_ptr<NativeReference> callback, const std::string&);
    void NotifyLanguageUpdated(std::shared_ptr<NativeReference> callback, const std::string&);
    void NotifyFontIdUpdated(std::shared_ptr<NativeReference> callback, const std::string&);
    void NotifyMCCUpdated(std::shared_ptr<NativeReference> callback, const std::string&);
    void NotifyMNCUpdated(std::shared_ptr<NativeReference> callback, const std::string&);
    void NotifyLocaleUpdated(std::shared_ptr<NativeReference> callback, const std::string&);
    void NotifyHasPointerDeviceUpdated(std::shared_ptr<NativeReference> callback, const std::string&);
    void Register(napi_value jsCallback);
    bool UnRegister(napi_value jsCallback = nullptr);
    bool IsEmpty() const;

private:
    bool HasJsMethodExist(napi_env env, std::shared_ptr<NativeReference> callback, const char *methodName);
    bool IsEqual(std::shared_ptr<NativeReference> callbackRef, const napi_value &jsCallback) const;
    void FreeNativeReference(std::shared_ptr<NativeReference>&& reference);
    void CallJsMethodInnerCommon(
        const std::string &methodName, const std::shared_ptr<NativeReference> &callback, const napi_value& value);
    template <class T>
    void CallJsMethod(std::shared_ptr<NativeReference> callback, const std::string &methodName, const T& value);
    napi_env env_ = nullptr;
    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<NativeReference>> callbacks_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_SYSTEM_CONFIGURATION_UPDATED_CALLBACK_H
