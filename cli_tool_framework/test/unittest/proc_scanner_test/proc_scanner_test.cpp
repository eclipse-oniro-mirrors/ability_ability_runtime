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
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <thread>

#include "proc_scanner.h"
#include "test_pid_max_helper.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
// Respawn-safe sleep child: the post-fork child must only use async-signal-safe calls, so
// sleep is nanosleep, not sleep()/std::this_thread (the parent may be multi-threaded).
void NapChild(int seconds)
{
    struct timespec ts = { seconds, 0 };
    nanosleep(&ts, nullptr);
    _exit(0);
}
} // namespace

class ProcScannerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ProcScanner_ParseProcStat_0100
 * @tc.desc: A well-formed stat line parses into pid/state/ppid.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, ParseProcStat_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0100 start";

    ProcStatInfo info;
    ASSERT_TRUE(ProcScanner::ParseProcStat("1234 (claw_sandbox) S 100 1234 100 0 1 0", info));
    EXPECT_EQ(info.pid, 1234);
    EXPECT_EQ(info.state, 'S');
    EXPECT_EQ(info.ppid, 100);

    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0100 end";
}

/**
 * @tc.name: ProcScanner_ParseProcStat_0200
 * @tc.desc: comm containing spaces and ')' characters: fields must be parsed after the
 *           LAST ')' (claw_sandbox commands may carry arbitrary argv[0] text).
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, ParseProcStat_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0200 start";

    ProcStatInfo info;
    ASSERT_TRUE(ProcScanner::ParseProcStat("123 (weird ) name (x)) R 45 123 0 0 1 0", info));
    EXPECT_EQ(info.pid, 123);
    EXPECT_EQ(info.state, 'R');
    EXPECT_EQ(info.ppid, 45);

    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0200 end";
}

/**
 * @tc.name: ProcScanner_ParseProcStat_0300
 * @tc.desc: Malformed input (no comm parens / missing fields) is rejected, not half-parsed.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, ParseProcStat_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0300 start";

    ProcStatInfo info;
    EXPECT_FALSE(ProcScanner::ParseProcStat("no parens at all", info));
    EXPECT_FALSE(ProcScanner::ParseProcStat("123 (comm)", info));
    EXPECT_FALSE(ProcScanner::ParseProcStat("", info));

    GTEST_LOG_(INFO) << "ProcScanner_ParseProcStat_0300 end";
}

/**
 * @tc.name: ProcScanner_IsProcessAlive_0100
 * @tc.desc: Self is alive; a live child is alive; a killed child reports not-alive within the
 *           zombie window (before the reap); a pid the running kernel can never allocate
 *           (the pid_max value) reports not-alive.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, IsProcessAlive_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_IsProcessAlive_0100 start";

    EXPECT_TRUE(ProcScanner::IsProcessAlive(getpid()));

    pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        NapChild(5);
    }
    EXPECT_TRUE(ProcScanner::IsProcessAlive(child));
    ASSERT_EQ(kill(child, SIGKILL), 0);
    // Poll the zombie window while WE still hold the unreaped zombie: until waitpid runs,
    // no other process can ever hold this pid, so the not-alive result here is
    // deterministic. kill(2) returns before the kernel transitions the target, hence poll
    // bounded instead of asserting immediately.
    bool alive = true;
    for (int waited = 0; waited < 1000 && alive; waited += 10) {
        alive = ProcScanner::IsProcessAlive(child);
        if (!alive) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_FALSE(alive);
    // Reap for hygiene only. No post-reap probe: after the reap the pid is unallocated and
    // concurrent device churn (agent workloads forking CLI tool processes at high rates) can
    // wrap pid allocation onto the just-reaped value, making an IsProcessAlive(child) call
    // environment-dependent. The same dead-pid coverage is obtained deterministically from
    // the never-allocatable pid_max value below.
    ASSERT_GT(waitpid(child, nullptr, 0), 0);
    int32_t pidMax = ReadKernelPidMax();
    if (pidMax > 0) {
        EXPECT_FALSE(ProcScanner::IsProcessAlive(pidMax));
    } else {
        // Skip keeps a real assertion floor: the zombie-window EXPECT_FALSE above still
        // fails the case if liveness detection regresses. WARNING (not INFO) so the skip is
        // visible in default log filtering.
        GTEST_LOG_(WARNING) << "ProcScanner_IsProcessAlive_0100 skip pid_max probe (sysctl unreadable)";
    }

    GTEST_LOG_(INFO) << "ProcScanner_IsProcessAlive_0100 end";
}

/**
 * @tc.name: ProcScanner_FindCommandProcess_0100
 * @tc.desc: The single live child of the anchor is resolved as the command process
 *           (claw_sandbox forks exactly one command child).
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, FindCommandProcess_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0100 start";

    pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        NapChild(5);
    }
    EXPECT_EQ(ProcScanner::FindCommandProcess(getpid()), child);

    ASSERT_EQ(kill(child, SIGKILL), 0);
    ASSERT_GT(waitpid(child, nullptr, 0), 0);

    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0100 end";
}

/**
 * @tc.name: ProcScanner_FindCommandProcess_0200
 * @tc.desc: A zombie child is never resolved as the command (the real command may still be
 *           forked later); -1 keeps the caller polling.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, FindCommandProcess_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0200 start";

    pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        _exit(0);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // The zombie child is filtered out of command resolution.
    EXPECT_EQ(ProcScanner::FindCommandProcess(getpid()), -1);
    ASSERT_GT(waitpid(child, nullptr, 0), 0);

    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0200 end";
}

/**
 * @tc.name: ProcScanner_FindCommandProcess_0300
 * @tc.desc: No children at all (claw_sandbox still initializing before its pid-ns fork):
 *           -1, never the anchor itself; an invalid parent pid returns -1 too.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, FindCommandProcess_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0300 start";

    EXPECT_EQ(ProcScanner::FindCommandProcess(getpid()), -1);
    EXPECT_EQ(ProcScanner::FindCommandProcess(-1), -1);

    GTEST_LOG_(INFO) << "ProcScanner_FindCommandProcess_0300 end";
}

/**
 * @tc.name: ProcScanner_CheckLiveness_0100
 * @tc.desc: Tri-state liveness: own pid ALIVE; invalid pid and the never-allocatable pid_max
 *           value (deterministic ENOENT/gone coverage, no pid-reuse caveat) DEAD; a zombie
 *           (exited, not yet reaped) is DEAD too.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, CheckLiveness_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_CheckLiveness_0100 start";

    EXPECT_EQ(ProcScanner::CheckLiveness(getpid()), ProcLiveness::ALIVE);
    EXPECT_EQ(ProcScanner::CheckLiveness(-1), ProcLiveness::DEAD);

    pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        NapChild(5);
    }
    EXPECT_EQ(ProcScanner::CheckLiveness(child), ProcLiveness::ALIVE);
    ASSERT_EQ(kill(child, SIGKILL), 0);
    // The zombie window opens once SIGKILL is delivered; kill(2) returns before the kernel
    // transitions the target, so poll bounded instead of asserting immediately.
    ProcLiveness liveness = ProcLiveness::ALIVE;
    for (int waited = 0; waited < 1000; waited += 10) {
        liveness = ProcScanner::CheckLiveness(child);
        if (liveness == ProcLiveness::DEAD) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(liveness, ProcLiveness::DEAD);
    ASSERT_GT(waitpid(child, nullptr, 0), 0);
    // No post-reap probe on the reaped pid: after waitpid the /proc entry is gone and
    // CheckLiveness would report DEAD, but concurrent device churn can re-allocate the pid
    // in that window, making the assertion environment-dependent. The same ENOENT-DEAD
    // branch is covered deterministically by the never-allocatable pid_max value.
    int32_t pidMax = ReadKernelPidMax();
    if (pidMax > 0) {
        EXPECT_EQ(ProcScanner::CheckLiveness(pidMax), ProcLiveness::DEAD);
    } else {
        // Skip keeps a real assertion floor: the zombie-window EXPECT_EQ above still fails
        // the case if liveness detection regresses. WARNING (not INFO) so the skip is
        // visible in default log filtering.
        GTEST_LOG_(WARNING) << "ProcScanner_CheckLiveness_0100 skip pid_max probe (sysctl unreadable)";
    }

    GTEST_LOG_(INFO) << "ProcScanner_CheckLiveness_0100 end";
}

/**
 * @tc.name: ProcScanner_CheckLiveness_0200
 * @tc.desc: A pid the running kernel can never allocate (the pid_max value) reports DEAD —
 *           deterministic ENOENT coverage without the pid-reuse caveat that broke the
 *           retired-pid premise on device.
 * @tc.type: FUNC
 */
HWTEST_F(ProcScannerTest, CheckLiveness_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcScanner_CheckLiveness_0200 start";

    // The original premise — a freshly reaped pid stays unallocated long enough to probe — is
    // exactly what broke on device: concurrent process churn wrapped pid allocation onto the
    // reaped value between waitpid and the /proc stat open, so the probe reported a foreign
    // live process. Linux allocates pids strictly below pid_max, so the pid_max value itself
    // can never be a live task and always exercises the tri-state ENOENT-DEAD branch here.
    // Unlike the two probe sites above, this pid_max check is the case's ONLY assertion:
    // an unreadable sysctl must fail the case red instead of skipping to a vacuous,
    // zero-assertion pass (false green — the exact anti-pattern this suite must not ship).
    int32_t pidMax = ReadKernelPidMax();
    ASSERT_GT(pidMax, 0) << "/proc/sys/kernel/pid_max unreadable: the never-allocatable "
        "ENOENT-DEAD branch cannot be verified in this environment";
    EXPECT_EQ(ProcScanner::CheckLiveness(pidMax), ProcLiveness::DEAD);

    GTEST_LOG_(INFO) << "ProcScanner_CheckLiveness_0200 end";
}
} // namespace CliTool
} // namespace OHOS
