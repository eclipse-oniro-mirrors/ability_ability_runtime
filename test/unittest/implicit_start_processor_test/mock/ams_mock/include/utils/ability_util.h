#ifndef OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H

#include <memory>
#include <string>
#include "bundle_mgr_helper.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityUtil {
public:
    static std::shared_ptr<AppExecFwk::BundleMgrHelper> GetBundleManagerHelper()
    {
        return DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    }
    static void WantSetParameterWindowMode(Want &want, int32_t windowMode)
    {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    static const std::string DLP_PARAMS_SANDBOX;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
