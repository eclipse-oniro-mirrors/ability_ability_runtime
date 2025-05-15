/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_STARTUP_TASK_H
#define OHOS_ABILITY_RUNTIME_JS_STARTUP_TASK_H

#include "app_startup_task.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_startup_task_result.h"

namespace OHOS {
namespace AbilityRuntime {
class JsStartupTask : public AppStartupTask {
public:
    static const std::string TASK_TYPE;

    JsStartupTask(const std::string &name, JsRuntime &jsRuntime,
        std::unique_ptr<NativeReference> &startupJsRef, std::shared_ptr<NativeReference> &contextJsRef_);

    ~JsStartupTask() override;

    const std::string &GetType() const override;

    int32_t RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback) override;

    int32_t RunTaskOnDependencyCompleted(const std::string &dependencyName,
        const std::shared_ptr<StartupTaskResult> &result) override;

    void OnAsyncTaskCompleted(const std::shared_ptr<StartupTaskResult> &result);

    void UpdateContextRef(std::shared_ptr<NativeReference> &contextJsRef);

private:
    JsRuntime &jsRuntime_;
    std::unique_ptr<NativeReference> startupJsRef_;
    std::shared_ptr<NativeReference> contextJsRef_;
    std::unique_ptr<NativeReference> AsyncTaskExecutorJsRef_;
    std::unique_ptr<NativeReference> AsyncTaskExecutorCallbackJsRef_;
    std::unique_ptr<StartupTaskResultCallback> startupTaskResultCallback_;

    static napi_value GetDependencyResult(napi_env env, const std::string &dependencyName,
        const std::shared_ptr<StartupTaskResult> &result);

    int32_t LoadJsAsyncTaskExecutor();

    void LoadJsAsyncTaskCallback();
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_STARTUP_TASK_H
