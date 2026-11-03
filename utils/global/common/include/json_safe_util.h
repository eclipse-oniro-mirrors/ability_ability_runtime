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

#ifndef OHOS_ABILITY_RUNTIME_JSON_SAFE_UTIL_H
#define OHOS_ABILITY_RUNTIME_JSON_SAFE_UTIL_H

#include <cstddef>
#include <exception>
#include <string>

#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t JSON_SAFE_PARSE_MAX_LEN = 128 * 1024 * 1024;
constexpr size_t JSON_SAFE_MAX_DEPTH = 100;
constexpr size_t DUMP_TOKEN_MAX_LEN = 50;
constexpr size_t DUMP_LOG_MAX_LEN = 1000;

bool SafeParse(const std::string &jsonStr, nlohmann::json &jsonObject);
bool SafeDump(const nlohmann::json &jsonObject, std::string &out, size_t maxLen = 0);
std::string SafeDump(const nlohmann::json &jsonObject, size_t maxLen = 0);

// Header-only template using try/catch: any translation unit that includes
// this header must be built with exceptions enabled (use_exceptions = true).
template<typename T>
bool SafeJsonGet(const nlohmann::json &jsonObject, T &out, const char *tag)
{
    try {
        out = jsonObject.get<T>();
        return true;
    } catch (const nlohmann::json::exception &e) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s json exception: %{public}s", tag, e.what());
        return false;
    } catch (const std::exception &e) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s std exception: %{public}s", tag, e.what());
        return false;
    } catch (...) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s unknown exception", tag);
        return false;
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JSON_SAFE_UTIL_H
