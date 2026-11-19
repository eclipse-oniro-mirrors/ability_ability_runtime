/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MEM_DUMP_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_MEM_DUMP_CALLBACK_PROXY_H

#include "mem_dump_callback_interface.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class MemDumpCallbackProxy : public IRemoteProxy<IMemDumpCallback> {
public:
    explicit MemDumpCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~MemDumpCallbackProxy() = default;

    void OnMemDumpDone(const std::string &dumpResult) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<MemDumpCallbackProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MEM_DUMP_CALLBACK_PROXY_H