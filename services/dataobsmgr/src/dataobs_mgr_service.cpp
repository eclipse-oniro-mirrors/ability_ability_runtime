/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "dataobs_mgr_service.h"

#include <cstdint>
#include <memory>

#include "ability_connect_callback_stub.h"
#include "ability_manager_interface.h"
#include "ability_manager_proxy.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_helper.h"
#include "dataobs_mgr_errors.h"
#include "data_share_permission.h"
#include "datashare_log.h"
#include "dataobs_mgr_inner_common.h"
#include "datashare_errno.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#include "common_utils.h"
#include "securec.h"
#ifdef SCENE_BOARD_ENABLE
#include "window_manager_lite.h"
#else
#include "window_manager.h"
#endif

namespace OHOS {
namespace AAFwk {
using namespace DataShare;
using namespace Security::AccessToken;
using namespace AppExecFwk;
static constexpr const char *DIALOG_APP = "com.ohos.pasteboarddialog";
static constexpr const char *PROGRESS_ABILITY = "PasteboardProgressAbility";
static constexpr const char *PROMPT_TEXT = "PromptText_PasteBoard_Local";
static const int32_t CACHE_SIZE_THRESHOLD = 20;
static const int32_t DATA_MANAGER_SERVICE_UID = 3012;

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DataObsMgrService>::GetInstance().get());

DataObsMgrService::DataObsMgrService()
    : SystemAbility(DATAOBS_MGR_SERVICE_SA_ID, true),
      state_(DataObsServiceRunningState::STATE_NOT_START)
{
    dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
    dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();
    dataObsMgrInnerPref_ = std::make_shared<DataObsMgrInnerPref>();
    permission_ = std::make_shared<DataShare::DataSharePermission>();
}

DataObsMgrService::~DataObsMgrService()
{}

void DataObsMgrService::OnStart()
{
    if (state_ == DataObsServiceRunningState::STATE_RUNNING) {
        TAG_LOGI(AAFwkTag::DBOBSMGR, "dms started");
        return;
    }
    if (!Init()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "init failed");
        return;
    }
    state_ = DataObsServiceRunningState::STATE_RUNNING;
    /* Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program */
    if (!Publish(DelayedSingleton<DataObsMgrService>::GetInstance().get())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "publish init failed");
        return;
    }

    TAG_LOGI(AAFwkTag::DBOBSMGR, "dms called");
}

bool DataObsMgrService::Init()
{
    handler_ = TaskHandlerWrap::GetFfrtHandler();
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    return true;
}

void DataObsMgrService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOG_INFO("add system abilityid:%{public}d", systemAbilityId);
    (void)deviceId;
    if (permission_ == nullptr) {
        return;
    }
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        permission_->SubscribeCommonEvent();
    }
    return;
}

void DataObsMgrService::OnStop()
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "stop");
    handler_.reset();
    state_ = DataObsServiceRunningState::STATE_NOT_START;
}

DataObsServiceRunningState DataObsMgrService::QueryServiceState() const
{
    return state_;
}

std::pair<bool, struct ObserverNode> DataObsMgrService::ConstructObserverNode(sptr<IDataAbilityObserver> dataObserver,
    int32_t userId, uint32_t tokenId, int32_t pid)
{
    if (userId == -1) {
        userId = GetCallingUserId(tokenId);
    }
    if (userId == -1) {
        // return false, tokenId default 0
        return std::make_pair(false, ObserverNode(dataObserver, userId, 0, pid));
    }
    return std::make_pair(true, ObserverNode(dataObserver, userId, tokenId, pid));
}

int32_t DataObsMgrService::GetCallingUserId(uint32_t tokenId)
{
    auto type = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (type == Security::AccessToken::TOKEN_NATIVE || type == Security::AccessToken::TOKEN_SHELL) {
        return 0;
    } else {
        Security::AccessToken::HapTokenInfo tokenInfo;
        auto result = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, tokenInfo);
        if (result != Security::AccessToken::RET_SUCCESS) {
            TAG_LOGE(AAFwkTag::DBOBSMGR, "token:0x%{public}x, result:%{public}d", tokenId, result);
            return -1;
        }
        return tokenInfo.userID;
    }
}

bool DataObsMgrService::IsSystemApp(uint32_t tokenId, uint64_t fullTokenId)
{
    Security::AccessToken::ATokenTypeEnum tokenType =
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return false;
    }
    // IsSystemAppByFullTokenID here is not IPC
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Not system app, token:%{public}" PRIx64 "", fullTokenId);
        return false;
    }
    return true;
}

bool DataObsMgrService::IsDataMgrService(uint32_t tokenId, int32_t uid)
{
    Security::AccessToken::ATokenTypeEnum tokenType =
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        return false;
    }
    if (uid != DATA_MANAGER_SERVICE_UID) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "request not from DataMgr, uid %{public}d, DataMgr %{public}d",
            uid, DATA_MANAGER_SERVICE_UID);
        return false;
    }
    return true;
}

// Check wheather calling process has system permission, with some providers who allow calls from non-system app.
// Existing function IsCallingPermissionValid checks based on an option passed from client, which is not reliable
// in some situation. Try use this function for system permission checking instead.
bool DataObsMgrService::IsSystemAppWithAllowedProvider(uint32_t callerTokenId, uint64_t fullTokenId,
    const std::string &callingFunc, const Uri &uri, bool isExtension)
{
    if (DataShare::DataSharePermission::IsUriPathSegmentAllowed(uri)) {
        // allowed provider
        return true;
    }
    Security::AccessToken::ATokenTypeEnum tokenType =
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerTokenId);
    // Do not report system ability
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        return true;
    }

    // Do not report system hap
    if (IsSystemApp(callerTokenId, fullTokenId)) {
        return true;
    }
    // mask uri using interval of 2
    std::string uriAnis = CommonUtils::AlternateMask(uri.ToString(), 2);
    std::string appendix = callingFunc + "isExtension:" + std::to_string(isExtension);
    TAG_LOGE(AAFwkTag::DBOBSMGR, "normal app calling uri:%{public}s,"
        "fullToken %{public}" PRId64, uriAnis.c_str(), fullTokenId);
    DataShare::DataSharePermission::ReportExtensionFault(E_NOT_SYSTEM_APP, callerTokenId, uriAnis, appendix);
    return false;
}

bool DataObsMgrService::IsCallingPermissionValid(DataObsOption &opt)
{
    if (opt.IsSystem()) {
        uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
        uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
        bool isValid = IsSystemApp(tokenId, fullTokenId);
        if (!isValid) {
            TAG_LOGE(AAFwkTag::DBOBSMGR, "CallingPermission invalid, token %{public}d", tokenId);
            return false;
        }
    }
    return true;
}

bool DataObsMgrService::IsCallingPermissionValid(DataObsOption &opt, int32_t userId, int32_t callingUserId)
{
    if (callingUserId < 0) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "invalid userId %{public}d", callingUserId);
        return false;
    }
    bool acrossUser = false;
    if (userId == DATAOBS_DEFAULT_CURRENT_USER || userId == callingUserId) {
        acrossUser = false;
    } else {
        acrossUser = true;
    }
    
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    int32_t uid  = IPCSkeleton::GetCallingUid();
    bool isValid = true;
    if (acrossUser) {
        isValid = IsSystemApp(tokenId, fullTokenId) || IsDataMgrService(tokenId, uid);
    } else if (opt.IsSystem()) {
        isValid = IsSystemApp(tokenId, fullTokenId);
    }
    if (!isValid) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "CallingPermission invalid, token %{public}d, from %{public}d to %{public}d",
            tokenId, callingUserId, userId);
        return false;
    }
    return true;
}

int32_t DataObsMgrService::RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    int32_t userId, DataObsOption opt)
{
    if (dataObserver == nullptr) {
        LOG_ERROR("null dataObserver, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        LOG_ERROR("null dataObsMgrInner, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    return RegisterObserverInner(uri, dataObserver, userId, opt, false);
}

int32_t DataObsMgrService::RegisterObserverFromExtension(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    int32_t userId, DataObsOption opt)
{
    if (dataObserver == nullptr) {
        LOG_ERROR("null dataObserver, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        LOG_ERROR("null dataObsMgrInner, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    opt.SetDataShare(true);
    return RegisterObserverInner(uri, dataObserver, userId, opt, true);
}

int32_t DataObsMgrService::VerifyDataShareExtension(Uri &uri, ObserverInfo &info)
{
    std::string uriStr = uri.ToString();
    uint32_t tokenId = info.tokenId;
    uint64_t fullTokenId = info.fullTokenId;
    int ret;
    bool isExtension = info.isFromExtension;
    if (isExtension) {
        ret = DataShare::DataSharePermission::IsExtensionValid(tokenId, fullTokenId, info.callingUserId);
        if (ret != DataShare::E_OK) {
            info.errMsg.append(std::to_string(info.isFromExtension) + "_IsExtensionValid");
            TAG_LOGE(AAFwkTag::DBOBSMGR, "IsExtensionValid failed, uri:%{public}s, ret %{public}d,"
                "fullToken %{public}" PRId64 " msg %{public}s", uriStr.c_str(), ret, fullTokenId, info.errMsg.c_str());
            DataShare::DataSharePermission::ReportExtensionFault(ret, tokenId, uriStr, info.errMsg);
            return ret;
        }
    }
    return DataShare::E_OK;
}

// just hisysevent now
int32_t DataObsMgrService::VerifyDataSharePermission(Uri &uri, bool isRead, ObserverInfo &info)
{
    int32_t ret = VerifyDataShareExtension(uri, info);
    if (ret != 0) {
        return ret;
    }
    return VerifyDataSharePermissionInner(uri, isRead, info);
}

std::pair<Status, std::string> DataObsMgrService::GetUriPermission(Uri &uri, bool isRead, ObserverInfo &info)
{
    uint32_t tokenId = info.tokenId;
    std::string uriStr = uri.ToString();
    if (permission_ == nullptr) {
        LOG_ERROR("permission_ nullptr");
        return std::make_pair(COMMON_ERROR, "");
    }
    auto [ret, permission] = permission_->GetUriPermission(uri, info.userId, isRead, info.isSilentUri);
    if (ret != DataShare::E_OK) {
        info.errMsg.append(std::to_string(info.isFromExtension) + "_GetUriPermission");
        TAG_LOGE(AAFwkTag::DBOBSMGR, "GetUriPermission failed, uri:%{public}s,token %{public}d pid %{public}d",
            uriStr.c_str(), tokenId, info.pid);
        DataShare::DataSharePermission::ReportExtensionFault(ret, tokenId, uriStr, info.errMsg);
        return std::make_pair(DATAOBS_INVALID_URI, permission);
    }
    return std::make_pair(SUCCESS, permission);
}

Status DataObsMgrService::VerifyDataSharePermissionInner(Uri &uri, bool isRead, ObserverInfo &info)
{
    std::string uriStr = uri.ToString();
    uint32_t tokenId = info.tokenId;
    uint64_t fullTokenId = info.fullTokenId;
    int ret;
    bool isExtension = info.isFromExtension;
    if (permission_ == nullptr) {
        LOG_ERROR("permission_ nullptr");
        return COMMON_ERROR;
    }
    std::tie(ret, info.permission) = GetUriPermission(uri, isRead, info);
    if (ret != DataShare::E_OK) {
        return DATAOBS_INVALID_URI;
    }
    uint32_t verifyToken = isExtension ? info.firstCallerTokenId : tokenId;
    if (!DataShare::DataSharePermission::VerifyPermission(uri, verifyToken, info.permission, info.isSilentUri)) {
        info.errMsg.append(std::to_string(info.isFromExtension) + "_VerifyPermission");
        TAG_LOGE(AAFwkTag::DBOBSMGR, "VerifyPermission failed, uri:%{public}s, isExtension %{public}d,"
            "token %{public}d pid %{public}d isRead %{public}d", uriStr.c_str(), isExtension,
            verifyToken, info.pid, isRead);
        DataShare::DataSharePermission::ReportExtensionFault(ret, tokenId, uriStr, info.errMsg);
        return DATAOBS_PERMISSION_DENY;
    }
    return SUCCESS;
}

int32_t DataObsMgrService::ConstructRegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    uint32_t token, int32_t userId, int32_t pid)
{
    auto [success, observerNode] = ConstructObserverNode(dataObserver, userId, token, pid);
    if (!success) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "ConstructObserverNode fail, uri:%{public}s, userId:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), userId);
        return DATAOBS_INVALID_USERID;
    }

    if (const_cast<Uri &>(uri).GetScheme() == SHARE_PREFERENCES) {
        return dataObsMgrInnerPref_->HandleRegisterObserver(uri, observerNode);
    }
    return dataObsMgrInner_->HandleRegisterObserver(uri, observerNode);
}

int32_t DataObsMgrService::RegisterObserverInner(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    int32_t userId, DataObsOption opt, bool isExtension)
{
    uint32_t callingToken = IPCSkeleton::GetCallingTokenID();
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    int32_t callingUserId = GetCallingUserId(callingToken);
    if (callingUserId < 0) {
        return DATAOBS_INVALID_USERID;
    }
    if (!IsCallingPermissionValid(opt, userId, callingUserId)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }
    // If no user is specified, use current user.
    if (userId == DATAOBS_DEFAULT_CURRENT_USER) {
        userId = callingUserId;
    }
    int32_t pid = isExtension ? opt.FirstCallerPid() : IPCSkeleton::GetCallingPid();
    uint32_t token = isExtension ? opt.FirstCallerTokenID() : callingToken;
    ObserverInfo info(callingToken, fullTokenId, opt.FirstCallerTokenID(), userId, isExtension);
    info.callingUserId = callingUserId;
    info.errMsg = __FUNCTION__;
    info.pid = pid;
    Uri uriInner = uri;
    bool isDataShareUri = DataSharePermission::IsDataShareUri(uriInner);
    // If callling through DataShare, then uri must be datashare-style valid.
    if (opt.IsDataShare() && !isDataShareUri) {
        LOG_ERROR("uri invalid, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_INVALID_URI;
    }
    int status;
    if (opt.IsDataShare() || isDataShareUri) {
        // when calling from provider, check origininal caller's token not provider's token
        auto firstFullTokenId = isExtension ? opt.FirstCallerFullTokenID() : fullTokenId;
        // check for fault report purpose
        if (!IsSystemAppWithAllowedProvider(token, firstFullTokenId, __FUNCTION__, uri, isExtension)) {
            return DATAOBS_NOT_SYSTEM_APP;
        }
        status = VerifyDataSharePermission(uriInner, true, info);
        if (status != 0) {
            return status;
        }
    }
    status = ConstructRegisterObserver(uri, dataObserver, token, userId, pid);
    if (status != NO_ERROR) {
        LOG_ERROR("register failed:%{public}d,uri:%{public}s", status, CommonUtils::Anonymous(uri.ToString()).c_str());
        return status;
    }
    return NO_ERROR;
}

int DataObsMgrService::UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, int32_t userId,
    DataObsOption opt)
{
    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObserver, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObsMgrInner, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    if (!IsCallingPermissionValid(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }
    Uri uriInner = uri;
    bool isDataShareUri = DataSharePermission::IsDataShareUri(uriInner);
    if (opt.IsDataShare() && !isDataShareUri) {
        LOG_ERROR("uri invalid, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_INVALID_URI;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    int32_t callingUserId = GetCallingUserId(tokenId);
    auto [success, observerNode] = ConstructObserverNode(dataObserver, userId, tokenId, 0);
    if (!success) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "ConstructObserverNode fail, uri:%{public}s, userId:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), userId);
        return DATAOBS_INVALID_USERID;
    }
    int status;
    if (const_cast<Uri &>(uri).GetScheme() == SHARE_PREFERENCES) {
        status = dataObsMgrInnerPref_->HandleUnregisterObserver(uri, observerNode);
    } else {
        status = dataObsMgrInner_->HandleUnregisterObserver(uri, observerNode);
    }

    if (status != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "unregister failed:%{public}d, uri:%{public}s", status,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return status;
    }
    return NO_ERROR;
}

int DataObsMgrService::NotifyChangeFromExtension(const Uri &uri, int32_t userId, DataObsOption opt)
{
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null handler, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }
    if (dataObsMgrInner_ == nullptr || dataObsMgrInnerExt_ == nullptr || dataObsMgrInnerPref_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObsMgr,uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    Uri innerUri = uri;
    opt.SetDataShare(true);
    return NotifyChangeInner(innerUri, userId, opt, true);
}

int DataObsMgrService::NotifyChange(const Uri &uri, int32_t userId, DataObsOption opt)
{
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null handler, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }
    if (dataObsMgrInner_ == nullptr || dataObsMgrInnerExt_ == nullptr || dataObsMgrInnerPref_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObsMgr,uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    Uri innerUri = uri;
    return NotifyChangeInner(innerUri, userId, opt, false);
}

bool DataObsMgrService::IsTaskOverLimit()
{
    std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
    if (taskCount_ >= TASK_COUNT_MAX) {
        LOG_ERROR("task num reached limit, count %{public}d", taskCount_);
        return true;
    }
    ++taskCount_;
    return false;
}

void DataObsMgrService::SubmitNotifyChangeTask(Uri &uri, int32_t userId, std::string readPermission, ObserverInfo &info)
{
    ChangeInfo changeInfo = { ChangeInfo::ChangeType::OTHER, { uri } };
    handler_->SubmitTask([this, uri, changeInfo, userId, readPermission,
        isSilentUri = info.isSilentUri, tokenId = info.tokenId]() {
        if (const_cast<Uri &>(uri).GetScheme() == SHARE_PREFERENCES) {
            dataObsMgrInnerPref_->HandleNotifyChange(uri, userId, tokenId);
        } else {
            dataObsMgrInner_->HandleNotifyChange(uri, userId, readPermission, isSilentUri, tokenId);
            std::vector<NotifyInfo> verifyInfo = {NotifyInfo(uri, readPermission, isSilentUri)};
            dataObsMgrInnerExt_->HandleNotifyChange(changeInfo, userId, verifyInfo);
        }
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        --taskCount_;
    });
}

int32_t DataObsMgrService::NotifyChangeInner(Uri &uri, int32_t userId, DataObsOption opt, bool isExtension)
{
    int32_t uid  = IPCSkeleton::GetCallingUid();
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    int32_t callingUserId = GetCallingUserId(tokenId);
    if (callingUserId < 0) {
        return DATAOBS_INVALID_USERID;
    }
    if (!IsCallingPermissionValid(opt, userId, callingUserId)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }
    if (userId == DATAOBS_DEFAULT_CURRENT_USER) {
        userId = callingUserId;
    }
    ObserverInfo info(tokenId, fullTokenId, opt.FirstCallerTokenID(), userId, isExtension);
    info.callingUserId = callingUserId;
    info.pid = isExtension ? opt.FirstCallerPid() : IPCSkeleton::GetCallingPid();
    info.errMsg = __FUNCTION__;
    bool isDataShareUri = DataSharePermission::IsDataShareUri(uri);
    if (opt.IsDataShare() && !isDataShareUri) {
        LOG_ERROR("uri invalid, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_INVALID_URI;
    }
    bool checkPermission = opt.IsDataShare() || isDataShareUri;
    int32_t ret;
    if (uid != DATA_MANAGER_SERVICE_UID && checkPermission) {
        // when calling from provider, check origininal caller's token not provider's token
        auto firstTokenID = isExtension ? opt.FirstCallerTokenID() : tokenId;
        auto firstFullTokenId = isExtension ? opt.FirstCallerFullTokenID() : fullTokenId;
        // check for fault report purpose
        if (!IsSystemAppWithAllowedProvider(firstTokenID, firstFullTokenId, __FUNCTION__, uri, isExtension)) {
            return DATAOBS_NOT_SYSTEM_APP;
        }
        ret = VerifyDataSharePermission(uri, false, info);
        if (ret != 0) {
            return ret;
        }
    }
    Uri uriStr = uri;
    std::string readPermission = DataSharePermission::NO_PERMISSION;
    if (checkPermission) {
        std::tie(ret, readPermission) = GetUriPermission(uri, true, info);
        if (ret != 0) {
            return DATAOBS_INVALID_URI;
        }
    }
    if (IsTaskOverLimit()) {
        return DATAOBS_SERVICE_TASK_LIMMIT;
    }
    SubmitNotifyChangeTask(uri, userId, readPermission, info);
    return NO_ERROR;
}

Status DataObsMgrService::RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    bool isDescendants, DataObsOption opt)
{
    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObserver, uri:%{public}s, isDescendants:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInnerExt_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObsMgrInner, uri:%{public}s, isDescendants:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    if (!IsCallingPermissionValid(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }

    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    int32_t userId = GetCallingUserId(tokenId);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "GetCallingUserId fail, uri:%{public}s, userId:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), userId);
        return DATAOBS_INVALID_USERID;
    }
    Uri uriInner = uri;
    bool isDataShareUri = DataSharePermission::IsDataShareUri(uriInner);
    if (opt.IsDataShare() && !isDataShareUri) {
        LOG_ERROR("uri invalid, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_INVALID_URI;
    }
    if (opt.IsDataShare() || isDataShareUri) {
        // check for fault report purpose
        if (!IsSystemAppWithAllowedProvider(tokenId, IPCSkeleton::GetCallingFullTokenID(), __FUNCTION__, uri, false)) {
            return DATAOBS_NOT_SYSTEM_APP;
        }
    }

    ObserverInfo info(tokenId, 0, 0, userId, false);
    info.errMsg = __FUNCTION__;
    info.pid = IPCSkeleton::GetCallingPid();

    auto innerUri = uri;
    return dataObsMgrInnerExt_->HandleRegisterObserver(innerUri, dataObserver, info, isDescendants);
}

Status DataObsMgrService::UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    DataObsOption opt)
{
    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObserver, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInnerExt_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObsMgrInner, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    if (!IsCallingPermissionValid(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }

    Uri innerUri = uri;
    bool isDataShareUri = DataSharePermission::IsDataShareUri(innerUri);
    if (opt.IsDataShare() && !isDataShareUri) {
        LOG_ERROR("uri invalid, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_INVALID_URI;
    }
    return dataObsMgrInnerExt_->HandleUnregisterObserver(innerUri, dataObserver);
}

Status DataObsMgrService::UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver, DataObsOption opt)
{
    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObserver");
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInnerExt_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObsMgrInner");
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    if (!IsCallingPermissionValid(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }

    return dataObsMgrInnerExt_->HandleUnregisterObserver(dataObserver);
}

Status DataObsMgrService::DeepCopyChangeInfo(const ChangeInfo &src, ChangeInfo &dst) const
{
    dst = src;
    if (dst.size_ == 0) {
        return SUCCESS;
    }
    dst.data_ = new (std::nothrow) uint8_t[dst.size_];
    if (dst.data_ == nullptr) {
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    errno_t ret = memcpy_s(dst.data_, dst.size_, src.data_, src.size_);
    if (ret != EOK) {
        delete [] static_cast<uint8_t *>(dst.data_);
        dst.data_ = nullptr;
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    return SUCCESS;
}

std::pair<Status, std::vector<NotifyInfo>> DataObsMgrService::MakeNotifyInfos(ChangeInfo &changes, DataObsOption opt,
    uint32_t tokenId, int32_t userId)
{
    bool isDataShare = opt.IsDataShare();
    std::vector<NotifyInfo> notifyInfo;
    Status status = SUCCESS;
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    // datashare remove permission denied uri
    changes.uris_.remove_if([this, &notifyInfo, tokenId, userId, isDataShare, &status, fullTokenId](Uri &uri) {
        bool isDataShareUri = DataSharePermission::IsDataShareUri(uri);
        if (isDataShare && !isDataShareUri) {
            LOG_ERROR("uri invalid, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
            return true;
        }
        if (!isDataShare && !isDataShareUri) {
            notifyInfo.push_back(NotifyInfo(DataSharePermission::NO_PERMISSION, false));
            return false;
        }
        // check for fault report purpose
        if (!IsSystemAppWithAllowedProvider(tokenId, fullTokenId, __FUNCTION__, uri, false)) {
            return true;
        }
        ObserverInfo info(tokenId, 0, tokenId, userId, false);
        info.errMsg = "NotifyChangeExt";
        info.pid = IPCSkeleton::GetCallingPid();
        // check write permission
        status = VerifyDataSharePermissionInner(uri, false, info);
        if (status != SUCCESS) {
            return true;
        }
        // get read permission
        std::string readPermission;
        std::tie(status, readPermission) = GetUriPermission(uri, true, info);
        if (status != SUCCESS) {
            return true;
        }
        notifyInfo.push_back(NotifyInfo(readPermission, info.isSilentUri));
        return false;
    });

    return std::make_pair(status, notifyInfo);
}

Status DataObsMgrService::NotifyChangeExt(const ChangeInfo &changeInfo, DataObsOption opt)
{
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null handler");
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }
    if (dataObsMgrInner_ == nullptr || dataObsMgrInnerExt_ == nullptr) {
        LOG_ERROR("dataObsMgrInner_:%{public}d or null dataObsMgrInnerExt", dataObsMgrInner_ == nullptr);
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    if (!IsCallingPermissionValid(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    int userId = GetCallingUserId(tokenId);
    if (userId == -1) {
        LOG_ERROR("GetCallingUserId fail, type:%{public}d, userId:%{public}d", changeInfo.changeType_, userId);
        return DATAOBS_INVALID_USERID;
    }
    ChangeInfo changes;
    Status result = DeepCopyChangeInfo(changeInfo, changes);
    if (result != SUCCESS) {
        LOG_ERROR("copy data failed,changeType:%{public}ud,uris num:%{public}zu,null data:%{public}d,size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return result;
    }
    if (IsTaskOverLimit()) {
        return DATAOBS_SERVICE_TASK_LIMMIT;
    }
    std::vector<NotifyInfo> notifyInfo;
    std::tie (result, notifyInfo) = MakeNotifyInfos(changes, opt, tokenId, userId);
    if (changes.uris_.empty()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "uris_ is empty");
        return result;
    }
    handler_->SubmitTask([this, changes, userId, tokenId, notifyInfo]() {
        std::vector<NotifyInfo> info = notifyInfo;
        dataObsMgrInnerExt_->HandleNotifyChange(changes, userId, info);
        int32_t count = 0;
        for (auto &uri : changes.uris_) {
            dataObsMgrInner_->HandleNotifyChange(uri, userId, info[count].readPermission,
                info[count].isSilentUri, tokenId);
            count++;
        }
        delete [] static_cast<uint8_t *>(changes.data_);
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        --taskCount_;
    });
    return SUCCESS;
}

DataObsMgrService::FocusedAppInfo DataObsMgrService::GetFocusedWindowInfo() const
{
    Rosen::FocusChangeInfo info;
    DataObsMgrService::FocusedAppInfo appInfo = { 0 };
    std::vector<sptr<Rosen::WindowVisibilityInfo>> windowVisibilityInfos;
    Rosen::WMError result = Rosen::WMError::WM_OK;
#ifdef SCENE_BOARD_ENABLE
    Rosen::WindowManagerLite::GetInstance().GetFocusWindowInfo(info);
    result = Rosen::WindowManagerLite::GetInstance().GetVisibilityWindowInfo(windowVisibilityInfos);
#else
    Rosen::WindowManager::GetInstance().GetFocusWindowInfo(info);
    result = Rosen::WindowManager::GetInstance().GetVisibilityWindowInfo(windowVisibilityInfos);
#endif
    if (result == Rosen::WMError::WM_OK) {
        for (const auto& windowInfo : windowVisibilityInfos) {
            if (windowInfo == nullptr) {
                continue;
            }
            if (windowInfo->windowId_ == static_cast<uint32_t>(info.windowId_)) {
                appInfo.left = windowInfo->rect_.posX_;
                appInfo.top = windowInfo->rect_.posY_;
                appInfo.width = windowInfo->rect_.width_;
                appInfo.height = windowInfo->rect_.height_;
                break;
            }
        }
    }
    appInfo.abilityToken = info.abilityToken_;
    return appInfo;
}

sptr<IRemoteObject> DataObsMgrService::GetAbilityManagerService() const
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Failed to get ability manager.");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (!remoteObject) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Failed to get ability manager service.");
        return nullptr;
    }
    return remoteObject;
}

Status DataObsMgrService::NotifyProcessObserver(const std::string &key, const sptr<IRemoteObject> &observer,
    DataObsOption opt)
{
    if (!IsCallingPermissionValid(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }
    auto remote = GetAbilityManagerService();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Get ability manager failed.");
        return DATAOBS_PROXY_INNER_ERR;
    }
    auto abilityManager = iface_cast<IAbilityManager>(remote);

    FocusedAppInfo appInfo = GetFocusedWindowInfo();

    Want want;
    want.SetElementName(DIALOG_APP, PROGRESS_ABILITY);
    want.SetAction(PROGRESS_ABILITY);
    want.SetParam("promptText", std::string(PROMPT_TEXT));
    want.SetParam("remoteDeviceName", std::string());
    want.SetParam("progressKey", key);
    want.SetParam("isRemote", false);
    want.SetParam("ipcCallback", observer);
    want.SetParam("rectLeft", appInfo.left);
    want.SetParam("rectTop", appInfo.top);
    want.SetParam("rectWidth", static_cast<int32_t>(appInfo.width));
    want.SetParam("rectHeight", static_cast<int32_t>(appInfo.height));
    if (appInfo.abilityToken != nullptr) {
        want.SetParam("tokenKey", appInfo.abilityToken);
    } else {
        TAG_LOGW(AAFwkTag::DBOBSMGR, "abilityToken is nullptr.");
    }

    int32_t status = IN_PROCESS_CALL(abilityManager->StartAbility(want));
    if (status != SUCCESS) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "ShowProgress fail, status:%{public}d", status);
        return DATAOBS_PROXY_INNER_ERR;
    }
    return SUCCESS;
}

int DataObsMgrService::Dump(int fd, const std::vector<std::u16string>& args)
{
    std::string result;
    Dump(args, result);
    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dprintf error");
        return DATAOBS_HIDUMP_ERROR;
    }
    return SUCCESS;
}

void DataObsMgrService::Dump(const std::vector<std::u16string>& args, std::string& result) const
{
    auto size = args.size();
    if (size == 0) {
        ShowHelp(result);
        return;
    }

    std::string optionKey = Str16ToStr8(args[0]);
    if (optionKey != "-h") {
        result.append("error: unkown option.\n");
    }
    ShowHelp(result);
}

void DataObsMgrService::ShowHelp(std::string& result) const
{
    result.append("Usage:\n")
        .append("-h                          ")
        .append("help text for the tool\n");
}
}  // namespace AAFwk
}  // namespace OHOS
