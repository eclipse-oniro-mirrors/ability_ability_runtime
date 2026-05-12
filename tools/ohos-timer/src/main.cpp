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

#include <cerrno>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace {
constexpr int PROGRESS_MAX = 100;
constexpr int DEFAULT_INTERVAL = 1;
constexpr int MIN_DURATION = 1;
constexpr int MIN_INTERVAL = 1;
}

struct TimerConfig {
    int duration = 0;
    int interval = DEFAULT_INTERVAL;
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
    std::cout << "Usage: ohos-timer [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --duration <sec>      Duration in seconds (required, minimum 1)" << std::endl;
    std::cout << "  --interval <sec>      Progress update interval in seconds (default 1)" << std::endl;
    std::cout << "  --showProgress        Enable progress events" << std::endl;
    std::cout << "  --verbose             Enable verbose mode" << std::endl;
    std::cout << "  --help, -h            Show this help message" << std::endl;
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
    int i = 1;
    while (i < argc) {
        std::string arg = argv[i];
        if (arg == "--duration") {
            if (i + 1 >= argc) {
                EmitError("ERR_MISSING_PARAM", "Missing value for parameter 'duration'.",
                    "Use: ohos-timer --duration <sec> [--interval <sec>] [--showProgress] [--verbose]");
                return 1;
            }
            ++i;
            if (!ParseInteger(argv[i], config.duration)) {
                EmitError("ERR_INVALID_PARAM", "Parameter 'duration' must be an integer.",
                    "Use a positive integer, for example: --duration 5");
                return 1;
            }
            ++i;
            continue;
        }
        if (arg == "--interval") {
            if (i + 1 >= argc) {
                EmitError("ERR_MISSING_PARAM", "Missing value for parameter 'interval'.",
                    "Use: ohos-timer --interval <sec> [--duration <sec>] [--showProgress] [--verbose]");
                return 1;
            }
            ++i;
            if (!ParseInteger(argv[i], config.interval)) {
                EmitError("ERR_INVALID_PARAM", "Parameter 'interval' must be an integer.",
                    "Use a positive integer, for example: --interval 1");
                return 1;
            }
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

        EmitError("ERR_UNKNOWN_PARAM", "Unknown parameter '" + arg + "'.",
            "Supported parameters are: --duration, --interval, --showProgress, --verbose");
        return 1;
    }
    return 0;
}

int ExecuteTimer(const TimerConfig& config)
{
    auto startTime = std::chrono::steady_clock::now();
    int elapsed = 0;

    if (config.showProgress) {
        EmitProgress(0, "starting");
    }

    while (elapsed < config.duration) {
        std::this_thread::sleep_for(std::chrono::seconds(config.interval));
        elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime).count());
        if (config.showProgress && elapsed < config.duration) {
            int percentage = elapsed * PROGRESS_MAX / config.duration;
            EmitProgress(percentage, config.verbose ? "running (verbose)" : "running");
        }
    }

    if (config.showProgress) {
        EmitProgress(PROGRESS_MAX, "completed");
    }
    EmitSuccessResult("{\"duration\":" + std::to_string(config.duration) +
        ",\"actual_duration\":" + std::to_string(elapsed) + "}");
    return 0;
}

int main(int argc, char* argv[])
{
    TimerConfig config;
    int parseResult = ParseArguments(argc, argv, config);
    if (parseResult != 0) {
        return parseResult == 2 ? 0 : 1;
    }

    if (config.duration == 0) {
        EmitError("ERR_MISSING_PARAM", "Missing required parameter 'duration'.",
            "Use: ohos-timer --duration <sec> [--interval <sec>] [--showProgress] [--verbose]");
        return 1;
    }
    if (config.duration < MIN_DURATION) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'duration' must be greater than or equal to 1.",
            "Use a positive integer, for example: --duration 5");
        return 1;
    }
    if (config.interval < MIN_INTERVAL) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'interval' must be greater than or equal to 1.",
            "Use a positive integer, for example: --interval 1");
        return 1;
    }
    if (config.interval > config.duration) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'interval' must not be greater than 'duration'.",
            "Use values such as: --duration 5 --interval 1");
        return 1;
    }

    return ExecuteTimer(config);
}
