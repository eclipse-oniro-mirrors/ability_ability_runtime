/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_IPC_IREMOTE_OBJECT_H
#define MOCK_OHOS_IPC_IREMOTE_OBJECT_H

#include "parcel.h"
#include "refbase.h"
#include "oh_mock_utils.h"

namespace OHOS {
class IRemoteBroker;
class IRemoteObject : public virtual Parcelable, public virtual RefBase {
public:
    enum {
        IF_PROT_DEFAULT,
        IF_PROT_BINDER = IF_PROT_DEFAULT,
        IF_PROT_DATABUS,
        IF_PROT_ERROR,
    };
    enum {
        DATABUS_TYPE,
    };
    class DeathRecipient : public virtual RefBase {
    public:
        enum {
            ADD_DEATH_RECIPIENT,
            REMOVE_DEATH_RECIPIENT,
            NOTICE_DEATH_RECIPIENT,
            TEST_SERVICE_DEATH_RECIPIENT,
            TEST_DEVICE_DEATH_RECIPIENT,
        };
        virtual void OnRemoteDied(const wptr<IRemoteObject> &object) {}
    };

    class RefreshRecipient : public virtual RefBase {
    public:
        virtual void OnRemoteRefreshed(const wptr<IRemoteObject> &object) {};
    };

    virtual std::u16string GetInterfaceDescriptor()
    {
        return u"";
    }

    OH_MOCK_METHOD_WITH_DECORATOR(virtual, bool, IRemoteObject, IsProxyObject);
    OH_MOCK_METHOD_WITH_DECORATOR(virtual, bool, IRemoteObject, AddDeathRecipient, const sptr<DeathRecipient> &);

    virtual bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient)
    {
        return false;
    }

    virtual bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return false;
    }

    virtual bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return false;
    }

    virtual bool Marshalling(Parcel &parcel) const override
    {
        return false;
    }

    static sptr<IRemoteObject> Unmarshalling(Parcel &parcel)
    {
        return nullptr;
    }

    static bool Marshalling(Parcel &parcel, const sptr<IRemoteObject> &object)
    {
        return false;
    }

    virtual sptr<IRemoteBroker> AsInterface()
    {
        return nullptr;
    }

    std::u16string GetObjectDescriptor() const
    {
        return u"";
    }
};
} // namespace OHOS
#endif