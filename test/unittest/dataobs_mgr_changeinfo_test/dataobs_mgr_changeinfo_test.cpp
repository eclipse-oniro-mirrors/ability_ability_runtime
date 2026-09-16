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
#include <cstring>
#include <gtest/gtest.h>
#include <memory>

#include "dataobs_mgr_changeinfo.h"
#include "message_parcel.h"
#include "uri.h"

using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace DataObsMgrChangeInfoTest {
using namespace AAFwk;

class DataObsMgrChangeInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataObsMgrChangeInfoTest::SetUpTestCase(void) {}
void DataObsMgrChangeInfoTest::TearDownTestCase(void) {}
void DataObsMgrChangeInfoTest::SetUp() {}
void DataObsMgrChangeInfoTest::TearDown() {}

/*
 * Feature: DataObsMgrChangeInfo
 * Function: ChangeInfo Marshalling test
 * SubFunction: 0100
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Marshalling change info whose data size exceeds MAX_DATA_SIZE should fail
 */
HWTEST_F(DataObsMgrChangeInfoTest, DataObsMgrChangeInfo_Marshalling_0100, TestSize.Level1)
{
    ChangeInfo changeInfo;
    changeInfo.changeType_ = ChangeInfo::ChangeType::INSERT;
    changeInfo.uris_ = { Uri("datashare://Authority/com.domainname.dataability.persondata/Person") };
    changeInfo.size_ = ChangeInfo::MAX_DATA_SIZE + 1;
    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(changeInfo.size_);
    changeInfo.data_ = data.get();

    MessageParcel parcel;
    // enlarge the parcel capacity so this case verifies the MAX_DATA_SIZE check instead of the parcel capacity limit
    ASSERT_TRUE(parcel.SetMaxCapacity(ChangeInfo::MAX_DATA_SIZE * 2));
    EXPECT_FALSE(ChangeInfo::Marshalling(changeInfo, parcel));
}

/*
 * Feature: DataObsMgrChangeInfo
 * Function: ChangeInfo Marshalling test
 * SubFunction: 0200
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Marshalling and unmarshalling change info without data
 */
HWTEST_F(DataObsMgrChangeInfoTest, DataObsMgrChangeInfo_Marshalling_0200, TestSize.Level1)
{
    ChangeInfo changeInfo;
    changeInfo.changeType_ = ChangeInfo::ChangeType::UPDATE;
    changeInfo.uris_ = { Uri("datashare://Authority/com.domainname.dataability.persondata/Person/2") };
    changeInfo.size_ = 0;
    changeInfo.data_ = nullptr;

    MessageParcel parcel;
    EXPECT_TRUE(ChangeInfo::Marshalling(changeInfo, parcel));

    ChangeInfo output;
    EXPECT_TRUE(ChangeInfo::Unmarshalling(output, parcel));
    EXPECT_EQ(output.changeType_, ChangeInfo::ChangeType::UPDATE);
    ASSERT_EQ(output.uris_.size(), static_cast<size_t>(1));
    EXPECT_EQ(output.uris_.front().ToString(), changeInfo.uris_.front().ToString());
    EXPECT_EQ(output.size_, 0U);
    EXPECT_EQ(output.data_, nullptr);
}

/*
 * Feature: DataObsMgrChangeInfo
 * Function: ChangeInfo Marshalling test
 * SubFunction: 0300
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Marshalling and unmarshalling change info with data size of MAX_DATA_SIZE
 */
HWTEST_F(DataObsMgrChangeInfoTest, DataObsMgrChangeInfo_Marshalling_0300, TestSize.Level1)
{
    ChangeInfo changeInfo;
    changeInfo.changeType_ = ChangeInfo::ChangeType::DELETE;
    changeInfo.uris_ = { Uri("datashare://Authority/com.domainname.dataability.persondata/Person/3") };
    changeInfo.size_ = ChangeInfo::MAX_DATA_SIZE;
    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(changeInfo.size_);
    for (uint32_t i = 0; i < changeInfo.size_; i++) {
        data[i] = static_cast<uint8_t>(i);
    }
    changeInfo.data_ = data.get();

    MessageParcel parcel;
    // a default parcel caps its total capacity at MAX_DATA_SIZE, which cannot hold the boundary data plus headers
    ASSERT_TRUE(parcel.SetMaxCapacity(ChangeInfo::MAX_DATA_SIZE * 2));
    EXPECT_TRUE(ChangeInfo::Marshalling(changeInfo, parcel));

    ChangeInfo output;
    EXPECT_TRUE(ChangeInfo::Unmarshalling(output, parcel));
    EXPECT_EQ(output.changeType_, ChangeInfo::ChangeType::DELETE);
    ASSERT_EQ(output.uris_.size(), static_cast<size_t>(1));
    EXPECT_EQ(output.uris_.front().ToString(), changeInfo.uris_.front().ToString());
    EXPECT_EQ(output.size_, ChangeInfo::MAX_DATA_SIZE);
    ASSERT_NE(output.data_, nullptr);
    EXPECT_EQ(std::memcmp(output.data_, data.get(), changeInfo.size_), 0);
}

/*
 * Feature: DataObsMgrChangeInfo
 * Function: ChangeInfo Marshalling test
 * SubFunction: 0400
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:Marshalling and unmarshalling change info with data and value buckets
 */
HWTEST_F(DataObsMgrChangeInfoTest, DataObsMgrChangeInfo_Marshalling_0400, TestSize.Level1)
{
    ChangeInfo changeInfo;
    changeInfo.changeType_ = ChangeInfo::ChangeType::OTHER;
    Uri uri4("datashare://Authority/com.domainname.dataability.persondata/Person/4");
    Uri uri5("datashare://Authority/com.domainname.dataability.persondata/Person/5");
    changeInfo.uris_ = { uri4, uri5 };
    changeInfo.size_ = 8;
    std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(changeInfo.size_);
    for (uint32_t i = 0; i < changeInfo.size_; i++) {
        data[i] = static_cast<uint8_t>(i);
    }
    changeInfo.data_ = data.get();
    ChangeInfo::VBucket bucket;
    bucket.emplace("id", static_cast<int64_t>(1));
    bucket.emplace("name", std::string("test"));
    changeInfo.valueBuckets_.emplace_back(bucket);

    MessageParcel parcel;
    EXPECT_TRUE(ChangeInfo::Marshalling(changeInfo, parcel));

    ChangeInfo output;
    EXPECT_TRUE(ChangeInfo::Unmarshalling(output, parcel));
    EXPECT_EQ(output.changeType_, ChangeInfo::ChangeType::OTHER);
    ASSERT_EQ(output.uris_.size(), static_cast<size_t>(2));
    EXPECT_EQ(output.uris_.front().ToString(), uri4.ToString());
    EXPECT_EQ(output.uris_.back().ToString(), uri5.ToString());
    EXPECT_EQ(output.size_, 8U);
    ASSERT_NE(output.data_, nullptr);
    EXPECT_EQ(std::memcmp(output.data_, data.get(), changeInfo.size_), 0);
    ASSERT_EQ(output.valueBuckets_.size(), static_cast<size_t>(1));
    EXPECT_EQ(output.valueBuckets_, changeInfo.valueBuckets_);
}
} // namespace DataObsMgrChangeInfoTest
} // namespace OHOS
