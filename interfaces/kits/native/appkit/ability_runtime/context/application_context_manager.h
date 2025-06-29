/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_CONTEXT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_CONTEXT_MANAGER_H

#include "native_engine/native_engine.h"

#include <mutex>
#include <unordered_map>

namespace OHOS {
namespace AppExecFwk {
struct ETSNativeReference;
}

namespace AbilityRuntime {
struct EnvData {
    napi_env env;
    EnvData(napi_env napienv) : env(napienv) {}
};
class ApplicationContextManager {
public:
    ApplicationContextManager(const ApplicationContextManager&) = delete;

    ApplicationContextManager& operator=(const ApplicationContextManager&) = delete;

    static ApplicationContextManager& GetApplicationContextManager();

    void AddGlobalObject(napi_env env, std::shared_ptr<NativeReference> applicationContextObj);

    std::shared_ptr<NativeReference> GetGlobalObject(napi_env env);

    void RemoveGlobalObject(napi_env env);

    void SetEtsGlobalObject(std::shared_ptr<AppExecFwk::ETSNativeReference> applicationContextObj);

    std::shared_ptr<AppExecFwk::ETSNativeReference> GetEtsGlobalObject();

private:
    ApplicationContextManager();

    ~ApplicationContextManager();

    std::unordered_map<napi_env, std::shared_ptr<NativeReference>> applicationContextMap_;
    std::shared_ptr<AppExecFwk::ETSNativeReference> etsApplicationContextRef_;
    std::mutex applicationContextMutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_CONTEXT_MANAGER_H
