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

#ifndef OHOS_ABILITY_RUNTIME_ASSERT_FAULT_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_ASSERT_FAULT_CALLBACK_H

#include <iremote_object.h>
#include <iremote_stub.h>
#include "assert_fault_interface.h"
#include "nocopyable.h"

namespace OHOS {
namespace AbilityRuntime {
class AssertFaultTaskThread;
class AssertFaultCallback : public IRemoteStub<IAssertFaultInterface> {
public:
    AssertFaultCallback(const std::weak_ptr<AssertFaultTaskThread> &assertFaultThread);
    virtual ~AssertFaultCallback() = default;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * Notify listeners of user operation results.
     *
     * @param status - User action result.
     */
    void NotifyDebugAssertResult(AAFwk::UserStatus status) override;

    AAFwk::UserStatus GetAssertResult();
private:
    DISALLOW_COPY_AND_MOVE(AssertFaultCallback);
    std::weak_ptr<AssertFaultTaskThread> assertFaultThread_;
    AAFwk::UserStatus status_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ASSERT_FAULT_CALLBACK_H