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

#include "param_update/ability_cloud_push_reader.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#include "param_update/ability_cloud_push_sign_tools.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr int DEC = 10;
constexpr size_t MAX_VERSION_FILE_SIZE = 4096;  // 4KB: enough for version.txt
constexpr size_t MIN_SPLIT_TOKENS = 2;  // separator present: at least 2 tokens after split

std::vector<std::string> SplitByChar(const std::string &s, char sep)
{
    std::vector<std::string> tokens;
    if (s.empty()) {
        return tokens;
    }
    std::string token;
    std::istringstream iss(s);
    while (std::getline(iss, token, sep)) {
        tokens.push_back(token);
    }
    return tokens;
}

void TrimString(std::string &s)
{
    auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        s.clear();
        return;
    }
    auto last = s.find_last_not_of(" \t\r\n");
    s = s.substr(first, last - first + 1);
}

std::string FindDigestInManifest(const std::string &manifestPath, const std::string &fileName)
{
    char canonicalPath[PATH_MAX] = {0};
    if (realpath(manifestPath.c_str(), canonicalPath) == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "FindDigestInManifest manifest path irregular");
        return "";
    }
    std::ifstream mFile(canonicalPath);
    if (!mFile.good()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "FindDigestInManifest manifest open failed");
        return "";
    }
    std::string sha256Digest;
    std::string key = "Name: " + fileName;
    std::string line;
    while (std::getline(mFile, line)) {
        if (line.find(key) == std::string::npos) {
            continue;
        }
        std::string nextline;
        if (!std::getline(mFile, nextline)) {
            break;
        }
        auto tokens = SplitByChar(nextline, ':');
        if (tokens.size() >= MIN_SPLIT_TOKENS) {
            sha256Digest = tokens[1];
            TrimString(sha256Digest);
        }
        break;
    }
    mFile.close();
    return sha256Digest;
}
}  // namespace

std::string AbilityCloudPushReader::GetPathVersion()
{
    return GetVersionInfoStr(AbilityCloudPushPaths::CLOUD_PARAM_DIR + "/" + AbilityCloudPushPaths::VERSION_FILE_NAME);
}

std::string AbilityCloudPushReader::GetBaselineVersion()
{
    return GetVersionInfoStr(
        AbilityCloudPushPaths::BASELINE_PARAM_DIR + "/" + AbilityCloudPushPaths::VERSION_FILE_NAME);
}

bool AbilityCloudPushReader::VerifyCertSfFile(const std::string &certFile, const std::string &verifyFile,
    const std::string &manifestFile)
{
    char canonicalPath[PATH_MAX] = {0};
    if (realpath(verifyFile.c_str(), canonicalPath) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyCertSfFile verifyFile path irregular");
        return false;
    }
    if (!AbilityCloudPushSignTool::VerifyFileSign(AbilityCloudPushPaths::PUBKEY_PATH, certFile, canonicalPath)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyFileSign failed");
        return false;
    }
    std::ifstream file(canonicalPath);
    if (!file.good()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CERT.SF open failed");
        return false;
    }
    std::string line;
    std::getline(file, line);
    file.close();
    auto tokens = SplitByChar(line, ':');
    if (tokens.size() < MIN_SPLIT_TOKENS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CERT.SF first line invalid");
        return false;
    }
    std::string sha256Digest = tokens[1];
    TrimString(sha256Digest);
    std::tuple<int, std::string> ret = AbilityCloudPushSignTool::CalcFileSha256Digest(manifestFile);
    if (std::get<0>(ret) != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CalcFileSha256Digest for manifest failed, err=%{public}d", std::get<0>(ret));
        return false;
    }
    return sha256Digest == std::get<1>(ret);
}

bool AbilityCloudPushReader::VerifyParamFile(const std::string &cfgDirPath, const std::string &filePathStr)
{
    char canonicalPath[PATH_MAX] = {0};
    if (realpath((cfgDirPath + "/" + filePathStr).c_str(), canonicalPath) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyParamFile path irregular: %{public}s", filePathStr.c_str());
        return false;
    }
    std::string absFilePath = std::string(canonicalPath);
    std::ifstream paramFile(absFilePath);
    if (!paramFile.good()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "param file open failed: %{public}s", absFilePath.c_str());
        return false;
    }
    paramFile.close();
    std::string sha256Digest = FindDigestInManifest(cfgDirPath + "/MANIFEST.MF", filePathStr);
    if (sha256Digest.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyParamFile digest empty: %{public}s", filePathStr.c_str());
        return false;
    }
    std::tuple<int, std::string> ret = AbilityCloudPushSignTool::CalcFileSha256Digest(absFilePath);
    if (std::get<0>(ret) != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CalcFileSha256Digest failed, err=%{public}d", std::get<0>(ret));
        return false;
    }
    return sha256Digest == std::get<1>(ret);
}

std::string AbilityCloudPushReader::GetVersionInfoStr(const std::string &filePathStr)
{
    char canonicalPath[PATH_MAX] = {0};
    if (realpath(filePathStr.c_str(), canonicalPath) == nullptr) {
        return AbilityCloudPushPaths::DEFAULT_VERSION;
    }
    struct stat st;
    if (stat(canonicalPath, &st) != 0 || !S_ISREG(st.st_mode) ||
        static_cast<size_t>(st.st_size) > MAX_VERSION_FILE_SIZE) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetVersionInfoStr file not regular or too large");
        return AbilityCloudPushPaths::DEFAULT_VERSION;
    }
    std::ifstream file(canonicalPath);
    if (!file.good()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetVersionInfoStr file open failed");
        return AbilityCloudPushPaths::DEFAULT_VERSION;
    }
    std::string line;
    std::getline(file, line);
    file.close();
    auto tokens = SplitByChar(line, '=');
    if (tokens.size() < MIN_SPLIT_TOKENS) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetVersionInfoStr no separator in first line");
        return AbilityCloudPushPaths::DEFAULT_VERSION;
    }
    std::string versionStr = tokens[1];
    TrimString(versionStr);
    if (versionStr.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetVersionInfoStr version value empty");
        return AbilityCloudPushPaths::DEFAULT_VERSION;
    }
    return versionStr;
}

bool AbilityCloudPushReader::VersionStrToNumber(const std::string &versionStr, std::vector<std::string> &versionNum)
{
    versionNum.clear();
    if (versionStr.empty()) {
        return false;
    }
    versionNum = SplitByChar(versionStr, '.');
    if (static_cast<int>(versionNum.size()) != AbilityCloudPushPaths::VERSION_LEN) {
        return false;
    }
    for (const auto &seg : versionNum) {
        if (seg.empty()) {
            return false;
        }
        errno = 0;
        char *endPtr = nullptr;
        long val = strtol(seg.c_str(), &endPtr, DEC);
        if (errno == ERANGE || endPtr == seg.c_str() || *endPtr != '\0' || val < 0) {
            return false;
        }
    }
    return true;
}

bool AbilityCloudPushReader::CompareVersion(const std::vector<std::string> &localVersion,
    const std::vector<std::string> &pathVersion)
{
    if (static_cast<int>(localVersion.size()) != AbilityCloudPushPaths::VERSION_LEN ||
        static_cast<int>(pathVersion.size()) != AbilityCloudPushPaths::VERSION_LEN) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "version num not valid");
        return false;
    }
    for (int i = 0; i < AbilityCloudPushPaths::VERSION_LEN; i++) {
        if (localVersion[i] != pathVersion[i]) {
            return strtol(localVersion[i].c_str(), nullptr, DEC) < strtol(pathVersion[i].c_str(), nullptr, DEC);
        }
    }
    return false;  // equal versions, no reload
}
}  // namespace AAFwk
}  // namespace OHOS
