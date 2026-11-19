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
#include <iremote_stub.h>
#define private public
#define protected public
#include "ability_manager_errors.h"
#include "dms_intent_caller_info.h"
#include "distributed_client.h"
#include "distributed_parcel_helper.h"
#include "iservice_registry.h"
#include "iremote_object.h"
#include "mock_ability_connect_callback.h"
#include "mock_remote_intent_result_callback.h"
#include "parcel.h"
#undef protected
#undef private

using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace testing;
using namespace testing::ext;

class StartRemoteIntentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StartRemoteIntentTest::SetUpTestCase()
{}

void StartRemoteIntentTest::TearDownTestCase()
{}

void StartRemoteIntentTest::SetUp()
{}

void StartRemoteIntentTest::TearDown()
{}

/**
 * @tc.number: StartRemoteIntent_0100
 * @tc.name: StartRemoteIntent
 * @tc.desc: StartRemoteIntent Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(StartRemoteIntentTest, StartRemoteIntent_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    OHOS::AAFwk::IntentCallerInfo callerInfo;
    callerInfo.callerUid = 100;
    callerInfo.requestCode = 1;
    callerInfo.accessToken = 200;
    callerInfo.specifyTokenId = 300;
    sptr<IRemoteObject> callback = new (std::nothrow) AAFwk::RemoteIntentResultCallback();
    int32_t result = client->StartRemoteIntent(want, callerInfo, callback);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0100 end";
}

/**
 * @tc.number: StartRemoteIntent_0200
 * @tc.name: StartRemoteIntent
 * @tc.desc: StartRemoteIntent Test, return INVALID_PARAMETERS_ERR when samgr is nullptr.
 */
HWTEST_F(StartRemoteIntentTest, StartRemoteIntent_0200, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0200 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    OHOS::AAFwk::IntentCallerInfo callerInfo;
    sptr<IRemoteObject> callback = new (std::nothrow) AAFwk::RemoteIntentResultCallback();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = nullptr;
    int32_t result = client->StartRemoteIntent(want, callerInfo, callback);
    EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0200 end";
}

/**
 * @tc.number: StartRemoteIntent_0300
 * @tc.name: StartRemoteIntent
 * @tc.desc: StartRemoteIntent Test with specifyTokenId, return INVALID_PARAMETERS_ERR when samgr is nullptr.
 */
HWTEST_F(StartRemoteIntentTest, StartRemoteIntent_0300, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0300 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    OHOS::AAFwk::IntentCallerInfo callerInfo;
    callerInfo.callerUid = 100;
    callerInfo.requestCode = 1;
    callerInfo.accessToken = 200;
    callerInfo.specifyTokenId = 300;
    sptr<IRemoteObject> callback = new (std::nothrow) AAFwk::RemoteIntentResultCallback();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = nullptr;
    int32_t result = client->StartRemoteIntent(want, callerInfo, callback);
    EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0300 end";
}

/**
 * @tc.number: StartRemoteIntent_0400
 * @tc.name: StartRemoteIntent
 * @tc.desc: StartRemoteIntent Test with null callback, return DMS_PERMISSION_DENIED or INVALID_PARAMETERS_ERR.
 */
HWTEST_F(StartRemoteIntentTest, StartRemoteIntent_0400, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0400 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    OHOS::AAFwk::IntentCallerInfo callerInfo;
    callerInfo.callerUid = 0;
    int32_t result = client->StartRemoteIntent(want, callerInfo, nullptr);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0400 end";
}

/**
 * @tc.number: StartRemoteIntent_0500
 * @tc.name: StartRemoteIntent
 * @tc.desc: StartRemoteIntent Test with null callback and samgr nullptr, return INVALID_PARAMETERS_ERR.
 */
HWTEST_F(StartRemoteIntentTest, StartRemoteIntent_0500, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0500 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    OHOS::AAFwk::IntentCallerInfo callerInfo;
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = nullptr;
    int32_t result = client->StartRemoteIntent(want, callerInfo, nullptr);
    EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0500 end";
}

/**
 * @tc.number: StartRemoteIntent_0600
 * @tc.name: StartRemoteIntent
 * @tc.desc: StartRemoteIntent Test with Want containing ElementName, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(StartRemoteIntentTest, StartRemoteIntent_0600, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0600 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    AppExecFwk::ElementName element("deviceId", "com.test.bundle", "MainAbility");
    want.SetElement(element);
    OHOS::AAFwk::IntentCallerInfo callerInfo;
    callerInfo.callerUid = 100;
    callerInfo.accessToken = 1000;
    sptr<IRemoteObject> callback = new (std::nothrow) AAFwk::RemoteIntentResultCallback();
    int32_t result = client->StartRemoteIntent(want, callerInfo, callback);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0600 end";
}

/**
 * @tc.number: StartRemoteIntent_0700
 * @tc.name: StartRemoteIntent
 * @tc.desc: StartRemoteIntent Test with zero IntentCallerInfo, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(StartRemoteIntentTest, StartRemoteIntent_0700, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0700 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    OHOS::AAFwk::IntentCallerInfo callerInfo;
    callerInfo.callerUid = 0;
    callerInfo.requestCode = 0;
    callerInfo.accessToken = 0;
    callerInfo.specifyTokenId = 0;
    sptr<IRemoteObject> callback = new (std::nothrow) AAFwk::RemoteIntentResultCallback();
    int32_t result = client->StartRemoteIntent(want, callerInfo, callback);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0700 end";
}

/**
 * @tc.number: StartRemoteIntent_0800
 * @tc.name: StartRemoteIntent
 * @tc.desc: StartRemoteIntent Test with negative callerUid, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(StartRemoteIntentTest, StartRemoteIntent_0800, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0800 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    OHOS::AAFwk::IntentCallerInfo callerInfo;
    callerInfo.callerUid = -1;
    callerInfo.requestCode = -1;
    sptr<IRemoteObject> callback = new (std::nothrow) AAFwk::RemoteIntentResultCallback();
    int32_t result = client->StartRemoteIntent(want, callerInfo, callback);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "StartRemoteIntentTest StartRemoteIntent_0800 end";
}
