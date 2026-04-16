/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_PENDING_WANT_RECORD_H
#define OHOS_ABILITY_RUNTIME_PENDING_WANT_RECORD_H

#include <list>
#include <mutex>
#include "cpp/mutex.h"

#include "iremote_object.h"
#include "pending_want_key.h"
#include "refbase.h"
#include "sender_info.h"
#include "want_sender_stub.h"

namespace OHOS {
namespace AAFwk {
class PendingWantManager;
class PendingWantRecord : public WantSenderStub {
public:

    PendingWantRecord();
    PendingWantRecord(const std::shared_ptr<PendingWantManager> &pendingWantManager, int32_t uid,
        int32_t callerTokenId, const sptr<IRemoteObject> &callerToken, std::shared_ptr<PendingWantKey> key);
    virtual ~PendingWantRecord();

    virtual void Send(SenderInfo &senderInfo);
    virtual void RegisterCancelListener(const sptr<IWantReceiver> &receiver);
    virtual void UnregisterCancelListener(const sptr<IWantReceiver> &receiver);

    virtual int32_t SenderInner(SenderInfo &senderInfo);

public:
    std::shared_ptr<PendingWantKey> GetKey();
    int32_t GetUid() const;
    void SetCallerUid(const int32_t callerUid);
    void SetCanceled();
    bool GetCanceled();
    std::list<sptr<IWantReceiver>> GetCancelCallbacks();
    void BuildSendWant(SenderInfo &senderInfo, Want &want);
    int32_t GetAppIndexbyUid(int32_t uid, const std::string bundleName, int32_t &appIndex);

private:
    int32_t ExecuteOperation(
        std::shared_ptr<PendingWantManager> pendingWantManager, SenderInfo &senderInfo, Want &want);
    void CheckAppInstanceKey(const std::string& bundleName, WantParams &wantParams);
    bool IsBundleNotExistForUser0(const Want &want);

private:
    int32_t uid_ = 0;
    int32_t callerTokenId_ = 0;
    sptr<IRemoteObject> callerToken_ = {};
    std::weak_ptr<PendingWantManager> pendingWantManager_ = {};
    std::shared_ptr<PendingWantKey> key_ = {};
    std::list<sptr<IWantReceiver>> mCancelCallbacks_ = {};
    int32_t callerUid_ = 0;
    bool canceled_ = false;
    ffrt::mutex lock_ = {};
    std::mutex mCancelCallbacksMutex_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_PENDING_WANT_RECORD_H
