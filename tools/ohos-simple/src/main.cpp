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
#include <climits>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
constexpr int MAX_COUNT = 10;
constexpr int MIN_COUNT = 1;
}

struct SimpleConfig {
    std::string message = "Hello from ohos-simple";
    int count = 1;
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
    std::cout << "Usage: ohos-simple [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --message <msg>  Set message to display (default: 'Hello from ohos-simple')" << std::endl;
    std::cout << "  --count <num>    Number of repetitions (1-10, default: 1)" << std::endl;
    std::cout << "  --verbose        Enable verbose output" << std::endl;
    std::cout << "  --help, -h       Show this help message" << std::endl;
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

int ParseArguments(int argc, char* argv[], SimpleConfig& config)
{
    int i = 1;
    while (i < argc) {
        std::string arg = argv[i];
        if (arg == "--message") {
            if (i + 1 >= argc) {
                EmitError("ERR_MISSING_PARAM", "Missing value for parameter 'message'.",
                    "Use: ohos-simple --message <msg> [--count <num>] [--verbose]");
                return 1;
            }
            ++i;
            config.message = argv[i];
            ++i;
            continue;
        }
        if (arg == "--count") {
            if (i + 1 >= argc) {
                EmitError("ERR_MISSING_PARAM", "Missing value for parameter 'count'.",
                    "Use: ohos-simple --count <num> [--message <msg>] [--verbose]");
                return 1;
            }
            ++i;
            if (!ParseInteger(argv[i], config.count)) {
                EmitError("ERR_INVALID_PARAM", "Parameter 'count' must be an integer.",
                    "Use an integer between 1 and 10, for example: --count 2");
                return 1;
            }
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
            "Supported parameters are: --message, --count, --verbose");
        return 1;
    }
    return 0;
}

std::string ExecuteTask(const std::string& message, int count)
{
    std::string result;
    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            result += " ";
        }
        result += message;
    }
    return result;
}

int main(int argc, char* argv[])
{
    SimpleConfig config;
    int parseResult = ParseArguments(argc, argv, config);
    if (parseResult != 0) {
        return parseResult == 2 ? 0 : 1;
    }

    if (config.message.empty()) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'message' must not be empty.",
            "Provide a non-empty string, for example: --message hello");
        return 1;
    }
    if (config.count < MIN_COUNT || config.count > MAX_COUNT) {
        EmitError("ERR_INVALID_PARAM", "Parameter 'count' must be between 1 and 10.",
            "Use an integer between 1 and 10, for example: --count 2");
        return 1;
    }

    std::string result = ExecuteTask(config.message, config.count);
    if (config.verbose) {
        result = "[verbose] " + result;
    }
    EmitSuccessResult("{\"message\":\"" + EscapeJson(result) + "\",\"repeat_count\":" +
        std::to_string(config.count) + "}");
    return 0;
}
