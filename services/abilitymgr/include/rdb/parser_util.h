/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RDB_PARSER_UTIL_H
#define OHOS_ABILITY_RUNTIME_RDB_PARSER_UTIL_H

#include <nlohmann/json.hpp>
#include <tuple>
#include <unordered_map>

namespace OHOS {
namespace AbilityRuntime {
/* This class is used to parse the resident process information section in files(install_list_capability.json) */
class ParserUtil final {
public:
    static ParserUtil &GetInstance();
    void GetResidentProcessRawData(
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> &list);

private:
    void ParsePreInstallAbilityConfig(const std::string &filePath,
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> &list);
    void GetPreInstallRootDirList(std::vector<std::string> &rootDirList);
    bool ReadFileIntoJson(const std::string &filePath, nlohmann::json &jsonBuf);
    bool FilterInfoFromJson(nlohmann::json &jsonBuf,
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> &list);
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_RDB_PARSER_UTIL_H