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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CLOUD_PUSH_SIGN_TOOLS_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CLOUD_PUSH_SIGN_TOOLS_H

#include <cstdint>
#include <functional>
#include <string>
#include <tuple>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

namespace OHOS {
namespace AAFwk {

class AbilityCloudPushSignTool {
public:
    AbilityCloudPushSignTool() = default;
    ~AbilityCloudPushSignTool() = default;

    static bool VerifyFileSign(const std::string &pubKeyPath, const std::string &signPath,
        const std::string &digestPath);
    static bool VerifyRsa(RSA *pubKey, const std::string &digest, const std::string &sign);
    static std::tuple<int, std::string> CalcFileSha256Digest(const std::string &fpath);
    static int ForEachFileSegment(const std::string &fpath, std::function<void(char *, size_t)> executor);
    static void CalcBase64(uint8_t *input, uint32_t inputLen, std::string &encodedStr);
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CLOUD_PUSH_SIGN_TOOLS_H
