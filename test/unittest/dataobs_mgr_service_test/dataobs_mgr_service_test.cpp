/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <cstdint>
#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "access_token.h"
#include "accesstoken_kit.h"
#include "dataobs_mgr_interface.h"
#include "datashare_errno.h"
#include "mock_data_ability_observer_stub.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#define private public
#include "dataobs_mgr_service.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
static int USER_100 = 100;
class DataObsMgrServiceTest : public testing::Test {
public:
    DataObsMgrServiceTest() = default;
    virtual ~DataObsMgrServiceTest() = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DataObsMgrServiceTest::SetUpTestCase(void)
{}
void DataObsMgrServiceTest::TearDownTestCase(void)
{}
void DataObsMgrServiceTest::SetUp()
{}
void DataObsMgrServiceTest::TearDown()
{}

/*
 * Feature: DataObsMgrService
 * Function: QueryServiceState
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService QueryServiceState
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService could query service state.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_QueryServiceState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_QueryServiceState_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_NOT_START;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testValue, dataObsMgrServer->QueryServiceState());

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_QueryServiceState_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));

    testing::Mock::AllowLeak(dataobsAbility);
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0200 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, nullptr));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInner_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));
    dataObsMgrServer->dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();

    testing::Mock::AllowLeak(dataobsAbility);
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0300 start";
    const int testVal = static_cast<int>(DATAOBS_INVALID_USERID);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    auto originalToken = GetSelfTokenID();

    SetSelfTokenID(0);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));
    dataObsMgrServer->dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
    testing::Mock::AllowLeak(dataobsAbility);
    int count = 0;
    ON_CALL(*dataobsAbility, OnChange()).WillByDefault(testing::Invoke([&count]() {
        count++;
    }));
    dataObsMgrServer->NotifyChange(*uri);
    EXPECT_EQ(count, 0);

    SetSelfTokenID(originalToken);
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserver(*uri, dataobsAbility));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_UnregisterObserver_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0200 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserver(*uri, nullptr));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_UnregisterObserver_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInner_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserver(*uri, dataobsAbility));
    dataObsMgrServer->dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0100 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0200 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->OnStop();
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInner_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));
    dataObsMgrServer->dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0400 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->taskCount_ = 50;
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));
    dataObsMgrServer->taskCount_ = 0;

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0400 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_RegisterObserverExt_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, false));
    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, true));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_RegisterObserverExt_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0200 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, nullptr, true));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_RegisterObserverExt_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInnerExt_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, true));
    dataObsMgrServer->dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is normal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, true));
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(uri, dataobsAbility));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is normal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0200 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, true));
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(dataobsAbility));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0300 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(uri, nullptr));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0400 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInnerExt_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(uri, dataobsAbility));
    dataObsMgrServer->dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0400 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0500 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(nullptr));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0500 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0600 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInnerExt_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(dataobsAbility));
    dataObsMgrServer->dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0600 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is normal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_NotifyChangeExt_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0100 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_NotifyChangeExt_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0200 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->OnStop();
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_NotifyChangeExt_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInner_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    dataObsMgrServer->dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: OnStop
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService OnStop
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService OnStop is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_OnStop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStop_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_NOT_START;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->OnStop();
    EXPECT_EQ(testValue, dataObsMgrServer->QueryServiceState());

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStop_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: Dump
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService Dump
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService Dump is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_Dump_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_Dump_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_RUNNING;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    std::string fileName = "test.txt";
    std::vector<std::u16string> args;
    args.push_back(u"-h");
    FILE *fp = fopen(fileName.c_str(), "w");
    int ret = dataObsMgrServer->Dump(fileno(fp), args);
    fclose(fp);
    EXPECT_EQ(SUCCESS, ret);

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_Dump_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: CheckSystemCallingPermission
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService CheckSystemCallingPermission
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService CheckSystemCallingPermission is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_CheckSystemCallingPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_CheckSystemCallingPermission_0100 start");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    auto originalToken = GetSelfTokenID();

    // set system app
    uint64_t systemAppMask = (static_cast<uint64_t>(1) << 32);
    uint32_t tokenID = Security::AccessToken::DEFAULT_TOKEN_VERSION;
    Security::AccessToken::AccessTokenIDInner *idInner =
        reinterpret_cast<Security::AccessToken::AccessTokenIDInner *>(&tokenID);
    idInner->type = Security::AccessToken::TOKEN_HAP;
    uint64_t fullTokenId = systemAppMask | tokenID;
    SetSelfTokenID(fullTokenId);
    DataObsOption opt;
    bool ret = 0;
    opt.isSystem = true;
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, IDataObsMgr::DATAOBS_DEFAULT_CURRENT_USER, -1);
    EXPECT_EQ(ret, false);
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, IDataObsMgr::DATAOBS_DEFAULT_CURRENT_USER, 100);
    EXPECT_EQ(ret, true);
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, 100, 101);
    EXPECT_EQ(ret, true);
    ret = dataObsMgrServer->IsCallingPermissionValid(opt);
    EXPECT_EQ(ret, true);

    opt.isSystem = false;
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, IDataObsMgr::DATAOBS_DEFAULT_CURRENT_USER, -1);
    EXPECT_EQ(ret, false);
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, IDataObsMgr::DATAOBS_DEFAULT_CURRENT_USER, 100);
    EXPECT_EQ(ret, true);
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, 100, 101);
    EXPECT_EQ(ret, true);
    ret = dataObsMgrServer->IsCallingPermissionValid(opt);
    EXPECT_EQ(ret, true);

    SetSelfTokenID(originalToken);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_CheckSystemCallingPermission_0100 end");
}

/*
 * Feature: DataObsMgrService
 * Function: CheckSystemCallingPermission
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService CheckSystemCallingPermission
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService CheckSystemCallingPermission is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_CheckSystemCallingPermission_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_CheckSystemCallingPermission_0200 start");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    auto originalToken = GetSelfTokenID();

    // set token native
    uint32_t tokenID = Security::AccessToken::DEFAULT_TOKEN_VERSION;
    Security::AccessToken::AccessTokenIDInner *idInner =
        reinterpret_cast<Security::AccessToken::AccessTokenIDInner *>(&tokenID);
    idInner->type = Security::AccessToken::TOKEN_NATIVE;
    SetSelfTokenID(tokenID);

    DataObsOption opt;
    bool ret = 0;
    opt.isSystem = false;
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, IDataObsMgr::DATAOBS_DEFAULT_CURRENT_USER, -1);
    EXPECT_EQ(ret, false);
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, IDataObsMgr::DATAOBS_DEFAULT_CURRENT_USER, 100);
    EXPECT_EQ(ret, true);
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, 100, 101);
    EXPECT_EQ(ret, false);
    opt.isSystem = true;
    ret = dataObsMgrServer->IsCallingPermissionValid(opt, IDataObsMgr::DATAOBS_DEFAULT_CURRENT_USER, 100);
    EXPECT_EQ(ret, false);

    SetSelfTokenID(originalToken);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_CheckSystemCallingPermission_0200 end");
}

/*
 * Feature: DataObsMgrService
 * Function: IsSystemApp
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService IsSystemApp
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService IsSystemApp is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_IsSystemApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_IsSystemApp_0100 start");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    auto originalToken = GetSelfTokenID();

    // set system app
    uint64_t systemAppMask = (static_cast<uint64_t>(1) << 32);
    uint32_t tokenID = Security::AccessToken::DEFAULT_TOKEN_VERSION;
    Security::AccessToken::AccessTokenIDInner *idInner =
        reinterpret_cast<Security::AccessToken::AccessTokenIDInner *>(&tokenID);
    idInner->type = Security::AccessToken::TOKEN_HAP;
    uint64_t fullTokenId = systemAppMask | tokenID;

    bool ret = dataObsMgrServer->IsSystemApp(tokenID, fullTokenId);
    EXPECT_EQ(ret, true);
    idInner->type = Security::AccessToken::TOKEN_NATIVE;
    ret = dataObsMgrServer->IsSystemApp(tokenID, fullTokenId);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_IsSystemApp_0100 end");
}

HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_VerifyDataSharePermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_VerifyDataSharePermission_0100 start");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    std::string proxyUriOk = "datashareproxy://com.acts.datasharetest/test";
    Uri uri(proxyUriOk);
    ObserverInfo info(0, 0, 0, 0, true);
    int32_t ret = dataObsMgrServer->VerifyDataSharePermission(uri, true, info);
    EXPECT_EQ(ret, DataShare::E_NOT_HAP);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_VerifyDataSharePermission_0100 end");
}

HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_Init_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_Init_0100 start");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    EXPECT_NE(dataObsMgrServer->permission_, nullptr);
    EXPECT_EQ(dataObsMgrServer->permission_->subscriber_, nullptr);
    dataObsMgrServer->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
    EXPECT_NE(dataObsMgrServer->permission_->subscriber_, nullptr);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_Init_0100 end");
}

HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_Init_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_Init_0200 start");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    dataObsMgrServer->permission_ = nullptr;
    dataObsMgrServer->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");

    dataObsMgrServer = std::make_shared<DataObsMgrService>();
    dataObsMgrServer->OnAddSystemAbility(0, "");
    EXPECT_EQ(dataObsMgrServer->permission_->subscriber_, nullptr);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_Init_0200 end");
}

HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_VerifyDataSharePermissionInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_VerifyDataSharePermissionInner_0100 start");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    dataObsMgrServer->permission_ = nullptr;
    Uri uri("");
    ObserverInfo info;
    int32_t ret = dataObsMgrServer->VerifyDataSharePermissionInner(uri, true, info);
    EXPECT_EQ(ret, COMMON_ERROR);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_VerifyDataSharePermissionInner_0100 end");
}

HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_VerifyDataSharePermissionInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_VerifyDataSharePermissionInner_0200 start");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();

    Uri uri("");
    ObserverInfo info;
    int32_t ret = dataObsMgrServer->VerifyDataSharePermissionInner(uri, true, info);
    EXPECT_EQ(ret, DATAOBS_INVALID_URI);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_VerifyDataSharePermissionInner_0200 end");
}

/*
 * Feature: DataObsMgrService
 * Function: test DATA_MANAGER_SERVICE_UID
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService DATA_MANAGER_SERVICE_UID
 * EnvConditions: NA
 * CaseDescription: Verify that the DATA_MANAGER_SERVICE_UID is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_DataMgrServiceUid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_GetDataMgrServiceUid_0100 start");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    // DATA_MANAGER_SERVICE_UID is 3012
    int32_t uid = 3012;
    uint32_t tokenID = Security::AccessToken::DEFAULT_TOKEN_VERSION;
    Security::AccessToken::AccessTokenIDInner *idInner =
        reinterpret_cast<Security::AccessToken::AccessTokenIDInner *>(&tokenID);
    idInner->type = Security::AccessToken::TOKEN_NATIVE;
    bool ret = dataObsMgrServer->IsDataMgrService(tokenID, uid);
    EXPECT_EQ(ret, true);
    idInner->type = Security::AccessToken::TOKEN_HAP;
    ret = dataObsMgrServer->IsDataMgrService(tokenID, uid);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_GetDataMgrServiceUid_0100 end");
}

/*
 * Feature: DataObsMgrService
 * Function: test ConstructObserverNode normal func
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService ConstructObserverNode
 * EnvConditions: NA
 * CaseDescription: Verify that the ConstructObserverNode is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_ConstructObserverNode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_ConstructObserverNode_0100 start");
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    int32_t userId = 0;
    uint32_t tokenId = 0;
    int32_t pid = 0;
    auto [ret1, obsNode1] = dataObsMgrServer->ConstructObserverNode(dataobsAbility, userId, tokenId, pid);
    EXPECT_TRUE(ret1);
    tokenId = 1;
    pid = 1;
    auto [ret2, obsNode2] = dataObsMgrServer->ConstructObserverNode(dataobsAbility, userId, tokenId, pid);
    EXPECT_TRUE(ret2);
    EXPECT_EQ(obsNode2.nodeId_, obsNode1.nodeId_ + 1);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_ConstructObserverNode_0100 end");
}

/*
 * Feature: DataObsMgrService with rdb uri
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RdbUriTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RdbUriTest_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("rdb://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserver(*uri, dataobsAbility));

    testing::Mock::AllowLeak(dataobsAbility);
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RdbUriTest_0100 end";
}

/*
 * Feature: DataObsMgrService with sharepreferences uri
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_SharePreferencesUri_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_SharePreferencesUri_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("sharepreferences://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserver(*uri, dataobsAbility));

    testing::Mock::AllowLeak(dataobsAbility);
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_SharePreferencesUri_0100 end";
}

/*
 * Feature: DataObsMgrService with NotifyProcessObserver
 * Function: GetFocusedWindowInfo
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService GetFocusedWindowInfo
 * EnvConditions: NA
 * CaseDescription: Check whether the default window information is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_GetFocusedWindowInfo_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_GetFocusedWindowInfo_0001 start";
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    DataObsMgrService::FocusedAppInfo appinfo = dataObsMgrServer->GetFocusedWindowInfo();
    EXPECT_EQ(appinfo.top, 0);
    EXPECT_EQ(appinfo.left, 0);
    EXPECT_EQ(appinfo.height, 0);
    EXPECT_EQ(appinfo.width, 0);
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_GetFocusedWindowInfo_0001 end";
}

/**
 * @tc.name: AaFwk_DataObsMgrServiceTest_RegisterObserver_0500
 * @tc.desc: Test dataObsMgrServer RegisterObserver calling from process that possess TOKEN_NATIVE
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to shell calling
    2. uri is in allowed provider list
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrService instance
    3. Call NotifyChange using DataObsMgrService
 * @tc.expect:
    1. NotifyChange return E_OK
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_RegisterObserver_0500 start");
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare:///com.ohos.contactsdataability");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility, -1, DataObsOption(false, true)));

    testing::Mock::AllowLeak(dataobsAbility);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_RegisterObserver_0500 end");
}

/**
 * @tc.name: AaFwk_DataObsMgrServiceTest_RegisterObserver_0510
 * @tc.desc: Test RegisterObserver with system permission
 * @tc.type: FUNC
 * @tc.require:
 * @tc.precon:
    1. process is equivalent to a system ability
    2. uri is in allowed provider list
 * @tc.step:
    1. Define a test Uri and an observer
    2. Get a DataObsMgrService instance
    3. Call RegisterObserver using DataObsMgrService
 * @tc.expect:
    1. RegisterObserver return E_OK
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0510, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_RegisterObserver_0510 start");
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare:///com.ohos.contactsdataability");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    auto originalToken = GetSelfTokenID();

    // set token native
    uint32_t tokenID = Security::AccessToken::DEFAULT_TOKEN_VERSION;
    Security::AccessToken::AccessTokenIDInner *idInner =
        reinterpret_cast<Security::AccessToken::AccessTokenIDInner *>(&tokenID);
    idInner->type = Security::AccessToken::TOKEN_NATIVE;
    SetSelfTokenID(tokenID);

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility, -1, DataObsOption(false, true)));

    SetSelfTokenID(originalToken);
    TAG_LOGI(AAFwkTag::DBOBSMGR, "AaFwk_DataObsMgrServiceTest_RegisterObserver_0510 end");
}
}  // namespace AAFwk
}  // namespace OHOS
