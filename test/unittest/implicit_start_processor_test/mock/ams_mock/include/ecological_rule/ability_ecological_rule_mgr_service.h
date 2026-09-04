#ifndef SERVICES_INCLUDE_ECOLOGICAL_RULE_MANAGER_SERVICE_PROXY_H
#define SERVICES_INCLUDE_ECOLOGICAL_RULE_MANAGER_SERVICE_PROXY_H

#include <vector>
#include <string>
#include "iremote_object.h"
#include "want.h"
#include "ability_info.h"
#include "extension_ability_info.h"

namespace OHOS {
namespace EcologicalRuleMgrService {

struct AbilityCallerInfo {
    std::string packageName;
    int32_t uid = 0;
    int32_t pid = 0;
    int32_t targetAppType = 0;
    int32_t callerAppType = 0;

    static constexpr int32_t TYPE_INVALID = 0;
    static constexpr int32_t TYPE_ATOM_SERVICE = 1;
    static constexpr int32_t TYPE_HARMONY_APP = 2;
    static constexpr int32_t LINK_TYPE_UNIVERSAL_LINK = 1;
    static constexpr int32_t LINK_TYPE_DEEP_LINK = 2;
    static constexpr int32_t LINK_TYPE_WEB_LINK = 3;
};

class AbilityEcologicalRuleMgrServiceClient : public RefBase {
public:
    static sptr<AbilityEcologicalRuleMgrServiceClient> GetInstance()
    {
        static sptr<AbilityEcologicalRuleMgrServiceClient> instance = new AbilityEcologicalRuleMgrServiceClient();
        return instance;
    }
    int32_t EvaluateResolveInfos(const AAFwk::Want &want, const AbilityCallerInfo &callerInfo, int32_t type,
        std::vector<AppExecFwk::AbilityInfo> &abInfo,
        const std::vector<AppExecFwk::ExtensionAbilityInfo> &extInfo =
            std::vector<AppExecFwk::ExtensionAbilityInfo>())
    {
        return 0;
    }
};
}  // namespace EcologicalRuleMgrService
}  // namespace OHOS
#endif  // SERVICES_INCLUDE_ECOLOGICAL_RULE_MANAGER_SERVICE_PROXY_H
