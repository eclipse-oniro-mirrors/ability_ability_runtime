/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_QUICKFIX_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_QUICKFIX_CALLBACK_H

#include <vector>
#include <string>
#include <map>

namespace OHOS {
namespace AbilityRuntime {
class JsRuntime;
class JsQuickfixCallback final {
public:
    explicit JsQuickfixCallback(
        const std::map<std::string, std::string> moduleAndHqfPath) : moduleAndHqfPath_(moduleAndHqfPath) {};
    ~JsQuickfixCallback() = default;

    bool operator()(std::string baseFileName,
                    std::string &patchFileName,
                    uint8_t **patchBuffer,
                    size_t &patchSize);

private:
    std::vector<uint8_t> newpatchBuffer_;
    std::map<std::string, std::string> moduleAndHqfPath_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_QUICKFIX_CALLBACK_H
