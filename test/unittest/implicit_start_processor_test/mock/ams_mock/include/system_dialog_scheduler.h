#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_DIALOG_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_DIALOG_SCHEDULER_H

#include <string>
#include <vector>
#include "singleton.h"
#include "want.h"
#include "iremote_object.h"
#include "application_info.h"

namespace OHOS {
namespace AAFwk {

struct DialogPosition {
    int32_t window_width = 0;
    int32_t window_height = 0;
    int32_t window_offsetX = 0;
    int32_t window_offsetY = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t width_narrow = 0;
    int32_t height_narrow = 0;
    int32_t offsetX = 0;
    int32_t offsetY = 0;
    bool wideScreen = true;
    bool oversizeHeight = false;
};

struct DialogAppInfo {
    int32_t abilityIconId = 0;
    int32_t abilityLabelId = 0;
    int32_t bundleIconId = 0;
    int32_t bundleLabelId = 0;
    std::string bundleName = {};
    std::string abilityName = {};
    std::string moduleName = {};
    std::string codePath = "";
    std::string installSource = "";
    bool visible = true;
    bool isAppLink = false;
    AppExecFwk::MultiAppModeData multiAppMode;
    int32_t appIndex = 0;
};

enum class DialogType {
    SELECTOR_DIALOG = 0,
    TIPS_DIALOG,
    SWITCH_USER_DIALOG,
    ASSERT_FAULT_DIALOG,
};

class SystemDialogScheduler : public DelayedSingleton<SystemDialogScheduler> {
public:
    explicit SystemDialogScheduler() = default;
    virtual ~SystemDialogScheduler() = default;
    int GetSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &requestWant,
        Want &targetWant, const sptr<IRemoteObject> &callerToken);
    int GetPcSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &requestWant,
        Want &targetWant, const std::string &type, int32_t userId, const sptr<IRemoteObject> &callerToken);
    Want GetTipsDialogWant(const sptr<IRemoteObject> &callerToken);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SYSTEM_DIALOG_SCHEDULER_H
