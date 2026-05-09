/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INNER_MISSION_INFO_H
#define OHOS_ABILITY_RUNTIME_INNER_MISSION_INFO_H

#include <string>

#include "mission_info.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
enum class JsonType {
    NULLABLE,
    BOOLEAN,
    NUMBER,
    OBJECT,
    ARRAY,
    STRING,
};
enum class StartMethod {
    START_NORMAL,
    START_CALL,
};

/**
 * @struct InnerMissionInfo
 * InnerMissionInfo is used to save informations about mission information.
 */
struct InnerMissionInfo {
    std::string bundleName;
    std::string specifiedFlag;
    std::string missionName;
    std::string missionAffinity;
    MissionInfo missionInfo;
    int32_t launchMode;
    int32_t startMethod;
    int32_t collaboratorType = 0;
    int32_t uid;
    bool isTemporary;
    bool hasRecoverInfo;
    std::string ToJsonStr() const;
    bool FromJsonStr(const std::string &jsonStr);
    void Dump(std::vector<std::string> &info) const;
    bool CheckJsonNode(nlohmann::json &value, const std::string &node, JsonType jsonType);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_INNER_MISSION_INFO_H
