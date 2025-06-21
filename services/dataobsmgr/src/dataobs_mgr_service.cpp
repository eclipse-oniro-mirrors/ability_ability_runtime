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

#include <functional>
#include <memory>
#include <string>
#include <unistd.h>

#include "ability_connect_callback_stub.h"
#include "ability_manager_interface.h"
#include "ability_manager_proxy.h"
#include "accesstoken_kit.h"
#include "dataobs_mgr_errors.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
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
static constexpr const char *DIALOG_APP = "com.ohos.pasteboarddialog";
static constexpr const char *PROGRESS_ABILITY = "PasteboardProgressAbility";
static constexpr const char *PROMPT_TEXT = "PromptText_PasteBoard_Local";

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DataObsMgrService>::GetInstance().get());

DataObsMgrService::DataObsMgrService()
    : SystemAbility(DATAOBS_MGR_SERVICE_SA_ID, true),
      state_(DataObsServiceRunningState::STATE_NOT_START)
{
    dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
    dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();
    dataObsMgrInnerPref_ = std::make_shared<DataObsMgrInnerPref>();
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
    return true;
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
    int32_t userId)
{
    if (userId == -1) {
        userId = GetCallingUserId();
    }
    if (userId == -1) {
        // return false, tokenId default 0
        return std::make_pair(false, ObserverNode(dataObserver, userId, 0));
    }
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return std::make_pair(true, ObserverNode(dataObserver, userId, tokenId));
}

int32_t DataObsMgrService::GetCallingUserId()
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
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

// GetTokenType use tokenId, and IsSystemApp use fullTokenId, these are different
bool DataObsMgrService::CheckSystemCallingPermission(DataObsOption &opt, int32_t userId, int32_t callingUserId)
{
    bool checkUser = false;
    if (userId == -1 || userId == callingUserId) {
        checkUser = false;
    } else {
        checkUser = true;
    }
    if (!opt.IsSystem() && !checkUser) {
        return true;
    }
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    Security::AccessToken::ATokenTypeEnum tokenType =
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        return true;
    }
    // IsSystemAppByFullTokenID here is not IPC
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Not system app, token:%{public}" PRIx64 "", fullTokenId);
        return false;
    }
    return true;
}

int DataObsMgrService::RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    int32_t userId, DataObsOption opt)
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

    int32_t callingUserId = GetCallingUserId();
    if (callingUserId == -1) {
        return DATAOBS_INVALID_USERID;
    }

    if (!CheckSystemCallingPermission(opt, userId, callingUserId)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }
    // If no user is specified, use current user.
    if (userId == -1) {
        userId = callingUserId;
    }

    auto [success, observerNode] = ConstructObserverNode(dataObserver, userId);
    if (!success) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "ConstructObserverNode fail, uri:%{public}s, userId:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), userId);
        return DATAOBS_INVALID_USERID;
    }
    int status;
    if (const_cast<Uri &>(uri).GetScheme() == SHARE_PREFERENCES) {
        status = dataObsMgrInnerPref_->HandleRegisterObserver(uri, observerNode);
    } else {
        status = dataObsMgrInner_->HandleRegisterObserver(uri, observerNode);
    }

    if (status != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "register failed:%{public}d, uri:%{public}s", status,
            CommonUtils::Anonymous(uri.ToString()).c_str());
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
    if (!CheckSystemCallingPermission(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }

    auto [success, observerNode] = ConstructObserverNode(dataObserver, userId);
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

int DataObsMgrService::NotifyChange(const Uri &uri, int32_t userId, DataObsOption opt)
{
    if (handler_ == nullptr) {
        TAG_LOGE(
            AAFwkTag::DBOBSMGR, "null handler, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr || dataObsMgrInnerExt_ == nullptr || dataObsMgrInnerPref_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObsMgr, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    int32_t callingUserId = GetCallingUserId();
    if (callingUserId == -1) {
        return DATAOBS_INVALID_USERID;
    }
    if (!CheckSystemCallingPermission(opt, userId, callingUserId)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }
    // If no user is specified, the current user is notified.
    if (userId == -1) {
        userId = callingUserId;
    }

    {
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        if (taskCount_ >= TASK_COUNT_MAX) {
            TAG_LOGE(AAFwkTag::DBOBSMGR, "task num reached limit, uri:%{public}s",
                CommonUtils::Anonymous(uri.ToString()).c_str());
            return DATAOBS_SERVICE_TASK_LIMMIT;
        }
        ++taskCount_;
    }

    ChangeInfo changeInfo = { ChangeInfo::ChangeType::OTHER, { uri } };
    handler_->SubmitTask([this, uri, changeInfo, userId]() {
        if (const_cast<Uri &>(uri).GetScheme() == SHARE_PREFERENCES) {
            dataObsMgrInnerPref_->HandleNotifyChange(uri, userId);
        } else {
            dataObsMgrInner_->HandleNotifyChange(uri, userId);
            dataObsMgrInnerExt_->HandleNotifyChange(changeInfo, userId);
        }
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        --taskCount_;
    });

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
    if (!CheckSystemCallingPermission(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }

    int userId = GetCallingUserId();
    if (userId == -1) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "GetCallingUserId fail, uri:%{public}s, userId:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), userId);
        return DATAOBS_INVALID_USERID;
    }

    auto innerUri = uri;
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return dataObsMgrInnerExt_->HandleRegisterObserver(innerUri, dataObserver, userId, tokenId, isDescendants);
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
    if (!CheckSystemCallingPermission(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }

    auto innerUri = uri;
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
    if (!CheckSystemCallingPermission(opt)) {
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

Status DataObsMgrService::NotifyChangeExt(const ChangeInfo &changeInfo, DataObsOption opt)
{
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null handler");
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr || dataObsMgrInnerExt_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObsMgrInner_:%{public}d or null dataObsMgrInnerExt",
            dataObsMgrInner_ == nullptr);
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    if (!CheckSystemCallingPermission(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }

    int userId = GetCallingUserId();
    if (userId == -1) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "GetCallingUserId fail, type:%{public}d, userId:%{public}d",
            changeInfo.changeType_, userId);
        return DATAOBS_INVALID_USERID;
    }

    ChangeInfo changes;
    Status result = DeepCopyChangeInfo(changeInfo, changes);
    if (result != SUCCESS) {
        TAG_LOGE(AAFwkTag::DBOBSMGR,
            "copy data failed, changeType:%{public}ud,uris num:%{public}zu, "
            "null data:%{public}d, size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return result;
    }

    {
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        if (taskCount_ >= TASK_COUNT_MAX) {
            TAG_LOGE(AAFwkTag::DBOBSMGR,
                "task num maxed, changeType:%{public}ud,"
                "uris num:%{public}zu, null data:%{public}d, size:%{public}ud",
                changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
            return DATAOBS_SERVICE_TASK_LIMMIT;
        }
        ++taskCount_;
    }

    handler_->SubmitTask([this, changes, userId]() {
        dataObsMgrInnerExt_->HandleNotifyChange(changes, userId);
        for (auto &uri : changes.uris_) {
            dataObsMgrInner_->HandleNotifyChange(uri, userId);
        }
        delete [] static_cast<uint8_t *>(changes.data_);
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        --taskCount_;
    });
    return SUCCESS;
}

void DataObsMgrService::GetFocusedAppInfo(int32_t &windowId, sptr<IRemoteObject> &abilityToken) const
{
    Rosen::FocusChangeInfo info;
#ifdef SCENE_BOARD_ENABLE
    Rosen::WindowManagerLite::GetInstance().GetFocusWindowInfo(info);
#else
    Rosen::WindowManager::GetInstance().GetFocusWindowInfo(info);
#endif
    windowId = info.windowId_;
    abilityToken = info.abilityToken_;
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
    if (!CheckSystemCallingPermission(opt)) {
        return DATAOBS_NOT_SYSTEM_APP;
    }
    auto remote = GetAbilityManagerService();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Get ability manager failed.");
        return DATAOBS_PROXY_INNER_ERR;
    }
    auto abilityManager = iface_cast<IAbilityManager>(remote);

    int32_t windowId;
    sptr<IRemoteObject> callerToken;
    GetFocusedAppInfo(windowId, callerToken);

    Want want;
    want.SetElementName(DIALOG_APP, PROGRESS_ABILITY);
    want.SetAction(PROGRESS_ABILITY);
    want.SetParam("promptText", std::string(PROMPT_TEXT));
    want.SetParam("remoteDeviceName", std::string());
    want.SetParam("progressKey", key);
    want.SetParam("isRemote", false);
    want.SetParam("windowId", windowId);
    want.SetParam("ipcCallback", observer);
    if (callerToken != nullptr) {
        want.SetParam("tokenKey", callerToken);
    } else {
        TAG_LOGW(AAFwkTag::DBOBSMGR, "CallerToken is nullptr.");
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
