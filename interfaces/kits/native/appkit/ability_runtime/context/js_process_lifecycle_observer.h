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

#ifndef OHOS_ABILITY_RUNTIME_JS_PROCESS_LIFECYCLE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_PROCESS_LIFECYCLE_OBSERVER_H

#include <memory>
#include <mutex>
#include <set>

#include "native_engine/native_engine.h"
#include "nocopyable.h"
#include "process_lifecycle_observer_stub.h"

class NativeReference;
typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;

namespace OHOS {
namespace AbilityRuntime {
class JsProcessLifecycleObserver : public OHOS::AppExecFwk::ProcessLifecycleObserverStub {
public:
    explicit JsProcessLifecycleObserver(napi_env env);
    ~JsProcessLifecycleObserver() override = default;
    void OnProcessCreated(const OHOS::AppExecFwk::ProcessData &processData) override;
    void OnProcessDied(const OHOS::AppExecFwk::ProcessData &processData) override;
    void HandleOnAppProcessStateChangeEvent(const OHOS::AppExecFwk::ProcessData &processData,
        const std::string &methodName);
    void CallJsFunction(napi_value value, const char *methodName, const napi_value *argv, size_t argc);
    void Register(napi_value jsObserver);
    bool UnRegister(napi_value jsObserver = nullptr);
    bool IsEmpty();

private:
    napi_env env_ = nullptr;
    std::set<std::shared_ptr<NativeReference>> observers_;
    std::mutex observerLock_;

    DISALLOW_COPY_AND_MOVE(JsProcessLifecycleObserver);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_PROCESS_LIFECYCLE_OBSERVER_H
