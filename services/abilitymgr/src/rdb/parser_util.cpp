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

#include "parser_util.h"

#include <fstream>
#include <unistd.h>

#include "config_policy_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *DEFAULT_PRE_BUNDLE_ROOT_DIR = "/system";
constexpr const char *PRODUCT_SUFFIX = "/etc/app";
constexpr const char *INSTALL_LIST_CAPABILITY_CONFIG = "/install_list_capability.json";
constexpr const char *INSTALL_LIST = "install_list";
constexpr const char *BUNDLE_NAME = "bundleName";
constexpr const char *KEEP_ALIVE = "keepAlive";
constexpr const char *KEEP_ALIVE_ENABLE = "keepAliveEnable";
constexpr const char *KEEP_ALIVE_CONFIGURED_LIST = "keepAliveConfiguredList";
constexpr const char *KEEP_ALIVE_SA_UID_LIST = "keepAliveSaUidList";

} // namespace
ParserUtil &ParserUtil::GetInstance()
{
    static ParserUtil instance;
    return instance;
}

void ParserUtil::GetResidentProcessRawData(
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> &list)
{
    std::vector<std::string> rootDirList;
    GetPreInstallRootDirList(rootDirList);

    for (auto &root : rootDirList) {
        auto fileDir = root.append(PRODUCT_SUFFIX).append(INSTALL_LIST_CAPABILITY_CONFIG);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Search file dir : %{public}s", fileDir.c_str());
        ParsePreInstallAbilityConfig(fileDir, list);
    }
}

void ParserUtil::ParsePreInstallAbilityConfig(const std::string &filePath,
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> &list)
{
    nlohmann::json jsonBuf;
    if (!ReadFileIntoJson(filePath, jsonBuf)) {
        return;
    }

    if (jsonBuf.is_discarded()) {
        return;
    }

    FilterInfoFromJson(jsonBuf, list);
}

bool ParserUtil::FilterInfoFromJson(nlohmann::json &jsonBuf,
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> &list)
{
    if (jsonBuf.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "format error");
        return false;
    }

    if (jsonBuf.find(INSTALL_LIST) == jsonBuf.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "installList absent");
        return false;
    }

    auto arrays = jsonBuf.at(INSTALL_LIST);
    if (!arrays.is_array() || arrays.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not found");
        return false;
    }

    std::string bundleName;
    std::string KeepAliveEnable = "1";
    std::string KeepAliveConfiguredList;
    std::string KeepAliveSaUidList;
    for (const auto &array : arrays) {
        if (!array.is_object()) {
            continue;
        }

        // Judgment logic exists, not found, not bool, not resident process
        if (!(array.find(KEEP_ALIVE) != array.end() && array.at(KEEP_ALIVE).is_boolean() &&
                array.at(KEEP_ALIVE).get<bool>())) {
            continue;
        }

        if (!(array.find(BUNDLE_NAME) != array.end() && array.at(BUNDLE_NAME).is_string())) {
            continue;
        }

        bundleName = array.at(BUNDLE_NAME).get<std::string>();

        if (array.find(KEEP_ALIVE_ENABLE) != array.end() && array.at(KEEP_ALIVE_ENABLE).is_boolean()) {
            auto val = array.at(KEEP_ALIVE_ENABLE).get<bool>();
            KeepAliveEnable = std::to_string(val);
        }

        if (array.find(KEEP_ALIVE_CONFIGURED_LIST) != array.end() && array.at(KEEP_ALIVE_CONFIGURED_LIST).is_array()) {
            // Save directly in the form of an array and parse it when in use
            KeepAliveConfiguredList = array.at(KEEP_ALIVE_CONFIGURED_LIST).dump();
        }

        if (array.find(KEEP_ALIVE_SA_UID_LIST) != array.end() && array.at(KEEP_ALIVE_SA_UID_LIST).is_array()) {
            // Save directly in the form of an array and parse it when in use
            KeepAliveSaUidList = array.at(KEEP_ALIVE_SA_UID_LIST).dump();
        }

        list.emplace_back(std::make_tuple(bundleName, KeepAliveEnable, KeepAliveConfiguredList, KeepAliveSaUidList));
        bundleName.clear();
        KeepAliveEnable = "1";
        KeepAliveConfiguredList.clear();
        KeepAliveSaUidList.clear();
    }

    return true;
}

void ParserUtil::GetPreInstallRootDirList(std::vector<std::string> &rootDirList)
{
    auto cfgDirList = GetCfgDirList();
    if (cfgDirList != nullptr) {
        for (const auto &cfgDir : cfgDirList->paths) {
            if (cfgDir == nullptr) {
                continue;
            }
            rootDirList.emplace_back(cfgDir);
        }

        FreeCfgDirList(cfgDirList);
    }
    bool ret = std::find(rootDirList.begin(), rootDirList.end(), DEFAULT_PRE_BUNDLE_ROOT_DIR) != rootDirList.end();
    if (!ret) {
        rootDirList.emplace_back(DEFAULT_PRE_BUNDLE_ROOT_DIR);
    }
}

bool ParserUtil::ReadFileIntoJson(const std::string &filePath, nlohmann::json &jsonBuf)
{
    if (access(filePath.c_str(), F_OK) != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "path not exist");
        return false;
    }

    if (filePath.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "empty path");
        return false;
    }

    char path[PATH_MAX] = {0};
    if (realpath(filePath.c_str(), path) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "realpath error:%{public}d", errno);
        return false;
    }

    std::ifstream fin(path);
    if (!fin.is_open()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "path exception");
        return false;
    }

    fin.seekg(0, std::ios::end);
    int64_t size = fin.tellg();
    if (size <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "empty file");
        fin.close();
        return false;
    }

    fin.seekg(0, std::ios::beg);
    jsonBuf = nlohmann::json::parse(fin, nullptr, false);
    fin.close();
    if (jsonBuf.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bad profile");
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS