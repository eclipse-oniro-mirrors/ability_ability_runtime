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

#include "js_environment.h"

#include <chrono>

#include "console.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "js_environment_impl.h"
#include "uncaught_exception_callback.h"
#include "worker_info.h"

namespace OHOS {
namespace JsEnv {
namespace {
static const std::string DEBUGGER = "@Debugger";
static const std::string NOT_INIT = "SourceMap is not initialized yet \n";
}

static panda::DFXJSNApi::ProfilerType ConvertProfilerType(JsEnvironment::PROFILERTYPE type)
{
    if (type == JsEnvironment::PROFILERTYPE::PROFILERTYPE_CPU) {
        return panda::DFXJSNApi::ProfilerType::CPU_PROFILER;
    } else {
        return panda::DFXJSNApi::ProfilerType::HEAP_PROFILER;
    }
}

JsEnvironment::JsEnvironment(std::unique_ptr<JsEnvironmentImpl> impl) : impl_(std::move(impl))
{}

JsEnvironment::~JsEnvironment()
{
    TAG_LOGD(AAFwkTag::JSENV, "called");

    if (engine_ != nullptr) {
        delete engine_;
        engine_ = nullptr;
    }

    if (vm_ != nullptr) {
        panda::JSNApi::DestroyJSVM(vm_);
        vm_ = nullptr;
    }
}

bool JsEnvironment::Initialize(const panda::RuntimeOption& pandaOption, void* jsEngine)
{
    TAG_LOGD(AAFwkTag::JSENV, "Js environment initialize");
    vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Create vm failed");
        return false;
    }

    engine_ = new ArkNativeEngine(vm_, jsEngine);
    return true;
}

void JsEnvironment::InitTimerModule()
{
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid native engine");
        return;
    }

    if (impl_ != nullptr) {
        impl_->InitTimerModule(engine_);
    }
}

void JsEnvironment::InitWorkerModule(std::shared_ptr<WorkerInfo> workerInfo)
{
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid native engine");
        return;
    }

    if (impl_ != nullptr) {
        impl_->InitWorkerModule(engine_, workerInfo);
    }
}

void JsEnvironment::InitSyscapModule()
{
    if (impl_ != nullptr) {
        impl_->InitSyscapModule();
    }
}

void JsEnvironment::PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime)
{
    if (impl_ != nullptr) {
        impl_->PostTask(task, name, delayTime);
    }
}

void JsEnvironment::PostSyncTask(const std::function<void()>& task, const std::string& name)
{
    if (impl_ != nullptr) {
        impl_->PostSyncTask(task, name);
    }
}

void JsEnvironment::RemoveTask(const std::string& name)
{
    if (impl_ != nullptr) {
        impl_->RemoveTask(name);
    }
}

void JsEnvironment::InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj)
{
    sourceMapOperator_ = operatorObj;
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid Native Engine");
        return;
    }

    if (sourceMapOperator_ != nullptr) {
        sourceMapOperator_->InitSourceMap();
    }

    auto translateBySourceMapFunc = [&](const std::string& rawStack) -> std::string {
        if (sourceMapOperator_ != nullptr && sourceMapOperator_->GetInitStatus()) {
            return sourceMapOperator_->TranslateBySourceMap(rawStack);
        } else {
            return NOT_INIT + rawStack;
        }
    };
    engine_->RegisterTranslateBySourceMap(translateBySourceMapFunc);

    auto translateUrlBySourceMapFunc = [&](std::string& url, int& line, int& column, std::string& packageName) -> bool {
        if (sourceMapOperator_ != nullptr && sourceMapOperator_->GetInitStatus()) {
            return sourceMapOperator_->TranslateUrlPositionBySourceMap(url, line, column, packageName);
        }
        return false;
    };
    engine_->RegisterSourceMapTranslateCallback(translateUrlBySourceMapFunc);
}

void JsEnvironment::RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo& uncaughtExceptionInfo,
                                                     bool isStatic)
{
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid Native Engine");
        return;
    }

    engine_->RegisterNapiUncaughtExceptionHandler(NapiUncaughtExceptionCallback(uncaughtExceptionInfo.uncaughtTask,
        sourceMapOperator_, reinterpret_cast<napi_env>(engine_), isStatic));
}

void JsEnvironment::RegisterUncatchableExceptionHandler(const JsEnv::UncatchableTask& uncatchableTask, bool isStatic)
{
    if (uncatchableTask == nullptr) {
        return;
    }

    std::weak_ptr<JsEnvironment> weakThis = shared_from_this();
    panda::JSNApi::RegisterUncatchableErrorHandler(const_cast<EcmaVM *>(engine_->GetEcmaVm()),
        [weakThis, uncatchableTask, isStatic] (auto& trycatch) {
            auto sharedThis = weakThis.lock();
            if (sharedThis) {
                void* env = trycatch.GetEnv();
                NapiUncaughtExceptionCallback napiUncaughtExceptionCallback(uncatchableTask,
                    sharedThis->sourceMapOperator_, reinterpret_cast<napi_env>(env), isStatic);
                napiUncaughtExceptionCallback(trycatch);
            } else {
                TAG_LOGE(AAFwkTag::JSENV, "JsEnvironment has been destructed.");
            }
        });
}

bool JsEnvironment::LoadScript(const std::string& path, std::vector<uint8_t>* buffer, bool isBundle)
{
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid Native Engine");
        return false;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(reinterpret_cast<napi_env>(engine_), &scope);
    if (scope == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "scope is nullptr!");
        return false;
    }

    if (buffer == nullptr) {
        napi_close_handle_scope(reinterpret_cast<napi_env>(engine_), scope);
        return engine_->RunScriptPath(path.c_str());
    }
    auto start = std::chrono::high_resolution_clock::now();
    bool ret = engine_->RunScriptBuffer(path.c_str(), *buffer, isBundle) != nullptr;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    TAG_LOGI(AAFwkTag::JSENV, "timing: %{public}lld", duration_ms);
    napi_close_handle_scope(reinterpret_cast<napi_env>(engine_), scope);
    return ret;
}

bool JsEnvironment::StartDebugger(
    std::string& option, uint32_t socketFd, bool isDebugApp)
{
    TAG_LOGD(AAFwkTag::JSENV, "call");
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid vm");
        return false;
    }
    int32_t identifierId = ParseHdcRegisterOption(option);
    if (identifierId == -1) {
        TAG_LOGE(AAFwkTag::JSENV, "Abnormal parsing of tid results");
        return false;
    }
    debugMode_ = panda::JSNApi::StartDebuggerForSocketPair(identifierId, socketFd);
    return debugMode_;
}

void JsEnvironment::StopDebugger()
{
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid vm");
        return;
    }

    (void)panda::JSNApi::StopDebugger(vm_);
}

void JsEnvironment::StopDebugger(std::string& option)
{
    int32_t identifierId = ParseHdcRegisterOption(option);
    if (identifierId == -1) {
        TAG_LOGE(AAFwkTag::JSENV, "Abnormal parsing of tid results");
        return;
    }
    panda::JSNApi::StopDebugger(identifierId);
}

void JsEnvironment::InitConsoleModule()
{
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid Native Engine");
        return;
    }

    if (impl_ != nullptr) {
        impl_->InitConsoleModule(engine_);
    }
}

bool JsEnvironment::InitLoop(bool isStage)
{
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid Native Engine");
        return false;
    }

    if (impl_ != nullptr) {
        impl_->InitLoop(engine_, isStage);
    }
    return true;
}

void JsEnvironment::DeInitLoop()
{
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid Native Engine");
        return;
    }

    if (impl_ != nullptr) {
        impl_->DeInitLoop(engine_);
    }
}

bool JsEnvironment::LoadScript(const std::string& path, uint8_t* buffer, size_t len, bool isBundle)
{
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid Native Engine");
        return false;
    }

    auto start = std::chrono::high_resolution_clock::now();
    bool ret = engine_->RunScriptBuffer(path, buffer, len, isBundle);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    TAG_LOGI(AAFwkTag::JSENV, "timing: %{public}lld", duration_ms);

    return ret;
}

void JsEnvironment::StartProfiler(const char* libraryPath, uint32_t instanceId, PROFILERTYPE profiler,
    int32_t interval, int tid, bool isDebugApp)
{
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid vm");
        return;
    }

    auto debuggerPostTask = [weak = weak_from_this()](std::function<void()>&& task) {
        auto jsEnv = weak.lock();
        if (jsEnv == nullptr) {
            TAG_LOGE(AAFwkTag::JSENV, "JsEnv is invalid");
            return;
        }
        jsEnv->PostTask(task, "JsEnvironment::StartProfiler");
    };

    panda::DFXJSNApi::ProfilerOption option;
    option.libraryPath = libraryPath;
    option.profilerType = ConvertProfilerType(profiler);
    option.interval = interval;

    panda::DFXJSNApi::StartProfiler(vm_, option, tid, instanceId, debuggerPostTask, isDebugApp);
}

void JsEnvironment::DestroyHeapProfiler()
{
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid vm");
        return;
    }
    panda::DFXJSNApi::DestroyHeapProfiler(vm_);
}

void JsEnvironment::GetHeapPrepare()
{
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid vm");
        return;
    }
    panda::DFXJSNApi::GetHeapPrepare(vm_);
}

void JsEnvironment::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate> moduleCheckerDelegate)
{
    if (engine_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid native engine");
        return;
    }

    engine_->SetModuleLoadChecker(moduleCheckerDelegate);
}

void JsEnvironment::ReInitJsEnvImpl(std::unique_ptr<JsEnvironmentImpl> impl)
{
    TAG_LOGD(AAFwkTag::JSENV, "ReInit jsenv impl.");
    impl_ = std::move(impl);
}

void JsEnvironment::SetRequestAotCallback(const RequestAotCallback& cb)
{
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid vm");
        return;
    }

    panda::JSNApi::SetRequestAotCallback(vm_, cb);
}

void JsEnvironment::SetDeviceDisconnectCallback(const std::function<bool()> &cb)
{
    panda::JSNApi::SetDeviceDisconnectCallback(vm_, std::move(cb));
}

DebuggerPostTask JsEnvironment::GetDebuggerPostTask()
{
    auto debuggerPostTask = [weak = weak_from_this()](std::function<void()>&& task) {
        auto jsEnv = weak.lock();
        if (jsEnv == nullptr) {
            TAG_LOGE(AAFwkTag::JSENV, "JsEnv is invalid");
            return;
        }
        jsEnv->PostTask(task, "JsEnvironment:GetDebuggerPostTask");
    };
    return debuggerPostTask;
}

void JsEnvironment::NotifyDebugMode(
    int tid, const char* libraryPath, uint32_t instanceId, bool debug, bool debugMode)
{
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Invalid vm");
        return;
    }
    panda::JSNApi::DebugOption debugOption = {libraryPath, debug ? debugMode : false};
    auto debuggerPostTask = [weak = weak_from_this()](std::function<void()>&& task) {
        auto jsEnv = weak.lock();
        if (jsEnv == nullptr) {
            TAG_LOGE(AAFwkTag::JSENV, "JsEnv is invalid");
            return;
        }
        jsEnv->PostTask(task, "JsEnvironment:NotifyDebugMode");
    };
    panda::JSNApi::NotifyDebugMode(tid, vm_, debugOption, instanceId, debuggerPostTask, debug);
}

int32_t JsEnvironment::ParseHdcRegisterOption(std::string& option)
{
    TAG_LOGD(AAFwkTag::JSENV, "Start");
    std::size_t pos = option.find_first_of(":");
    if (pos == std::string::npos) {
        return -1;
    }
    std::string idStr = option.substr(pos + 1);
    pos = idStr.find(DEBUGGER);
    if (pos == std::string::npos) {
        return -1;
    }
    idStr = idStr.substr(0, pos);
    pos = idStr.find("@");
    if (pos != std::string::npos) {
        idStr = idStr.substr(pos + 1);
    }
    return std::atoi(idStr.c_str());
}

bool JsEnvironment::GetDebugMode() const
{
    return debugMode_;
}
} // namespace JsEnv
} // namespace OHOS
