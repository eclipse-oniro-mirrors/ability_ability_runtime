/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_MANAGER_H
#define OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_MANAGER_H

#include <map>
#include "bundle_skill/skill_info.h"
#include "global_constant.h"
#include "bundle_skill/skill_manager_interface.h"
#include "cpp/mutex.h"
#include "extension_ability_info.h"
#include "iremote_object.h"
#include "singleton.h"
#include "skill/skill_execute_param.h"
#include "skill/skill_execute_record.h"
#include "skill/skill_execute_result.h"

namespace OHOS {
namespace AAFwk {

class SkillExecuteManager {
DECLARE_DELAYED_SINGLETON(SkillExecuteManager)
public:
    int32_t GenerateSkillWant(const AppExecFwk::SkillInfo &skillInfo, Want &want,
        int32_t userId, const std::string &requestCode, AppExecFwk::ExtensionAbilityType &targetType,
        const std::string &scriptPath = "", const std::string &functionName = "",
        const std::shared_ptr<AAFwk::WantParams> &skillArgs = nullptr);

    int32_t QuerySkillInfo(const std::string &bundleName, const std::string &moduleName,
        const std::string &skillName, int32_t userId, AppExecFwk::SkillInfo &skillInfo);

    int32_t CheckSkillPermission(const AppExecFwk::SkillInfo &skillInfo);

    std::string CreateExecuteRecord(const sptr<IRemoteObject> &callerToken,
        const std::string &targetBundleName, const std::string &callerBundleName,
        uint32_t callerTokenId,
        const sptr<ISkillExecuteCallback> &callback = nullptr,
        const std::string &externalRequestCode = "");

    int32_t ExecuteSkillDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result,
        const std::string &callerBundleName);

    void OnTimeout(int64_t requestCodeSeq);

private:
    class CallerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        CallerDeathRecipient(std::function<void(const std::string &)> callback, std::string requestCode)
            : callback_(std::move(callback)), requestCode_(std::move(requestCode)) {}
        void OnRemoteDied(const wptr<IRemoteObject> &object) override
        {
            if (callback_ != nullptr) {
                callback_(requestCode_);
            }
        }
    private:
        std::function<void(const std::string &)> callback_;
        std::string requestCode_;
    };

    sptr<AppExecFwk::IBundleSkillManager> GetSkillManagerProxy();
    std::string ResolveDefaultAbilityName(const std::string &bundleName,
        const std::string &moduleName, int32_t userId);
    AppExecFwk::ExtensionAbilityType ResolveTargetType(const std::string &bundleName,
        const std::string &moduleName, const std::string &abilityName, int32_t userId);
    void RemoveRecord(const std::string &requestCode);
    void OnCallerDied(const std::string &requestCode);
    void PostSkillExecuteTimeout(const std::string &requestCode, uint64_t requestCodeSeq);
    void RemoveSkillExecuteTimeoutLocked(uint64_t requestCodeSeq);

    ffrt::mutex mutex_;
    uint64_t requestCodeSeq_ = 0;
    std::map<std::string, std::shared_ptr<SkillExecuteRecord>> records_;
    std::map<uint64_t, std::string> seqToRequestCodeMap_;
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_MANAGER_H
