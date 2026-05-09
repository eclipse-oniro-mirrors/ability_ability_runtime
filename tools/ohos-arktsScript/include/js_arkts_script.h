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

#ifndef ARKTSSCRIPT_JS_H
#define ARKTSSCRIPT_JS_H

#include <memory>
#include <string>
#include <vector>

#include "arkts_script.h"
#include "context.h"
#include "napi/native_api.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
struct ApplicationInfo;
}
namespace ArktsScript {

class JsArktsScript final {
public:
    JsArktsScript() = delete;
    ~JsArktsScript() = delete;

    static bool BindContextToGlobal(napi_env env, const std::shared_ptr<ContextImpl>& context);
    static bool BindCompletearktsScript(napi_env env, const ResultCallback& callback);
    static bool ResolveFunction(JsRuntime* runtime, napi_env env, const std::string& abcPath,
        const std::string& scriptName, const std::string& funName, napi_value& receiver, napi_value& func);
    static napi_value ConvertArgumentsToNapi(napi_env env, const std::vector<ScriptArg>& arguments);
    static std::string StringifyObject(napi_env env, napi_value result);

private:
    struct CompletePayload {
        int32_t code = 0;
        nlohmann::json resultJson;
        bool hasResult = false;
        std::string uri;
        bool hasUri = false;
        int32_t flag = 0;
        bool hasFlag = false;
    };

    static std::string NormalizeScriptName(const std::string& scriptName);
    static bool ResolveFunctionFromExports(JsRuntime* runtime, napi_env env, const std::string& abcPath,
        const std::string& scriptName, const std::string& funName, napi_value& receiver, napi_value& func);
    static bool ResolveFunctionFromGlobal(napi_env env, const std::string& scriptName, const std::string& funName,
        napi_value& receiver, napi_value& func);
    static bool SetNamedStringOrNull(napi_env env, napi_value object, const char* name, const std::string& value);
    static bool SetNamedNull(napi_env env, napi_value object, const char* name);
    static napi_value CreateApplicationInfoObject(napi_env env,
        const std::shared_ptr<AppExecFwk::ApplicationInfo>& appInfo);
    static void SetApplicationInfoProperty(napi_env env, napi_value object,
        const std::shared_ptr<OHOS::AbilityRuntime::Context>& context);
    static void SetNullContextProperties(napi_env env, napi_value object);
    static void SetContextDirectoryProperties(napi_env env, napi_value object,
        const std::shared_ptr<OHOS::AbilityRuntime::Context>& context);
    static napi_value CreateApplicationContextCallback(napi_env callbackEnv, napi_callback_info info);
    static napi_value CreateBundleContextCallback(napi_env callbackEnv, napi_callback_info info);
    static void BindContextFunction(napi_env env, napi_value object, const char* name, napi_callback callback);
    static napi_value CreateScriptContextObject(napi_env env,
        const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
        const std::string& bundleNameOverride = std::string());
    static bool GetUtf8String(napi_env env, napi_value value, std::string& output);
    static bool GetInt32Property(napi_env env, napi_value object, const char* name, int32_t& output);
    static bool GetOptionalInt32Property(napi_env env, napi_value object, const char* name,
        int32_t& output, bool& hasValue);
    static bool GetOptionalStringProperty(napi_env env, napi_value object, const char* name,
        std::string& output, bool& hasValue);
    static bool GetOptionalJsonObjectProperty(napi_env env, napi_value object, const char* name,
        nlohmann::json& output, bool& hasValue);
    static bool ReadCompletePayload(napi_env env, napi_value value, CompletePayload& payload, std::string& error);
    static nlohmann::json BuildCompletePayloadJson(const CompletePayload& payload);
    static void ClearCompleteCallbackData();
    static void FinishCompleteCallback(bool success, const std::string& result, const ScriptError& error);
    static napi_value CompletearktsScriptWithError(const std::string& error);
    static napi_value CompletearktsScript(napi_env env, napi_callback_info info);
};

} // namespace ArktsScript
} // namespace OHOS

#endif // ARKTSSCRIPT_JS_H
