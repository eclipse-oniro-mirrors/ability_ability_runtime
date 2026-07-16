/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ability_manager_client.h"
#include "assert_fault_proxy.h"
#include "hilog_tag_wrapper.h"
#include "scene_board_judgement.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char ASSERT_FAULT_DETAIL[] = "assertFaultDialogDetail";
constexpr char UIEXTENSION_TYPE_KEY[] = "ability.want.params.uiExtensionType";
constexpr int32_t INVALID_USERID = -1;
constexpr int32_t MESSAGE_PARCEL_KEY_SIZE = 3;
constexpr uint32_t COMMAND_START_DIALOG = 1;
}
AssertFaultProxy::AssertFaultProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAssertFaultInterface>(impl)
{}

void AssertFaultProxy::NotifyDebugAssertResult(AAFwk::UserStatus status)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Notify user action result to assert fault application.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(AssertFaultProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(status))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write status failed");
        return;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (remote->SendRequest(MessageCode::NOTIFY_DEBUG_ASSERT_RESULT, data, reply, option) != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote send request failed");
    }

    ModalSystemAssertUIExtension::GetInstance().DisconnectSystemUI();
}

AssertFaultRemoteDeathRecipient::AssertFaultRemoteDeathRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

void AssertFaultRemoteDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null handler_");
        return;
    }
    handler_(remote);
}

ModalSystemAssertUIExtension &ModalSystemAssertUIExtension::GetInstance()
{
    static ModalSystemAssertUIExtension instance;
    return instance;
}

ModalSystemAssertUIExtension::~ModalSystemAssertUIExtension()
{
    dialogConnectionCallback_ = nullptr;
}

sptr<ModalSystemAssertUIExtension::AssertDialogConnection> ModalSystemAssertUIExtension::GetConnection()
{
    if (dialogConnectionCallback_ == nullptr) {
        std::lock_guard lock(dialogConnectionMutex_);
        if (dialogConnectionCallback_ == nullptr) {
            dialogConnectionCallback_ = new (std::nothrow) AssertDialogConnection();
        }
    }

    return dialogConnectionCallback_;
}

bool ModalSystemAssertUIExtension::CreateModalUIExtension(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::unique_lock<std::mutex> lockAssertResult(assertResultMutex_);
    if (reqeustCount_++ != 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Task busy, waiting for processing.");
        assertResultCV_.wait(lockAssertResult);
    }
    auto callback = GetConnection();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null callback");
        TryNotifyOneWaitingThread();
        return false;
    }
    callback->SetReqeustAssertDialogWant(want);
    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManagerClient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityManagerClient");
        TryNotifyOneWaitingThread();
        return false;
    }
    AAFwk::Want systemUIWant;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        systemUIWant.SetElementName("com.ohos.sceneboard", "com.ohos.sceneboard.systemdialog");
    } else {
        systemUIWant.SetElementName("com.ohos.systemui", "com.ohos.systemui.dialog");
    }
    auto result = abilityManagerClient->ConnectAbility(systemUIWant, callback, INVALID_USERID);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectSystemUi ConnectAbility dialog failed, result: %{public}d", result);
        TryNotifyOneWaitingThread();
        return false;
    }
    return true;
}

bool ModalSystemAssertUIExtension::DisconnectSystemUI()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    bool retVal = true;
    do {
        auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
        if (abilityManagerClient == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityManagerClient");
            retVal = false;
            break;
        }
        auto callback = GetConnection();
        if (callback == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null callback");
            retVal = false;
            break;
        }
        auto result = abilityManagerClient->DisconnectAbility(callback);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "disconnectAbility dialog failed, result: %{public}d", result);
            retVal = false;
            break;
        }
    } while (false);

    return retVal;
}

void ModalSystemAssertUIExtension::TryNotifyOneWaitingThreadInner()
{
    std::unique_lock<std::mutex> lockAssertResult(assertResultMutex_);
    if (--reqeustCount_ > 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Notify waiting Thread count is %{public}d.", reqeustCount_);
        assertResultCV_.notify_one();
        return;
    }
    reqeustCount_ = 0;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Counter reset to 0.");
}

void ModalSystemAssertUIExtension::TryNotifyOneWaitingThread()
{
    auto handler = AAFwk::TaskHandlerWrap::GetFfrtHandler();
    if (handler != nullptr) {
        auto notifyTask = [] () {
            ModalSystemAssertUIExtension::GetInstance().TryNotifyOneWaitingThreadInner();
        };
        handler->SubmitTask(notifyTask, "TryNotifyOneWaitingThread");
    }
}

void ModalSystemAssertUIExtension::AssertDialogConnection::SetReqeustAssertDialogWant(const AAFwk::Want &want)
{
    want_ = want;
}

void ModalSystemAssertUIExtension::AssertDialogConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remote, int resultCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(MESSAGE_PARCEL_KEY_SIZE);
    data.WriteString16(u"bundleName");
    data.WriteString16(Str8ToStr16(want_.GetBundleNameRef()));
    data.WriteString16(u"abilityName");
    data.WriteString16(Str8ToStr16(want_.GetAbilityNameRef()));
    data.WriteString16(u"parameters");
    nlohmann::json param;
    param[UIEXTENSION_TYPE_KEY] = want_.GetStringParam(UIEXTENSION_TYPE_KEY);
    param[ASSERT_FAULT_DETAIL] = want_.GetStringParam(ASSERT_FAULT_DETAIL);
    param[AAFwk::Want::PARAM_ASSERT_FAULT_SESSION_ID] =
        want_.GetStringParam(AAFwk::Want::PARAM_ASSERT_FAULT_SESSION_ID);
    std::string paramStr = param.dump();
    data.WriteString16(Str8ToStr16(paramStr));
    uint32_t code = !Rosen::SceneBoardJudgement::IsSceneBoardEnabled() ? COMMAND_START_DIALOG :
        AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE;
    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "show dialog failed");
        return;
    }
}

void ModalSystemAssertUIExtension::AssertDialogConnection::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    ModalSystemAssertUIExtension::GetInstance().TryNotifyOneWaitingThread();
}
} // namespace AbilityRuntime
} // namespace OHOS