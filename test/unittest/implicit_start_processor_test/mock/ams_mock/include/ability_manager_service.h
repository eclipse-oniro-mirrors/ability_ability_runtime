#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H

#include "system_ability.h"
#include "singleton.h"
#include "want.h"
#include "ability_record.h"
#include "start_options.h"
#include "ability_start_setting.h"
#include "ability_manager_errors.h"
#include "utils/start_ability_utils.h"
#include "parameters.h"
#include "kiosk_manager.h"
#include "deeplink_reserve/deeplink_reserve_config.h"

namespace OHOS {
namespace AAFwk {

using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;

constexpr const char* KEY_REQUEST_ID = "com.ohos.param.requestId";

class AbilityManagerService : public SystemAbility,
    public std::enable_shared_from_this<AbilityManagerService> {
    DECLARE_DELAYED_SINGLETON(AbilityManagerService)
public:
    int IsCallFromBackground(const AbilityRequest &request, bool &isBackgroundCall, bool fromNotification)
    {
        return ERR_OK;
    }
    int StartAbility(const Want &want)
    {
        return ERR_OK;
    }
    int StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken)
    {
        return ERR_OK;
    }
    int ImplicitStartAbilityAsCaller(const Want &want, const sptr<IRemoteObject> &callerToken,
        const sptr<IRemoteObject> &specifiedToken)
    {
        return ERR_OK;
    }
    int ImplicitStartAbility(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
    {
        return RESOLVE_ABILITY_ERR;
    }
    int ImplicitStartAbility(const Want &want, const AbilityStartSetting &startSetting,
        const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
    {
        return RESOLVE_ABILITY_ERR;
    }
    int ImplicitStartExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
        int32_t userId, ExtensionAbilityType extensionType)
    {
        return RESOLVE_ABILITY_ERR;
    }
    int StartAbilityInner(const StartAbilityWrapParam &param)
    {
        return RESOLVE_ABILITY_ERR;
    }
    void OnStart() {}
    void OnStop() {}
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
