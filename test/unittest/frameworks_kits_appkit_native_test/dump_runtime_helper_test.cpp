/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "dump_runtime_helper.h"
#undef private
#include "parameters.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class DumpRuntimeHelperTest : public testing::Test {
public:
    DumpRuntimeHelperTest()
    {}
    ~DumpRuntimeHelperTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DumpRuntimeHelperTest::SetUpTestCase(void)
{}

void DumpRuntimeHelperTest::TearDownTestCase(void)
{}

void DumpRuntimeHelperTest::SetUp(void)
{
    OHOS::system::SetParameter("hiview.oomdump.switch", "");
}

void DumpRuntimeHelperTest::TearDown(void)
{
    OHOS::system::SetParameter("hiview.oomdump.switch", "");
}

/**
 * @tc.number: DumpJsHeap_0100
 * @tc.name: DumpJsHeap
 * @tc.desc: Test whether DumpJsHeap and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpJsHeap_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpJsHeap_0100 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = 1;
    info.tid = 1;
    info.needGc = false;
    info.needSnapshot = true;
    info.needLeakobj = false;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpJsHeap_0100 end";
}

/**
 * @tc.number: DumpCjHeap_0100
 * @tc.name: DumpCjHeap
 * @tc.desc: Test whether DumpCjHeap and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpCjHeap_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpCjHeap_0100 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    OHOS::AppExecFwk::CjHeapDumpInfo info;
    info.pid = 1;
    info.needGc = false;
    info.needSnapshot = true;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpCjHeap(info);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpCjHeap_0100 end";
}

/**
 * @tc.number: DumpCjHeap_0200
 * @tc.name: DumpCjHeap
 * @tc.desc: Test whether DumpCjHeap and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpCjHeap_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpCjHeap_0200 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    OHOS::AppExecFwk::CjHeapDumpInfo info;
    info.pid = 1;
    info.needGc = true;
    info.needSnapshot = false;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpCjHeap(info);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpCjHeap_0200 end";
}

/**
 * @tc.number: DumpCjHeap_0300
 * @tc.name: DumpCjHeap
 * @tc.desc: Test whether DumpCjHeap and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpCjHeap_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpCjHeap_0300 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    OHOS::AppExecFwk::CjHeapDumpInfo info;
    info.pid = 1;
    info.needGc = false;
    info.needSnapshot = false;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpCjHeap(info);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpCjHeap_0300 end";
}

/**
 * @tc.number: DumpMem_0100
 * @tc.name: DumpMem
 * @tc.desc: Test whether DumpMem and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpMem_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0100 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    OHOS::AppExecFwk::MemDumpInfo info;
    info.pid = 1;
    info.needLeakobj = false;
    info.dumpType = MemDumpType::KMP_KOTLIN;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpMem(info, nullptr);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0100 end";
}

/**
 * @tc.number: DumpMem_0200
 * @tc.name: DumpMem
 * @tc.desc: Test whether DumpMem and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpMem_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0200 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    OHOS::AppExecFwk::MemDumpInfo info;
    info.pid = 1;
    info.needLeakobj = false;
    info.dumpType = MemDumpType::NATIVE;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpMem(info, nullptr);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0200 end";
}

/**
 * @tc.number: DumpMem_0300
 * @tc.name: DumpMem
 * @tc.desc: Test whether DumpMem and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpMem_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0300 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    OHOS::AppExecFwk::MemDumpInfo info;
    info.pid = 1;
    info.needLeakobj = false;
    info.dumpType = MemDumpType::JSVM;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpMem(info, nullptr);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0300 end";
}

/**
 * @tc.number: DumpMem_0400
 * @tc.name: DumpMem
 * @tc.desc: Test whether DumpMem and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpMem_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0400 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    OHOS::AppExecFwk::MemDumpInfo info;
    info.pid = 1;
    info.needLeakobj = false;
    info.dumpType = MemDumpType::ARKWEB_JS;
    info.renderPid = 1234;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpMem(info, nullptr);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0400 end";
}

/**
 * @tc.number: CheckOomdumpSwitch_0100
 * @tc.name: CheckOomdumpSwitch
 * @tc.desc: Test the function of CheckOomdumpSwitch.
 */
HWTEST_F(DumpRuntimeHelperTest, CheckOomdumpSwitch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CheckOomdumpSwitch_0100 start";
    EXPECT_TRUE(DumpRuntimeHelper::CheckOomdumpSwitch());
    OHOS::system::SetParameter("hiview.oomdump.switch", "disable");
    EXPECT_FALSE(DumpRuntimeHelper::CheckOomdumpSwitch());
    OHOS::system::SetParameter("hiview.oomdump.switch", "unknown");
    EXPECT_TRUE(DumpRuntimeHelper::CheckOomdumpSwitch());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CheckOomdumpSwitch_0100 end";
}
}
}
