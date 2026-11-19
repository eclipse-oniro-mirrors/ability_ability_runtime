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

#include "accesstoken_kit.h"       // real header (from access_token:libaccesstoken_sdk include)
#include "accesstoken_kit_mock.h"

namespace OHOS {
namespace CliToolTest {
bool AccessTokenKitMock::getTokenTypeFlagIsHap = false;
int AccessTokenKitMock::getHapTokenInfoRet = 1;  // non-zero default = failure
std::string AccessTokenKitMock::hapBundleName;
int32_t AccessTokenKitMock::hapInstIndex = 0;
int32_t AccessTokenKitMock::hapUserID = 0;

void AccessTokenKitMock::Reset()
{
    getTokenTypeFlagIsHap = false;
    getHapTokenInfoRet = 1;
    hapBundleName.clear();
    hapInstIndex = 0;
    hapUserID = 0;
}
}  // namespace CliToolTest
}  // namespace OHOS

namespace OHOS {
namespace Security {
namespace AccessToken {
// Strong overrides for the two AccessTokenKit entry points ToolUtil::GetBundleInfoByTokenId calls.
// Linked ahead of access_token:libaccesstoken_sdk (shared) so these definitions take precedence for
// the test executable's own references; the SDK still supplies every other AccessTokenKit entry point
// (e.g. VerifyAccessToken used by permission_util.cpp).
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return OHOS::CliToolTest::AccessTokenKitMock::getTokenTypeFlagIsHap
        ? static_cast<ATokenTypeEnum>(0)   // TOKEN_HAP == 0
        : static_cast<ATokenTypeEnum>(1);   // any non-zero -> "caller is not hap" early-return
}

int AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapInfo)
{
    if (OHOS::CliToolTest::AccessTokenKitMock::getHapTokenInfoRet != 0) {
        return OHOS::CliToolTest::AccessTokenKitMock::getHapTokenInfoRet;
    }
    hapInfo.bundleName = OHOS::CliToolTest::AccessTokenKitMock::hapBundleName;
    hapInfo.instIndex = OHOS::CliToolTest::AccessTokenKitMock::hapInstIndex;
    hapInfo.userID = OHOS::CliToolTest::AccessTokenKitMock::hapUserID;
    return 0;  // AccessTokenKitRet::RET_SUCCESS == 0
}
}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS
