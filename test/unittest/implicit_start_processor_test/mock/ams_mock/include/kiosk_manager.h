#ifndef OHOS_ABILITY_RUNTIME_KIOSK_MANAGER_H
#define OHOS_ABILITY_RUNTIME_KIOSK_MANAGER_H

#include <vector>
#include "ability_info.h"

namespace OHOS {
namespace AAFwk {
struct DialogAppInfo;
class KioskManager {
public:
    static KioskManager &GetInstance()
    {
        static KioskManager instance;
        return instance;
    }
    void FilterAbilityInfos(std::vector<AppExecFwk::AbilityInfo> &abilityInfos) {}
    void FilterDialogAppInfos(std::vector<DialogAppInfo> &dialogAppInfos) {}
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_KIOSK_MANAGER_H
