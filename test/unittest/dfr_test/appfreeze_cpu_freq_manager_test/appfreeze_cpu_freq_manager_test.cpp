/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <cstdlib>

#define private public
#include "appfreeze_cpu_freq_manager.h"
#undef private

#include "appfreeze_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class AppfreezeCpuFreqManagerTest : public testing::Test {
public:
    AppfreezeCpuFreqManagerTest()
    {}
    ~AppfreezeCpuFreqManagerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppfreezeCpuFreqManagerTest::SetUpTestCase(void)
{}

void AppfreezeCpuFreqManagerTest::TearDownTestCase(void)
{}

void AppfreezeCpuFreqManagerTest::SetUp(void)
{}

void AppfreezeCpuFreqManagerTest::TearDown(void)
{}

/**
 * @tc.number: InsertCpuDetailInfo_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, InsertCpuDetailInfo_001, TestSize.Level1)
{
    uint32_t checkMapSize = 10;
    std::string type = "test0";
    int32_t pid = getpid();
    int32_t uid = static_cast<int>(getuid());
    bool ret = AppfreezeCpuFreqManager::GetInstance().InsertCpuDetailInfo(type, pid);
    EXPECT_TRUE(ret);
    for (auto i = 1; i < checkMapSize; i++) {
        type = "test" + std::to_string(i);
        AppfreezeCpuFreqManager::GetInstance().InsertCpuDetailInfo(type, pid);
    }
    int left = 10; // over 10s
    while (left > 0) {
        left = sleep(left);
    }
    type = "test0";
    ret = AppfreezeCpuFreqManager::GetInstance().InsertCpuDetailInfo(type, pid);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: GetCpuDetailInfo_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetCpuDetailInfo_001, TestSize.Level1)
{
    int32_t pid = getpid();
    CpuDataProcessor data = AppfreezeCpuFreqManager::GetInstance().GetCpuDetailInfo(pid);
    EXPECT_TRUE(pid >= 0);
}

/**
 * @tc.number: GetInfoByCpuCountTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetInfoByCpuCountTest_001, TestSize.Level1)
{
    std::vector<CpuFreqData> parseDatas;
    TotalTime totalTime;
    int32_t num = 100;
    AppfreezeCpuFreqManager::GetInstance().GetInfoByCpuCount(num, parseDatas, totalTime);
    EXPECT_TRUE(parseDatas.size() == 0);
    num = 0;
    AppfreezeCpuFreqManager::GetInstance().GetInfoByCpuCount(num, parseDatas, totalTime);
    EXPECT_TRUE(parseDatas.size() >= 0);
}

/**
 * @tc.number: GetCpuStrTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetCpuStrTest_001, TestSize.Level1)
{
    int code = 0;
    std::vector<FrequencyPair> freqPairs;
    float percentage = 44.1000f;
    std::string str = AppfreezeCpuFreqManager::GetInstance().GetCpuStr(code, freqPairs, percentage);
    EXPECT_TRUE(!str.empty());
    FrequencyPair freqPair1 = {
        .frequency = 100,
        .percentage = 10.000f,
    };
    FrequencyPair freqPair2 = {
        .frequency = 120,
        .percentage = 33.000f,
    };
    FrequencyPair freqPair3 = {
        .frequency = 50,
        .percentage = 1.000f,
    };
    freqPairs.push_back(freqPair1);
    freqPairs.push_back(freqPair2);
    freqPairs.push_back(freqPair3);
    str = AppfreezeCpuFreqManager::GetInstance().GetCpuStr(code, freqPairs, percentage);
    EXPECT_TRUE(!str.empty());
}

/**
 * @tc.number: GetCpuTotalValueTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetCpuTotalValueTest_001, TestSize.Level1)
{
    TotalTime time1 = {
        .totalRunningTime = 50,
        .totalCpuTime = 1000,
    };
    std::vector<TotalTime> totalTimeList;
    totalTimeList.push_back(time1);
    std::vector<TotalTime> blockTotalTimeList;
    TotalTime totalTime;
    bool ret = AppfreezeCpuFreqManager::GetInstance().GetCpuTotalValue(0, totalTimeList, blockTotalTimeList, totalTime);
    EXPECT_TRUE(!ret);
    TotalTime time2 = {
        .totalRunningTime = 100,
        .totalCpuTime = 1000,
    };
    blockTotalTimeList.push_back(time2);
    ret = AppfreezeCpuFreqManager::GetInstance().GetCpuTotalValue(0, totalTimeList, blockTotalTimeList, totalTime);
    EXPECT_TRUE(!ret);
    blockTotalTimeList.clear();
    TotalTime time3 = {
        .totalRunningTime = 10,
        .totalCpuTime = 20,
    };
    blockTotalTimeList.push_back(time3);
    ret = AppfreezeCpuFreqManager::GetInstance().GetCpuTotalValue(0, totalTimeList, blockTotalTimeList, totalTime);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: GetAppCpuTimeTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetAppCpuTimeTest_001, TestSize.Level1)
{
    uint64_t ret = AppfreezeCpuFreqManager::GetInstance().GetAppCpuTime(-1);
    EXPECT_TRUE(ret == 0);
    ret = AppfreezeCpuFreqManager::GetInstance().GetAppCpuTime(getpid());
    EXPECT_TRUE(ret >= 0);
    ret = AppfreezeCpuFreqManager::GetInstance().GetAppCpuTime(12345670);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.number: GetProcessCpuTimeTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetProcessCpuTimeTest_001, TestSize.Level0)
{
    uint64_t ret = AppfreezeCpuFreqManager::GetInstance().GetProcessCpuTime(-1);
    EXPECT_TRUE(ret == 0);
    ret = AppfreezeCpuFreqManager::GetInstance().GetProcessCpuTime(getpid());
    EXPECT_TRUE(ret >= 0);
    ret = AppfreezeCpuFreqManager::GetInstance().GetProcessCpuTime(12345670);
    EXPECT_TRUE(ret == 0);
    ret = AppfreezeCpuFreqManager::GetInstance().GetDeviceRuntime();
    EXPECT_TRUE(ret >= 0);
}

/**
 * @tc.number: GetOptimalCpuTimeTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetOptimalCpuTimeTest_001, TestSize.Level0)
{
    double ret = AppfreezeCpuFreqManager::GetInstance().GetOptimalCpuTime(getpid());
    EXPECT_TRUE(ret >= 0);
    int count = AppfreezeCpuFreqManager::GetInstance().cpuCount_;
    AppfreezeCpuFreqManager::GetInstance().cpuCount_ = 0;
    ret = AppfreezeCpuFreqManager::GetInstance().GetOptimalCpuTime(getpid());
    EXPECT_TRUE(ret >= 0);
    AppfreezeCpuFreqManager::GetInstance().cpuCount_ = count;
}

/**
 * @tc.number: GetThreadCpuLoadFuncTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetThreadCpuLoadFuncTest_001, TestSize.Level0)
{
    GetThreadCpuLoad ret1 = AppfreezeCpuFreqManager::GetInstance().GetThreadCpuLoadFunc();
    GetThreadCpuLoad ret2 = AppfreezeCpuFreqManager::GetInstance().GetThreadCpuLoadFunc();
    EXPECT_TRUE(ret1 == ret2);
}

/**
 * @tc.number: GetDimpsTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetDimpsTest_001, TestSize.Level0)
{
    int count = AppfreezeCpuFreqManager::GetInstance().cpuCount_;
    AppfreezeCpuFreqManager::GetInstance().cpuCount_ = 0;
    double ret1 = AppfreezeCpuFreqManager::GetInstance().GetDimps();
    EXPECT_TRUE(ret1 >= 0);
    AppfreezeCpuFreqManager::GetInstance().cpuCount_ = count;
    ret1 = AppfreezeCpuFreqManager::GetInstance().GetDimps();
    EXPECT_TRUE(ret1 >= 0);
    double ret2 = AppfreezeCpuFreqManager::GetInstance().GetDimps();
    EXPECT_DOUBLE_EQ(ret1, ret2);
}

/**
 * @tc.number: GetCpuInfoTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetCpuInfoTest_001, TestSize.Level0)
{
    uint64_t start = AppfreezeUtil::GetMilliseconds();
    std::string ret = AppfreezeCpuFreqManager::GetInstance().GetTimeStampStr(start);
    EXPECT_TRUE(!ret.empty());
    CpuConsumeTime cpuConsumeTime1;
    CpuConsumeTime cpuConsumeTime2;
    ret = AppfreezeCpuFreqManager::GetInstance().GetConsumeTimeInfo(getpid(), cpuConsumeTime1,
        cpuConsumeTime2);
    EXPECT_TRUE(!ret.empty());
    ret = AppfreezeCpuFreqManager::GetInstance().GetStaticInfoHead();
    EXPECT_TRUE(!ret.empty());
}

/**
 * @tc.number: GetCpuInfoContentTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetCpuInfoContentTest_001, TestSize.Level0)
{
    std::vector<std::vector<CpuFreqData>> datas1;
    std::vector<TotalTime> totalTimeList1;
    std::vector<std::vector<CpuFreqData>> datas2;
    std::vector<TotalTime> totalTimeList2;
    std::string ret = AppfreezeCpuFreqManager::GetInstance().GetCpuInfoContent(totalTimeList1, datas1,
        totalTimeList2, datas2);
    EXPECT_TRUE(ret.empty());
    std::vector<CpuFreqData> parseDatas;
    CpuFreqData cpuFreqData1 = {
        .frequency = 100,
        .runningTime = 100,
    };
    parseDatas.push_back(cpuFreqData1);
    datas1.push_back(parseDatas);
    TotalTime time1 = {
        .totalRunningTime = 50,
        .totalCpuTime = 1000,
    };
    totalTimeList1.push_back(time1);
    ret = AppfreezeCpuFreqManager::GetInstance().GetCpuInfoContent(totalTimeList1, datas1,
        totalTimeList2, datas2);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.number: GetFreezeLogHeadTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetFreezeLogHeadTest_001, TestSize.Level0)
{
    std::string ret = AppfreezeCpuFreqManager::GetInstance().GetFreezeLogHead("bundleName");
    EXPECT_TRUE(!ret.empty());
}

/**
 * @tc.number: GetIntervalTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetIntervalTest_001, TestSize.Level0)
{
    uint64_t warnTime = 1234; // test value
    uint64_t blockTime = 2345; // test value
    uint64_t ret = AppfreezeCpuFreqManager::GetInstance().GetInterval(warnTime, blockTime);
    EXPECT_EQ(ret, 1111);
}

/**
 * @tc.number: GetIntervalTest_002
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetIntervalTest_002, TestSize.Level0)
{
    uint64_t warnTime = 1200; // test value
    uint64_t blockTime = 1000; // test value
    uint64_t ret = AppfreezeCpuFreqManager::GetInstance().GetInterval(warnTime, blockTime);
    EXPECT_EQ(ret, 200);
}

/**
 * @tc.number: GetConsumeTimeInfoTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetConsumeTimeInfoTest_001, TestSize.Level0)
{
    int32_t pid = getpid();
    CpuConsumeTime warnTimes = {
        .optimalCpuTime = AppfreezeCpuFreqManager::GetInstance().GetOptimalCpuTime(pid),
        .cpuFaultTime = AppfreezeUtil::GetMilliseconds(),
        .processCpuTime = AppfreezeCpuFreqManager::GetInstance().GetProcessCpuTime(pid),
        .deviceRunTime = AppfreezeCpuFreqManager::GetInstance().GetDeviceRuntime(),
        .cpuTime = AppfreezeCpuFreqManager::GetInstance().GetAppCpuTime(pid),
    };
    uint64_t testValue = 1234; // testValue
    CpuConsumeTime blockTimes = {
        .optimalCpuTime = AppfreezeCpuFreqManager::GetInstance().GetOptimalCpuTime(pid) + testValue,
        .cpuFaultTime = AppfreezeUtil::GetMilliseconds() + testValue,
        .processCpuTime = AppfreezeCpuFreqManager::GetInstance().GetProcessCpuTime(pid) + testValue,
        .deviceRunTime = AppfreezeCpuFreqManager::GetInstance().GetDeviceRuntime() + testValue,
        .cpuTime = AppfreezeCpuFreqManager::GetInstance().GetAppCpuTime(pid) + testValue,
    };
    std::string ret = AppfreezeCpuFreqManager::GetInstance().GetConsumeTimeInfo(pid,
        warnTimes, blockTimes);
    EXPECT_TRUE(!ret.empty());
}

/**
 * @tc.number: GetCpuInfoPathTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, GetCpuInfoPathTest_001, TestSize.Level0)
{
    int32_t pid = getpid();
    int32_t uid = static_cast<int>(getuid());
    std::string type = "GetCpuInfoPathTest_001";
    std::string testValue = "AppfreezeCpuFreqManagerTest";
    bool result = AppfreezeCpuFreqManager::GetInstance().InsertCpuDetailInfo(type, pid);
    EXPECT_TRUE(result);
    int32_t newPid = pid + 10;
    std::string ret = AppfreezeCpuFreqManager::GetInstance().GetCpuInfoPath(type,
        testValue, uid, newPid);
    EXPECT_TRUE(ret.empty());
    int left = 10; // test value
    while (left > 0) {
        left = sleep(left);
    }
    ret = AppfreezeCpuFreqManager::GetInstance().GetCpuInfoPath(type,
        testValue, uid, pid);
    EXPECT_TRUE(!ret.empty());
}

/**
 * @tc.number: FreezePathToRealPathTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, FreezePathToRealPathTest_001, TestSize.Level0)
{
    std::string logFile = AppfreezeUtil::FreezePathToRealPath("../FreezePathToRealPathTest_001");
    EXPECT_EQ(logFile, "");
    logFile = AppfreezeUtil::FreezePathToRealPath("/data/log/faultlog");
    EXPECT_TRUE(!logFile.empty());
}

/**
 * @tc.number: ClockTicksToMsTest_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeCpuFreqManagerTest, ClockTicksToMsTest_001, TestSize.Level0)
{
    uint64_t cpuTime = 0;
    auto ret = AppfreezeCpuFreqManager::GetInstance().ClockTicksToMs(cpuTime);
    EXPECT_EQ(ret, 0);
    cpuTime = 100;
    ret = AppfreezeCpuFreqManager::GetInstance().ClockTicksToMs(cpuTime);
    EXPECT_TRUE(ret >= 0);
    cpuTime = 10000;
    ret = AppfreezeCpuFreqManager::GetInstance().ClockTicksToMs(cpuTime);
    EXPECT_TRUE(ret >= 0);
    cpuTime = 1000000;
    ret = AppfreezeCpuFreqManager::GetInstance().ClockTicksToMs(cpuTime);
    EXPECT_TRUE(ret >= 0);
    cpuTime = UINT64_MAX;
    ret = AppfreezeCpuFreqManager::GetInstance().ClockTicksToMs(cpuTime);
    EXPECT_TRUE(ret >= 0);
}
}  // namespace AppExecFwk
}  // namespace OHOS
