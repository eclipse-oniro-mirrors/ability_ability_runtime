/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_SA_INTERCEPTOR_MANAGER_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_SA_INTERCEPTOR_MANAGER_H

#include "sa_interceptor_manager.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

// Mock class for SAInterceptor
class MockSAInterceptor : public ISAInterceptor {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.AbiilityRuntime.ISAInterceptor");
    int32_t OnCheckStarting(const std::string &params, Rule &rule) override { return 0; }
    sptr<IRemoteObject> AsObject() override { return nullptr; }
};

// Mock class for SAInterceptor return false
class MockSAInterceptorRetFalse : public ISAInterceptor {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.AbiilityRuntime.ISAInterceptor");
    int32_t OnCheckStarting(const std::string &params, Rule &rule) override { return -1; }
    sptr<IRemoteObject> AsObject() override { return nullptr; }
};

class MockIRemoteObject : public IRemoteObject {
public:
    static sptr<MockIRemoteObject> instance;
    MockIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}
    ~MockIRemoteObject() {}
    int32_t GetObjectRefCount() override
    {
        return 0;
    }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        return ERR_OK;
    }
    bool IsProxyObject() const override
    {
        return true;
    }
    bool CheckObjectLegality() const override
    {
        return true;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }
    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }
    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }
    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }
    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }
    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }
    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }
};

class MockDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    enum {
        ADD_DEATH_RECIPIENT,
        REMOVE_DEATH_RECIPIENT,
        NOTICE_DEATH_RECIPIENT,
        TEST_SERVICE_DEATH_RECIPIENT,
        TEST_DEVICE_DEATH_RECIPIENT,
    };
    void OnRemoteDied(const wptr<IRemoteObject> &remote)
    {
        return;
    }
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_SA_INTERCEPTOR_MANAGER_H