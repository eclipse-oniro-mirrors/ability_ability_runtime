/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <map>
#include <mutex>
#include <sys/types.h>

#include "ability_connect_callback_stub.h"
#include "cpp/mutex.h"
#include "event_report.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_execution_info.h"
#include "insight_intent_query_param.h"
#include "insight_intent_will_execute_callback_interface.h"
#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
// Mirror of AttributionTool::INSIGHT in services/appmgr/include/task_mgr/attribution_record.h.
// The enum is not exported across services, keep values append-only and in sync;
// thirteenth_test pins this mirror against the enum with a static_assert.
static constexpr int32_t ATTRIBUTION_TOOL_INSIGHT = 1;

enum class InsightIntentExecuteState {
    UNKNOWN = 0,
    // Pre-registered by the consent pre-check; only a grant moves it forward, a
    // deny or timeout drops the record.
    PENDING_CONSENT,
    EXECUTING,
    EXECUTE_DONE,
    REMOTE_DIED
};

struct InsightIntentExecuteRecord {
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    uint64_t key = 0;
    std::string bundleName;           // target bundle name
    std::string callerBundleName;     // caller bundle name
    uint32_t callerTokenId = 0;       // caller access token id (IPC caller identity)
    AppExecFwk::InsightIntentExecutionInfo executionInfo;
    pid_t targetPid = 0;
    InsightIntentExecuteState state = InsightIntentExecuteState::UNKNOWN;
    bool isDistributed = false;
    std::string deviceId;
    uint64_t requestCode = 0;
    std::string taskId;
    // Attribution id delegated to TaskManagerService via AppScheduler IPC (0 means not registered).
    int64_t attributionId = 0;
    // Bundle uid of the caller that supplied the task id, used for ownership check on IPC delegation.
    int32_t callerUid = 0;
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

struct ExecuteParam {
    std::string callerBundleName;
    bool ignoreAbilityName = false;
    bool isDistributed = false;
    std::string srcDeviceId;
    uint64_t requestCode = 0;
    uint64_t specifiedFullTokenId = 0;
};

class InsightIntentExecuteManager : public std::enable_shared_from_this<InsightIntentExecuteManager> {
    DECLARE_DELAYED_SINGLETON(InsightIntentExecuteManager)
public:
    int32_t CheckAndUpdateParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        const ExecuteParam &execParam = ExecuteParam{});

    // Public invoke entry (invokePublicFunction) counterpart of CheckAndUpdateParam:
    // runs the exported + ACCESS_PUBLIC_FUNCTION visibility pre-condition and the
    // consent grant gate, then performs the same validation and record acquisition
    // as the system entry. Kept separate so the system entry keeps its fail-closed
    // CheckCallerPermission (ERR_NOT_SYSTEM_APP for non-system callers).
    int32_t CheckAndUpdatePublicParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        const ExecuteParam &execParam = ExecuteParam{});

    // Reuse when insightIntentId_ != 0: same bundle in PENDING_CONSENT (promote) or
    // EXECUTING (reuse); otherwise AddRecord allocates a fresh id. No validation here.
    int32_t AcquireExecuteRecord(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        const ExecuteParam &execParam);

    // Pre-register in PENDING_CONSENT (agent path): queryable while the consent dialog
    // shows, later reused via param->insightIntentId_.
    int32_t RegisterExecuteRecord(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const std::string &bundleName, const ExecuteParam &execParam, uint64_t &intentId);

    int32_t CheckAndUpdateWant(Want &want, AppExecFwk::ExecuteMode executeMode, int32_t userId,
        std::string callerBundleName = "");

    int32_t RemoveExecuteIntent(uint64_t intentId);

    // Ownership-gated removal: drop only when the current caller token id matches the
    // token id captured at pre-check; a missing record is a no-op.
    int32_t RemoveExecuteRecordIfCaller(uint64_t intentId);

    int32_t ExecuteIntentDone(uint64_t intentId, int32_t resultCode,
        const AppExecFwk::InsightIntentExecuteResult &result, int32_t callerUid = 0, uint32_t accessToken = 0);

    int32_t GetDistributedInfo(uint64_t intentId, std::string &srcDeviceId,
        int32_t &requestCode, uint64_t &callbackKey) const;

    int32_t RemoteDied(uint64_t intentId);

    int32_t GetBundleName(uint64_t intentId, std::string &bundleName) const;

    int32_t GetCallerBundleName(uint64_t intentId, std::string &callerBundleName) const;

    // Returns the caller bundle name of a live (PENDING_CONSENT / EXECUTING) record,
    // or an empty string otherwise.
    std::string GetInsightIntentCallerBundleName(uint64_t intentId) const;

    // CAS-style state transition: moves the record to `next` only when it currently
    // sits in `expected`. Returns ERR_OK when the transition happened.
    int32_t UpdateRecordState(uint64_t intentId, InsightIntentExecuteState expected,
        InsightIntentExecuteState next);

    static int32_t GenerateWant(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        const AbilityRuntime::ExtractInsightIntentGenericInfo &decoratorInfo,
        AbilityRuntime::ExtractInsightIntentInfo &extractInfo,
        Want &want);

    std::map<int32_t, int64_t> GetAllIntentExemptionInfo() const;

    void SetIntentExemptionInfo(int32_t uid);

    bool CheckIntentIsExemption(int32_t uid);

    static int32_t CheckCallerPermission(uint64_t specifiedFullTokenId = 0,
        const std::string &callerBundleName = "",
        const std::string &targetBundleName = "");

    // Third-party gate: the target intent must be exported, and the caller must
    // hold ACCESS_PUBLIC_FUNCTION. A private target is reported with the same code
    // as a missing one (INTENT_NOT_EXIST) so its existence is not leaked.
    static int32_t CheckExportedIntentCallerPermission(const std::string &targetBundleName,
        const std::string &targetModuleName, const std::string &targetIntentName,
        int32_t userId);

    // Consent grant gate: requires the calling controller to hold
    // AutomationControlAccess NO_POPUP_NEEDED_GRANTED for the target right before
    // dispatch. Applies to every caller that takes the public invoke path
    // (invokePublicFunction), system app and third-party alike. The
    // exported/ACCESS_PUBLIC_FUNCTION check is only a visibility pre-condition;
    // without this the caller could reach ExecuteIntentByFunctionCall directly over
    // IPC and bypass the client-side consent dialog. The automation control access
    // state is the objective evidence that the user authorized it.
    static int32_t CheckConsentGranted(const std::string &targetBundleName);

    // Query the exported flag of the target intent profile (dual-mode storage:
    // config info first, extract info fallback); false when not found / private.
    static bool GetInsightIntentExported(const std::string &targetBundleName,
        const std::string &targetModuleName, const std::string &targetIntentName,
        int32_t userId);

    static int32_t CheckGetInsightIntenInfoPermission(
        const std::string &targetBundleName = "");

    static std::string GetCallerBundleNameByUid();

    void OnInsightAppDied(const std::string &bundleName, pid_t pid = 0);

    void SetTargetPid(uint64_t intentId, pid_t pid);

    int32_t CheckAndUpdateQueryEntityParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
        std::shared_ptr<OHOS::AppExecFwk::InsightIntentQueryParam> &param, const std::string &callerBundleName);

    static int32_t GenerateQueryEntityWant(const std::shared_ptr<AppExecFwk::InsightIntentQueryParam> &param,
        Want &want);

    int32_t RegisterWillInsightIntentExecuteCallback(pid_t pid,
        const sptr<IRemoteObject> &callback);
    int32_t UnregisterWillInsightIntentExecuteCallback(pid_t pid);
    void NotifyInsightIntentWillExecute(pid_t pid,
        const AppExecFwk::InsightIntentExecutionInfo &info);
    void RemoveInsightIntentWillExecuteCallback(pid_t pid);
    void SetInsightIntentExecutionInfo(uint64_t intentId,
        const AppExecFwk::InsightIntentExecutionInfo &info);
private:
    static void BuildQueryEntityWantParams(Want &want,
        const AppExecFwk::InsightIntentQueryParam &param);
    static void SetQueryEntityWantExtras(Want &want,
        const AppExecFwk::InsightIntentQueryParam &param,
        const AbilityRuntime::ExtractInsightIntentInfo &intentInfo);
    static int32_t ResolveSrcEntry(Want &want,
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        const AbilityRuntime::ExtractInsightIntentGenericInfo &decoratorInfo);
    static void SetInsightIntentExecuteParam(Want &want,
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param);
    static void SetInsightIntentBaseParams(Want &want,
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param);
    mutable ffrt::mutex mutex_;
    mutable ffrt::mutex intentExemptionLock_;
    uint64_t intentIdCount_ = 0;
    std::map<uint64_t, std::shared_ptr<InsightIntentExecuteRecord>> records_;
    std::map<int32_t, int64_t> intentExemptionDeadlineTime_;

    std::mutex insightIntentWillExecuteMutex_;
    std::map<pid_t, sptr<IInsightIntentWillExecuteCallback>> insightIntentWillExecuteCallbacks_;

    int32_t AddRecord(uint64_t key, const sptr<IRemoteObject> &callerToken, const std::string &bundleName,
        uint64_t &intentId, const ExecuteParam &execParam = ExecuteParam{}, const std::string &taskId = "",
        InsightIntentExecuteState initialState = InsightIntentExecuteState::EXECUTING, int32_t callerUid = 0);

    static int32_t IsValidCall(const Want &want);

    static int32_t AddWantUirsAndFlagsFromParam(
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, Want &want);
    static int32_t CheckAndUpdateDecoratorParams(
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        AbilityRuntime::ExtractInsightIntentInfo &extractInfo,
        Want &want);
    static int32_t UpdateFuncDecoratorParams(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        AbilityRuntime::ExtractInsightIntentInfo &info, Want &want);
    static int32_t UpdatePageDecoratorParams(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        AbilityRuntime::ExtractInsightIntentInfo &info, Want &want);
    static int32_t UpdateEntryDecoratorParams(const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param,
        AbilityRuntime::ExtractInsightIntentInfo &info, Want &want);
    static int32_t UpdateEntryDecoratorParams(Want &want, AppExecFwk::ExecuteMode executeMode, int32_t userId);
    static std::string GetAbilityNameFromMatched(
        AbilityRuntime::ExtractInsightIntentGenericInfo &info);
    static std::string GetMainElementName(
        const std::string &bundleName, const std::string &moduleName, int32_t userId = -1);
    static std::shared_ptr<AbilityRuntime::InsightIntentEntityInfo> CheckEntityQueryable(
        const AbilityRuntime::ExtractInsightIntentInfo& intentInfo, const std::string& className,
        const AppExecFwk::InsightIntentQueryEntityParam& queryParams);

    void SendIntentReport(EventInfo &eventInfo, int32_t errCode);

    void UnregisterAttributedProcess(const std::string &taskId, int64_t attributionId, int32_t callerUid);

    void RegisterAttributedProcess(const std::string &taskId, pid_t pid, int32_t callerUid, int64_t &attributionId);

    void HandleDiedRecordsCleanup(const std::vector<uint64_t> &toRemove,
        const AppExecFwk::InsightIntentExecuteResult &errorResult);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_MANAGER_H
