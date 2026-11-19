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

#ifndef OHOS_ABILITY_RUNTIME_PROC_SCANNER_H
#define OHOS_ABILITY_RUNTIME_PROC_SCANNER_H

#include <string>
#include <sys/types.h>

namespace OHOS {
namespace CliTool {

// The /proc/<pid>/stat fields the transitional command-process discovery needs.
struct ProcStatInfo {
    pid_t pid = -1;
    char state = '\0';
    pid_t ppid = -1;
};

// Tri-state anchor liveness. DEAD is definitive (pid gone or zombie): callers may skip work
// that would only ever target a dead process. UNKNOWN means /proc could not be read
// (e.g. EACCES before the SELinux policy reload): callers must fall back, not conclude death.
enum class ProcLiveness {
    ALIVE,
    DEAD,
    UNKNOWN,
};

// Transitional discovery of the sandboxed command process.
//
// Background: climgr forks and execs claw_sandbox (the session anchor pid), but claw_sandbox
// then forks the real command process inside a new pid namespace (ForkAfterUnshare, triggered
// by the nsFlags=["pid"] sandbox config), keeping itself alive as the monitor. The command
// pid is therefore only observable by scanning /proc from the initial pid namespace.
//
// Why /proc/<pid>/stat and not /proc/<pid>/task/<tid>/children: the children file is compiled
// out on standard OHOS kernels (CONFIG_PROC_CHILDREN is not set on rk3568/rk3588/qemu) and it
// is per-thread (it would miss a fork done by a non-main sandbox thread). The stat ppid field
// is always present and holds the parent's TGID, so it is thread-agnostic.
//
// SELinux prerequisite: TWO kinds of labels are involved. The /proc root directory carries
// the genfscon proc_file label, but every /proc/<pid> entry is labeled with the TASK's own
// domain (security_task_to_inode): the pid directory is <task_domain>:dir and stat is
// <task_domain>:file. aimgr therefore needs task-domain rules for the anchor (claw_sandbox_loader)
// and for the command process after its exec transition (claw_cli for CLI tools, ai_shell for
// the sh form) — see aimgr.te; a missing rule shows up as EACCES on the stat open.
// Kernel prerequisite: the system /proc is mounted hidepid=2,gid=3009 (init device.c), so the
// caller must be in the readproc (3009) group (aimgr.cfg gid list) or hold CAP_SYS_PTRACE to
// see processes of other uids — the sandbox runs as root (the loader) and drops the command to
// the tool uid, both invisible to aimgr otherwise, and a hidden pid's stat open returns
// ENOENT (indistinguishable from a dead pid). The terminal fd-handshake plan removes this
// dependency; the transitional scan lives with it.
class ProcScanner {
public:
    // Parses one /proc/<pid>/stat line. The comm field may contain spaces and ')' characters,
    // so fields are parsed after the LAST ')' of the line. Returns false on malformed input.
    static bool ParseProcStat(const std::string &content, ProcStatInfo &out);

    // Whether pid currently exists and is not a zombie. A zombie anchor means the session is
    // already tearing down and discovery is pointless. UNKNOWN liveness reports false here
    // (conservative "not alive"); use CheckLiveness when the distinction matters.
    static bool IsProcessAlive(pid_t pid);

    // Tri-state liveness: DEAD only when definitive (pid gone: ENOENT/ENOTDIR, or zombie state).
    // UNKNOWN when /proc is unreadable — the caller must keep its fallback behavior.
    static ProcLiveness CheckLiveness(pid_t pid);

    // The sandboxed command process. claw_sandbox forks exactly one command child
    // (ForkAfterUnshare is the only fork in its source), so the first live child of parentPid
    // IS the command process — return it. Zombies are skipped (a dead child is a racing
    // exit, not something to attribute or kill); -1 while no live child is visible yet.
    static pid_t FindCommandProcess(pid_t parentPid);

private:
    ProcScanner() = default;
    ~ProcScanner() = default;

    // Reads and parses /proc/<pid>/stat. False when the pid is gone (race with exit);
    // openErrno then carries the failing errno (0 when the read or parse failed instead).
    static bool ReadStatFile(pid_t pid, ProcStatInfo &out, int32_t &openErrno);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PROC_SCANNER_H
