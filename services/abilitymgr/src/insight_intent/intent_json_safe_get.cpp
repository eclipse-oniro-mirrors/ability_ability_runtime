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

#include "intent_json_safe_get.h"

#include <functional>
#include <utility>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {

bool IsJsonDepthOk(const nlohmann::json &jsonObject, size_t maxDepth)
{
    std::vector<std::pair<std::reference_wrapper<const nlohmann::json>, size_t>> stack;
    stack.emplace_back(std::cref(jsonObject), 1);
    while (!stack.empty()) {
        auto back = stack.back();
        stack.pop_back();
        const nlohmann::json &node = back.first.get();
        size_t depth = back.second;
        if (depth > maxDepth) {
            return false;
        }
        if (node.is_object()) {
            for (auto it = node.begin(); it != node.end(); ++it) {
                stack.emplace_back(std::cref(it.value()), depth + 1);
            }
        } else if (node.is_array()) {
            for (const auto &item : node) {
                stack.emplace_back(std::cref(item), depth + 1);
            }
        }
    }
    return true;
}

std::string SafeDump(const nlohmann::json &jsonObject, size_t maxLen)
{
    if (!IsJsonDepthOk(jsonObject, JSON_DUMP_MAX_DEPTH)) {
        TAG_LOGE(AAFwkTag::INTENT, "json depth exceeds limit %{public}zu", JSON_DUMP_MAX_DEPTH);
        return "";
    }
    try {
        auto s = jsonObject.dump();
        return (maxLen > 0 && s.size() > maxLen) ? s.substr(0, maxLen) + "..." : s;
    } catch (...) {
        return "";
    }
}

bool SafeDumpTo(const nlohmann::json &jsonObject, std::string &out)
{
    if (!IsJsonDepthOk(jsonObject, JSON_DUMP_MAX_DEPTH)) {
        TAG_LOGE(AAFwkTag::INTENT, "json depth exceeds limit %{public}zu", JSON_DUMP_MAX_DEPTH);
        return false;
    }
    try {
        out = jsonObject.dump();
    } catch (...) {
        return false;
    }
    return true;
}

} // namespace AbilityRuntime
} // namespace OHOS
