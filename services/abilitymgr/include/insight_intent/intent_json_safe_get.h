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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_JSON_SAFE_GET_H
#define OHOS_ABILITY_RUNTIME_INTENT_JSON_SAFE_GET_H

#include <exception>
#include <string>

#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {

template<typename T>
bool SafeJsonGet(const nlohmann::json &jsonObject, T &out, const char *tag)
{
    try {
        out = jsonObject.get<T>();
        return true;
    } catch (const nlohmann::json::exception &e) {
        TAG_LOGE(AAFwkTag::INTENT, "%{public}s json exception: %{public}s", tag, e.what());
        return false;
    } catch (const std::exception &e) {
        TAG_LOGE(AAFwkTag::INTENT, "%{public}s std exception: %{public}s", tag, e.what());
        return false;
    } catch (...) {
        TAG_LOGE(AAFwkTag::INTENT, "%{public}s unknown exception", tag);
        return false;
    }
}

inline std::string SafeDump(const nlohmann::json &jsonObject, size_t maxLen = 0)
{
    try {
        auto s = jsonObject.dump();
        return (maxLen > 0 && s.size() > maxLen) ? s.substr(0, maxLen) + "..." : s;
    } catch (...) {
        return "<dump_failed>";
    }
}

inline bool SafeDumpTo(const nlohmann::json &jsonObject, std::string &out)
{
    try {
        out = jsonObject.dump();
    } catch (...) {
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INTENT_JSON_SAFE_GET_H
