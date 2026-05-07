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

#ifndef OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_H
#define OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_H

#include <memory>
#include "ecmascript/napi/include/dfx_jsnapi.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "js_environment_impl.h"
#include "source_map_operator.h"
#include "uncaught_exception_callback.h"

namespace OHOS {
namespace JsEnv {
struct WorkerInfo;
class JsEnvironmentImpl;
using DebuggerPostTask = std::function<void(std::function<void()>&&)>;
using RequestAotCallback =
    std::function<int32_t(const std::string& bundleName, const std::string& moduleName, int32_t triggerMode)>;
using UncatchableTask = std::function<void(std::string summary, const JsEnv::ErrorObject errorObject, napi_env env,
    napi_value exception)>;
class JsEnvironment final : public std::enable_shared_from_this<JsEnvironment> {
public:
    JsEnvironment() {}
    explicit JsEnvironment(std::unique_ptr<JsEnvironmentImpl> impl);
    ~JsEnvironment();

    enum class PROFILERTYPE {
        PROFILERTYPE_CPU,
        PROFILERTYPE_HEAP
    };

    bool Initialize(const panda::RuntimeOption& pandaOption, void* jsEngine);

    NativeEngine* GetNativeEngine() const
    {
        return engine_;
    }

    std::shared_ptr<SourceMapOperator> GetSourceMapOperator() const
    {
        return sourceMapOperator_;
    }

    panda::ecmascript::EcmaVM* GetVM() const
    {
        return vm_;
    }

    void InitTimerModule();

    void InitWorkerModule(std::shared_ptr<WorkerInfo> workerInfo);

    void InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj);

    void InitSyscapModule();

    void PostTask(const std::function<void()>& task, const std::string& name = "", int64_t delayTime = 0);

    void PostSyncTask(const std::function<void()>& task, const std::string& name);

    void RemoveTask(const std::string& name);

    void RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo& uncaughtExceptionInfo,
        bool isStatic = false);

    void RegisterUncatchableExceptionHandler(const JsEnv::UncatchableTask& uncatchableTask,
        bool isStatic = false);

    bool LoadScript(const std::string& path, std::vector<uint8_t>* buffer = nullptr, bool isBundle = false);

    bool StartDebugger(
        std::string& option, uint32_t socketFd, bool isDebugApp);

    void StopDebugger();

    void StopDebugger(std::string& option);

    void InitConsoleModule();

    bool InitLoop(bool isStage = true);

    void DeInitLoop();

    bool LoadScript(const std::string& path, uint8_t* buffer, size_t len, bool isBundle);

    DebuggerPostTask GetDebuggerPostTask();

    void StartProfiler(const char* libraryPath,
        uint32_t instanceId, PROFILERTYPE profiler, int32_t interval, int tid, bool isDebugApp);

    void DestroyHeapProfiler();

    void GetHeapPrepare();

    void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate);

    void ReInitJsEnvImpl(std::unique_ptr<JsEnvironmentImpl> impl);

    void SetRequestAotCallback(const RequestAotCallback& cb);

    void SetDeviceDisconnectCallback(const std::function<bool()> &cb);

    void NotifyDebugMode(int tid, const char* libraryPath, uint32_t instanceId, bool isDebugApp, bool debugMode);

    bool GetDebugMode() const;

    int32_t ParseHdcRegisterOption(std::string& option);

private:
    std::unique_ptr<JsEnvironmentImpl> impl_ = nullptr;
    NativeEngine* engine_ = nullptr;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
    std::shared_ptr<SourceMapOperator> sourceMapOperator_ = nullptr;
    bool debugMode_ = false;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_H
