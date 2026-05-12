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

#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr int PROGRESS_MAX = 100;
constexpr int MIN_ARGC = 2;
constexpr const char* VERSION = "1.0.0";
constexpr const char* BUILD_TIME = "2026-04-04 00:00:00";
}

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
    std::cout << "Usage: ohos-example <subcommand> [options]" << std::endl;
    std::cout << "Subcommands:" << std::endl;
    std::cout << "  run --argLine <value>  Run the tool with a single string argument" << std::endl;
    std::cout << "  version                Show version information" << std::endl;
    std::cout << "  help                   Show this help message" << std::endl;
}

void ShowRunHelp()
{
    std::cout << "Usage: ohos-example run [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --argLine <value>  Argument string to pass to the tool" << std::endl;
    std::cout << "  --help, -h         Show this help message" << std::endl;
}

void ShowVersionHelp()
{
    std::cout << "Usage: ohos-example version [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h  Show this help message" << std::endl;
}

int RunCommand(int argc, char* argv[])
{
    std::string argLine;
    int i = 2;
    while (i < argc) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            ShowRunHelp();
            return 0;
        }
        if (arg == "--argLine") {
            if (i + 1 >= argc) {
                EmitError("ERR_MISSING_PARAM", "Missing value for parameter 'argLine'.",
                    "Use: ohos-example run --argLine <value>");
                return 1;
            }
            ++i;
            argLine = argv[i];
            ++i;
            continue;
        }

        EmitError("ERR_UNKNOWN_PARAM", "Unknown parameter '" + arg + "' for subcommand 'run'.",
            "Use: ohos-example run --argLine <value>");
        return 1;
    }

    if (argLine.empty()) {
        EmitError("ERR_MISSING_PARAM", "Missing required parameter 'argLine'.",
            "Use: ohos-example run --argLine <value>");
        return 1;
    }

    EmitProgress(0, "starting");
    EmitProgress(50, "running");
    EmitProgress(PROGRESS_MAX, "completed");
    EmitSuccessResult("{\"result\":\"执行完成 " + EscapeJson(argLine) + "\"}");
    return 0;
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
            "Use: ohos-example version");
        return 1;
    }

    EmitSuccessResult("{\"version\":\"" + std::string(VERSION) + "\",\"build_time\":\"" +
        std::string(BUILD_TIME) + "\"}");
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < MIN_ARGC) {
        EmitError("ERR_MISSING_PARAM", "Missing required subcommand.",
            "Use one of: ohos-example run --argLine <value>, ohos-example version");
        return 1;
    }

    std::string subcommand = argv[1];
    if (subcommand == "run") {
        return RunCommand(argc, argv);
    }
    if (subcommand == "version") {
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
