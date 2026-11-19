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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_TEST_PID_MAX_HELPER_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_TEST_PID_MAX_HELPER_H

#include <cstdint>
#include <fstream>
#include <string>

namespace OHOS {
namespace CliTool {

// Shared test helper: reads /proc/sys/kernel/pid_max at runtime (digit-parsed, no scanf
// family). Linux allocates pids via idr_alloc_cyclic strictly BELOW pid_max and wraps back
// to the low free slots once past it, so the pid_max value itself can never be a live task
// or a process-group leader: (a) Kill/Killpg on it deterministically returns ESRCH and can
// never hit a foreign live process — kill-class tests use it as the safe "dead pid" (a
// freshly reaped pid is NOT safe: cyclic wraparound reuse of just-reaped pids was really
// observed on device under process churn, which would SIGKILL a foreign live process
// mid-test) — and (b) its /proc entry is deterministically absent (the stat open fails with
// ENOENT), which exercises the gone/dead liveness branches without that reuse caveat.
// Returns -1 only when the sysctl is unreadable; callers own the red/skip policy: a caller
// whose ONLY assertion is this probe must ASSERT red on -1 (kill-test safety and the
// ENOENT coverage must not be environment-guessed into a vacuous pass), while a caller
// with another assertion floor may skip the probe with a WARNING so the skip stays
// visible in default log filtering.
inline int32_t ReadKernelPidMax()
{
    std::ifstream fin("/proc/sys/kernel/pid_max");
    std::string line;
    if (!fin.is_open() || !std::getline(fin, line)) {
        return -1;
    }
    int32_t value = 0;
    for (char c : line) {
        if (c < '0' || c > '9') {
            break;
        }
        value = value * 10 + (c - '0');
    }
    return value > 0 ? value : -1;
}
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_TEST_PID_MAX_HELPER_H
