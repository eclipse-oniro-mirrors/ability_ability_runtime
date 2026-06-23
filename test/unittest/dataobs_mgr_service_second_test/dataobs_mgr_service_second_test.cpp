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
#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "hilog_tag_wrapper.h"

#include "dataobs_mgr_service.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
class DataObsMgrServiceSecondTest : public testing::Test {
public:
    DataObsMgrServiceSecondTest() = default;
    virtual ~DataObsMgrServiceSecondTest() = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DataObsMgrServiceSecondTest::SetUpTestCase(void)
{}
void DataObsMgrServiceSecondTest::TearDownTestCase(void)
{}
void DataObsMgrServiceSecondTest::SetUp()
{}
void DataObsMgrServiceSecondTest::TearDown()
{}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is normal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyChange_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChange_0100 start");
    const int testVal = static_cast<int>(NO_ERROR);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    dataObsMgrServer->Init();

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChange_0100 end");
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is normal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyChange_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChange_0200 start");
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    dataObsMgrServer->Init();
    auto tmp = dataObsMgrServer->dataObsMgrInner_;
    dataObsMgrServer->dataObsMgrInner_ = nullptr;

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));
    dataObsMgrServer->dataObsMgrInner_ = tmp;
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChange_0200 end");
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is normal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyChange_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChange_0300 start");
    const int testVal = static_cast<int>(DATAOBS_SERVICE_TASK_LIMMIT);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    dataObsMgrServer->Init();
    auto tmp = dataObsMgrServer->taskCount_;
    dataObsMgrServer->taskCount_ = DataObsMgrService::TASK_COUNT_MAX;

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));
    dataObsMgrServer->taskCount_ = tmp;
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChange_0300 end");
}

/*
 * Feature: DataObsMgrService
 * Function: DeepCopyChangeInfo
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService DeepCopyChangeInfo
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService DeepCopyChangeInfo is normal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_DeepCopyChangeInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_DeepCopyChangeInfo_0100 start");
    const int testVal = static_cast<int>(SUCCESS);
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    ChangeInfo src;
    ChangeInfo dst;

    EXPECT_EQ(testVal, dataObsMgrServer->DeepCopyChangeInfo(src, dst));
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_DeepCopyChangeInfo_0100 end");
}

/*
 * Feature: DataObsMgrService
 * Function: DeepCopyChangeInfo
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService DeepCopyChangeInfo
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService DeepCopyChangeInfo is normal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_DeepCopyChangeInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_DeepCopyChangeInfo_0200 start");
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    ChangeInfo src;
    ChangeInfo dst;
    src.size_ = std::numeric_limits<decltype(src.size_)>::max();

    EXPECT_EQ(testVal, dataObsMgrServer->DeepCopyChangeInfo(src, dst));
    src.size_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_DeepCopyChangeInfo_0200 end");
}

/*
 * Feature: DataObsMgrService
 * Function: DeepCopyChangeInfo
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService DeepCopyChangeInfo
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService DeepCopyChangeInfo is normal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_DeepCopyChangeInfo_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_DeepCopyChangeInfo_0300 start");
    const int testVal = static_cast<int>(SUCCESS);
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    ChangeInfo src;
    src.size_ = 1;
    src.data_ = new uint8_t[src.size_];
    ChangeInfo dst;

    EXPECT_EQ(testVal, dataObsMgrServer->DeepCopyChangeInfo(src, dst));
    delete [] static_cast<uint8_t *>(src.data_);
    src.data_ = nullptr;
    src.size_ = 0;
    delete [] static_cast<uint8_t *>(dst.data_);
    dst.data_ = nullptr;
    dst.size_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_DeepCopyChangeInfo_0300 end");
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyChangeExt_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0100 start");
    const int testVal = static_cast<int>(SUCCESS);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    dataObsMgrServer->Init();

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0100 end");
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyChangeExt_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0200 start");
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    dataObsMgrServer->Init();
    auto tmp = dataObsMgrServer->dataObsMgrInnerExt_;
    dataObsMgrServer->dataObsMgrInnerExt_ = nullptr;

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    dataObsMgrServer->dataObsMgrInnerExt_ = tmp;
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0200 end");
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyChangeExt_0300, TestSize.Level1)


{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0300 start");
    const int testVal = static_cast<int>(DATAOBS_SERVICE_TASK_LIMMIT);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    dataObsMgrServer->Init();
    auto tmp = dataObsMgrServer->taskCount_;
    dataObsMgrServer->taskCount_ = DataObsMgrService::TASK_COUNT_MAX;

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    dataObsMgrServer->taskCount_ = tmp;
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0300 end");
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyChangeExt_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0400 start");
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    dataObsMgrServer->Init();
    ChangeInfo changeInfo;
    auto tmp = changeInfo.size_;
    changeInfo.size_ = std::numeric_limits<decltype(changeInfo.size_)>::max();

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt(changeInfo));
    changeInfo.size_ = tmp;
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0400 end");
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyChangeExt_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0500 start");
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    dataObsMgrServer->handler_ = nullptr;

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0500 end");
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyChangeExt_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0600 start");
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = std::make_shared<DataObsMgrService>();
    dataObsMgrServer->Init();
    auto tmp = dataObsMgrServer->dataObsMgrInner_;
    dataObsMgrServer->dataObsMgrInner_ = nullptr;

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    dataObsMgrServer->dataObsMgrInner_ = tmp;
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyChangeExt_0600 end");
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyProcessObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyProcessObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyProcessObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceSecondTest, DataObsMgrServiceSecondTest_NotifyProcessObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyProcessObserver_0100 start");
    const int testVal = static_cast<int>(DATAOBS_PROXY_INNER_ERR);
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();
    const std::string key;
    const sptr<IRemoteObject> observer;

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyProcessObserver(key, observer));
    TAG_LOGI(AAFwkTag::TEST, "DataObsMgrServiceSecondTest_NotifyProcessObserver_0100 end");
}

}  // namespace AAFwk
}  // namespace OHOS
