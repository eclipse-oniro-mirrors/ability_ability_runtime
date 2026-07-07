/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "shell_command_config_loader.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include "hilog_tag_wrapper.h"

using json = nlohmann::json;
namespace OHOS {
namespace AAFwk {
namespace {
    constexpr const char* AA_TOOL_COMMAND_LIST = "command_list";
    constexpr static int  COMMANDS_MAX_SIZE = 100;
}

bool ShellCommandConfigLoader::configState_ = false;
std::set<std::string> ShellCommandConfigLoader::commands_ = {};

bool ShellCommandConfigLoader::ReadConfig(const std::string &filePath)
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "called");
    if (configState_) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "config read");
        return true;
    }

    if (filePath.empty()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "empty file path");
        return false;
    }

    if (access(filePath.c_str(), F_OK) != 0) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "access file: %{private}s failed", filePath.c_str());
        return false;
    }

    std::ifstream inFile;
    inFile.open(filePath, std::ios::in);
    if (!inFile.is_open()) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "read aa config error");
        return false;
    }

    json aaJson = json::parse(inFile, nullptr, false);
    inFile.close();
    if (aaJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "JSON parse error");
        return false;
    }

    if (aaJson.is_null() || aaJson.empty()) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "invalid jsonObj");
        return false;
    }

    if (!aaJson.contains(AA_TOOL_COMMAND_LIST)) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "config not contains the key");
        return false;
    }

    if (aaJson[AA_TOOL_COMMAND_LIST].is_null() || !aaJson[AA_TOOL_COMMAND_LIST].is_array() ||
        aaJson[AA_TOOL_COMMAND_LIST].empty()) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "invalid command obj size");
        return false;
    }

    if (aaJson[AA_TOOL_COMMAND_LIST].size() > COMMANDS_MAX_SIZE) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "command obj size overflow");
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mtxRead_);
    for (size_t i = 0; i < aaJson[AA_TOOL_COMMAND_LIST].size(); i++) {
        if (aaJson[AA_TOOL_COMMAND_LIST][i].is_null() || !aaJson[AA_TOOL_COMMAND_LIST][i].is_string()) {
            continue;
        }
        std::string cmd = aaJson[AA_TOOL_COMMAND_LIST][i].get<std::string>();
        TAG_LOGD(AAFwkTag::AA_TOOL, "add cmd: %{public}s", cmd.c_str());
        commands_.emplace(cmd);
    }

    aaJson.clear();
    TAG_LOGI(AAFwkTag::AA_TOOL, "read config success");
    configState_ = true;
    return true;
}

}  // namespace AAFwk
}  // namespace OHOS