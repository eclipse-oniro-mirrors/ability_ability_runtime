/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_UPMS_UDMF_UTILS_H
#define OHOS_ABILITY_RUNTIME_UPMS_UDMF_UTILS_H

#include <sys/types.h>
#include <vector>

namespace OHOS {
namespace AAFwk {

class UDMFUtils {
public:
    static int32_t ProcessUdmfKey(const std::string &key, uint32_t callerTokenId, uint32_t targetTokenId,
        std::vector<std::string> &uris);
private:
    static int32_t GetBatchData(const std::string &key, std::vector<std::string> &uris);
    static int32_t AddPrivilege(const std::string &key, uint32_t tokenId, const std::string &readPermission);
    static bool IsUdKeyCreateByCaller(uint32_t callerTokenId, const std::string &key);
};
} // OHOS
} // AAFwk
#endif // OHOS_ABILITY_RUNTIME_UPMS_UDMF_UTILS_H