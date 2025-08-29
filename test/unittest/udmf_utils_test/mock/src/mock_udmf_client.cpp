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

#include "udmf_client.h"

namespace OHOS {
namespace UDMF {

UdmfClient &UdmfClient::GetInstance()
{
    static UdmfClient utils;
    return utils;
}

void UdmfClient::Init()
{
    getBatchDataRet_ = 0;
    addPrivilegeRet_ = 0;
    UdmfClient::unifiedData_.clear();
}

int32_t UdmfClient::GetBatchData(const QueryOption &query, std::vector<UnifiedData> &unifiedDataset)
{
    unifiedDataset = unifiedData_;
    return getBatchDataRet_;
}

int32_t UdmfClient::AddPrivilege(const QueryOption &query, const Privilege &privilege)
{
    if (privilege.tokenId == privilegeTokenId_) {
        return 0;
    }
    return addPrivilegeRet_;
}

std::string UdmfClient::GetBundleNameByUdKey(const std::string &key)
{
    return keyAuthority;
}

int32_t UdmfClient::getBatchDataRet_ = 0;
int32_t UdmfClient::addPrivilegeRet_ = 0;
int32_t UdmfClient::privilegeTokenId_ = 10000;
std::vector<UnifiedData> UdmfClient::unifiedData_ = {};
std::string UdmfClient::keyAuthority = "";
} // UDMF
} // OHOS