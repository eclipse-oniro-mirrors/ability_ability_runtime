/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "account_error_no.h"
#include "os_account_manager_wrapper.h"
#include "mock_sa_call.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t UID = 0;
const int32_t ACCOUNT_ID = 0;
const int32_t ACCOUNT_VALUE = -1;
const int32_t RESULT_OK = 0;
const std::string ACCOUNT_NAME = "ACCOUNT";
}  // namespace
class OsAccountManagerWrapperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void OsAccountManagerWrapperTest::SetUpTestCase()
{}

void OsAccountManagerWrapperTest::TearDownTestCase()
{}

void OsAccountManagerWrapperTest::SetUp()
{}

void OsAccountManagerWrapperTest::TearDown()
{}

/**
 * @tc.name: GetOsAccountLocalIdFromUid_0100
 * @tc.desc: get os account local Id from Uid.
 * @tc.type: FUNC
 */
HWTEST_F(OsAccountManagerWrapperTest, GetOsAccountLocalIdFromUid_0100, TestSize.Level1)
{
    int account = ACCOUNT_VALUE;
    int ret = OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(UID, account);
    EXPECT_EQ(account, ACCOUNT_ID);
    EXPECT_EQ(ret, RESULT_OK);
}

/**
 * @tc.name: GetOsAccountLocalIdFromProcess_0100
 * @tc.desc: get os account local Id from process.
 * @tc.type: FUNC
 */
HWTEST_F(OsAccountManagerWrapperTest, GetOsAccountLocalIdFromProcess_0100, TestSize.Level1)
{
    int account = ACCOUNT_VALUE;
    int ret = OsAccountManagerWrapper::GetOsAccountLocalIdFromProcess(account);
    EXPECT_EQ(ret, RESULT_OK);
}

/**
 * @tc.name: IsOsAccountExists_0100
 * @tc.desc: Is os account exists.
 * @tc.type: FUNC
 */
HWTEST_F(OsAccountManagerWrapperTest, IsOsAccountExists_0100, TestSize.Level2)
{
    bool isOsAccountExists = false;
    OsAccountManagerWrapper::IsOsAccountExists(ACCOUNT_VALUE, isOsAccountExists);
    EXPECT_EQ(isOsAccountExists, false);
}

/**
 * @tc.name: CreateOsAccount_0100
 * @tc.desc: Create os account.
 * @tc.type: FUNC
 */
HWTEST_F(OsAccountManagerWrapperTest, CreateOsAccount_0100, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    int account = ACCOUNT_VALUE;
    int ret = OsAccountManagerWrapper::CreateOsAccount(ACCOUNT_NAME, account);
    EXPECT_NE(ret, ERR_ACCOUNT_COMMON_ACCOUNT_NOT_EXIST_ERROR);
}

/**
 * @tc.name: RemoveOsAccount_0100
 * @tc.desc: Remove os account.
 * @tc.type: FUNC
 */
HWTEST_F(OsAccountManagerWrapperTest, RemoveOsAccount_0100, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    int account = ACCOUNT_VALUE;
    int ret = OsAccountManagerWrapper::CreateOsAccount(ACCOUNT_NAME, account);
    EXPECT_NE(ret, ERR_OK);
    ret = OsAccountManagerWrapper::RemoveOsAccount(account);
    EXPECT_EQ(ret, ERR_ACCOUNT_COMMON_ACCOUNT_NOT_EXIST_ERROR);
}

/**
 * @tc.name: GetCurrentActiveAccountId_0100
 * @tc.desc: Get current accountId.
 * @tc.type: FUNC
 */
HWTEST_F(OsAccountManagerWrapperTest, GetCurrentActiveAccountId_0100, TestSize.Level1)
{
    int ret = OsAccountManagerWrapper::GetCurrentActiveAccountId();
    EXPECT_EQ(ret, 100);
}
}  // namespace AAFwk
}  // namespace OHOS
