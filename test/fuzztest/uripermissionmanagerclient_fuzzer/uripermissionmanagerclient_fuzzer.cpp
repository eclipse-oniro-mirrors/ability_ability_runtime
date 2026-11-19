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

#include "uripermissionmanagerclient_fuzzer.h"
#include "fuzz_util.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "uri.h"
#include "uri_permission_manager_client.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace {
// Build a valid file URI string via FuzzedDataProvider, e.g. "file:///path/to/resource",
// to avoid empty or invalid URIs triggering early returns in lower layers.
std::string BuildFuzzUriString(FuzzedDataProvider &fdp)
{
    const std::string scheme = "file";
    std::string host = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    std::string path = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    if (host.empty()) {
        host = "localhost";
    }
    if (path.empty()) {
        path = "/fuzz_resource";
    }
    return scheme + "://" + host + path;
}

// Build a URI vector via FuzzedDataProvider, length controlled to avoid memory explosion.
std::vector<Uri> BuildFuzzUriVector(FuzzedDataProvider &fdp)
{
    std::vector<Uri> uriVec;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % OHOS::FuzzUtil::VEC_MAX_SIZE;
    for (uint8_t i = 0; i < count; i++) {
        std::string uriStr = BuildFuzzUriString(fdp);
        uriVec.emplace_back(Uri(uriStr));
    }
    return uriVec;
}

// Build a permission type vector via FuzzedDataProvider, length controlled.
std::vector<int32_t> BuildFuzzPermissionTypes(FuzzedDataProvider &fdp)
{
    std::vector<int32_t> permissionTypes;
    uint8_t count = fdp.ConsumeIntegral<uint8_t>() % OHOS::FuzzUtil::VEC_MAX_SIZE;
    for (uint8_t i = 0; i < count; i++) {
        permissionTypes.push_back(fdp.ConsumeIntegral<int32_t>());
    }
    return permissionTypes;
}
} // namespace

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    // Consume all fuzz input parameters first to avoid inter-method data dependency.
    std::string targetBundleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    if (targetBundleName.empty()) {
        targetBundleName = "com.example.fuzztarget";
    }
    std::string bundleName = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    if (bundleName.empty()) {
        bundleName = "com.example.fuzzbundle";
    }
    std::string key = fdp.ConsumeRandomLengthString(OHOS::FuzzUtil::STRING_MAX_LENGTH);
    if (key.empty()) {
        key = "fuzz_key";
    }
    int32_t appIndex = fdp.ConsumeIntegral<int32_t>();
    uint32_t flag = fdp.ConsumeIntegral<uint32_t>();
    uint32_t initiatorTokenId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t targetTokenId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t oriCallerTokenId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t callerTokenId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t tokenId = fdp.ConsumeIntegral<uint32_t>();
    int32_t hideSensitiveType = fdp.ConsumeIntegral<int32_t>();
    std::string uriStr = BuildFuzzUriString(fdp);
    Uri uri(uriStr);
    std::vector<Uri> uriVec = BuildFuzzUriVector(fdp);
    std::vector<std::string> uriStrVec = OHOS::FuzzUtil::BuildStringVector(fdp);
    std::vector<int32_t> permissionTypes = BuildFuzzPermissionTypes(fdp);

    auto &client = UriPermissionManagerClient::GetInstance();

    // 1. GrantUriPermission(const Uri&, uint32_t, const std::string, int32_t, uint32_t)
    client.GrantUriPermission(uri, flag, targetBundleName, appIndex, initiatorTokenId);

    // 2. GrantUriPermission(const std::vector<Uri>&, uint32_t, const std::string, int32_t, uint32_t)
    client.GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId);

    // 3. GrantUriPermissionPrivileged(const std::vector<Uri>&, uint32_t, const std::string&,
    //                                 int32_t, uint32_t, int32_t)
    client.GrantUriPermissionPrivileged(uriVec, flag, targetBundleName, appIndex,
        initiatorTokenId, hideSensitiveType);

    // 4. GrantUriPermissionWithType(const std::vector<Uri>&, uint32_t, const std::string&,
    //                               int32_t, uint32_t, int32_t, const std::vector<int32_t>&)
    client.GrantUriPermissionWithType(uriVec, flag, targetBundleName, appIndex,
        initiatorTokenId, hideSensitiveType, permissionTypes);

    // 5. GrantUriPermission(const std::vector<std::string>&, uint32_t, uint32_t, uint32_t)
    client.GrantUriPermission(uriStrVec, flag, targetTokenId, oriCallerTokenId);

    // 6. RevokeAllUriPermissions(const uint32_t)
    client.RevokeAllUriPermissions(tokenId);

    // 7. RevokeUriPermissionManually(const Uri&, const std::string, int32_t)
    client.RevokeUriPermissionManually(uri, bundleName, appIndex);

    // 8. VerifyUriPermission(const Uri&, uint32_t, uint32_t)
    client.VerifyUriPermission(uri, flag, tokenId);

    // 9. CheckUriAuthorization(const std::vector<std::string>&, uint32_t, uint32_t)
    client.CheckUriAuthorization(uriStrVec, flag, tokenId);

    // 10. CheckUriAuthorizationWithType(const std::vector<std::string>&, uint32_t, uint32_t)
    client.CheckUriAuthorizationWithType(uriStrVec, flag, tokenId);

    // 11. ClearPermissionTokenByMap(uint32_t)
    client.ClearPermissionTokenByMap(tokenId);

    // 12. GrantUriPermissionByKey(const std::string&, uint32_t, uint32_t)
    client.GrantUriPermissionByKey(key, flag, targetTokenId);

    // 13. GrantUriPermissionByKeyAsCaller(const std::string&, uint32_t, uint32_t, uint32_t)
    client.GrantUriPermissionByKeyAsCaller(key, flag, callerTokenId, targetTokenId);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
