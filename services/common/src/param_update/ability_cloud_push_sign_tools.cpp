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

#include "param_update/ability_cloud_push_sign_tools.h"

#include <climits>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <memory>
#include <unistd.h>
#include <sys/stat.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr size_t MAX_SIGN_FILE_SIZE = 8 * 1024;  // 8KB: enough for RSA-PSS signature + CERT.SF header
constexpr size_t MAX_VERIFY_FILE_SIZE = 256 * 1024;  // 256KB cap for verify-path files (MANIFEST/json)

bool PathExists(const std::string &path)
{
    return access(path.c_str(), F_OK) == 0;
}

std::string ReadFileContent(const std::string &path)
{
    char realPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), realPath) == nullptr) {
        return "";
    }
    struct stat st;
    if (stat(realPath, &st) != 0 || !S_ISREG(st.st_mode)) {
        return "";
    }
    if (static_cast<size_t>(st.st_size) > MAX_SIGN_FILE_SIZE) {
        return "";
    }
    std::ifstream file(realPath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::string content;
    content.reserve(static_cast<size_t>(st.st_size));
    content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return content;
}
}  // namespace

bool AbilityCloudPushSignTool::VerifyFileSign(const std::string &pubKeyPath, const std::string &signPath,
    const std::string &digestPath)
{
    if (!PathExists(pubKeyPath) || !PathExists(signPath) || !PathExists(digestPath)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sign verify file not exist");
        return false;
    }
    std::string signStr = ReadFileContent(signPath);
    std::string digestStr = ReadFileContent(digestPath);
    BIO *bio = BIO_new_file(pubKeyPath.c_str(), "r");
    if (bio == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "BIO_new_file failed");
        return false;
    }
    RSA *pubKey = RSA_new();
    if (pubKey == nullptr) {
        BIO_free(bio);
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RSA_new failed");
        return false;
    }
    if (PEM_read_bio_RSA_PUBKEY(bio, &pubKey, nullptr, nullptr) == nullptr) {
        BIO_free(bio);
        RSA_free(pubKey);
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PEM_read_bio_RSA_PUBKEY failed");
        return false;
    }
    bool verify = false;
    if (!signStr.empty() && !digestStr.empty()) {
        verify = VerifyRsa(pubKey, digestStr, signStr);
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sign or digest empty");
    }
    BIO_free(bio);
    RSA_free(pubKey);
    return verify;
}

bool AbilityCloudPushSignTool::VerifyRsa(RSA *pubKey, const std::string &digest, const std::string &sign)
{
    EVP_PKEY *rawKey = EVP_PKEY_new();
    if (rawKey == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EVP_PKEY_new failed");
        return false;
    }
    std::shared_ptr<EVP_PKEY> evpKey(rawKey, EVP_PKEY_free);
    if (EVP_PKEY_set1_RSA(evpKey.get(), pubKey) != 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EVP_PKEY_set1_RSA failed");
        return false;
    }
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (ctx == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EVP_MD_CTX_new failed");
        return false;
    }
    EVP_PKEY_CTX *pctx = nullptr;
    if (EVP_DigestVerifyInit(ctx.get(), &pctx, EVP_sha256(), nullptr, evpKey.get()) != 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EVP_DigestVerifyInit failed");
        return false;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) != 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set RSA_PKCS1_PSS_PADDING failed");
        return false;
    }
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(pctx, EVP_sha256()) != 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set rsa mgf1 md failed");
        return false;
    }
    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, RSA_PSS_SALTLEN_AUTO) != 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set rsa pss saltlen failed");
        return false;
    }
    if (EVP_DigestVerifyUpdate(ctx.get(), digest.c_str(), digest.size()) != 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EVP_DigestVerifyUpdate failed");
        return false;
    }
    if (EVP_DigestVerifyFinal(ctx.get(), reinterpret_cast<const unsigned char *>(sign.c_str()),
        sign.size()) != 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EVP_DigestVerifyFinal failed");
        return false;
    }
    return true;
}

std::tuple<int, std::string> AbilityCloudPushSignTool::CalcFileSha256Digest(const std::string &fpath)
{
    auto res = std::make_unique<unsigned char[]>(SHA256_DIGEST_LENGTH);
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    auto sha256Update = [&ctx](char *buf, size_t len) { SHA256_Update(&ctx, buf, len); };
    int err = ForEachFileSegment(fpath, sha256Update);
    SHA256_Final(res.get(), &ctx);
    if (err) {
        return std::make_tuple(err, "");
    }
    std::string dist;
    CalcBase64(res.get(), SHA256_DIGEST_LENGTH, dist);
    return std::make_tuple(err, dist);
}

int AbilityCloudPushSignTool::ForEachFileSegment(const std::string &fpath,
    std::function<void(char *, size_t)> executor)
{
    char canonicalPath[PATH_MAX] = {0};
    if (realpath(fpath.c_str(), canonicalPath) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ForEachFileSegment path irregular");
        return errno;
    }
    struct stat st = {};
    if (stat(canonicalPath, &st) != 0 || !S_ISREG(st.st_mode) ||
        static_cast<size_t>(st.st_size) > MAX_VERIFY_FILE_SIZE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ForEachFileSegment invalid file or too large, size=%{public}lld",
            static_cast<long long>(st.st_size));
        return -1;
    }
    std::unique_ptr<FILE, decltype(&fclose)> filp = { fopen(canonicalPath, "rb"), fclose };
    if (filp == nullptr) {
        return errno;
    }
    const size_t pageSize = static_cast<size_t>(getpagesize());
    auto buf = std::make_unique<char[]>(pageSize);
    size_t actLen = 0;
    do {
        actLen = fread(buf.get(), 1, pageSize, filp.get());
        if (actLen > 0) {
            executor(buf.get(), actLen);
        }
    } while (actLen == pageSize);
    return ferror(filp.get()) ? errno : 0;
}

void AbilityCloudPushSignTool::CalcBase64(uint8_t *input, uint32_t inputLen, std::string &encodedStr)
{
    if (inputLen == 0) {
        encodedStr.clear();
        return;
    }
    size_t expectedLength = 4 * ((inputLen + 2) / 3);  // 4/3 fixed algorithm
    encodedStr.resize(expectedLength);
    int lengthTemp = EVP_EncodeBlock(reinterpret_cast<uint8_t *>(&encodedStr[0]), input, inputLen);
    if (lengthTemp < 0) {
        encodedStr.clear();
        return;
    }
    size_t actualLength = static_cast<size_t>(lengthTemp);
    encodedStr.resize(actualLength);
}
}  // namespace AAFwk
}  // namespace OHOS
