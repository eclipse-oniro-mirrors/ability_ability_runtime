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
#ifndef SANDBOXMANAGER_KIT_H
#define SANDBOXMANAGER_KIT_H

#include <sys/types.h>
#include <vector>

#include "policy_info.h"

namespace OHOS {
namespace AccessControl {
namespace SandboxManager {
class SandboxManagerKit {
public:
    static int32_t SetPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys, uint64_t policyFlag,
        std::vector<uint32_t> &results, const SetInfo &setInfo);
    static int32_t CheckPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys, std::vector<bool> &results);
    static int32_t CheckPersistPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
        std::vector<bool> &results);
    static int32_t PersistPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
        std::vector<uint32_t> &results);
    static int32_t UnSetPolicy(uint32_t tokenid, const PolicyInfo &policy);
    static int32_t UnSetAllPolicyByTokenAsync(uint32_t tokenId, uint64_t timestamp);
    static int32_t StartAccessingPolicy(const std::vector<PolicyInfo> &policy, std::vector<uint32_t> &result,
        bool useCallerToken, uint32_t tokenId, uint64_t timestamp);
    static void Init();
public:
    static int32_t setPolicyRet_;
    static int32_t checkPolicyRet_;
    static int32_t checkPersistPolicyRet_;
    static int32_t persistPolicyRet_;
    static int32_t unSetPolicyRet_;
    static int32_t startAccessingPolicyRet_;
    static int32_t unSetAllPolicyByTokenAsyncRet_;
    static std::vector<uint32_t> setPolicyResult_;
    static std::vector<bool> checkPolicyResult_;
    static std::vector<bool> checkPersistPolicyResult_;
};
} // SandboxManager
} // AccessControl
} // OHOS
#endif // SANDBOXMANAGER_KIT_H