/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_FREE_INSTALL_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_FREE_INSTALL_OBSERVER_H

#include <memory>
#include <string>
#include <vector>

#include "native_engine/native_engine.h"
#include "free_install_observer_stub.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
struct JsFreeInstallObserverObject {
    std::string bundleName;
    std::string abilityName;
    std::string startTime;
    std::string url;
    napi_deferred deferred;
    napi_ref callback;
    bool isAbilityResult = false;
};

class JsFreeInstallObserver : public FreeInstallObserverStub {
public:
    explicit JsFreeInstallObserver(napi_env env);
    ~JsFreeInstallObserver();

    /**
     * OnInstallFinished, return free install result.
     *
     * @param bundleName Free install bundleName.
     * @param abilityName Free install abilityName.
     * @param startTime Free install start request time.
     * @param resultCode The result of this free install.
     */
    void OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, const int &resultCode) override;

    /**
     * OnInstallFinishedByUrl, return free install result.
     *
     * @param startTime Free install start request time.
     * @param url Free install url.
     * @param resultCode The result of this free install.
     */
    void OnInstallFinishedByUrl(const std::string &startTime, const std::string &url,
        const int &resultCode) override;

    /**
     * OnInstallFinished, return free install result.
     *
     * @param bundleName Free install bundleName.
     * @param abilityName Free install abilityName.
     * @param startTime Free install start request time.
     * @param abilityResult The result of this free install.
     */
    void OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, napi_value abilityResult);

    /**
     * @brief Use for context to add an callback into the observer.
     *
     * @param bundleName The bundleName of want.
     * @param abilityName The abilityName of want.
     * @param startTime The startTime that want requested.
     * @param jsObserverObject The js object instance.
     * @param result the promise to return.
     */
    void AddJsObserverObject(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, napi_value jsObserverObject, napi_value* result, bool isAbilityResult = false);

    /**
     * @brief Use for context to add an callback into the observer.
     *
     * @param startTime The startTime that want requested.
     * @param url Free install url.
     * @param jsObserverObject The js object instance.
     * @param result the promise to return.
     */
    void AddJsObserverObject(const std::string &startTime, const std::string &url,
        napi_value jsObserverObject, napi_value* result, bool isAbilityResult = false);

private:
    void CallPromise(napi_deferred deferred, int32_t resultCode);
    void CallPromise(napi_deferred deferred, napi_value abilityResult);
    void CallCallback(napi_ref callback, int32_t resultCode);
    void CallCallback(napi_ref callback, napi_value abilityResult);
    void HandleOnInstallFinished(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime, const int &resultCode);
    void HandleOnInstallFinishedByUrl(const std::string &startTime, const std::string &url,
        const int &resultCode);
    void AddJsObserverCommon(JsFreeInstallObserverObject &object,
        napi_value jsObserverObject, napi_value* result, bool isAbilityResult);
    napi_env env_;
    std::vector<JsFreeInstallObserverObject> jsObserverObjectList_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_FREE_INSTALL_OBSERVER_H