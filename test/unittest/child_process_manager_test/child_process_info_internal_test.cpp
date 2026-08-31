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

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "child_process_info_internal.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace {
const int32_t TEST_PID_1 = 1001;
const int32_t TEST_PID_2 = 1002;
const int32_t TEST_PARENT_PID = 2000;
const char* TEST_PROCESS_NAME_1 = "com.test.child1";
const char* TEST_PROCESS_NAME_2 = "com.test.child2";
}  // namespace

class ChildProcessInfoInternalTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ChildProcessInfoInternalTest::SetUpTestCase()
{}

void ChildProcessInfoInternalTest::TearDownTestCase()
{}

void ChildProcessInfoInternalTest::SetUp()
{}

void ChildProcessInfoInternalTest::TearDown()
{}

static OHOS::AppExecFwk::ChildProcessInfo MakeChildProcessInfo(int32_t pid, int32_t parentPid,
    const std::string& processName)
{
    OHOS::AppExecFwk::ChildProcessInfo info;
    info.pid = pid;
    info.hostPid = parentPid;
    info.processName = processName;
    return info;
}

HWTEST_F(ChildProcessInfoInternalTest, CreateAndFillChildProcessInfos_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "CreateAndFillChildProcessInfos_001 called.");
    std::vector<OHOS::AppExecFwk::ChildProcessInfo> childInfos;
    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    uint32_t count = 1;
    auto ret = CreateAndFillChildProcessInfos(childInfos, &infos, &count);
    EXPECT_TRUE(ret);
    EXPECT_EQ(count, 0);
    EXPECT_EQ(infos, nullptr);
}

HWTEST_F(ChildProcessInfoInternalTest, CreateAndFillChildProcessInfos_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "CreateAndFillChildProcessInfos_002 called.");
    std::vector<OHOS::AppExecFwk::ChildProcessInfo> childInfos;
    childInfos.push_back(MakeChildProcessInfo(TEST_PID_1, TEST_PARENT_PID, TEST_PROCESS_NAME_1));
    childInfos.push_back(MakeChildProcessInfo(TEST_PID_2, TEST_PARENT_PID, TEST_PROCESS_NAME_2));

    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    uint32_t count = 0;
    auto ret = CreateAndFillChildProcessInfos(childInfos, &infos, &count);
    EXPECT_TRUE(ret);
    EXPECT_EQ(count, 2);
    ASSERT_NE(infos, nullptr);

    OH_AbilityRuntime_ChildProcessInfoHandle item0 = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_GetChildProcessInfoByIndex(infos, 0, &item0),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(item0, nullptr);
    int32_t pid = 0;
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetPid(item0, &pid), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(pid, TEST_PID_1);
    int32_t parentPid = 0;
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetParentPid(item0, &parentPid),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(parentPid, TEST_PARENT_PID);
    char name[64] = {0};
    uint32_t requiredSize = 0;
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetProcessName(item0, name, 64, &requiredSize),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(name, TEST_PROCESS_NAME_1);
    EXPECT_EQ(requiredSize, static_cast<uint32_t>(strlen(TEST_PROCESS_NAME_1)) + 1);

    OH_AbilityRuntime_ChildProcessInfoHandle item1 = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_GetChildProcessInfoByIndex(infos, 1, &item1),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(item1, nullptr);
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetPid(item1, &pid), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(pid, TEST_PID_2);
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetProcessName(item1, name, 64, &requiredSize),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(name, TEST_PROCESS_NAME_2);

    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
    EXPECT_EQ(infos, nullptr);
}

HWTEST_F(ChildProcessInfoInternalTest, CreateAndFillChildProcessInfos_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "CreateAndFillChildProcessInfos_003 called.");
    std::vector<OHOS::AppExecFwk::ChildProcessInfo> childInfos;
    uint32_t count = 0;
    EXPECT_FALSE(CreateAndFillChildProcessInfos(childInfos, nullptr, &count));
    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    EXPECT_FALSE(CreateAndFillChildProcessInfos(childInfos, &infos, nullptr));
}

HWTEST_F(ChildProcessInfoInternalTest, OH_AbilityRuntime_GetChildProcessInfoByIndex_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetChildProcessInfoByIndex_001 called.");
    std::vector<OHOS::AppExecFwk::ChildProcessInfo> childInfos;
    childInfos.push_back(MakeChildProcessInfo(TEST_PID_1, TEST_PARENT_PID, TEST_PROCESS_NAME_1));

    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    uint32_t count = 0;
    CreateAndFillChildProcessInfos(childInfos, &infos, &count);
    ASSERT_NE(infos, nullptr);

    OH_AbilityRuntime_ChildProcessInfoHandle item = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_GetChildProcessInfoByIndex(infos, 1, &item),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(item, nullptr);

    EXPECT_EQ(OH_AbilityRuntime_GetChildProcessInfoByIndex(nullptr, 0, &item),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    EXPECT_EQ(OH_AbilityRuntime_GetChildProcessInfoByIndex(infos, 0, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
}

HWTEST_F(ChildProcessInfoInternalTest, OH_AbilityRuntime_ChildProcessInfo_GetPid_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetPid_001 called.");
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetPid(nullptr, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    int32_t pid = 0;
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetPid(nullptr, &pid),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    std::vector<OHOS::AppExecFwk::ChildProcessInfo> childInfos;
    childInfos.push_back(MakeChildProcessInfo(TEST_PID_1, TEST_PARENT_PID, TEST_PROCESS_NAME_1));
    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    uint32_t count = 0;
    CreateAndFillChildProcessInfos(childInfos, &infos, &count);
    ASSERT_NE(infos, nullptr);
    OH_AbilityRuntime_ChildProcessInfoHandle item = nullptr;
    OH_AbilityRuntime_GetChildProcessInfoByIndex(infos, 0, &item);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetPid(item, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetPid(item, &pid), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(pid, TEST_PID_1);
    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
}

HWTEST_F(ChildProcessInfoInternalTest, OH_AbilityRuntime_ChildProcessInfo_GetParentPid_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetParentPid_001 called.");
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetParentPid(nullptr, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    int32_t parentPid = 0;
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetParentPid(nullptr, &parentPid),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    std::vector<OHOS::AppExecFwk::ChildProcessInfo> childInfos;
    childInfos.push_back(MakeChildProcessInfo(TEST_PID_1, TEST_PARENT_PID, TEST_PROCESS_NAME_1));
    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    uint32_t count = 0;
    CreateAndFillChildProcessInfos(childInfos, &infos, &count);
    ASSERT_NE(infos, nullptr);
    OH_AbilityRuntime_ChildProcessInfoHandle item = nullptr;
    OH_AbilityRuntime_GetChildProcessInfoByIndex(infos, 0, &item);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetParentPid(item, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetParentPid(item, &parentPid),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(parentPid, TEST_PARENT_PID);
    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
}

HWTEST_F(ChildProcessInfoInternalTest, OH_AbilityRuntime_ChildProcessInfo_GetProcessName_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetProcessName_001 called.");
    std::vector<OHOS::AppExecFwk::ChildProcessInfo> childInfos;
    childInfos.push_back(MakeChildProcessInfo(TEST_PID_1, TEST_PARENT_PID, TEST_PROCESS_NAME_1));
    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    uint32_t count = 0;
    CreateAndFillChildProcessInfos(childInfos, &infos, &count);
    ASSERT_NE(infos, nullptr);
    OH_AbilityRuntime_ChildProcessInfoHandle item = nullptr;
    OH_AbilityRuntime_GetChildProcessInfoByIndex(infos, 0, &item);
    ASSERT_NE(item, nullptr);

    char name[64] = {0};
    uint32_t requiredSize = 0;
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetProcessName(item, name, 64, &requiredSize),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(name, TEST_PROCESS_NAME_1);
    EXPECT_EQ(requiredSize, static_cast<uint32_t>(strlen(TEST_PROCESS_NAME_1)) + 1);

    char smallName[4] = {0};
    uint32_t smallRequired = 0;
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetProcessName(item, smallName, 4, &smallRequired),
        ABILITY_RUNTIME_ERROR_CODE_BUFFER_TOO_SMALL);
    EXPECT_EQ(smallRequired, static_cast<uint32_t>(strlen(TEST_PROCESS_NAME_1)) + 1);

    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetProcessName(item, name, 0, &requiredSize),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetProcessName(item, nullptr, 64, &requiredSize),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetProcessName(item, name, 64, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ChildProcessInfo_GetProcessName(nullptr, name, 64, &requiredSize),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
}

HWTEST_F(ChildProcessInfoInternalTest, OH_AbilityRuntime_ReleaseChildProcessInfos_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ReleaseChildProcessInfos_001 called.");
    OH_AbilityRuntime_ReleaseChildProcessInfos(nullptr);
    OH_AbilityRuntime_ChildProcessInfosHandle infos = nullptr;
    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
    EXPECT_EQ(infos, nullptr);

    std::vector<OHOS::AppExecFwk::ChildProcessInfo> childInfos;
    childInfos.push_back(MakeChildProcessInfo(TEST_PID_1, TEST_PARENT_PID, TEST_PROCESS_NAME_1));
    uint32_t count = 0;
    CreateAndFillChildProcessInfos(childInfos, &infos, &count);
    ASSERT_NE(infos, nullptr);
    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
    EXPECT_EQ(infos, nullptr);
    OH_AbilityRuntime_ReleaseChildProcessInfos(&infos);
    EXPECT_EQ(infos, nullptr);
}
