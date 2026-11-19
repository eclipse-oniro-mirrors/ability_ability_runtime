/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#ifndef OHOS_ABILITY_RUNTIME_RAW_DATA_UTILS_H
#define OHOS_ABILITY_RUNTIME_RAW_DATA_UTILS_H

#include <cstdint>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

// Read exactly len bytes into buf; returns false if the stream enters a failed state.
inline bool ReadRaw(std::stringstream &ss, void *buf, std::streamsize len)
{
    ss.read(reinterpret_cast<char *>(buf), len);
    return static_cast<bool>(ss);
}

int32_t ReadItemToJson(std::stringstream &ss, uint32_t ssLength, bool allowComments,
    int32_t parseFailCode, nlohmann::json &out);

template <typename T>
int32_t ReadOneItem(std::stringstream &ss, uint32_t ssLength, T &out,
    bool allowComments, int32_t parseFailCode)
{
    nlohmann::json j;
    int32_t ret = ReadItemToJson(ss, ssLength, allowComments, parseFailCode, j);
    if (ret != ERR_OK) {
        return ret;
    }
    if (!T::ParseFromJson(j, out)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse item from JSON");
        return parseFailCode;
    }
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_RAW_DATA_UTILS_H
