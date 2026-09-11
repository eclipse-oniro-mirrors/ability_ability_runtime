/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "uripermissionmanagerstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "message_parcel.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

#include "parcelable_constructors.h"
#include "securec.h"
#include "uri.h"
#include "uri_permission_manager_stub.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
namespace {
constexpr uint32_t HANDLE_COUNT = 18;
}

class UriPermissionManagerStubFuzz : public UriPermissionManagerStub {
public:
    UriPermissionManagerStubFuzz() = default;
    ~UriPermissionManagerStubFuzz() = default;

    ErrCode GrantUriPermission(const Uri& uri, uint32_t flag, const std::string& targetBundleName,
        int32_t appIndex, uint32_t initiatorTokenId, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode GrantUriPermission(const std::vector<std::string>& uriVec, uint32_t flag,
        const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId,
        int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode GrantUriPermission(const UriPermissionRawData& rawData, uint32_t flag,
        const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId,
        int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode GrantUriPermissionWithType(const std::vector<Uri>& uriVec, uint32_t flag,
        const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId,
        int32_t hideSensitiveType, const std::vector<int32_t>& permissionTypes, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode GrantUriPermissionPrivileged(const std::vector<std::string>& uriVec, uint32_t flag,
        const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId,
        int32_t hideSensitiveType, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode GrantUriPermissionPrivileged(const UriPermissionRawData& rawData, uint32_t flag,
        const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId,
        int32_t hideSensitiveType, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode GrantUriPermission(const std::vector<std::string>& uriVec, uint32_t flag,
        uint32_t targetTokenId, uint32_t oriCallerTokenId, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode GrantUriPermission(const UriPermissionRawData& rawData, uint32_t flag,
        uint32_t targetTokenId, uint32_t oriCallerTokenId, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode GrantUriPermissionByKey(const std::string& key, uint32_t flag,
        uint32_t targetTokenId, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode GrantUriPermissionByKeyAsCaller(const std::string& key, uint32_t flag,
        uint32_t callerTokenId, uint32_t targetTokenId, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode RevokeAllUriPermissions(uint32_t tokenId, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode RevokeUriPermissionManually(const Uri& uri, const std::string& bundleName,
        int32_t appIndex, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
    ErrCode VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId, bool& funcResult) override
    {
        funcResult = false;
        return 0;
    }
    ErrCode CheckUriAuthorization(const std::vector<std::string>& uriVec, uint32_t flag,
        uint32_t tokenId, std::vector<bool>& funcResult) override
    {
        return 0;
    }
    ErrCode CheckUriAuthorizationWithType(const std::vector<std::string>& uriVec, uint32_t flag,
        uint32_t tokenId, std::vector<CheckResult>& funcResult) override
    {
        return 0;
    }
    ErrCode CheckUriAuthorization(const UriPermissionRawData& rawData, uint32_t flag,
        uint32_t tokenId, UriPermissionRawData& funcResult) override
    {
        return 0;
    }
    ErrCode ClearPermissionTokenByMap(uint32_t tokenId, int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    ErrCode Active(const UriPermissionRawData& policyRawData, std::vector<uint32_t>& res,
        int32_t& funcResult) override
    {
        funcResult = 0;
        return 0;
    }
#endif
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % HANDLE_COUNT) {
        case 0:  // code 1: GrantUriPermission(Uri, flag, targetBundleName, appIndex, initiatorTokenId)
            actualCode = 1;
            FuzzUtil::WriteMaliciousUri(parcel, fdp);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 1:  // code 2: GrantUriPermission(String[], flag, targetBundleName, appIndex, initiatorTokenId)
            actualCode = 2;
            parcel.WriteStringVector(OHOS::FuzzUtil::BuildMaliciousStringVector(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 2:  // code 3: GrantUriPermission(RawData, flag, targetBundleName, appIndex, initiatorTokenId)
            actualCode = 3;
            FuzzUtil::WriteMaliciousRawData(parcel, fdp);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 3:  // code 4: GrantUriPermissionWithType(Uri[], flag, targetBundleName, appIndex,
                 //          initiatorTokenId, hideSensitiveType, permissionTypes)
            actualCode = 4;
            FuzzUtil::WriteMaliciousUriVector(parcel, fdp);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32Vector(OHOS::FuzzUtil::BuildMaliciousInt32Vector(fdp));
            break;
        case 4:  // code 5: GrantUriPermissionPrivileged(String[], flag, targetBundleName, appIndex,
                 //          initiatorTokenId, hideSensitiveType)
            actualCode = 5;
            parcel.WriteStringVector(OHOS::FuzzUtil::BuildMaliciousStringVector(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 5:  // code 6: GrantUriPermissionPrivileged(RawData, flag, targetBundleName, appIndex,
                 //          initiatorTokenId, hideSensitiveType)
            actualCode = 6;
            FuzzUtil::WriteMaliciousRawData(parcel, fdp);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 6:  // code 7: GrantUriPermission(String[], flag, targetTokenId, oriCallerTokenId)
            actualCode = 7;
            parcel.WriteStringVector(OHOS::FuzzUtil::BuildMaliciousStringVector(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 7:  // code 8: GrantUriPermission(RawData, flag, targetTokenId, oriCallerTokenId)
            actualCode = 8;
            FuzzUtil::WriteMaliciousRawData(parcel, fdp);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 8:  // code 9: GrantUriPermissionByKey(key, flag, targetTokenId)
            actualCode = 9;
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 9:  // code 10: GrantUriPermissionByKeyAsCaller(key, flag, callerTokenId, targetTokenId)
            actualCode = 10;
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 10:  // code 11: RevokeAllUriPermissions(tokenId)
            actualCode = 11;
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 11:  // code 12: RevokeUriPermissionManually(Uri, bundleName, appIndex)
            actualCode = 12;
            FuzzUtil::WriteMaliciousUri(parcel, fdp);
            parcel.WriteString(OHOS::FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 12:  // code 13: VerifyUriPermission(Uri, flag, tokenId)
            actualCode = 13;
            FuzzUtil::WriteMaliciousUri(parcel, fdp);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 13:  // code 14: CheckUriAuthorization(String[], flag, tokenId)
            actualCode = 14;
            parcel.WriteStringVector(OHOS::FuzzUtil::BuildMaliciousStringVector(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 14:  // code 15: CheckUriAuthorizationWithType(String[], flag, tokenId)
            actualCode = 15;
            parcel.WriteStringVector(OHOS::FuzzUtil::BuildMaliciousStringVector(fdp));
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 15:  // code 16: CheckUriAuthorization(RawData, flag, tokenId)
            actualCode = 16;
            FuzzUtil::WriteMaliciousRawData(parcel, fdp);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 16:  // code 17: ClearPermissionTokenByMap(tokenId)
            actualCode = 17;
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        case 17:  // code 18: Active(policyRawData)
            actualCode = 18;
            FuzzUtil::WriteMaliciousRawData(parcel, fdp);
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(UriPermissionManagerStubFuzz, FuzzUtil::Tokens::URI_PERMISSION_MGR)
} // namespace OHOS
