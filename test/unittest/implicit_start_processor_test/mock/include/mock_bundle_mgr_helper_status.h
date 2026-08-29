#ifndef OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MGR_HELPER_STATUS_H
#define OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MGR_HELPER_STATUS_H

#include <string>
#include <vector>
#include <map>
#include "ability_info.h"
#include "app_clone_preference.h"
#include "application_info.h"
#include "bundle_info.h"
#include "errors.h"
#include "extension_ability_info.h"

namespace OHOS {
namespace AAFwk {
struct MockBundleMgrHelperStatus {
    static void Reset();

    // GetAppClonePreference
    static ErrCode getAppClonePreferenceRet_;
    static AppExecFwk::AppClonePreference appClonePreference_;
    static std::string lastClonePreferenceBundleName_;
    static int32_t lastClonePreferenceUserId_;

    // GetDefaultAppProxy / GetDefaultApplication
    static bool returnNullDefaultApp_;
    static ErrCode getDefaultAppRet_;
    static AppExecFwk::BundleInfo defaultBundleInfo_;

    // ImplicitQueryInfos
    static bool implicitQueryInfosRet_;
    static std::vector<AppExecFwk::AbilityInfo> queryAbilityInfos_;
    static std::vector<AppExecFwk::ExtensionAbilityInfo> queryExtensionInfos_;
    static bool queryFindDefaultApp_;

    // GetBundleInfo (BundleFlag version)
    static bool getBundleInfoRet_;
    static AppExecFwk::BundleInfo bundleInfo_;
    static std::map<std::string, std::string> bundleAppIdentifierMap_;

    // GetApplicationInfo (ApplicationFlag version)
    static bool getApplicationInfoRet_;
    static AppExecFwk::ApplicationInfo applicationInfo_;

    // GetNameForUid
    static ErrCode getNameForUidRet_;
    static std::string nameForUid_;

    // QueryCloneAbilityInfo
    static ErrCode queryCloneAbilityInfoRet_;
    static AppExecFwk::AbilityInfo cloneAbilityInfo_;

    // QueryCloneExtensionAbilityInfoWithAppIndex
    static ErrCode queryCloneExtensionRet_;
    static AppExecFwk::ExtensionAbilityInfo cloneExtensionInfo_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MGR_HELPER_STATUS_H
