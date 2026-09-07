/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ohos_js_env_logger.h"

#include <mutex>
#include <string>
#include "hilog/log.h"
#include "js_env_logger.h"

#ifndef ENV_LOG_DOMAIN
#define ENV_LOG_DOMAIN 0xD001320
#endif

#ifndef ENV_LOG_TAG
#define ENV_LOG_TAG "AAFwkJsEnv"
#endif

namespace OHOS {
namespace AbilityRuntime {
void JsEnvLogger(JsEnv::JsEnvLogLevel level, const char* fileName, const char* functionName, int line,
    const char* fmt, ...)
{
    std::string cFormat = "[%{public}s(%{public}s:%{public}d)]";
    cFormat += fmt;
    va_list printArgs;
    va_start(printArgs, fmt);
    switch (level) {
        case JsEnv::JsEnvLogLevel::DEBUG:
            HILOG_IMPL(LOG_CORE, LOG_DEBUG, ENV_LOG_DOMAIN, ENV_LOG_TAG,
                cFormat.c_str(), fileName, functionName, line, printArgs);
            break;
        case JsEnv::JsEnvLogLevel::INFO:
            HILOG_IMPL(LOG_CORE, LOG_INFO, ENV_LOG_DOMAIN, ENV_LOG_TAG,
                cFormat.c_str(), fileName, functionName, line, printArgs);
            break;
        case JsEnv::JsEnvLogLevel::WARN:
            HILOG_IMPL(LOG_CORE, LOG_WARN, ENV_LOG_DOMAIN, ENV_LOG_TAG,
                cFormat.c_str(), fileName, functionName, line, printArgs);
            break;
        case JsEnv::JsEnvLogLevel::ERROR:
            HILOG_IMPL(LOG_CORE, LOG_ERROR, ENV_LOG_DOMAIN, ENV_LOG_TAG,
                cFormat.c_str(), fileName, functionName, line, printArgs);
            break;
        case JsEnv::JsEnvLogLevel::FATAL:
            HILOG_IMPL(LOG_CORE, LOG_FATAL, ENV_LOG_DOMAIN, ENV_LOG_TAG,
                cFormat.c_str(), fileName, functionName, line, printArgs);
            break;
        default:
            HILOG_IMPL(LOG_CORE, LOG_INFO, ENV_LOG_DOMAIN, ENV_LOG_TAG,
                cFormat.c_str(), fileName, functionName, line, printArgs);
            break;
    }
    va_end(printArgs);
}

void OHOSJsEnvLogger::RegisterJsEnvLogger()
{
    static std::once_flag registerFlag;
    std::call_once(registerFlag, []() {
        JsEnv::JsEnvLogger::logger = JsEnvLogger;
    });
}
} // namespace AbilityRuntime
} // namespace OHOS
