/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_MANAGER_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_MANAGER_H

#include <map>
#include "ability_connect_callback_stub.h"
#include "cpp/mutex.h"
#include "event_report.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
enum class InsightIntentExecuteState {
    UNKNOWN = 0,
    EXECUTING,
    EXECUTE_DONE,
    REMOTE_DIED
};

struct InsightIntentExecuteRecord {
    InsightIntentExecuteState state = InsightIntentExecuteState::UNKNOWN;
    uint64_t key = 0;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    std::string bundleName;
    std::string callerBundleName;
};

class InsightIntentExecuteConnection : public AbilityConnectionStub {
public:
    InsightIntentExecuteConnection() = default;

    ~InsightIntentExecuteConnection() override = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {}

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {}
};

class InsightIntentExecuteRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit InsightIntentExecuteRecipient(uint64_t intentId) : intentId_(intentId)
    {}

    ~InsightIntentExecuteRecipient() override = default;

    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    uint64_t intentId_ = 0;
};

class InsightIntentExecuteManager : public std::enable_shared_from_this<InsightIntentExecuteManager> {
DECLARE_DELAYED_SINGLETON(InsightIntentExecuteManager)
public:
    int32_t CheckAndUpdateParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, std::string callerBundleName = "",
        const bool ignoreAbilityName = false);

    int32_t CheckAndUpdateWant(Want &want, AppExecFwk::ExecuteMode executeMode, std::string callerBundleName = "");

    int32_t RemoveExecuteIntent(uint64_t intentId);

    int32_t ExecuteIntentDone(uint64_t intentId, int32_t resultCode,
        const AppExecFwk::InsightIntentExecuteResult &result);

    int32_t RemoteDied(uint64_t intentId);

    int32_t GetBundleName(uint64_t intentId, std::string &bundleName) const;

    int32_t GetCallerBundleName(uint64_t intentId, std::string &callerBundleName) const;

    static int32_t GenerateWant(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        const AbilityRuntime::ExtractInsightIntentGenericInfo &decoratorInfo,
        Want &want);

    std::map<int32_t, int64_t> GetAllIntentExemptionInfo() const;

    void SetIntentExemptionInfo(int32_t uid);

    bool CheckIntentIsExemption(int32_t uid);

    static int32_t CheckCallerPermission();
private:
    mutable ffrt::mutex mutex_;
    mutable ffrt::mutex intentExemptionLock_;
    uint64_t intentIdCount_ = 0;
    std::map<uint64_t, std::shared_ptr<InsightIntentExecuteRecord>> records_;
    std::map<int32_t, int64_t> intentExemptionDeadlineTime_;

    int32_t AddRecord(uint64_t key, const sptr<IRemoteObject> &callerToken, const std::string &bundleName,
        uint64_t &intentId, const std::string &callerBundleName);

    static int32_t IsValidCall(const Want &want);

    static int32_t AddWantUirsAndFlagsFromParam(
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, Want &want);
    static int32_t CheckAndUpdateDecoratorParams(
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        const AbilityRuntime::ExtractInsightIntentGenericInfo &decoratorInfo,
        Want &want);
    static int32_t UpdateFuncDecoratorParams(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        AbilityRuntime::ExtractInsightIntentInfo &info, Want &want);
    static int32_t UpdatePageDecoratorParams(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        AbilityRuntime::ExtractInsightIntentInfo &info, Want &want);
    static int32_t UpdateEntryDecoratorParams(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        AbilityRuntime::ExtractInsightIntentInfo &info, Want &want);
    static std::string GetMainElementName(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param);

    void SendIntentReport(EventInfo &eventInfo, int32_t errCode);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_MANAGER_H
