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

#include "raw_data_utils.h"

#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

// Read a length-prefixed JSON item from the stream and parse it into out.
int32_t ReadItemToJson(std::stringstream &ss, uint32_t ssLength, bool allowComments,
    int32_t parseFailCode, nlohmann::json &out)
{
    uint32_t itemSize = 0;
    if (!ReadRaw(ss, &itemSize, sizeof(itemSize))) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read item size, stream state invalid");
        return ERR_INVALID_VALUE;
    }
    std::streamoff curPos = ss.tellg();
    std::streamoff total = static_cast<std::streamoff>(ssLength);
    if (curPos < 0 || curPos > total || itemSize > total - curPos) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "itemSize:%{public}u is invalid", itemSize);
        return ERR_INVALID_VALUE;
    }
    std::string itemStr(itemSize, '\0');
    if (!ReadRaw(ss, itemStr.data(), itemSize)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read item data, stream state invalid");
        return ERR_INVALID_VALUE;
    }
    out = nlohmann::json::parse(itemStr, nullptr, false, allowComments);
    if (out.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse JSON");
        return parseFailCode;
    }
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
