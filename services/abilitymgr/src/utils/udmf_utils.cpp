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

#include "udmf_utils.h"

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "singleton.h"
#include "udmf_client.h"

namespace OHOS {
namespace AbilityRuntime {

int32_t UdmfUtils::AddPrivilege(const std::string &key, uint32_t tokenId, const std::string &readPermission)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AddPrivilege for tokenId:%{public}u", tokenId);
    UDMF::QueryOption query = { .key = key };
    UDMF::Privilege privilege = { .tokenId = tokenId, .readPermission = readPermission };
    auto ret = IN_PROCESS_CALL(UDMF::UdmfClient::GetInstance().AddPrivilege(query, privilege));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AddPrivilege failed:%{public}d", ret);
    }
    return ret;
}

bool UdmfUtils::GetDirByBundleNameAndAppIndex(const std::string &bundleName, int32_t appIndex, std::string &dirName)
{
    dirName = bundleName;
    auto bmsClient = DelayedSingleton<AppExecFwk::BundleMgrClient>::GetInstance();
    if (bmsClient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleMgrClient is nullptr.");
        return false;
    }
    auto bmsRet = IN_PROCESS_CALL(bmsClient->GetDirByBundleNameAndAppIndex(bundleName, appIndex, dirName));
    if (bmsRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetDirByBundleNameAndAppIndex failed, ret:%{public}d", bmsRet);
        return false;
    }
    return true;
}

bool UdmfUtils::GetAlterableBundleNameByTokenId(uint32_t tokenId, std::string &bundleName)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        Security::AccessToken::HapTokenInfo hapInfo;
        auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
        if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetHapTokenInfo failed, ret:%{public}d", ret);
            return false;
        }
        return GetDirByBundleNameAndAppIndex(hapInfo.bundleName, hapInfo.instIndex, bundleName);
    }
    return false;
}

bool UdmfUtils::IsUdKeyCreateByCaller(uint32_t callerTokenId, const std::string &key)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto keyAuthority = IN_PROCESS_CALL(UDMF::UdmfClient::GetInstance().GetBundleNameByUdKey(key));
    std::string callerAuthority;
    GetAlterableBundleNameByTokenId(callerTokenId, callerAuthority);
    if (callerAuthority != keyAuthority) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Authority: %{public}s-%{public}s", keyAuthority.c_str(),
            callerAuthority.c_str());
        return false;
    }
    return true;
}

int32_t UdmfUtils::ProcessUdmfKey(const std::string &key, uint32_t callerTokenId, uint32_t targetTokenId)
{
    if (!IsUdKeyCreateByCaller(callerTokenId, key)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Key is not create by caller");
        return AAFwk::ERR_UPMS_KEY_IS_NOT_CREATE_BY_CALLER;
    }
    auto ret = AddPrivilege(key, targetTokenId, "");
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ProcessUdmfKey failed:%{public}d", ret);
        return AAFwk::ERR_UPMS_ADD_PRIVILEGED_FAILED;
    }
    return ERR_OK;
}
} // AbilityRuntime
} // OHOS
