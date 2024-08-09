/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "configuration_observer_stub.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
ConfigurationObserverStub::ConfigurationObserverStub() {}

ConfigurationObserverStub::~ConfigurationObserverStub() {}

int ConfigurationObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGI(AAFwkTag::APPMGR, "ConfigurationObserverStub::OnRemoteRequest, code = %{public}u, flags= %{public}d.",
        code, option.GetFlags());
    std::u16string descriptor = ConfigurationObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is not equivalent to remote");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(IConfigurationObserver::Message::TRANSACT_ON_CONFIGURATION_UPDATED)) {
        return HandleOnConfigurationUpdated(data, reply);
    }

    TAG_LOGI(AAFwkTag::APPMGR, "ConfigurationObserverStub::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void ConfigurationObserverStub::OnConfigurationUpdated(const Configuration& configuration)
{}

int32_t ConfigurationObserverStub::HandleOnConfigurationUpdated(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<Configuration> configuration(data.ReadParcelable<Configuration>());
    if (!configuration) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<Configuration> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnConfigurationUpdated(*configuration);
    return NO_ERROR;
}
}
}
