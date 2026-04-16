/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_WANT_AGENT_CLIENT_H
#define OHOS_ABILITY_RUNTIME_WANT_AGENT_CLIENT_H

#include <mutex>

#include "iremote_object.h"
#include "want.h"
#include "want_receiver_interface.h"
#include "want_sender_info.h"
#include "want_sender_interface.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class WantAgentClient
 * WantAgentClient is used to access want agent in ability manager service.
 */
class WantAgentClient {
public:
    static WantAgentClient &GetInstance();

    ErrCode GetWantSender(const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken,
        sptr<IWantSender> &wantSender, int32_t uid = -1);

    ErrCode SendWantSender(sptr<IWantSender> target, SenderInfo &senderInfo);

    ErrCode SendLocalWantSender(const SenderInfo &senderInfo);

    ErrCode CancelWantSender(const sptr<IWantSender> &sender, uint32_t flags = 0);

    ErrCode GetPendingWantUid(const sptr<IWantSender> &target, int32_t &uid);

    ErrCode GetPendingWantUserId(const sptr<IWantSender> &target, int32_t &userId);

    ErrCode GetPendingWantBundleName(const sptr<IWantSender> &target, std::string &bundleName);

    ErrCode GetPendingWantCode(const sptr<IWantSender> &target, int32_t &code);

    ErrCode GetPendingWantType(sptr<IWantSender> target, int32_t &type);

    void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver);

    void UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver);

    ErrCode GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want);

    ErrCode GetPendingRequestWantFromProxy(const sptr<IWantSender> &target, std::shared_ptr<Want> &want);

    ErrCode GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info);

    void RemoveDeathRecipient();
private:
    class WantAgentDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        WantAgentDeathRecipient() = default;
        ~WantAgentDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        DISALLOW_COPY_AND_MOVE(WantAgentDeathRecipient);
    };
    WantAgentClient();
    virtual ~WantAgentClient();
    DISALLOW_COPY_AND_MOVE(WantAgentClient);

    sptr<IRemoteObject> GetAbilityManager();
    void ResetProxy(const wptr<IRemoteObject>& remote);
    bool WriteInterfaceToken(MessageParcel &data);
    bool CheckSenderAndRecevier(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver);
    bool SendRequest(int32_t operation, const sptr<IRemoteObject> &abms,
        const sptr<IRemoteObject> &remoteObject, MessageParcel &reply, ErrCode &error);

    std::mutex mutex_;
    sptr<IRemoteObject> proxy_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_WANT_AGENT_CLIENT_H
