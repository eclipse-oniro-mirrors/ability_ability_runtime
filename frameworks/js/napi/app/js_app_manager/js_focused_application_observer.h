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

#ifndef OHOS_ABILITY_RUNTIME_JS_FOCUSED_APPLICATION_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_FOCUSED_APPLICATION_OBSERVER_H

#include <atomic>
#include <map>
#include <mutex>

#include "event_handler.h"
#include "focused_application_info.h"
#include "focused_application_observer_stub.h"
#include "native_engine/native_engine.h"
#include "nocopyable.h"

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::FocusedApplicationObserverStub;
using OHOS::AppExecFwk::FocusedApplicationInfo;
class JSFocusedApplicationObserver : public FocusedApplicationObserverStub {
public:
    explicit JSFocusedApplicationObserver(napi_env engine);
    ~JSFocusedApplicationObserver() override = default;
    void OnFocusedApplicationChanged(const FocusedApplicationInfo &focusedApplicationInfo) override;
    void HandleOnFocusedApplicationChanged(const FocusedApplicationInfo &focusedApplicationInfo);
    void CallJsFunction(napi_value value, const char *methodName, const napi_value *argv, size_t argc);
    void AddJsObserverObjectWithId(int32_t observerId, const napi_value &jsObserverObject);
    bool RemoveJsObserverObjectById(int32_t observerId);
    bool IsEmpty();
    void SetValid(bool valid);

private:
    napi_env env_;
    std::atomic<bool> valid_ = true;
    std::map<int32_t, std::shared_ptr<NativeReference>> jsObserverMap_;
    std::mutex jsObserverMapLock_;

    DISALLOW_COPY_AND_MOVE(JSFocusedApplicationObserver);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_FOCUSED_APPLICATION_OBSERVER_H
