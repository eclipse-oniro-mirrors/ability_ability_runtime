#ifndef OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H
#define OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H

#include <string>
#include <vector>
#include "want.h"
#include "iremote_object.h"
#include "system_dialog_scheduler.h"

namespace OHOS {
namespace AAFwk {

class DialogSessionManager {
public:
    static DialogSessionManager &GetInstance()
    {
        static DialogSessionManager instance;
        return instance;
    }

    int CreateImplicitSelectorModalDialog(const AbilityRequest &request, Want &want, int32_t userId,
        std::vector<DialogAppInfo> &dialogAppInfos, bool needGrantUriPermission)
    {
        return ERR_OK;
    }

    int CreateImplicitSelectorModalDialog(const AbilityRequest &request, Want &want, int32_t userId,
        std::vector<DialogAppInfo> &dialogAppInfos)
    {
        return ERR_OK;
    }

    int CreateCloneSelectorModalDialog(const AbilityRequest &request, Want &want, int32_t userId,
        std::vector<DialogAppInfo> &dialogAppInfos, const std::string &replaceWantString)
    {
        return ERR_OK;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H
