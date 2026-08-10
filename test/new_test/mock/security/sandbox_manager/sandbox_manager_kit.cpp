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

#include "sandbox_manager_kit.h"

namespace OHOS {
namespace AccessControl {
namespace SandboxManager {
int32_t SandboxManagerKit::SetPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys, uint64_t policyFlag,
    std::vector<uint32_t> &results, const SetInfo &setInfo)
{
    results = setPolicyResult_;
    return setPolicyRet_;
}

int32_t SandboxManagerKit::CheckPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
    std::vector<bool> &results)
{
    results = checkPolicyResult_;
    return checkPolicyRet_;
}

int32_t SandboxManagerKit::CheckPersistPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
    std::vector<bool> &results)
{
    results = checkPersistPolicyResult_;
    return checkPersistPolicyRet_;
}

int32_t SandboxManagerKit::PersistPolicy(uint32_t tokenid, const std::vector<PolicyInfo> &policys,
    std::vector<uint32_t> &results)
{
    return persistPolicyRet_;
}

int32_t SandboxManagerKit::UnSetPolicy(uint32_t tokenid, const PolicyInfo &policy)
{
    return unSetPolicyRet_;
}

int32_t SandboxManagerKit::StartAccessingPolicy(const std::vector<PolicyInfo> &policy,
    std::vector<uint32_t> &result, bool useCallerToken, uint32_t tokenId, uint64_t timestamp)
{
    return startAccessingPolicyRet_;
}

int32_t SandboxManagerKit::UnSetAllPolicyByTokenAsync(uint32_t tokenId, uint64_t timestamp)
{
    return unSetAllPolicyByTokenAsyncRet_;
}

void SandboxManagerKit::Init()
{
    setPolicyRet_ = 0;
    checkPolicyRet_ = 0;
    checkPersistPolicyRet_ = 0;
    persistPolicyRet_ = 0;
    unSetPolicyRet_ = 0;
    startAccessingPolicyRet_ = 0;
    unSetAllPolicyByTokenAsyncRet_ = 0;
    setPolicyResult_.clear();
    checkPolicyResult_.clear();
    checkPersistPolicyResult_.clear();
}

int32_t SandboxManagerKit::setPolicyRet_ = 0;
int32_t SandboxManagerKit::checkPolicyRet_ = 0;
int32_t SandboxManagerKit::checkPersistPolicyRet_ = 0;
int32_t SandboxManagerKit::persistPolicyRet_ = 0;
int32_t SandboxManagerKit::unSetPolicyRet_ = 0;
int32_t SandboxManagerKit::startAccessingPolicyRet_ = 0;
int32_t SandboxManagerKit::unSetAllPolicyByTokenAsyncRet_ = 0;
std::vector<uint32_t> SandboxManagerKit::setPolicyResult_ = {};
std::vector<bool> SandboxManagerKit::checkPolicyResult_ = {};
std::vector<bool> SandboxManagerKit::checkPersistPolicyResult_ = {};
} // SandboxManager
} // AccessControl
} // OHOS