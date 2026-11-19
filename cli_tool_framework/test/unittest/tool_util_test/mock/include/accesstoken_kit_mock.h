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

#ifndef OHOS_ABILITY_RUNTIME_TOOL_UTIL_TEST_ACCESSTOKEN_KIT_MOCK_H
#define OHOS_ABILITY_RUNTIME_TOOL_UTIL_TEST_ACCESSTOKEN_KIT_MOCK_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace CliToolTest {
// Test seams for the two AccessTokenKit entry points ToolUtil::GetBundleInfoByTokenId depends on
// (issue-16055 ForCli callerIdentity env-injection branch in GenerateSandboxConfig). The strong
// overrides live in mock/src/mock_accesstoken_kit.cpp, linked ahead of access_token:libaccesstoken_sdk
// (shared), so they take precedence; the SDK still provides AccessTokenID / HapTokenInfo /
// ATokenTypeEnum types and any non-overridden entry points (e.g. VerifyAccessToken used by
// permission_util.cpp). Defaults mirror the failure path (non-HAP token, GetHapTokenInfo error) so a
// test must opt into the success path explicitly via these statics.
struct AccessTokenKitMock {
    static bool getTokenTypeFlagIsHap;   // default false (non-HAP -> GetBundleInfoByTokenId fails early)
    static int getHapTokenInfoRet;       // default 1 (non-zero -> GetHapTokenInfo fails)
    static std::string hapBundleName;
    static int32_t hapInstIndex;
    static int32_t hapUserID;
    static void Reset();
};
}  // namespace CliToolTest
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_TOOL_UTIL_TEST_ACCESSTOKEN_KIT_MOCK_H
