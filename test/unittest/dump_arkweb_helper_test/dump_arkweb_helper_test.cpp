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

#include "dump_arkweb_helper.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class DumpArkWebHelperTest : public testing::Test {
public:
    DumpArkWebHelperTest()
    {}
    ~DumpArkWebHelperTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DumpArkWebHelperTest::SetUpTestCase(void)
{}

void DumpArkWebHelperTest::TearDownTestCase(void)
{}

void DumpArkWebHelperTest::SetUp(void)
{}

void DumpArkWebHelperTest::TearDown(void)
{}

/**
 * @tc.number: DumpArkWebHelperTest_001
 * @tc.name: DumpArkWeb
 * @tc.desc: Test whether DumpArkWeb is called normally.
 * @tc.type: FUNC
 */
HWTEST_F(DumpArkWebHelperTest, DumpArkWebHelperTest_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpArkWebHelperTest_001 start";
    std::string result = "result";
    std::string customArgs;
    auto ret = DumpArkWebHelper::DumpArkWeb(customArgs, result);
    EXPECT_EQ(ret, 0);
    GTEST_LOG_(INFO) << "DumpArkWebHelperTest_001 end";
}

/**
 * @tc.number: DumpArkWebHelperTest_002
 * @tc.name: DumpArkWebJSHeap
 * @tc.desc: Test whether DumpArkWebJSHeap is called normally.
 * @tc.type: FUNC
 */
HWTEST_F(DumpArkWebHelperTest, DumpArkWebHelperTest_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpArkWebHelperTest_002 start";
    int32_t appPid = 1001;
    int32_t renderPid = 2002;
    EXPECT_NO_FATAL_FAILURE(DumpArkWebHelper::DumpArkWebJSHeap(appPid, renderPid, false, false, false));
    GTEST_LOG_(INFO) << "DumpArkWebHelperTest_002 end";
}

/**
 * @tc.number: DumpArkWebHelperTest_003
 * @tc.name: DumpArkWebJSHeap
 * @tc.desc: Test whether DumpArkWebJSHeap handles snapshot fd request failure.
 * @tc.type: FUNC
 */
HWTEST_F(DumpArkWebHelperTest, DumpArkWebHelperTest_003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpArkWebHelperTest_003 start";
    int32_t appPid = -1;
    int32_t renderPid = -1;
    EXPECT_NO_FATAL_FAILURE(DumpArkWebHelper::DumpArkWebJSHeap(appPid, renderPid, true, false, false));
    GTEST_LOG_(INFO) << "DumpArkWebHelperTest_003 end";
}

/**
 * @tc.number: DumpArkWebHelperTest_004
 * @tc.name: DumpArkWebJSHeap
 * @tc.desc: Test whether DumpArkWebJSHeap handles raw snapshot fd request failure.
 * @tc.type: FUNC
 */
HWTEST_F(DumpArkWebHelperTest, DumpArkWebHelperTest_004, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpArkWebHelperTest_004 start";
    int32_t appPid = -1;
    int32_t renderPid = -1;
    EXPECT_NO_FATAL_FAILURE(DumpArkWebHelper::DumpArkWebJSHeap(appPid, renderPid, true, false, true));
    GTEST_LOG_(INFO) << "DumpArkWebHelperTest_004 end";
}
}
}
