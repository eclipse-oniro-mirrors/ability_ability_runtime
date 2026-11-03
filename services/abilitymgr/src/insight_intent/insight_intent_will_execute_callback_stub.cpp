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

#include "insight_intent_will_execute_callback_stub.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t FOUNDATION_UID = 5523;
} // namespace

int32_t InsightIntentWillExecuteCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::INTENT, "will execute callback stub on remote request, code:%{public}u", code);
    if (data.ReadInterfaceToken() != IInsightIntentWillExecuteCallback::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::INTENT, "InterfaceToken not equal");
        return ERR_INVALID_STATE;
    }

    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::INTENT, "reject: callingUid=%{public}d, expected foundation uid",
            IPCSkeleton::GetCallingUid());
        return CHECK_PERMISSION_FAILED;
    }

    if (code == ON_WILL_INSIGHT_INTENT_EXECUTE) {
        return OnWillInsightIntentExecuteInner(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t InsightIntentWillExecuteCallbackStub::OnWillInsightIntentExecuteInner(
    MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::INTENT, "will execute callback stub inner called");
    std::shared_ptr<AppExecFwk::InsightIntentExecutionInfo> info(
        data.ReadParcelable<AppExecFwk::InsightIntentExecutionInfo>());
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null execution info");
        return ERR_INVALID_VALUE;
    }
    OnWillInsightIntentExecute(*info);
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS