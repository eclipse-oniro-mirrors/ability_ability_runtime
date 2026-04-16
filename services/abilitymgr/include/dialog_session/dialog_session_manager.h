/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H
#define OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H
#include <list>
#include <unordered_map>
#include <string>
#include "ability_record.h"
#include "cpp/mutex.h"
#include "dialog_session_info.h"
#include "json_serializer.h"
#include "nocopyable.h"
#include "parcel.h"
#include "refbase.h"
#include "system_dialog_scheduler.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
enum class SelectorType {
    WITHOUT_SELECTOR = -1,
    IMPLICIT_START_SELECTOR = 0,
    APP_CLONE_SELECTOR = 1,
    INTERCEPTOR_SELECTOR = 2
};

struct DialogCallerInfo {
    Want targetWant;
    sptr<IRemoteObject> callerToken;
    int32_t userId = -1;
    int requestCode = -1;
    uint32_t callerAccessTokenId = 0;
    SelectorType type = SelectorType::WITHOUT_SELECTOR;
    // for app gallery selector
    bool needGrantUriPermission = false;
    sptr<IRemoteObject> sessionToken = nullptr;
};

struct StartupSessionInfo {
    AbilityRequest abilityRequest;
};

struct QueryERMSInfo {
    int32_t recordId;
    std::string appId;
    std::string startTime;
    bool isEmbeddedAllowed;
};

class DialogSessionManager {
public:
    static DialogSessionManager &GetInstance();
    ~DialogSessionManager() = default;

    /**
     * @brief Get dialog session information by session ID
     * @param dialogSessionId The unique identifier of the dialog session to retrieve.
     * @return sptr<DialogSessionInfo> Smart pointer to the dialog session information,
     *         returns nullptr if the session ID cannot be found.
     */
    sptr<DialogSessionInfo> GetDialogSessionInfo(const std::string &dialogSessionId) const;

    /**
     * @brief Retrieves caller information for a specified dialog session.
     * @param dialogSessionId The unique identifier of the dialog session to retrieve.
     * @return std::shared_ptr<DialogCallerInfo> Shared pointer to caller info if found,
     *         nullptr if the session ID cannot be found.
     */
    std::shared_ptr<DialogCallerInfo> GetDialogCallerInfo(const std::string &dialogSessionId) const;

    /**
     * @brief Retrieves the startup session information based on the provided dialog session ID.
     * @param dialogSessionId The unique identifier of the dialog session to retrieve.
     * @return std::shared_ptr<StartupSessionInfo> Shared pointer to the StartupSessionInfo
     *         if found, nullptr if the session ID cannot be found.
     */
    std::shared_ptr<StartupSessionInfo> GetStartupSessionInfo(const std::string &dialogSessionId) const;

    /**
     * @brief Processes and sends the result of a dialog session to the caller.
     * @param want The Want object containing target ability information
     * @param dialogSessionId The unique identifier of the dialog session to retrieve.
     * @param isAllowed Boolean flag indicating whether user granted permission.
     * @return int ERR_OK on success, or specific error codes for failure cases.
     */
    int SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllowed);

    /**
     * @brief Creates a modal dialog for ability jumping with specified parameters.
     * @param abilityRequest The AbilityRequest object containing target ability information.
     * @param userId The user ID under which the ability operation is performed.
     * @param replaceWant The Want object specifying replacement parameters for the jump.
     * @return int ERR_OK on success, or specific error codes for failure cases.
     */
    int CreateJumpModalDialog(AbilityRequest &abilityRequest, int32_t userId, const Want &replaceWant);

    /**
     * @brief Creates a modal dialog for implicit ability selection with selector functionality.
     * @param abilityRequest The AbilityRequest object containing target ability information.
     * @param want The Want object containing target ability information
     * @param userId The user ID under which the ability operation is performed.
     * @param dialogAppInfos Output parameter containing collected application information.
     * @param needGrantUriPermission Flag indicating whether URI permission granting is required.
     * @return int ERR_OK on success, or specific error codes for failure cases.
     */
    int CreateImplicitSelectorModalDialog(AbilityRequest &abilityRequest, const Want &want, int32_t userId,
        std::vector<DialogAppInfo> &dialogAppInfos, bool needGrantUriPermission = false);

    /**
     * @brief Creates a modal dialog for application clone selection during ability invocation.
     * @param abilityRequest The AbilityRequest object containing target ability information.
     * @param want The original invocation parameters for the clone operation.
     * @param userId The user ID under which the ability operation is performed.
     * @param dialogAppInfos Output parameter containing collected application information.
     * @param replaceWant Optional string parameter specifying the replacement Want for ecological scenarios.
     * @return int ERR_OK on success, or specific error codes for failure cases.
     */
    int CreateCloneSelectorModalDialog(AbilityRequest &abilityRequest, const Want &want, int32_t userId,
        std::vector<DialogAppInfo> &dialogAppInfos, const std::string &replaceWant);

    int HandleErmsResult(AbilityRequest &abilityRequest, int32_t userId, const Want &replaceWant);

    int32_t HandleErmsResultBySCB(AbilityRequest &abilityRequest, const Want &replaceWant);

    bool IsCreateCloneSelectorDialog(const std::string &bundleName, int32_t userId);

    bool UpdateExtensionWantWithDialogCallerInfo(AbilityRequest &abilityRequest,
        const sptr<IRemoteObject> &callerToken, bool isSCBCall);

    std::string GenerateDialogSessionId();

    void OnlySetDialogCallerInfo(AbilityRequest &abilityRequest, int32_t userId, SelectorType type,
        const std::string &dialogSessionId, bool needGrantUriPermission);
private:
    DialogSessionManager() = default;

    void SetDialogSessionInfo(const std::string &dialogSessionId, sptr<DialogSessionInfo> &dilogSessionInfo,
        std::shared_ptr<DialogCallerInfo> &dialogCallerInfo);

    void SetStartupSessionInfo(const std::string &dialogSessionId, const AbilityRequest &abilityRequest);

    int32_t NotifySCBToRecoveryAfterInterception(const std::string &dialogSessionId,
        const AbilityRequest &abilityRequest);

    void ClearDialogContext(const std::string &dialogSessionId);

    void ClearAllDialogContexts();

    std::string GenerateDialogSessionRecordCommon(AbilityRequest &abilityRequest, int32_t userId,
        const AAFwk::WantParams &parameters, std::vector<DialogAppInfo> &dialogAppInfos, SelectorType type,
        bool needGrantUriPermission = false);

    void GenerateCallerAbilityInfo(AbilityRequest &abilityRequest, DialogAbilityInfo &callerAbilityInfo);

    void GenerateSelectorTargetAbilityInfos(std::vector<DialogAppInfo> &dialogAppInfos,
        std::vector<DialogAbilityInfo> &targetAbilityInfos);
    
    void GenerateJumpTargetAbilityInfos(AbilityRequest &abilityRequest,
        std::vector<DialogAbilityInfo> &targetAbilityInfos);

    void GenerateDialogCallerInfo(AbilityRequest &abilityRequest, int32_t userId,
        std::shared_ptr<DialogCallerInfo> dialogCallerInfo, SelectorType type,
        bool needGrantUriPermission = false);

    int CreateModalDialogCommon(const Want &replaceWant, sptr<IRemoteObject> callerToken,
        const std::string &dialogSessionId);

    void SetQueryERMSInfo(const std::string &dialogSessionId, const AbilityRequest &abilityRequest);

    bool NotifyQueryERMSFinished(const std::string &dialogSessionId, bool isAllowed);

    void NotifyAbilityRequestFailure(const std::string &dialogSessionId, const Want &want);

    void HandleUserRejected(const std::string &dialogSessionId, const Want &want,
        const std::shared_ptr<DialogCallerInfo> &dialogCallerInfo);

    mutable ffrt::mutex dialogSessionRecordLock_;
    std::unordered_map<std::string, sptr<DialogSessionInfo>> dialogSessionInfoMap_;
    std::unordered_map<std::string, std::shared_ptr<DialogCallerInfo>> dialogCallerInfoMap_;
    std::unordered_map<std::string, std::shared_ptr<StartupSessionInfo>> startupSessionInfoMap_;

    ffrt::mutex queryERMSInfoLock_;
    std::unordered_map<std::string, QueryERMSInfo> queryERMSInfoMap_;

    DISALLOW_COPY_AND_MOVE(DialogSessionManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H
