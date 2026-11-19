#ifndef OHOS_ABILITY_RUNTIME_MOCK_DEFAULT_APP_H
#define OHOS_ABILITY_RUNTIME_MOCK_DEFAULT_APP_H

#include "default_app_interface.h"
#include "mock_bundle_mgr_helper_status.h"

namespace OHOS {
namespace AppExecFwk {
class MockDefaultApp : public IDefaultApp {
public:
    sptr<IRemoteObject> AsObject() override { return nullptr; }

    ErrCode GetDefaultApplication(int32_t userId, const std::string& type, BundleInfo& bundleInfo) override
    {
        bundleInfo = AAFwk::MockBundleMgrHelperStatus::defaultBundleInfo_;
        return AAFwk::MockBundleMgrHelperStatus::getDefaultAppRet_;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_DEFAULT_APP_H
