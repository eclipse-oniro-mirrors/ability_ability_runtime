#include "ability_manager_service.h"
#include "system_dialog_scheduler.h"
#include "kiosk_manager.h"
#include "utils/multi_app_utils.h"
#include "utils/ability_util.h"
#include "token.h"
#include "ability_record.h"
#include "bundle_mgr_helper.h"
#include "ability_ecological_rule_mgr_service_param.h"
#include "mock_multi_app_utils_status.h"

namespace OHOS {
namespace AAFwk {

std::map<std::string, int32_t> MockMultiAppUtilsStatus::preferredIndexMap_ = {};

void MockMultiAppUtilsStatus::Reset()
{
    preferredIndexMap_.clear();
}

AbilityManagerService::AbilityManagerService() = default;
AbilityManagerService::~AbilityManagerService() = default;

std::shared_ptr<AbilityRecord> Token::GetAbilityRecordByToken(sptr<IRemoteObject> token)
{
    return nullptr;
}

sptr<SessionInfo> AbilityRecord::GetSessionInfo() const
{
    return nullptr;
}

const AppExecFwk::AbilityInfo& AbilityRecord::GetAbilityInfo() const
{
    static AppExecFwk::AbilityInfo info;
    return info;
}

const std::string AbilityUtil::DLP_PARAMS_SANDBOX = "ohos.dlp.params.sandbox";

int SystemDialogScheduler::GetSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &requestWant,
    Want &targetWant, const sptr<IRemoteObject> &callerToken)
{
    return 0;
}

int SystemDialogScheduler::GetPcSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos,
    Want &requestWant, Want &targetWant, const std::string &type, int32_t userId,
    const sptr<IRemoteObject> &callerToken)
{
    return 0;
}

Want SystemDialogScheduler::GetTipsDialogWant(const sptr<IRemoteObject> &callerToken)
{
    return Want();
}

bool MultiAppUtils::GetPreferredAppCloneIndex(const std::string &bundleName, int32_t userId, int32_t &appIndex)
{
    auto it = MockMultiAppUtilsStatus::preferredIndexMap_.find(bundleName);
    if (it != MockMultiAppUtilsStatus::preferredIndexMap_.end()) {
        appIndex = it->second;
        return true;
    }
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
