#ifndef OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H
#define OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H

#include <vector>
#include <string>
#include <memory>
#include "iremote_object.h"
#include "ability_info.h"
#include "want.h"
#include "start_specified_ability_params.h"
#include "irequest_start_ability_callback.h"

namespace OHOS {
namespace AAFwk {

struct StartAbilityWrapParam {
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = -1;
    bool isPendingWantCaller = false;
    int32_t userId = -1;
    bool isStartAsCaller = false;
    uint32_t specifyTokenId = 0;
    bool isForegroundToRestartApp = false;
    bool isImplicit = false;
    bool isUIAbilityOnly = false;
    bool isAppCloneSelector = false;
    bool hideFailureTipDialog = false;
    bool isBySCB = false;
    bool isFreeInstallFromService = false;
    bool removeInsightIntentFlag = false;
    bool isFromOpenLink = false;
    uint64_t specifiedFullTokenId = 0;
    std::shared_ptr<StartSpecifiedAbilityParams> startSpecifiedParams = nullptr;
    std::string hostBundleName;
    bool isStartByOEExt = false;
    std::string specifiedFlag;
    bool isGamePrelaunch = false;
    sptr<IRequestStartAbilityCallback> requestCallback = nullptr;
};

class StartAbilityUtils {
public:
    static bool IsCallFromAncoShellOrBroker(const sptr<IRemoteObject> &callerToken) { return false; }
    static int HandleSelfRedirection(bool isFromOpenLink, std::vector<AppExecFwk::AbilityInfo> &abilityInfos)
    {
        return ERR_OK;
    }
    static std::vector<int32_t> GetCloneAppIndexes(const std::string &bundleName, int32_t userId) { return {}; }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H
