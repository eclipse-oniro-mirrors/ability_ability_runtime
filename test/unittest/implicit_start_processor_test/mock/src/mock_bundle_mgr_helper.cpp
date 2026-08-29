#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "mock_bundle_mgr_helper_status.h"
#include "mock_default_app.h"

namespace OHOS {
namespace AAFwk {
ErrCode MockBundleMgrHelperStatus::getAppClonePreferenceRet_ = ERR_OK;
AppExecFwk::AppClonePreference MockBundleMgrHelperStatus::appClonePreference_ = {};
std::string MockBundleMgrHelperStatus::lastClonePreferenceBundleName_ = "";
int32_t MockBundleMgrHelperStatus::lastClonePreferenceUserId_ = -1;
bool MockBundleMgrHelperStatus::returnNullDefaultApp_ = true;
ErrCode MockBundleMgrHelperStatus::getDefaultAppRet_ = ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
AppExecFwk::BundleInfo MockBundleMgrHelperStatus::defaultBundleInfo_ = {};

bool MockBundleMgrHelperStatus::implicitQueryInfosRet_ = false;
std::vector<AppExecFwk::AbilityInfo> MockBundleMgrHelperStatus::queryAbilityInfos_ = {};
std::vector<AppExecFwk::ExtensionAbilityInfo> MockBundleMgrHelperStatus::queryExtensionInfos_ = {};
bool MockBundleMgrHelperStatus::queryFindDefaultApp_ = false;

bool MockBundleMgrHelperStatus::getBundleInfoRet_ = false;
AppExecFwk::BundleInfo MockBundleMgrHelperStatus::bundleInfo_ = {};
std::map<std::string, std::string> MockBundleMgrHelperStatus::bundleAppIdentifierMap_ = {};

bool MockBundleMgrHelperStatus::getApplicationInfoRet_ = false;
AppExecFwk::ApplicationInfo MockBundleMgrHelperStatus::applicationInfo_ = {};

ErrCode MockBundleMgrHelperStatus::getNameForUidRet_ = ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
std::string MockBundleMgrHelperStatus::nameForUid_ = "";

ErrCode MockBundleMgrHelperStatus::queryCloneAbilityInfoRet_ = ERR_OK;
AppExecFwk::AbilityInfo MockBundleMgrHelperStatus::cloneAbilityInfo_ = {};

ErrCode MockBundleMgrHelperStatus::queryCloneExtensionRet_ = ERR_OK;
AppExecFwk::ExtensionAbilityInfo MockBundleMgrHelperStatus::cloneExtensionInfo_ = {};

void MockBundleMgrHelperStatus::Reset()
{
    getAppClonePreferenceRet_ = ERR_OK;
    appClonePreference_ = {};
    lastClonePreferenceBundleName_ = "";
    lastClonePreferenceUserId_ = -1;
    returnNullDefaultApp_ = true;
    getDefaultAppRet_ = ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    defaultBundleInfo_ = {};

    implicitQueryInfosRet_ = false;
    queryAbilityInfos_ = {};
    queryExtensionInfos_ = {};
    queryFindDefaultApp_ = false;

    getBundleInfoRet_ = false;
    bundleInfo_ = {};
    bundleAppIdentifierMap_ = {};

    getApplicationInfoRet_ = false;
    applicationInfo_ = {};

    getNameForUidRet_ = ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    nameForUid_ = "";

    queryCloneAbilityInfoRet_ = ERR_OK;
    cloneAbilityInfo_ = {};

    queryCloneExtensionRet_ = ERR_OK;
    cloneExtensionInfo_ = {};
}
}  // namespace AAFwk

namespace AppExecFwk {
BundleMgrHelper::BundleMgrHelper() = default;
BundleMgrHelper::~BundleMgrHelper() = default;
void BundleMgrHelper::PreConnect() {}
void BundleMgrHelper::ConnectTillSuccess() {}
void BundleMgrHelper::SetBmsReady(bool bmsReady) {}

sptr<IDefaultApp> BundleMgrHelper::GetDefaultAppProxy()
{
    if (AAFwk::MockBundleMgrHelperStatus::returnNullDefaultApp_) {
        return nullptr;
    }
    static sptr<MockDefaultApp> mockDefaultApp = new MockDefaultApp();
    return mockDefaultApp;
}

ErrCode BundleMgrHelper::GetAppClonePreference(const std::string &bundleName, int32_t userId,
    AppClonePreference &preference)
{
    AAFwk::MockBundleMgrHelperStatus::lastClonePreferenceBundleName_ = bundleName;
    AAFwk::MockBundleMgrHelperStatus::lastClonePreferenceUserId_ = userId;
    preference = AAFwk::MockBundleMgrHelperStatus::appClonePreference_;
    return AAFwk::MockBundleMgrHelperStatus::getAppClonePreferenceRet_;
}

bool BundleMgrHelper::ImplicitQueryInfos(const Want &want, int32_t flags, int32_t userId, bool withDefault,
    std::vector<AbilityInfo> &abilityInfos, std::vector<ExtensionAbilityInfo> &extensionInfos,
    bool &findDefaultApp)
{
    abilityInfos = AAFwk::MockBundleMgrHelperStatus::queryAbilityInfos_;
    extensionInfos = AAFwk::MockBundleMgrHelperStatus::queryExtensionInfos_;
    findDefaultApp = AAFwk::MockBundleMgrHelperStatus::queryFindDefaultApp_;
    return AAFwk::MockBundleMgrHelperStatus::implicitQueryInfosRet_;
}

ErrCode BundleMgrHelper::QueryCloneAbilityInfo(const ElementName &element, int32_t flags, int32_t appCloneIndex,
    AbilityInfo &abilityInfo, int32_t userId)
{
    abilityInfo = AAFwk::MockBundleMgrHelperStatus::cloneAbilityInfo_;
    return AAFwk::MockBundleMgrHelperStatus::queryCloneAbilityInfoRet_;
}

ErrCode BundleMgrHelper::QueryCloneExtensionAbilityInfoWithAppIndex(const ElementName &element,
    int32_t flags, int32_t appCloneIndex, ExtensionAbilityInfo &extensionInfo, int32_t userId)
{
    extensionInfo = AAFwk::MockBundleMgrHelperStatus::cloneExtensionInfo_;
    return AAFwk::MockBundleMgrHelperStatus::queryCloneExtensionRet_;
}

bool BundleMgrHelper::GetBundleInfo(const std::string &bundleName, const BundleFlag flag,
    BundleInfo &bundleInfo, int32_t userId)
{
    auto it = AAFwk::MockBundleMgrHelperStatus::bundleAppIdentifierMap_.find(bundleName);
    if (it != AAFwk::MockBundleMgrHelperStatus::bundleAppIdentifierMap_.end()) {
        bundleInfo.signatureInfo.appIdentifier = it->second;
        return true;
    }
    bundleInfo = AAFwk::MockBundleMgrHelperStatus::bundleInfo_;
    return AAFwk::MockBundleMgrHelperStatus::getBundleInfoRet_;
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo &appInfo)
{
    appInfo = AAFwk::MockBundleMgrHelperStatus::applicationInfo_;
    return AAFwk::MockBundleMgrHelperStatus::getApplicationInfoRet_;
}

ErrCode BundleMgrHelper::GetNameForUid(const int32_t uid, std::string &name)
{
    name = AAFwk::MockBundleMgrHelperStatus::nameForUid_;
    return AAFwk::MockBundleMgrHelperStatus::getNameForUidRet_;
}

ErrCode BundleMgrHelper::GetCloneAppIndexes(const std::string &bundleName,
    std::vector<int32_t> &appIndexes, int32_t userId)
{
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
