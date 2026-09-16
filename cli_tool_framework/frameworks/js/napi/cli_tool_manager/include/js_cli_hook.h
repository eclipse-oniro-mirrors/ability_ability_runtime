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

#ifndef OHOS_CLI_TOOL_JS_CLI_HOOK_H
#define OHOS_CLI_TOOL_JS_CLI_HOOK_H

#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <string>

#include "cli_hook_interface_stub.h"
#include "exec_cmd_param.h"
#include "exec_result.h"
#include "exec_result_wrap.h"
#include "exec_tool_param.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief IPC Stub that wraps a JS CliHook object.
 *
 * When the CliToolManagerService invokes a hook callback (BeforeCallTool etc.),
 * this Stub receives the IPC on a binder thread and synchronously dispatches
 * the call to the JS thread via napi_threadsafe_function. The JS callback
 * returns a (possibly modified) object which is parsed back to native data.
 *
 * Timeout: 5000ms; on timeout the original parameter is left unchanged.
 */
class JsCliHook : public CliHookInterfaceStub {
public:
    explicit JsCliHook(napi_env env, napi_value jsHookObj);
    ~JsCliHook() override;

    ErrCode BeforeCallTool(ExecToolParam& param) override;
    ErrCode AfterCallTool(ExecResultWrap& execResultWrap) override;
    ErrCode BeforeCallCmd(ExecCmdParam& param) override;
    ErrCode AfterCallCmd(ExecResultWrap& execResultWrap) override;

    napi_ref GetCallbackRef() const { return callbackRef_; }
    bool IsValid() const { return callbackRef_ != nullptr && tsfn_ != nullptr; }
    bool IsSameEnv(napi_env env) const { return env == env_; }
    void ReleaseResources();

    enum class HookMethodType : uint8_t {
        BEFORE_CALL_TOOL,
        AFTER_CALL_TOOL,
        BEFORE_CALL_CMD,
        AFTER_CALL_CMD,
    };

    struct HookCallData {
        HookMethodType methodType;
        std::shared_ptr<ExecToolParam> toolParam;
        std::shared_ptr<ExecCmdParam> cmdParam;
        std::shared_ptr<ExecResultWrap> execResultWrap;
        std::shared_ptr<std::promise<void>> promise;
    };

private:
    napi_env env_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    napi_threadsafe_function tsfn_ = nullptr;
    std::atomic<bool> released_{false};
    std::mutex mutex_;

    static void CallJs(napi_env env, napi_value jsCb, void* context, void* data);
    static void Finalize(napi_env env, void* data, void* hint);
    void ReleaseInternal();

    bool DispatchToJs(std::shared_ptr<HookCallData> callData);
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_JS_CLI_HOOK_H
