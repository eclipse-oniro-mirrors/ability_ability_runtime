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

#ifndef ARKTSSCRIPT_H
#define ARKTSSCRIPT_H

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>

#include "context_impl.h"
#include "js_runtime.h"
#include "napi/native_api.h"

namespace OHOS {
namespace ArktsScript {

using ContextImpl = OHOS::AbilityRuntime::ContextImpl;
using JsRuntime = OHOS::AbilityRuntime::JsRuntime;

enum class ScriptArgType {
    UNDEFINED,
    STRING,
    INT32,
    DOUBLE,
    BOOLEAN,
    JSON_VALUE,
};

struct ScriptArg {
    ScriptArgType type = ScriptArgType::UNDEFINED;
    std::string value;
};

struct ScriptArgs {
    std::string abcPath;
    std::string scriptName;
    std::string funName;
    std::vector<ScriptArg> arguments;
    bool showHelp = false;
};

struct ScriptError {
    std::string message;
    std::string type;
};

struct ExecutionContext {
    std::atomic<bool> resultReady{false};
    std::atomic<bool> scriptDone{false};
    std::mutex mutex;
    std::string result;
    ScriptError error;
};

using ResultCallback = std::function<void(bool success, const std::string& result, const ScriptError& error)>;

struct CompletionChannel {
    int eventFd = -1;
    int epollFd = -1;
};

struct ExitState {
    std::shared_ptr<ExecutionContext> execContext;
};

struct ExecutionSnapshot {
    std::string result;
    ScriptError error;
    bool resultReady = false;
    bool scriptDone = false;
};

class ArktsScript final {
public:
    ArktsScript() = delete;
    ~ArktsScript() = delete;

    static bool ParseArguments(int argc, char* argv[], ScriptArgs& args);
    static std::shared_ptr<ContextImpl> CreateScriptContext();
    static std::unique_ptr<JsRuntime> CreateJsRuntime(const std::shared_ptr<ContextImpl>& context);
    static bool LoadAbcFile(JsRuntime* runtime, const std::string& path);
    static void OutputResult(const std::string& result);
    static void OutputError(const ScriptError& error);
    static int RunArkTsScript(int argc, char* argv[]);

private:
    static bool ParseHelpOption(int argc, char* argv[], ScriptArgs& args);
    static bool ParseScriptOption(int argc, char* argv[], int& index, ScriptArgs& args,
        std::map<uint32_t, ScriptArg>& indexedArgs, bool& hasArgsJson);
    static bool FinalizeParsedArguments(const std::map<uint32_t, ScriptArg>& indexedArgs,
        ScriptArgs& args);
    static bool ParseFunctionAndArguments(int argc, char* argv[], int& index, ScriptArgs& args);
    static void PrintUsage();
    static void CloseCompletionChannel(CompletionChannel& channel);
    static bool CreateCompletionChannel(CompletionChannel& channel);
    static bool SignalCompletion(int eventFd);
    static bool PublishResult(const std::shared_ptr<ExecutionContext>& execContext, int eventFd,
        const std::string& result);
    static bool PublishFailure(const std::shared_ptr<ExecutionContext>& execContext, int eventFd,
        const ScriptError& error);
    static bool MarkScriptDone(const std::shared_ptr<ExecutionContext>& execContext, int eventFd);
    static void ReadCompletionSignal(const CompletionChannel& channel);
    static ExecutionSnapshot TakeExecutionSnapshot(const std::shared_ptr<ExecutionContext>& execContext);
    static void HandleExecutionSnapshot(const ExecutionSnapshot& snapshot, const CompletionChannel& channel);
    static void MonitorCompletion(ExitState state, CompletionChannel channel);
    static bool StartMonitor(CompletionChannel& channel, std::shared_ptr<ExecutionContext>& execContext,
        std::thread& monitorThread);
    static int FinalizeAndJoin(const std::shared_ptr<ExecutionContext>& execContext, CompletionChannel& channel,
        std::thread& monitorThread, const ScriptError& error);
    static ResultCallback CreateCompleteCallback(const std::shared_ptr<ExecutionContext>& execContext,
        CompletionChannel channel);
    static bool PrepareRuntimeEnvironment(const ResultCallback& completeCallback,
        std::shared_ptr<ContextImpl>& context, std::unique_ptr<JsRuntime>& runtimeOwner, ScriptError& error);
    static bool LoadScriptFile(const ScriptArgs& args, JsRuntime* runtime, ScriptError& error);
    static bool ResolveScriptFunction(const ScriptArgs& args, JsRuntime* runtime,
        napi_value& receiver, napi_value& func, ScriptError& error);
    static bool CallResolvedFunction(const ScriptArgs& args, napi_env env, napi_value receiver, napi_value func,
        ScriptError& error);
    [[noreturn]] static void ExitFromMonitor(const CompletionChannel& channel, int exitCode);
};

} // namespace ArktsScript
} // namespace OHOS

#endif // ARKTSSCRIPT_H
