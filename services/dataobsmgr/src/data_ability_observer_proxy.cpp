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

#include "data_ability_observer_proxy.h"
#include "hilog_tag_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
static constexpr uint32_t MESSAGE_MAX_COUNT = 50;
DataAbilityObserverProxy::DataAbilityObserverProxy(const sptr<IRemoteObject> &remote)
    : IRemoteProxy<IDataAbilityObserver>(remote)
{}
DataAbilityObserverProxy::~DataAbilityObserverProxy()
{}

/**
 * @brief Set the message option.
 *
 * @param option Indicates the option of message.
 */
void DataAbilityObserverProxy::SetMessageOption(MessageOption &option)
{
    std::lock_guard<std::mutex> lock(countMutex_);
    // Send a wakeup IPC every 50 times. Otherwise, send a non-wakeup IPC.
    if (messageCount_ >= MESSAGE_MAX_COUNT) {
        option = MessageOption(MessageOption::TF_ASYNC);
        messageCount_ = 0;
    } else {
        option = MessageOption(MessageOption::TF_ASYNC | MessageOption::TF_ASYNC_WAKEUP_LATER);
        messageCount_++;
    }
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @param uri Indicates the path of the data to operate.
 */
void DataAbilityObserverProxy::OnChange()
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    MessageOption option;
    SetMessageOption(option);

    if (!data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write token false");
        return;
    }

    int result = SendTransactCmd(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE, data, reply, option);
    if (result != ERR_NONE) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "error,result:%{public}d", result);
    }
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @param changeInfo Indicates the info of the data to operate.
 */
void DataAbilityObserverProxy::OnChangeExt(const ChangeInfo &changeInfo)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    MessageOption option;
    SetMessageOption(option);

    if (!data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write token false");
        return;
    }

    if (!ChangeInfo::Marshalling(changeInfo, data)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "changeInfo marshalling fail");
        return;
    }

    int result = SendTransactCmd(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_EXT, data, reply, option);
    if (result != ERR_NONE) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "error result:%{public}d", result);
    }
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @param uri Indicates the path of the data to operate.
 */
void DataAbilityObserverProxy::OnChangePreferences(const std::string &key)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    MessageOption option;
    SetMessageOption(option);

    if (!data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write token false");
        return;
    }

    if (!data.WriteString(key)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write string false");
        return;
    }

    int result =
        SendTransactCmd(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_PREFERENCES, data, reply, option);
    if (result != ERR_NONE) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "error result:%{public}d", result);
    }
}

int32_t DataAbilityObserverProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null remote");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}

}  // namespace AAFwk
}  // namespace OHOS
