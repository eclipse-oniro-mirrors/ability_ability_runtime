/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "bundle_mgr_helper.h"

#include "bundle_mgr_service_death_recipient.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "app_utils.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
    const std::string profileJsonStr = "{"
        "\"insightIntents\":["
        "{"
            "\"intentName\":\"test1\","
            "\"domain\":\"domain1\","
            "\"intentVersion\":\"1.0\","
            "\"srcEntry\":\"entry1\","
            "\"uiAbility\":{"
                "\"ability\":\"ability1\","
                "\"executeMode\":[\"foreground\"]"
            "},"
            "\"uiExtension\":{"
                "\"ability\":\"ability1\""
            "},"
            "\"serviceExtension\":{"
                "\"ability\":\"ability1\""
            "},"
            "\"form\":{"
                "\"ability\":\"ability1\","
                "\"formName\":\"form1\""
            "}"
        "},"
        "{"
            "\"intentName\":\"test2\","
            "\"domain\":\"domain1\","
            "\"intentVersion\":\"1.0\","
            "\"srcEntry\":\"entry1\","
            "\"uiAbility\":{"
                "\"ability\":\"ability1\","
                "\"executeMode\":[\"foreground\"]"
            "},"
            "\"uiExtension\":{"
                "\"ability\":\"ability1\""
            "},"
            "\"serviceExtension\":{"
                "\"ability\":\"ability1\""
            "},"
            "\"form\": {"
                "\"ability\":\"ability1\","
                "\"formName\":\"form1\""
            "}"
        "}"
        "]"
    "}";
}
BundleMgrHelper::BundleMgrHelper() {}

BundleMgrHelper::~BundleMgrHelper()
{
    if (bundleMgr_ != nullptr && bundleMgr_->AsObject() != nullptr && deathRecipient_ != nullptr) {
        bundleMgr_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
}

ErrCode BundleMgrHelper::GetJsonProfile(ProfileType profileType, const std::string &bundleName,
    const std::string &moduleName, std::string &profile, int32_t userId)
{
    profile = profileJsonStr;
    return ERR_OK;
}

std::string BundleMgrHelper::GetStringById(
    const std::string &bundleName, const std::string &moduleName, uint32_t resId, int32_t userId)
{
    return "";
}

bool BundleMgrHelper::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    bundleInfo.applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    return true;
}

bool BundleMgrHelper::GetBundleInfo(
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
{
    return true;
}

ErrCode BundleMgrHelper::GetBundleInfoV9(
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetBundleInfosV9(int32_t flags,
    std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetLauncherAbilityInfoSync(
    const std::string &bundleName, int32_t userId, std::vector<AbilityInfo> &abilityInfo)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetCloneBundleInfo(const std::string &bundleName, int32_t flags, int32_t appCloneIndex,
    BundleInfo &bundleInfo, int32_t userId)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetSandboxBundleInfo(
    const std::string &bundleName, int32_t appIndex, int32_t userId, BundleInfo &info)
{
    return ERR_OK;
}

sptr<IAppControlMgr> BundleMgrHelper::GetAppControlProxy()
{
    return nullptr;
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo &appInfo)
{
    return true;
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo)
{
    return true;
}

ErrCode BundleMgrHelper::GetNameForUid(const int32_t uid, std::string &name)
{
    return ERR_OK;
}

bool BundleMgrHelper::QueryAppGalleryBundleName(std::string &bundleName)
{
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS