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

#include "sem_ver.h"

#include <cctype>
#include <vector>

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr size_t MIN_IDENTIFIER_LENGTH = 1;
constexpr size_t NEXT_CHARACTER_OFFSET = 1;
constexpr size_t CORE_VERSION_IDENTIFIER_COUNT = 3;
constexpr size_t MAJOR_VERSION_INDEX = 0;
constexpr size_t MINOR_VERSION_INDEX = 1;
constexpr size_t PATCH_VERSION_INDEX = 2;
constexpr int COMPARE_RESULT_LESS = -1;
constexpr int COMPARE_RESULT_EQUAL = 0;
constexpr int COMPARE_RESULT_GREATER = 1;

struct Identifier {
    std::string value;
    bool numeric = false;
};

struct SemVer {
    std::string major;
    std::string minor;
    std::string patch;
    std::vector<Identifier> prerelease;
};

bool IsAsciiDigitString(const std::string &value)
{
    if (value.empty()) {
        return false;
    }
    for (char ch : value) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

bool HasLeadingZero(const std::string &value)
{
    return value.size() > MIN_IDENTIFIER_LENGTH && value[0] == '0';
}

bool IsValidIdentifierChar(char ch)
{
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '-';
}

bool SplitIdentifiers(const std::string &value, std::vector<std::string> &identifiers)
{
    if (value.empty()) {
        return false;
    }

    size_t start = 0;
    while (start <= value.size()) {
        size_t end = value.find('.', start);
        size_t length = (end == std::string::npos) ? (value.size() - start) : (end - start);
        if (length == 0) {
            return false;
        }

        identifiers.emplace_back(value.substr(start, length));
        if (end == std::string::npos) {
            break;
        }
        start = end + NEXT_CHARACTER_OFFSET;
    }
    return !identifiers.empty();
}

bool ParseCore(const std::string &core, SemVer &version)
{
    std::vector<std::string> identifiers;
    if (!SplitIdentifiers(core, identifiers) || identifiers.size() != CORE_VERSION_IDENTIFIER_COUNT) {
        return false;
    }

    if (!IsAsciiDigitString(identifiers[MAJOR_VERSION_INDEX]) ||
        !IsAsciiDigitString(identifiers[MINOR_VERSION_INDEX]) ||
        !IsAsciiDigitString(identifiers[PATCH_VERSION_INDEX])) {
        return false;
    }
    if (HasLeadingZero(identifiers[MAJOR_VERSION_INDEX]) ||
        HasLeadingZero(identifiers[MINOR_VERSION_INDEX]) ||
        HasLeadingZero(identifiers[PATCH_VERSION_INDEX])) {
        return false;
    }

    version.major = identifiers[MAJOR_VERSION_INDEX];
    version.minor = identifiers[MINOR_VERSION_INDEX];
    version.patch = identifiers[PATCH_VERSION_INDEX];
    return true;
}

bool ParsePreRelease(const std::string &value, std::vector<Identifier> &prerelease)
{
    std::vector<std::string> identifiers;
    if (!SplitIdentifiers(value, identifiers)) {
        return false;
    }

    for (const auto &identifier : identifiers) {
        for (char ch : identifier) {
            if (!IsValidIdentifierChar(ch)) {
                return false;
            }
        }
        bool numeric = IsAsciiDigitString(identifier);
        if (numeric && HasLeadingZero(identifier)) {
            return false;
        }
        prerelease.emplace_back(Identifier { identifier, numeric });
    }
    return true;
}

bool ValidateBuild(const std::string &value)
{
    std::vector<std::string> identifiers;
    if (!SplitIdentifiers(value, identifiers)) {
        return false;
    }

    for (const auto &identifier : identifiers) {
        for (char ch : identifier) {
            if (!IsValidIdentifierChar(ch)) {
                return false;
            }
        }
    }
    return true;
}

bool ParseSemVerInternal(const std::string &value, SemVer &version)
{
    if (value.empty()) {
        return false;
    }

    std::string versionPart = value;
    size_t buildPos = versionPart.find('+');
    if (buildPos != std::string::npos) {
        if (!ValidateBuild(versionPart.substr(buildPos + NEXT_CHARACTER_OFFSET))) {
            return false;
        }
        versionPart = versionPart.substr(0, buildPos);
    }

    size_t prereleasePos = versionPart.find('-');
    std::string core = versionPart.substr(0, prereleasePos);
    if (!ParseCore(core, version)) {
        return false;
    }

    if (prereleasePos == std::string::npos) {
        return true;
    }

    return ParsePreRelease(versionPart.substr(prereleasePos + NEXT_CHARACTER_OFFSET), version.prerelease);
}

int CompareNumericString(const std::string &left, const std::string &right)
{
    if (left.size() != right.size()) {
        return left.size() < right.size() ? COMPARE_RESULT_LESS : COMPARE_RESULT_GREATER;
    }
    if (left == right) {
        return COMPARE_RESULT_EQUAL;
    }
    return left < right ? COMPARE_RESULT_LESS : COMPARE_RESULT_GREATER;
}

int CompareCore(const SemVer &left, const SemVer &right)
{
    int result = CompareNumericString(left.major, right.major);
    if (result != 0) {
        return result;
    }

    result = CompareNumericString(left.minor, right.minor);
    if (result != 0) {
        return result;
    }

    return CompareNumericString(left.patch, right.patch);
}

int ComparePrerelease(const std::vector<Identifier> &left, const std::vector<Identifier> &right)
{
    if (left.empty() && right.empty()) {
        return COMPARE_RESULT_EQUAL;
    }
    if (left.empty()) {
        return COMPARE_RESULT_GREATER;
    }
    if (right.empty()) {
        return COMPARE_RESULT_LESS;
    }

    size_t size = left.size() < right.size() ? left.size() : right.size();
    for (size_t i = 0; i < size; ++i) {
        if (left[i].value == right[i].value && left[i].numeric == right[i].numeric) {
            continue;
        }
        if (left[i].numeric && right[i].numeric) {
            return CompareNumericString(left[i].value, right[i].value);
        }
        if (left[i].numeric != right[i].numeric) {
            return left[i].numeric ? COMPARE_RESULT_LESS : COMPARE_RESULT_GREATER;
        }
        return left[i].value < right[i].value ? COMPARE_RESULT_LESS : COMPARE_RESULT_GREATER;
    }

    if (left.size() == right.size()) {
        return COMPARE_RESULT_EQUAL;
    }
    return left.size() < right.size() ? COMPARE_RESULT_LESS : COMPARE_RESULT_GREATER;
}

} // namespace

SemVerCompareResult CompareSemVer(const std::string &left, const std::string &right)
{
    SemVer leftVersion;
    SemVer rightVersion;
    if (!ParseSemVerInternal(left, leftVersion) || !ParseSemVerInternal(right, rightVersion)) {
        return SemVerCompareResult::INVALID;
    }

    int result = CompareCore(leftVersion, rightVersion);
    if (result == 0) {
        result = ComparePrerelease(leftVersion.prerelease, rightVersion.prerelease);
    }

    if (result < 0) {
        return SemVerCompareResult::LESS;
    }
    if (result > 0) {
        return SemVerCompareResult::GREATER;
    }
    return SemVerCompareResult::EQUAL;
}

bool IsValidSemVer(const std::string &version)
{
    SemVer parsedVersion;
    return ParseSemVerInternal(version, parsedVersion);
}
} // namespace AgentRuntime
} // namespace OHOS
