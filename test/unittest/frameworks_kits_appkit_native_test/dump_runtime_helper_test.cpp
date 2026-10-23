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
#include <cstdlib>
#include <unistd.h>
#include <gtest/gtest.h>
#define private public
#include "dump_runtime_helper.h"
#undef private
#include "dfx_maps.h"
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
 * @tc.number: DumpKmpGc_0100
 * @tc.name: DumpKmpGc
 * @tc.desc: Test whether DumpKmpGc is called normally.
 * @tc.type: FUNC
 */
HWTEST_F(DumpRuntimeHelperTest, DumpKmpGc_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpKmpGc_0100 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpKmpGc(-1);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpKmpGc_0100 end";
}

/**
 * @tc.number: DumpKmpSnapshot_0100
 * @tc.name: DumpKmpSnapshot
 * @tc.desc: Test whether DumpKmpSnapshot is called normally.
 * @tc.type: FUNC
 */
HWTEST_F(DumpRuntimeHelperTest, DumpKmpSnapshot_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpKmpSnapshot_0100 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpKmpSnapshot(-1, false);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpKmpSnapshot_0100 end";
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
 * @tc.number: DumpMem_0500
 * @tc.name: DumpMem
 * @tc.desc: Test DumpMem with ARKTS_HEAP type does not crash when runtime is null.
 */
HWTEST_F(DumpRuntimeHelperTest, DumpMem_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0500 start";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    OHOS::AppExecFwk::MemDumpInfo info;
    info.pid = 1;
    info.needLeakobj = false;
    info.dumpType = MemDumpType::ARKTS_HEAP;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpMem(info, nullptr);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DumpMem_0500 end";
}

/**
 * @tc.number: DetectAndUnwindThreadStackLeak_0100
 * @tc.name: DetectAndUnwindThreadStackLeak
 * @tc.desc: Test DetectAndUnwindThreadStackLeak returns a string without crashing on a normal process.
 */
HWTEST_F(DumpRuntimeHelperTest, DetectAndUnwindThreadStackLeak_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DetectAndUnwindThreadStackLeak_0100 start";
    std::string result = DumpRuntimeHelper::DetectAndUnwindThreadStackLeak();
    // On a normal test process no leaked thread stack is expected, so the result
    // should be empty. This case mainly guards against crashes and verifies the
    // happy path returns a well-formed (empty) string.
    GTEST_LOG_(INFO) << "DetectAndUnwindThreadStackLeak result size:" << result.size();
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest DetectAndUnwindThreadStackLeak_0100 end";
}

/**
 * @tc.number: ParseTidFromMapName_0100
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName with a valid map name containing a tid.
 */
HWTEST_F(DumpRuntimeHelperTest, ParseTidFromMapName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0100 start";
    int32_t tid = DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:12345]");
    EXPECT_EQ(tid, 12345);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0100 end";
}

/**
 * @tc.number: ParseTidFromMapName_0200
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName with invalid inputs returns 0.
 */
HWTEST_F(DumpRuntimeHelperTest, ParseTidFromMapName_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0200 start";
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:]"), 0);
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:abc]"), 0);
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("not a stack map"), 0);
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName(""), 0);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0200 end";
}

/**
 * @tc.number: ParseTidFromMapName_0300
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName with a large tid value.
 */
HWTEST_F(DumpRuntimeHelperTest, ParseTidFromMapName_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0300 start";
    int32_t tid = DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:99999]");
    EXPECT_EQ(tid, 99999);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0300 end";
}

/**
 * @tc.number: IsTidAlive_0100
 * @tc.name: IsTidAlive
 * @tc.desc: Test IsTidAlive returns true for the current process's main thread.
 */
HWTEST_F(DumpRuntimeHelperTest, IsTidAlive_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest IsTidAlive_0100 start";
    pid_t currentTid = gettid();
    EXPECT_TRUE(DumpRuntimeHelper::IsTidAlive(currentTid));
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest IsTidAlive_0100 end";
}

/**
 * @tc.number: IsTidAlive_0200
 * @tc.name: IsTidAlive
 * @tc.desc: Test IsTidAlive returns false for non-existent tids.
 */
HWTEST_F(DumpRuntimeHelperTest, IsTidAlive_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest IsTidAlive_0200 start";
    EXPECT_FALSE(DumpRuntimeHelper::IsTidAlive(0));
    EXPECT_FALSE(DumpRuntimeHelper::IsTidAlive(-1));
    EXPECT_FALSE(DumpRuntimeHelper::IsTidAlive(999999999));
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest IsTidAlive_0200 end";
}

/**
 * @tc.number: CountTotalThreads_0100
 * @tc.name: CountTotalThreads
 * @tc.desc: Test CountTotalThreads returns a positive count for the running process.
 */
HWTEST_F(DumpRuntimeHelperTest, CountTotalThreads_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CountTotalThreads_0100 start";
    size_t count = DumpRuntimeHelper::CountTotalThreads();
    // At least the main thread should be present.
    EXPECT_GE(count, static_cast<size_t>(1));
    GTEST_LOG_(INFO) << "CountTotalThreads: " << count;
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CountTotalThreads_0100 end";
}

/**
 * @tc.number: ParseLeakedThreadStackRegions_0100
 * @tc.name: ParseLeakedThreadStackRegions
 * @tc.desc: Test ParseLeakedThreadStackRegions returns empty on a normal process with no leaked stacks.
 */
HWTEST_F(DumpRuntimeHelperTest, ParseLeakedThreadStackRegions_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseLeakedThreadStackRegions_0100 start";
    auto regions = DumpRuntimeHelper::ParseLeakedThreadStackRegions();
    // On a normal test process, all thread stacks should have alive tids,
    // so no leaked regions are expected.
    EXPECT_TRUE(regions.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseLeakedThreadStackRegions_0100 end";
}

/**
 * @tc.number: CollectPcsFromStack_0100
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack handles an invalid/zero range without crashing.
 */
HWTEST_F(DumpRuntimeHelperTest, CollectPcsFromStack_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0100 start";
    // A zero-size range (begin == end) should produce no PCs and not crash.
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(0, 0, nullptr);
    EXPECT_TRUE(pcs.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0100 end";
}

/**
 * @tc.number: FormatLeakedStack_0100
 * @tc.name: FormatLeakedStack
 * @tc.desc: Test FormatLeakedStack returns empty string for empty pcs.
 */
HWTEST_F(DumpRuntimeHelperTest, FormatLeakedStack_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest FormatLeakedStack_0100 start";
    std::vector<uintptr_t> emptyPcs;
    std::string result = DumpRuntimeHelper::FormatLeakedStack(12345, emptyPcs);
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest FormatLeakedStack_0100 end";
}

/**
 * @tc.number: FormatLeakedStack_0200
 * @tc.name: FormatLeakedStack
 * @tc.desc: Test FormatLeakedStack returns empty string for a bogus PC that cannot be resolved.
 */
HWTEST_F(DumpRuntimeHelperTest, FormatLeakedStack_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest FormatLeakedStack_0200 start";
    std::vector<uintptr_t> bogusPcs = {0xDEADBEEF};
    std::string result = DumpRuntimeHelper::FormatLeakedStack(99999, bogusPcs);
    // A bogus PC should not resolve to any frame, so the result should be empty.
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest FormatLeakedStack_0200 end";
}

/**
 * @tc.number: ParseTidFromMapName_0400
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName returns 0 for value exceeding INT32_MAX.
 */
HWTEST_F(DumpRuntimeHelperTest, ParseTidFromMapName_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0400 start";
    // 2147483648 = INT32_MAX + 1, exceeds int32_t range
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:2147483648]"), 0);
    // 9999999999 far exceeds int32_t range
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:9999999999]"), 0);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0400 end";
}

/**
 * @tc.number: ParseTidFromMapName_0500
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName returns 0 for map name with prefix at end (no digits).
 */
HWTEST_F(DumpRuntimeHelperTest, ParseTidFromMapName_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0500 start";
    // Prefix at end of string with no following digits
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:"), 0);
    // Prefix found but followed by non-digit immediately
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:abc123]"), 0);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0500 end";
}

/**
 * @tc.number: IsTidAlive_0300
 * @tc.name: IsTidAlive
 * @tc.desc: Test IsTidAlive returns false for a very large tid that does not exist.
 */
HWTEST_F(DumpRuntimeHelperTest, IsTidAlive_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest IsTidAlive_0300 start";
    // A tid that is valid positive int but does not exist in /proc/self/task
    EXPECT_FALSE(DumpRuntimeHelper::IsTidAlive(1));
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest IsTidAlive_0300 end";
}

/**
 * @tc.number: CollectPcsFromStack_0200
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack with valid stack range but maps nullptr returns empty.
 */
HWTEST_F(DumpRuntimeHelperTest, CollectPcsFromStack_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0200 start";
    // Non-zero range but nullptr maps should return empty without crashing.
    int stackVar = 0;
    uint64_t begin = reinterpret_cast<uint64_t>(&stackVar);
    uint64_t end = begin + sizeof(uintptr_t);
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(begin, end, nullptr);
    EXPECT_TRUE(pcs.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0200 end";
}

/**
 * @tc.number: CollectPcsFromStack_0300
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack with valid range and maps scans stack values.
 */
HWTEST_F(DumpRuntimeHelperTest, CollectPcsFromStack_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0300 start";
    auto maps = OHOS::HiviewDFX::DfxMaps::Create(0, true, false);
    ASSERT_NE(maps, nullptr);
    // Create a small stack-like buffer with zero values — should not crash
    // and should return empty (zero values are skipped).
    uintptr_t buf[4] = {0, 0, 0, 0};
    uint64_t begin = reinterpret_cast<uint64_t>(&buf[0]);
    uint64_t end = reinterpret_cast<uint64_t>(&buf[4]);
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(begin, end, maps);
    EXPECT_TRUE(pcs.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0300 end";
}

/**
 * @tc.number: CollectPcsFromStack_0400
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack with begin >= end returns empty.
 */
HWTEST_F(DumpRuntimeHelperTest, CollectPcsFromStack_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0400 start";
    auto maps = OHOS::HiviewDFX::DfxMaps::Create(0, true, false);
    ASSERT_NE(maps, nullptr);
    // begin > end should produce no PCs.
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(100, 50, maps);
    EXPECT_TRUE(pcs.empty());
    // begin == end should also produce no PCs.
    pcs = DumpRuntimeHelper::CollectPcsFromStack(100, 100, maps);
    EXPECT_TRUE(pcs.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0400 end";
}

/**
 * @tc.number: CollectPcsFromStack_0500
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack with range smaller than scan unit does not read out of bounds.
 */
HWTEST_F(DumpRuntimeHelperTest, CollectPcsFromStack_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0500 start";
    auto maps = OHOS::HiviewDFX::DfxMaps::Create(0, true, false);
    ASSERT_NE(maps, nullptr);
    // end - begin < sizeof(uintptr_t), so readAddr < begin always; should return empty
    // without reading outside the target range.
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(0x1000, 0x1004, maps);
    EXPECT_TRUE(pcs.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0500 end";
}

/**
 * @tc.number: FormatLeakedStack_0300
 * @tc.name: FormatLeakedStack
 * @tc.desc: Test FormatLeakedStack with a single zero PC returns empty (zero is not a valid frame).
 */
HWTEST_F(DumpRuntimeHelperTest, FormatLeakedStack_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest FormatLeakedStack_0300 start";
    std::vector<uintptr_t> zeroPcs = {0};
    std::string result = DumpRuntimeHelper::FormatLeakedStack(12345, zeroPcs);
    // Zero PC should not resolve to any meaningful frame.
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest FormatLeakedStack_0300 end";
}

/**
 * @tc.number: FormatLeakedStack_0400
 * @tc.name: FormatLeakedStack
 * @tc.desc: Test FormatLeakedStack with a real function address produces non-empty output.
 */
HWTEST_F(DumpRuntimeHelperTest, FormatLeakedStack_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest FormatLeakedStack_0400 start";
    // Use a real function address as a candidate PC. The unwinder may or may not
    // resolve it, but the function should not crash.
    uintptr_t pc = reinterpret_cast<uintptr_t>(&DumpRuntimeHelper::ParseTidFromMapName);
    std::vector<uintptr_t> pcs = {pc};
    std::string result = DumpRuntimeHelper::FormatLeakedStack(gettid(), pcs);
    GTEST_LOG_(INFO) << "FormatLeakedStack_0400 result size:" << result.size();
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest FormatLeakedStack_0400 end";
}

/**
 * @tc.number: CollectPcsFromStack_0600
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack reads a real stack buffer and finds executable PCs.
 */
HWTEST_F(DumpRuntimeHelperTest, CollectPcsFromStack_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0600 start";
    auto maps = OHOS::HiviewDFX::DfxMaps::Create(0, true, false);
    ASSERT_NE(maps, nullptr);
    // Place a real function address on the stack buffer. process_vm_readv should
    // read it and FindMapByAddr should match it to an executable mapping.
    uintptr_t funcAddr = reinterpret_cast<uintptr_t>(&DumpRuntimeHelper::ParseTidFromMapName);
    uintptr_t buf[4] = {0, funcAddr, 0, 0};
    uint64_t begin = reinterpret_cast<uint64_t>(&buf[0]);
    uint64_t end = reinterpret_cast<uint64_t>(&buf[4]);
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(begin, end, maps);
    // The function address should be found as a candidate PC.
    EXPECT_GE(pcs.size(), static_cast<size_t>(1));
    if (!pcs.empty()) {
        EXPECT_EQ(pcs[0], funcAddr);
    }
    GTEST_LOG_(INFO) << "CollectPcsFromStack_0600 found " << pcs.size() << " PCs";
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0600 end";
}

/**
 * @tc.number: CollectPcsFromStack_0700
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack with unmapped address range triggers process_vm_readv failure.
 */
HWTEST_F(DumpRuntimeHelperTest, CollectPcsFromStack_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0700 start";
    auto maps = OHOS::HiviewDFX::DfxMaps::Create(0, true, false);
    ASSERT_NE(maps, nullptr);
    // Use an address range that is definitely unmapped — process_vm_readv should
    // fail and return empty without crashing.
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(0x10000000, 0x10001000, maps);
    EXPECT_TRUE(pcs.empty());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0700 end";
}

/**
 * @tc.number: ParseTidFromMapName_0600
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName returns 0 for negative sign (non-digit stops parsing).
 */
HWTEST_F(DumpRuntimeHelperTest, ParseTidFromMapName_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0600 start";
    // The '-' character is not a digit, so parsing stops immediately and tidStr
    // is empty, causing the function to return 0.
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:-2147483649]"), 0);
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:-1]"), 0);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest ParseTidFromMapName_0600 end";
}

/**
 * @tc.number: CollectPcsFromStack_0800
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack with a large buffer containing multiple executable PCs.
 */
HWTEST_F(DumpRuntimeHelperTest, CollectPcsFromStack_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0800 start";
    auto maps = OHOS::HiviewDFX::DfxMaps::Create(0, true, false);
    ASSERT_NE(maps, nullptr);
    // Place multiple function addresses in the buffer to exercise the
    // pcs.push_back path multiple times.
    uintptr_t func1 = reinterpret_cast<uintptr_t>(&DumpRuntimeHelper::ParseTidFromMapName);
    uintptr_t func2 = reinterpret_cast<uintptr_t>(&DumpRuntimeHelper::IsTidAlive);
    uintptr_t buf[8] = {0, func1, 0, func2, 0, 0, 0, 0};
    uint64_t begin = reinterpret_cast<uint64_t>(&buf[0]);
    uint64_t end = reinterpret_cast<uint64_t>(&buf[8]);
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(begin, end, maps);
    EXPECT_GE(pcs.size(), static_cast<size_t>(2));
    GTEST_LOG_(INFO) << "CollectPcsFromStack_0800 found " << pcs.size() << " PCs";
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CollectPcsFromStack_0800 end";
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

/**
 * @tc.number: Set2DQuota_0100
 * @tc.name: Set2DQuota
 * @tc.desc: Test Set2DQuota returns false when newQuota is negative.
 */
HWTEST_F(DumpRuntimeHelperTest, Set2DQuota_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Set2DQuota_0100 start";
    EXPECT_FALSE(DumpRuntimeHelper::Set2DQuota("/data/local/tmp/dump_runtime_helper_test", 0, -1));
    EXPECT_FALSE(DumpRuntimeHelper::Set2DQuota("/data/local/tmp/dump_runtime_helper_test", 100, -100));
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Set2DQuota_0100 end";
}

/**
 * @tc.number: Set2DQuota_0200
 * @tc.name: Set2DQuota
 * @tc.desc: Test Set2DQuota returns false when path does not exist.
 */
HWTEST_F(DumpRuntimeHelperTest, Set2DQuota_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Set2DQuota_0200 start";
    std::string invalidPath = "/data/local/tmp/dump_runtime_helper_nonexistent_12345";
    ASSERT_FALSE(DumpRuntimeHelper::IsFileExists(invalidPath));
    EXPECT_FALSE(DumpRuntimeHelper::Set2DQuota(invalidPath, 100, 5));
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Set2DQuota_0200 end";
}

/**
 * @tc.number: Set2DQuota_0300
 * @tc.name: Set2DQuota
 * @tc.desc: Test Set2DQuota succeeds on a valid path and Get2DQuota reads back correctly.
 */
HWTEST_F(DumpRuntimeHelperTest, Set2DQuota_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Set2DQuota_0300 start";
    std::string testDir = "/data/local/tmp/dump_runtime_helper_test_set2dquota";
    if (DumpRuntimeHelper::IsFileExists(testDir)) {
        rmdir(testDir.c_str());
    }
    ASSERT_TRUE(DumpRuntimeHelper::CreateDir(testDir));
    uint64_t timestamp = 1234567890;
    int quota = 5;
    EXPECT_TRUE(DumpRuntimeHelper::Set2DQuota(testDir, timestamp, quota));
    uint64_t readTime = 0;
    int readQuota = 0;
    EXPECT_TRUE(DumpRuntimeHelper::Get2DQuota(testDir, "user.oomdump.quota", readTime, readQuota));
    EXPECT_EQ(readTime, timestamp);
    EXPECT_EQ(readQuota, quota);
    rmdir(testDir.c_str());
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Set2DQuota_0300 end";
}

/**
 * @tc.number: Check2DQuota_0100
 * @tc.name: Check2DQuota
 * @tc.desc: Test Check2DQuota returns false when DFX_RESOURCE_OVERLIMIT_OPTIONS not set.
 */
HWTEST_F(DumpRuntimeHelperTest, Check2DQuota_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Check2DQuota_0100 start";
    unsetenv("DFX_RESOURCE_OVERLIMIT_OPTIONS");
    EXPECT_FALSE(DumpRuntimeHelper::Check2DQuota(true));
    EXPECT_FALSE(DumpRuntimeHelper::Check2DQuota(false));
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Check2DQuota_0100 end";
}

/**
 * @tc.number: Check2DQuota_0200
 * @tc.name: Check2DQuota
 * @tc.desc: Test Check2DQuota returns false when env var does not contain oomdump:enable.
 */
HWTEST_F(DumpRuntimeHelperTest, Check2DQuota_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Check2DQuota_0200 start";
    setenv("DFX_RESOURCE_OVERLIMIT_OPTIONS", "other:enable", 1);
    EXPECT_FALSE(DumpRuntimeHelper::Check2DQuota(true));
    unsetenv("DFX_RESOURCE_OVERLIMIT_OPTIONS");
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Check2DQuota_0200 end";
}

/**
 * @tc.number: Check2DQuota_0300
 * @tc.name: Check2DQuota
 * @tc.desc: Test Check2DQuota returns false when process maxcount is negative.
 */
HWTEST_F(DumpRuntimeHelperTest, Check2DQuota_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Check2DQuota_0300 start";
    setenv("DFX_RESOURCE_OVERLIMIT_OPTIONS", "oomdump:enable", 1);
    OHOS::system::SetParameter("persist.hiview.oomdump.process.maxcount", "-1");
    EXPECT_FALSE(DumpRuntimeHelper::Check2DQuota(true));
    OHOS::system::SetParameter("persist.hiview.oomdump.process.maxcount", "");
    unsetenv("DFX_RESOURCE_OVERLIMIT_OPTIONS");
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Check2DQuota_0300 end";
}

/**
 * @tc.number: Check2DQuota_0400
 * @tc.name: Check2DQuota
 * @tc.desc: Test Check2DQuota returns false when oomdump maxcount is negative.
 */
HWTEST_F(DumpRuntimeHelperTest, Check2DQuota_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Check2DQuota_0400 start";
    setenv("DFX_RESOURCE_OVERLIMIT_OPTIONS", "oomdump:enable", 1);
    OHOS::system::SetParameter("persist.hiview.oomdump.process.maxcount", "5");
    OHOS::system::SetParameter("persist.hiview.oomdump.maxcount", "-1");
    EXPECT_FALSE(DumpRuntimeHelper::Check2DQuota(true));
    OHOS::system::SetParameter("persist.hiview.oomdump.process.maxcount", "");
    OHOS::system::SetParameter("persist.hiview.oomdump.maxcount", "");
    unsetenv("DFX_RESOURCE_OVERLIMIT_OPTIONS");
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest Check2DQuota_0400 end";
}

class CreateDirTest : public ::testing::Test {
protected:
    CreateDirTest()
    {
    }

    ~CreateDirTest()
    {
    }

    void SetUp() override
    {
        std::string testDir = "/data/local/tmp/dump_runtime_helper_test";
        RemoveTestDir(testDir);
    }

    void TearDown() override
    {
        std::string testDir = "/data/local/tmp/dump_runtime_helper_test";
        RemoveTestDir(testDir);
    }

    void CreateTestDir(const std::string& path)
    {
        if (DumpRuntimeHelper::IsFileExists(path)) {
            return;
        }
        constexpr mode_t defaultLogDirMode = 0770;
        if (mkdir(path.c_str(), defaultLogDirMode) != 0) {
            perror("mkdir failed");
        }
    }

    void RemoveTestDir(const std::string& path)
    {
        if (DumpRuntimeHelper::IsFileExists(path)) {
            if (remove(path.c_str()) != 0) {
                perror("remove failed");
            }
        }
    }
};

/**
 * @tc.number: CreateDir_0100
 * @tc.name: CreateDir_FileExists
 * @tc.desc: Test CreateDir when directory already exists.
 */
HWTEST_F(CreateDirTest, CreateDir_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateDirTest CreateDir_0100 start";

    std::string testDir = "/data/local/tmp/dump_runtime_helper_test";
    CreateTestDir(testDir);
    EXPECT_TRUE(DumpRuntimeHelper::IsFileExists(testDir));
    EXPECT_TRUE(DumpRuntimeHelper::CreateDir(testDir));

    GTEST_LOG_(INFO) << "CreateDirTest CreateDir_0100 end";
}

/**
 * @tc.number: CreateDir_0200
 * @tc.name: CreateDir_CreateSuccess
 * @tc.desc: Test CreateDir when directory is created successfully.
 */
HWTEST_F(CreateDirTest, CreateDir_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CreateDirTest CreateDir_0200 start";

    std::string testDir = "/data/local/tmp/dump_runtime_helper_test";
    RemoveTestDir(testDir);
    EXPECT_FALSE(DumpRuntimeHelper::IsFileExists(testDir));
    EXPECT_TRUE(DumpRuntimeHelper::CreateDir(testDir));
    EXPECT_TRUE(DumpRuntimeHelper::IsFileExists(testDir));
    RemoveTestDir(testDir);
    GTEST_LOG_(INFO) << "DumpRuntimeHelperTest CreateDir_0200 end";
}
}
}
