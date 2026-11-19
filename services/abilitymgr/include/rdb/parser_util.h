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
#include <string>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
/* This class is used to parse the resident process information section in files(install_list_capability.json) */

// Parsed keepalive capability of one bundle from install_list_capability.json.
struct ResidentBundleCapability {
    std::string bundleName;
    std::string keepAliveEnable;
    std::string keepAliveConfiguredList;
    std::string keepAliveSaUidList;
};

class ParserUtil final {
public:
    static ParserUtil &GetInstance();
    void GetResidentProcessRawData(std::vector<ResidentBundleCapability> &list);

private:
    void ParsePreInstallAbilityConfig(const std::string &filePath,
        std::vector<ResidentBundleCapability> &list);
    void GetPreInstallRootDirList(std::vector<std::string> &rootDirList);
    bool ReadFileIntoJson(const std::string &filePath, nlohmann::json &jsonBuf);
    bool FilterInfoFromJson(nlohmann::json &jsonBuf, std::vector<ResidentBundleCapability> &list);
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_RDB_PARSER_UTIL_H