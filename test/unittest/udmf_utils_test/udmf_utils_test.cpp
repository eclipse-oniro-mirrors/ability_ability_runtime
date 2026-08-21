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

#include <gtest/gtest.h>
#define private public
#include "udmf_utils.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "udmf_client.h"
#include "ability_manager_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class UdmfUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UdmfUtilsTest::SetUpTestCase() {}

void UdmfUtilsTest::TearDownTestCase() {}

void UdmfUtilsTest::SetUp()
{
    UDMF::UdmfClient::Init();
}

void UdmfUtilsTest::TearDown() {}

/*
 * Feature: UdmfUtilsTest
 * Function: AddPrivilege_001
 * SubFunction: NA
 */
HWTEST_F(UdmfUtilsTest, AddPrivilege_001, TestSize.Level1)
{
    std::string key = "udmfKey";
    uint32_t targetTokenId = 100001;
    std::string readPermission = "";
    auto ret = UdmfUtils::AddPrivilege(key, targetTokenId, readPermission);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UdmfUtilsTest
 * Function: AddPrivilege_002
 * SubFunction: NA
 */
HWTEST_F(UdmfUtilsTest, AddPrivilege_002, TestSize.Level1)
{
    std::string key = "udmfKey";
    uint32_t targetTokenId = 100001;
    std::string readPermission = "";
    UDMF::UdmfClient::addPrivilegeRet_ = AAFwk::INNER_ERR;
    auto ret = UdmfUtils::AddPrivilege(key, targetTokenId, readPermission);
    EXPECT_EQ(ret, AAFwk::INNER_ERR);
}

/*
 * Feature: UdmfUtilsTest
 * Function: ProcessUdmfKey_001
 * SubFunction: NA
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_001, TestSize.Level1)
{
    std::string key = "udmfKey";
    uint32_t callerTokenId = 100001;
    uint32_t targetTokenId = 100001;
    UDMF::UdmfClient::keyAuthority = "com.example.owner";
    auto ret = UdmfUtils::ProcessUdmfKey(key, callerTokenId, targetTokenId);
    EXPECT_EQ(ret, AAFwk::ERR_UPMS_KEY_IS_NOT_CREATE_BY_CALLER);
}

/*
 * Feature: UdmfUtilsTest
 * Function: ProcessUdmfKey_002
 * SubFunction: NA
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_002, TestSize.Level1)
{
    std::string key = "udmfKey";
    uint32_t callerTokenId = 100001;
    uint32_t targetTokenId = 100001;
    UDMF::UdmfClient::keyAuthority = "";
    auto ret = UdmfUtils::ProcessUdmfKey(key, callerTokenId, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UdmfUtilsTest
 * Function: ProcessUdmfKey_003
 * SubFunction: NA
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_003, TestSize.Level1)
{
    std::string key = "udmfKey";
    uint32_t callerTokenId = 100001;
    uint32_t targetTokenId = 100001;
    UDMF::UdmfClient::keyAuthority = "";
    UDMF::UdmfClient::addPrivilegeRet_ = AAFwk::INNER_ERR;
    auto ret = UdmfUtils::ProcessUdmfKey(key, callerTokenId, targetTokenId);
    EXPECT_EQ(ret, AAFwk::ERR_UPMS_ADD_PRIVILEGED_FAILED);
}
} // namespace AAFwk
} // namespace OHOS
