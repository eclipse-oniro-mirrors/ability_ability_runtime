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

#include "param_update/ability_cloud_push_manager.h"

#include <climits>
#include <filesystem>
#include <fstream>
#include <unistd.h>

#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_subscribe_info.h>
#include <common_event_support.h>

#include "app_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr int32_t RETRY_SUBSCRIBER = 3;
const std::string RECEIVE_UPDATE_PERMISSION = "ohos.permission.RECEIVE_UPDATE_MESSAGE";
const std::string CONFIG_UPDATED_ACTION = "usual.event.DUE_SA_CFG_UPDATED";
const std::string EVENT_INFO_TYPE = "type";
const std::string EVENT_INFO_SUBTYPE = "subtype";
const std::string CONFIG_TYPE = "Ams";
}  // namespace

AbilityCloudPushManager &AbilityCloudPushManager::GetInstance()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetInstance called");
    static AbilityCloudPushManager instance;
    return instance;
}

void AbilityCloudPushManager::InitParam()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "InitParam called");
    std::lock_guard<std::mutex> lock(initMutex_);
    if (paramReader_ == nullptr) {
        paramReader_ = std::make_shared<AbilityCloudPushReader>();
    }
    if (paramReader_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "paramReader_ is null");
        return;
    }
    std::error_code ec;
    std::filesystem::create_directories(AbilityCloudPushPaths::LOCAL_PARAM_DIR, ec);
    if (ec.value() != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create_directories local param dir failed, err=%{public}d", ec.value());
    }

    std::string cloudVer = paramReader_->GetPathVersion();
    std::string baselineVer = paramReader_->GetBaselineVersion();
    std::string localVer = LoadVersion();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "InitParam cloud=%{public}s baseline=%{public}s local=%{public}s",
        cloudVer.c_str(), baselineVer.c_str(), localVer.c_str());

    std::vector<std::string> cloudNum;
    std::vector<std::string> baselineNum;
    std::vector<std::string> localNum;
    bool cloudOk = paramReader_->VersionStrToNumber(cloudVer, cloudNum);
    bool baselineOk = paramReader_->VersionStrToNumber(baselineVer, baselineNum);
    bool localOk = paramReader_->VersionStrToNumber(localVer, localNum);
    if (!localOk) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "local version unparseable, fallback to default");
        localVer = AbilityCloudPushPaths::DEFAULT_VERSION;
        localOk = paramReader_->VersionStrToNumber(localVer, localNum);
    }
    if (!baselineOk) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "baseline version unparseable, fallback to default");
        baselineVer = AbilityCloudPushPaths::DEFAULT_VERSION;
        baselineOk = paramReader_->VersionStrToNumber(baselineVer, baselineNum);
    }
    // Apply cloud push only if cloud > local AND cloud > baseline (effective = max(local, baseline))
    if (cloudOk && baselineOk && localOk) {
        bool cloudGtLocal = paramReader_->CompareVersion(localNum, cloudNum);
        bool cloudGtBaseline = paramReader_->CompareVersion(baselineNum, cloudNum);
        if (cloudGtLocal && cloudGtBaseline) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "cloud newer than effective local, apply");
            ReloadParam();
            localVer = LoadVersion();
        }
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "version parse incomplete, skip cloud apply");
    }

    // Staleness: if local <= baseline, local cloud-push is stale, purge it so CCM is used
    std::vector<std::string> localNumAfter;
    std::vector<std::string> baselineNumAfter;
    if (paramReader_->VersionStrToNumber(localVer, localNumAfter) &&
        paramReader_->VersionStrToNumber(baselineVer, baselineNumAfter)) {
        bool localGtBaseline = paramReader_->CompareVersion(baselineNumAfter, localNumAfter);
        if (!localGtBaseline) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "local stale or equal to baseline, purge local");
            PurgeLocalParam();
        }
    }

    AppUtils::GetInstance().ReloadAllowNativeChildProcessApps();
}

void AbilityCloudPushManager::ReloadParam()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ReloadParam called");
    if (paramReader_ == nullptr) {
        return;
    }
    const std::string &cloudDir = AbilityCloudPushPaths::CLOUD_PARAM_DIR;
    std::string certFile = cloudDir + "/CERT.ENC";
    std::string verifyFile = cloudDir + "/CERT.SF";
    std::string manifestFile = cloudDir + "/MANIFEST.MF";
    if (!paramReader_->VerifyCertSfFile(certFile, verifyFile, manifestFile)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyCertSfFile failed, abort");
        return;
    }
    if (!paramReader_->VerifyParamFile(cloudDir, AbilityCloudPushPaths::VERSION_FILE_NAME)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "verify version file failed");
        return;
    }
    if (!paramReader_->VerifyParamFile(cloudDir, AbilityCloudPushPaths::CONFIG_FILE_NAME)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "verify config file failed");
        return;
    }
    CopyFileToLocal();
}

void AbilityCloudPushManager::CopyFileToLocal()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CopyFileToLocal called");
    const std::string &cloudDir = AbilityCloudPushPaths::CLOUD_PARAM_DIR;
    const std::string &localDir = AbilityCloudPushPaths::LOCAL_PARAM_DIR;
    const std::string cfgSrc = cloudDir + "/" + AbilityCloudPushPaths::CONFIG_FILE_NAME;
    const std::string cfgDes = localDir + "/" + AbilityCloudPushPaths::CONFIG_FILE_NAME;
    const std::string verSrc = cloudDir + "/" + AbilityCloudPushPaths::VERSION_FILE_NAME;
    const std::string verDes = localDir + "/" + AbilityCloudPushPaths::VERSION_FILE_NAME;
    const std::string cfgTmp = cfgDes + ".tmp";
    const std::string verTmp = verDes + ".tmp";
    // two-phase: copy both to .tmp first, rename both only after both succeed (version.txt last as commit marker)
    if (!CopyToTmp(cfgSrc, cfgTmp) || !CopyToTmp(verSrc, verTmp)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "copy to tmp failed, abort");
        std::error_code ec;
        std::filesystem::remove(cfgTmp, ec);
        std::filesystem::remove(verTmp, ec);
        return;
    }
    std::error_code ec;
    std::filesystem::rename(cfgTmp, cfgDes, ec);
    if (ec.value() != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rename cfg tmp failed, err=%{public}d", ec.value());
        std::filesystem::remove(cfgTmp, ec);
        std::filesystem::remove(verTmp, ec);
        return;
    }
    std::filesystem::rename(verTmp, verDes, ec);
    if (ec.value() != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rename ver tmp failed, err=%{public}d", ec.value());
        std::filesystem::remove(verTmp, ec);
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CopyFileToLocal success");
}

bool AbilityCloudPushManager::CopyToTmp(const std::string &src, const std::string &tmpPath)
{
    char srcReal[PATH_MAX] = {0};
    if (realpath(src.c_str(), srcReal) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CopyToTmp src path irregular");
        return false;
    }
    std::error_code ec;
    std::filesystem::remove(tmpPath, ec);
    auto opts = std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::skip_symlinks;
    std::filesystem::copy(srcReal, tmpPath, opts, ec);
    if (ec.value() != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "copy to tmp failed, err=%{public}d", ec.value());
        std::filesystem::remove(tmpPath, ec);
        return false;
    }
    return true;
}

void AbilityCloudPushManager::PurgeLocalParam()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "PurgeLocalParam called");
    std::error_code ec;
    std::filesystem::remove(
        AbilityCloudPushPaths::LOCAL_PARAM_DIR + "/" + AbilityCloudPushPaths::VERSION_FILE_NAME, ec);
    std::filesystem::remove(
        AbilityCloudPushPaths::LOCAL_PARAM_DIR + "/" + AbilityCloudPushPaths::CONFIG_FILE_NAME, ec);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "PurgeLocalParam done");
}

std::string AbilityCloudPushManager::LoadVersion()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "LoadVersion called");
    if (paramReader_ == nullptr) {
        return AbilityCloudPushPaths::DEFAULT_VERSION;
    }
    std::string localPath = AbilityCloudPushPaths::LOCAL_PARAM_DIR + "/" + AbilityCloudPushPaths::VERSION_FILE_NAME;
    std::ifstream file(localPath);
    if (!file.good()) {
        return paramReader_->GetVersionInfoStr(
            AbilityCloudPushPaths::BASELINE_PARAM_DIR + "/" + AbilityCloudPushPaths::VERSION_FILE_NAME);
    }
    file.close();
    return paramReader_->GetVersionInfoStr(localPath);
}

void AbilityCloudPushManager::SubscribeEvent()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SubscribeEvent called");
    std::lock_guard<std::mutex> lock(subscriberMutex_);
    if (subscriber_ != nullptr) {
        return;
    }
    eventHandles_[CONFIG_UPDATED_ACTION] = [this](const Want &want) { HandleParamUpdate(want); };
    EventFwk::MatchingSkills matchingSkills;
    for (auto &event : eventHandles_) {
        matchingSkills.AddEvent(event.first);
    }
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(RECEIVE_UPDATE_PERMISSION);
    auto subscriber = std::make_shared<CloudPushEventSubscriber>(subscribeInfo, *this);
    int32_t retry = RETRY_SUBSCRIBER;
    bool subscribed = false;
    do {
        if (EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "SubscribeEvent success");
            subscribed = true;
            break;
        }
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SubscribeEvent failed, retry %{public}d", retry);
        retry--;
        sleep(1);
    } while (retry > 0);
    if (subscribed) {
        subscriber_ = std::move(subscriber);
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SubscribeEvent failed after retries, will retry on next call");
    }
}

void AbilityCloudPushManager::OnReceiveEvent(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnReceiveEvent called");
    std::string action = want.GetAction();
    EventHandle handler;
    {
        std::lock_guard<std::mutex> lock(subscriberMutex_);
        auto it = eventHandles_.find(action);
        if (it == eventHandles_.end()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ignore event: %{public}s", action.c_str());
            return;
        }
        handler = it->second;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "handle event: %{public}s", action.c_str());
    handler(want);
}

void AbilityCloudPushManager::HandleParamUpdate(const Want &want) const
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "HandleParamUpdate called");
    std::string action = want.GetAction();
    std::string type = want.GetStringParam(EVENT_INFO_TYPE);
    std::string subtype = want.GetStringParam(EVENT_INFO_SUBTYPE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "receive param update: action=%{public}s type=%{public}s subtype=%{public}s",
        action.c_str(), type.c_str(), subtype.c_str());
    if (action != CONFIG_UPDATED_ACTION || type != CONFIG_TYPE || subtype != AbilityCloudPushPaths::SUBTYPE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "invalid param update info, ignore");
        return;
    }
    AbilityCloudPushManager::GetInstance().InitParam();
}
}  // namespace AAFwk
}  // namespace OHOS
