// Copyright (c) 2025 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <signal.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <climits>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace {
constexpr int PROGRESS_MAX = 100;
constexpr int DEFAULT_INTERVAL = 1;
constexpr int MIN_DURATION = 1;
constexpr int MIN_INTERVAL = 1;
// Upper bound (in seconds) for --duration/--interval: one day. Keeps every millisecond
// conversion inside a bounded range (e.g. interval * MS_PER_SECOND fits comfortably in
// both int64 and, more importantly, never overflows on any intermediate computation),
// and a run longer than a day has no practical use for a sample timer tool.
constexpr int MAX_DURATION_SECONDS = 86400;
constexpr int MAX_INTERVAL_SECONDS = 86400;
constexpr int MIN_ARGC = 2;
constexpr int MS_PER_SECOND = 1000;
// Sleep slices are kept short so the run loop reacts to a cancel signal within
// SLEEP_SLICE_MS milliseconds instead of waiting for the whole interval.
constexpr int SLEEP_SLICE_MS = 100;
// Recommended exit code for a signal-terminated process is 128 + signo
// (130 for SIGINT, 143 for SIGTERM), matching shell conventions.
constexpr int EXIT_CODE_SIGNAL_BASE = 128;
constexpr const char* TOOL_VERSION = "1.0.0";
// Pre-allocated constant hints written by the signal handler with write(), which is
// async-signal-safe. The signal number is baked into the constants (one per supported
// signal) so the handler needs no formatting at all — snprintf and the *_s string
// functions are not signal-safe, and raw buffer copies are banned by the secure-coding
// rules, so constants are the only compliant form here.
constexpr char CANCEL_HINT_SIGINT[] = "ohos-timer: signal 2 (SIGINT) received, exiting after cleanup\n";
constexpr char CANCEL_HINT_SIGTERM[] = "ohos-timer: signal 15 (SIGTERM) received, exiting after cleanup\n";

// Cancel flag set inside the signal handler. volatile sig_atomic_t is the only
// type the C/C++ standards guarantee can be written safely from a signal
// handler and read from the main flow without further synchronization.
// 0 means "not cancelled"; a non-zero value is the received signal number.
volatile sig_atomic_t gCancelSignal = 0;

// SIGINT/SIGTERM handler. Only async-signal-safe operations are allowed here:
// - write() with a constant buffer (async-signal-safe output API);
// - assigning the signal number to the volatile sig_atomic_t flag.
// Never call printf/std::cout/HiLog/malloc or any other non-reentrant function
// from the handler. The real cleanup (progress/result output and exit) happens
// in the normal run loop once it observes the flag.
void HandleCancelSignal(int signo)
{
    gCancelSignal = signo;
    // Best-effort hint on stderr (with the received signal name and number for
    // diagnosis). The return value is deliberately ignored: the cleanup path in
    // the run loop does not depend on this write.
    if (signo == SIGINT) {
        (void)write(STDERR_FILENO, CANCEL_HINT_SIGINT, sizeof(CANCEL_HINT_SIGINT) - 1);
    } else if (signo == SIGTERM) {
        (void)write(STDERR_FILENO, CANCEL_HINT_SIGTERM, sizeof(CANCEL_HINT_SIGTERM) - 1);
    }
}
}

struct TimerConfig {
    int duration = 0;
    int interval = DEFAULT_INTERVAL;
    std::string errMsg;
    bool showProgress = false;
    bool verbose = false;
};

std::string EscapeJson(const std::string& input)
{
    std::string escaped;
    escaped.reserve(input.size());
    for (char ch : input) {
        switch (ch) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += ch;
                break;
        }
    }
    return escaped;
}

void EmitProgress(int percentage, const std::string& status)
{
    std::cout << "{\"type\":\"progress\",\"percentage\":" << percentage
              << ",\"status\":\"" << EscapeJson(status) << "\"}" << std::endl;
}

void EmitSuccessResult(const std::string& dataJson)
{
    std::cout << "{\"type\":\"result\",\"status\":\"success\",\"data\":"
              << dataJson << "}" << std::endl;
}

void EmitError(const std::string& errCode, const std::string& errMsg, const std::string& suggestion)
{
    std::cout << "{\"type\":\"result\",\"status\":\"failed\",\"errCode\":\""
              << EscapeJson(errCode) << "\",\"errMsg\":\"" << EscapeJson(errMsg)
              << "\",\"suggestion\":\"" << EscapeJson(suggestion) << "\"}" << std::endl;
}

void ShowHelp()
{
    std::cout << "Usage: ohos-timer <subcommand> [options]" << std::endl;
    std::cout << "Subcommands:" << std::endl;
    std::cout << "  run [options]         Run the timer (supports cancel via SIGINT/SIGTERM)" << std::endl;
    std::cout << "  version               Show version information" << std::endl;
    std::cout << "  help                  Show this help message" << std::endl;
    std::cout << "Run options:" << std::endl;
    std::cout << "  --duration <sec>      Duration in seconds (required, minimum 1)" << std::endl;
    std::cout << "  --interval <sec>      Progress update interval in seconds (default 1)" << std::endl;
    std::cout << "  --errMsg <message>    Error message written to stderr when duration elapses" << std::endl;
    std::cout << "  --showProgress        Enable progress events" << std::endl;
    std::cout << "  --verbose             Enable verbose mode" << std::endl;
    std::cout << "  --help, -h            Show run help message" << std::endl;
}

void ShowVersionHelp()
{
    std::cout << "Usage: ohos-timer version [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h  Show this help message" << std::endl;
}

bool ParseInteger(const std::string& value, int& result)
{
    char* end = nullptr;
    errno = 0;
    long parsed = std::strtol(value.c_str(), &end, 10);
    if (errno != 0 || end == value.c_str() || *end != '\0' || parsed < INT_MIN || parsed > INT_MAX) {
        return false;
    }
    result = static_cast<int>(parsed);
    return true;
}

int ParseArguments(int argc, char* argv[], TimerConfig& config)
{
    // argv[0] is the executable, argv[1] is the subcommand, options start at argv[2].
    int i = 2;
    while (i < argc) {
        std::string arg = argv[i];
        if (arg == "--duration") {
            if (i + 1 >= argc) {
                EmitError("ERR_MISSING_PARAM", "Missing value for parameter 'duration'.",
                    "Use: ohos-timer run --duration <sec> [--interval <sec>] [--showProgress] [--verbose]");
                return 1;
            }
            ++i;
            if (!ParseInteger(argv[i], config.duration)) {
                EmitError("ERR_INVALID_PARAM", "Parameter 'duration' must be an integer.",
                    "Use a positive integer, for example: ohos-timer run --duration 5");
                return 1;
            }
            ++i;
            continue;
        }
        if (arg == "--interval") {
            if (i + 1 >= argc) {
                EmitError("ERR_MISSING_PARAM", "Missing value for parameter 'interval'.",
                    "Use: ohos-timer run --interval <sec> [--duration <sec>] [--showProgress] [--verbose]");
                return 1;
            }
            ++i;
            if (!ParseInteger(argv[i], config.interval)) {
                EmitError("ERR_INVALID_PARAM", "Parameter 'interval' must be an integer.",
                    "Use a positive integer, for example: ohos-timer run --interval 1");
                return 1;
            }
            ++i;
            continue;
        }
        if (arg == "--errMsg") {
            if (i + 1 >= argc) {
                EmitError("ERR_MISSING_PARAM", "Missing value for parameter 'errMsg'.",
                    "Use: ohos-timer run --duration <sec> [--errMsg <message>] [--showProgress] [--verbose]");
                return 1;
            }
            ++i;
            config.errMsg = argv[i];
            ++i;
            continue;
        }
        if (arg == "--showProgress") {
            config.showProgress = true;
            ++i;
            continue;
        }
        if (arg == "--verbose") {
            config.verbose = true;
            ++i;
            continue;
        }
        if (arg == "--help" || arg == "-h") {
            ShowHelp();
            return 2;
        }

        EmitError("ERR_UNKNOWN_PARAM", "Unknown parameter '" + arg + "' for subcommand 'run'.",
            "Supported parameters are: --duration, --interval, --errMsg, --showProgress, --verbose");
        return 1;
    }
    return 0;
}

// Register SIGINT/SIGTERM handlers for the run path. SA_RESTART is deliberately
// NOT set: the timing loop relies on interrupted sleeps returning early so the
// cancel flag is observed within SLEEP_SLICE_MS instead of after the whole
// interval. The version path keeps the default disposition and is not
// registered, because it exits immediately and never declares cancelable.
bool RegisterCancelHandler()
{
    struct sigaction cancelAction = {};
    cancelAction.sa_handler = HandleCancelSignal;
    sigemptyset(&cancelAction.sa_mask);
    cancelAction.sa_flags = 0;
    if (sigaction(SIGINT, &cancelAction, nullptr) != 0) {
        return false;
    }
    return sigaction(SIGTERM, &cancelAction, nullptr) == 0;
}

// Sleep for the given milliseconds in short slices, checking the cancel flag
// between slices. std::this_thread::sleep_for is not used for the whole
// interval because its underlying nanosleep may silently retry after EINTR
// and hide the signal until the interval fully elapses. The parameter is int64_t:
// an int32 interval * MS_PER_SECOND would overflow to a negative value for large
// inputs (e.g. 3000000 s), the loop below would skip the sleep entirely and the
// main loop would busy-spin at 100% CPU.
void SleepWithCancelCheck(int64_t milliseconds)
{
    int64_t remainingMs = milliseconds;
    while (remainingMs > 0 && gCancelSignal == 0) {
        int64_t sliceMs = remainingMs < SLEEP_SLICE_MS ? remainingMs : SLEEP_SLICE_MS;
        std::this_thread::sleep_for(std::chrono::milliseconds(sliceMs));
        remainingMs -= sliceMs;
    }
}

int ExecuteTimer(const TimerConfig& config)
{
    auto startTime = std::chrono::steady_clock::now();
    int elapsed = 0;

    if (config.showProgress) {
        EmitProgress(0, "starting");
    }

    while (elapsed < config.duration && gCancelSignal == 0) {
        SleepWithCancelCheck(static_cast<int64_t>(config.interval) * MS_PER_SECOND);
        elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime).count());
        if (config.showProgress && elapsed < config.duration && gCancelSignal == 0) {
            int percentage = elapsed * PROGRESS_MAX / config.duration;
            EmitProgress(percentage, config.verbose ? "running (verbose)" : "running");
        }
    }

    if (gCancelSignal != 0) {
        // Cancelled by SIGINT/SIGTERM: finish the cleanup in normal context.
        // Report the partial result so the caller still receives the data
        // produced before the signal (actual_duration may be < duration).
        // The cancellation notice goes out unconditionally: regular progress events are
        // opt-in (--progress), but the caller must always learn the tool honoured the
        // cancel — this is the reliable, protocol-level counterpart of the best-effort
        // stderr hint the signal handler writes.
        int percentage = elapsed * PROGRESS_MAX / config.duration;
        EmitProgress(percentage, "cancelled");
        EmitSuccessResult("{\"duration\":" + std::to_string(config.duration) +
            ",\"actual_duration\":" + std::to_string(elapsed) + "}");
        std::cout.flush();
        std::cerr.flush();
        // Recommended exit code for a signal-terminated process: 128 + signo
        // (130 for SIGINT, 143 for SIGTERM). _exit is used because the cleanup
        // above has already flushed the streams explicitly.
        _exit(EXIT_CODE_SIGNAL_BASE + gCancelSignal);
    }

    if (config.showProgress) {
        EmitProgress(PROGRESS_MAX, "completed");
    }
    if (!config.errMsg.empty()) {
        std::cerr << config.errMsg << std::endl;
    }
    EmitSuccessResult("{\"duration\":" + std::to_string(config.duration) +
        ",\"actual_duration\":" + std::to_string(elapsed) + "}");
    return 0;
}

int RunCommand(int argc, char* argv[])
{
    TimerConfig config;
    int parseResult = ParseArguments(argc, argv, config);
    if (parseResult != 0) {
        return parseResult == 2 ? 0 : 1;
    }

    if (config.duration == 0) {
        EmitError("ERR_MISSING_PARAM", "Missing required parameter 'duration'.",
            "Use: ohos-timer run --duration <sec> [--interval <sec>] [--showProgress] [--verbose]");
        return 1;
    }
    if (config.duration < MIN_DURATION) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'duration' must be greater than or equal to 1.",
            "Use a positive integer, for example: ohos-timer run --duration 5");
        return 1;
    }
    if (config.duration > MAX_DURATION_SECONDS) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'duration' must not exceed " +
                std::to_string(MAX_DURATION_SECONDS) + " seconds.",
            "Use a value no larger than one day, for example: ohos-timer run --duration 60");
        return 1;
    }
    if (config.interval < MIN_INTERVAL) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'interval' must be greater than or equal to 1.",
            "Use a positive integer, for example: ohos-timer run --interval 1");
        return 1;
    }
    if (config.interval > MAX_INTERVAL_SECONDS) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'interval' must not exceed " +
                std::to_string(MAX_INTERVAL_SECONDS) + " seconds.",
            "Use a value no larger than one day, for example: ohos-timer run --interval 1");
        return 1;
    }
    if (config.interval > config.duration) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'interval' must not be greater than 'duration'.",
            "Use values such as: ohos-timer run --duration 5 --interval 1");
        return 1;
    }

    return ExecuteTimer(config);
}

int VersionCommand(int argc, char* argv[])
{
    if (argc == 3) {
        std::string arg = argv[2];
        if (arg == "--help" || arg == "-h") {
            ShowVersionHelp();
            return 0;
        }
    }
    if (argc != 2) {
        EmitError("ERR_UNKNOWN_PARAM", "Subcommand 'version' does not accept extra parameters.",
            "Use: ohos-timer version");
        return 1;
    }

    EmitSuccessResult("{\"version\":\"" + std::string(TOOL_VERSION) + "\"}");
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < MIN_ARGC) {
        EmitError("ERR_MISSING_PARAM", "Missing required subcommand.",
            "Use one of: ohos-timer run --duration <sec>, ohos-timer version");
        return 1;
    }

    std::string subcommand = argv[1];
    if (subcommand == "run") {
        // Only the run path registers signal handlers: config.json declares
        // subcommand-level cancelable for run, so cancelSession signals
        // are only routed here by the framework.
        if (!RegisterCancelHandler()) {
            EmitError("ERR_SIGNAL_SETUP", "Failed to register SIGINT/SIGTERM handlers.",
                "Check the signal resource, then retry.");
            return 1;
        }
        return RunCommand(argc, argv);
    }
    if (subcommand == "version") {
        // 'version' does not declare cancelable in config.json: an
        // undeclared subcommand falls back to the tool level, which is also
        // undeclared, so the framework never cancels this subcommand.
        return VersionCommand(argc, argv);
    }
    if (subcommand == "help" || subcommand == "--help" || subcommand == "-h") {
        ShowHelp();
        return 0;
    }

    EmitError("ERR_INVALID_PARAM", "Unknown subcommand '" + subcommand + "'.",
        "Use one of: run, version, help");
    return 1;
}
