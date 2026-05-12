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

#include "skill/skill_execute_callback_stub.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

SkillExecuteCallbackStub::SkillExecuteCallbackStub() {}

SkillExecuteCallbackStub::~SkillExecuteCallbackStub() {}

int32_t SkillExecuteCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != ISkillExecuteCallback::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interface token not match");
        return ERR_INVALID_STATE;
    }

    if (code == ON_SKILL_EXECUTE_DONE) {
        return OnExecuteDoneInner(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t SkillExecuteCallbackStub::OnExecuteDoneInner(MessageParcel &data, MessageParcel &reply)
{
    std::string requestCode = data.ReadString();
    int32_t resultCode = data.ReadInt32();
    auto *result = data.ReadParcelable<AppExecFwk::SkillExecuteResult>();
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null result");
        return ERR_INVALID_VALUE;
    }
    OnExecuteDone(requestCode, resultCode, *result);
    delete result;
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
