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

#include "appfreeze_event_report.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class AppfreezeEventReportTest : public testing::Test {
public:
    AppfreezeEventReportTest()
    {}
    ~AppfreezeEventReportTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppfreezeEventReportTest::SetUpTestCase(void)
{}

void AppfreezeEventReportTest::TearDownTestCase(void)
{}

void AppfreezeEventReportTest::SetUp(void)
{}

void AppfreezeEventReportTest::TearDown(void)
{}

AppfreezeEventInfo GetEventInfo()
{
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    return eventInfo;
}

/**
 * @tc.number: SendAppfreezeEvent_Test_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_001, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_3S";
    std::string testName = "SendAppfreezeEvent_Test_001";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_001: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_001: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_001: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_002
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_002, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_6S";
    std::string testName = "SendAppfreezeEvent_Test_002";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_002: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_002: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_002: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    eventInfo.applicationHeapInfo = "size:1,";
    eventInfo.processLifeTime = "2s";
    eventInfo.mainStack = "test";
    eventInfo.enableFreeze = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_003
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_003, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_HALF_TIMEOUT";
    std::string testName = "SendAppfreezeEvent_Test_003";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_003: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_003: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_003: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_004
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_004, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_HALF_TIMEOUT_WARNING";
    std::string testName = "SendAppfreezeEvent_Test_004";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_004: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_004: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_004: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_005
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_005, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_TIMEOUT";
    std::string testName = "SendAppfreezeEvent_Test_005";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_005: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_005: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_005: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    eventInfo.enableFreeze = false;
    eventInfo.applicationHeapInfo = "size1:1234,size2:1234";
    eventInfo.processLifeTime = "21s";
    eventInfo.reportLifecycleToFreeze = true;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_006
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_006, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_TIMEOUT_WARNING";
    std::string testName = "SendAppfreezeEvent_Test_006";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_006: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_006: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_006: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = true;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_007
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_007, TestSize.Level1)
{
    std::string eventName = "APP_LIFECYCLE_TIMEOUT";
    std::string testName = "SendAppfreezeEvent_Test_007";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_007: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_007: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_007: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = true;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_008
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_008, TestSize.Level1)
{
    std::string eventName = "APP_INPUT_BLOCK";
    std::string testName = "SendAppfreezeEvent_Test_008";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_008: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_008: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_008: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = true;
    eventInfo.applicationHeapInfo = "size1:1234,size2:1234";
    eventInfo.processLifeTime = "21s";
    eventInfo.dispatchedEventId = 12;
    eventInfo.processedId = 11;
    eventInfo.markedId = 10;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_009
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_009, TestSize.Level1)
{
    std::string eventName = "BUSSINESS_THREAD_BLOCK_3S";
    std::string testName = "SendAppfreezeEvent_Test_009";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_009: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_009: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_009: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_010
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_010, TestSize.Level1)
{
    std::string eventName = "BUSSINESS_THREAD_BLOCK_6S";
    std::string testName = "SendAppfreezeEvent_Test_010";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_010: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_010: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_010: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_011
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_011, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_3S";
    std::string testName = "SendAppfreezeEvent_Test_011";
    auto eventInfo = GetEventInfo();
    int testValue = 21; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinderCatcher -- pid==1234\nBinderCatcher --"
        "1234:0 to 901:4079 code 16 wait:0.25653125 s frz_state:3, "
        "ns:-1:-1 to -1:-1, debug:1234:0 to 901:4079, active_code:0, active_thread=0, pending_async_proc=0\n"
        "3712:0 to 13967:0 code d2 wait:0.703385417 s frz_state:1234,  "
        "ns:-1:-1 to -1:-1, debug:3712:0 to 13967:0, active_code:0, active_thread=0, pending_async_proc=0\n"
        "1733:2285 to 3712:0 code b wait:1.365925521 s frz_state:3,  "
        "ns:-1:-1 to -1:-1, debug:1733:2285 to 3712:0, active_code:0, active_thread=0, pending_async_proc=0\n";
    eventInfo.freezeMemory = "freeze memory ";
    eventInfo.errorStack = "#00 pc 00000000000015b8 [shmm](__kernel_gettimeofday+72)\\n"
        "#01 pc 00000000001d7e44 /system/lib64/ld-musl-aarck64.so.1(clock_gettime+48)"
        "(f8a0616c89b184992d0e8883cc78f638)\\n"
        "#03 pc 00000000000a0500 /system/lib64/platformsdk/libruntime.z.so"
        "(c2f75213ee12fdf08da323fe546923ff)\\n"
        "......\\n";
    eventInfo.errorName = "THREAD_BLOCK_3S";
    eventInfo.errorMessage = "MSG ="
        "Fault time:2025/06/28-14:08:34"
        "App main thread is not response!"
        "Main handler dump start time: 2025-06-28 14:08:34.067"
        "mainHandler dump is:"
        "EventHandler dump begin curTime: 2025-06-28 14:08:34.067";
    eventInfo.freezeMemory = "Get freeze memory start time: 2025-06-28 14:08:37.112\\n"
        "some avg10=56.81 avg60=56.81 avg300=56.81 total=56";
    eventInfo.freezeInfoFile = "/data/log/testFile/stackFile.txt,/data/log/testFile/cpuFile.txt";
    eventInfo.hitraceInfo = "HitraceIdInfo: hitrace_id: a92ab27238f409a, span_id: "
        "1cd61c9, parent_span_id: 3072e, trace_flag: 0";
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_012
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_012, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_6S";
    std::string testName = "SendAppfreezeEvent_Test_012";
    auto eventInfo = GetEventInfo();
    int testValue = 12; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "testValue";
    eventInfo.freezeMemory = "freeze memory ";
    eventInfo.errorStack = "errorStack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "MSG =";
    eventInfo.freezeInfoFile = "testFile";
    eventInfo.hitraceInfo = testName;
    eventInfo.foregroundState = true;
    eventInfo.processLifeTime = 16;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_013
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_013, TestSize.Level1)
{
    std::string eventName = "APP_INPUT_BLOCK";
    std::string testName = "SendAppfreezeEvent_Test_013";
    auto eventInfo = GetEventInfo();
    int testValue = 13; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = testName;
    eventInfo.freezeMemory = testName;
    eventInfo.appRunningUniqueId = testName;
    eventInfo.errorStack = testName;
    eventInfo.errorName = testName;
    eventInfo.errorMessage = testName;
    eventInfo.freezeInfoFile = testName;
    eventInfo.hitraceInfo = testName;
    eventInfo.foregroundState = true;
    eventInfo.applicationHeapInfo = testName;
    eventInfo.processLifeTime = testName;
    eventInfo.dispatchedEventId = 23;
    eventInfo.processedId = 24;
    eventInfo.markedId = 25;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_011
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_014, TestSize.Level1)
{
    std::string eventName = "BUSINESS_INPUT_BLOCK";
    std::string testName = "SendAppfreezeEvent_Test_014";
    AppfreezeEventInfo eventInfo;
    eventInfo.eventId = 14; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = testName;
    eventInfo.freezeMemory = testName;
    eventInfo.appRunningUniqueId = testName;
    eventInfo.errorStack = testName;
    eventInfo.errorName = testName;
    eventInfo.errorMessage = testName;
    eventInfo.freezeInfoFile = testName;
    eventInfo.hitraceInfo = testName;
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        HISYSEVENT_FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}
}  // namespace AppExecFwk
}  // namespace OHOS
