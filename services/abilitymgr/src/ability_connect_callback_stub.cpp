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

#include "ability_connect_callback_stub.h"

#include <dlfcn.h>
#include <mutex>
#include <string>
#include <vector>

#include "ability_connect_callback_proxy.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool AbilityConnectionProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilityConnectionProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "WriteInterfaceToken fail");
        return false;
    }
    return true;
}

void AbilityConnectionProxy::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityConnectDone resultCode: %{public}d", resultCode);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "WriteInterfaceToken fail");
        return;
    }

    if (!data.WriteParcelable(&element)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "element error");
        return;
    }

    if (!data.WriteRemoteObject(remoteObject)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remoteObject error");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resultCode error");
        return;
    }

    error = SendTransactCmd(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect done fail, error: %{public}d", error);
        return;
    }
}

void AbilityConnectionProxy::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityDisconnectDone resultCode: %{public}d", resultCode);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "WriteInterfaceToken fail");
        return;
    }
    if (!data.WriteParcelable(&element) || !data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "data write error");
        return;
    }

    error = SendTransactCmd(IAbilityConnection::ON_ABILITY_DISCONNECT_DONE, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Disconnect done fail, error: %{public}d", error);
        return;
    }
}

int32_t AbilityConnectionProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail. code: %{public}d, ret: %{public}d", code, ret);
        return ret;
    }
    return NO_ERROR;
}

AbilityConnectionStub::AbilityConnectionStub()
{}

AbilityConnectionStub::~AbilityConnectionStub()
{}

int AbilityConnectionStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "code: %{public}u", code);
    std::u16string descriptor = AbilityConnectionStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor &&
        !CompareDMSDescriptor(remoteDescriptor)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "descriptor not equal to remote");
        return ERR_INVALID_STATE;
    }

    std::unique_ptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null element");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call callback");
    auto abilityName = element->GetAbilityName();
    switch (code) {
        case IAbilityConnection::ON_ABILITY_CONNECT_DONE: {
            auto remoteObject = data.ReadRemoteObject();
            if (remoteObject == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null remoteObject");
                return ERR_INVALID_VALUE;
            }
            auto resultCode = data.ReadInt32();
            TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityConnectionStub ON_ABILITY_CONNECT_DONE");
            OnAbilityConnectDone(*element, remoteObject, resultCode);
            TAG_LOGI(AAFwkTag::EXT, "connect end,%{public}s",
                abilityName.c_str());
            return NO_ERROR;
        }
        case IAbilityConnection::ON_ABILITY_DISCONNECT_DONE: {
            auto resultCode = data.ReadInt32();
            TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityConnectionStub ON_ABILITY_DISCONNECT_DONE");
            OnAbilityDisconnectDone(*element, resultCode);
            TAG_LOGI(AAFwkTag::EXT, "disconnect end,%{public}s",
                abilityName.c_str());
            return NO_ERROR;
        }
        case IAbilityConnection::ON_REMOTE_STATE_CHANGED: {
            int32_t abilityState = data.ReadInt32();
            OnRemoteStateChanged(*element, abilityState);
            return NO_ERROR;
        }
        default: {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "AbilityConnectionStub default");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

static constexpr const char* DMS_DESCRIPTOR_LIB_PATH = "/system/lib64/libabilitywantadapter.z.so";

namespace {
std::once_flag g_dmsDescriptorOnceFlag;
std::vector<std::u16string> g_dmsDescriptors;

void LoadDmsDescriptors()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "dms Descriptor dlopen");
    void* handle = dlopen(DMS_DESCRIPTOR_LIB_PATH, RTLD_NOW);
    if (handle == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "dlopen descriptor failed");
        return;
    }
    using GetCountFunc = size_t (*)();
    using GetByIndexFunc = const char16_t* (*)(size_t);
    auto getCount = reinterpret_cast<GetCountFunc>(dlsym(handle, "GetDmsDescriptorCount"));
    auto getByIndex = reinterpret_cast<GetByIndexFunc>(dlsym(handle, "GetDmsDescriptorByIndex"));
    if (getCount == nullptr || getByIndex == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dms Descriptor get func failed");
        dlclose(handle);
        return;
    }
    size_t count = getCount();
    for (size_t i = 0; i < count; ++i) {
        const char16_t* desc = getByIndex(i);
        if (desc != nullptr) {
            g_dmsDescriptors.emplace_back(desc);
        }
    }
    dlclose(handle);
}
} // namespace

bool AbilityConnectionStub::CompareDMSDescriptor(const std::u16string& remoteDescriptor)
{
    std::call_once(g_dmsDescriptorOnceFlag, LoadDmsDescriptors);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "dms Descriptor loaded");
    for (const auto& dmsDescriptor : g_dmsDescriptors) {
        if (dmsDescriptor == remoteDescriptor) {
            return true;
        }
    }
    return false;
}

void AbilityConnectCallbackRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (handler_) {
        handler_(remote);
    }
}

AbilityConnectCallbackRecipient::AbilityConnectCallbackRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

AbilityConnectCallbackRecipient::~AbilityConnectCallbackRecipient()
{}
}  // namespace AAFwk
}  // namespace OHOS
