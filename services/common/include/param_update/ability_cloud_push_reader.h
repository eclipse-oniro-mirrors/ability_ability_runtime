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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CLOUD_PUSH_READER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CLOUD_PUSH_READER_H

#include <string>
#include <vector>

namespace OHOS {
namespace AAFwk {

namespace AbilityCloudPushPaths {
// Platform subtype ("phone" or "tv" or "generic"), driven by GN target_platform
#if defined(ABILITY_CLOUD_PUSH_SUBTYPE_TV)
inline const std::string SUBTYPE = "tv";
#elif defined(ABILITY_CLOUD_PUSH_SUBTYPE_PHONE)
inline const std::string SUBTYPE = "phone";
#else
inline const std::string SUBTYPE = "generic";
#endif
// Cloud-push download dir (untrusted, on /data)
inline const std::string CLOUD_PARAM_DIR =
    "/data/service/el1/public/update/param_service/install/system/etc/Ams/" + SUBTYPE;
// Baseline param dir (shipped with image, OTA-refreshed, read-only on /system)
inline const std::string BASELINE_PARAM_DIR = "/system/etc/Ams/" + SUBTYPE;
// Local applied param dir (verified cloud push copied here, on /data)
inline const std::string LOCAL_PARAM_DIR =
    "/data/service/el1/public/AbilityManagerService/param_update/Ams/" + SUBTYPE;
// Public key for signature verification (read-only on /system)
inline const std::string PUBKEY_PATH = "/system/etc/ability_runtime/configkey_ams_cloudpush_v1.pem";
inline const std::string VERSION_FILE_NAME = "version.txt";
inline const std::string CONFIG_FILE_NAME = "allow_native_child_process_apps.json";
inline const std::string DEFAULT_VERSION = "1.0.0.0";
inline constexpr int VERSION_LEN = 4;
}  // namespace AbilityCloudPushPaths

class AbilityCloudPushReader {
public:
    AbilityCloudPushReader() = default;
    virtual ~AbilityCloudPushReader() = default;

    virtual bool VerifyCertSfFile(const std::string &certFile, const std::string &verifyFile,
        const std::string &manifestFile);
    virtual bool VerifyParamFile(const std::string &cfgDirPath, const std::string &filePathStr);
    std::string GetPathVersion();
    std::string GetBaselineVersion();
    std::string GetVersionInfoStr(const std::string &filePathStr);
    bool VersionStrToNumber(const std::string &versionStr, std::vector<std::string> &versionNum);
    bool CompareVersion(const std::vector<std::string> &localVersion,
        const std::vector<std::string> &pathVersion);
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CLOUD_PUSH_READER_H
