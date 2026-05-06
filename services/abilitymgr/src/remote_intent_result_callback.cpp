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

#include "remote_intent_result_callback.h"
#include "insight_intent_execute_manager.h"
#include "insight_intent_execute_result.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
void RemoteIntentResultCallback::OnIntentResult(uint64_t requestCode, int32_t resultCode,
    const std::string& resultMsg)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "OnIntentResult requestCode=%{public}" PRIu64 ", resultCode=%{public}d", requestCode, resultCode);
    if (resultMsg.empty()) {
        AppExecFwk::InsightIntentExecuteResult errorResult{};
        errorResult.innerErr = AbilityRuntime::InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED;
        DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->ExecuteIntentDone(
            requestCode, resultCode, errorResult);
    } else {
        AppExecFwk::InsightIntentExecuteResult result;
        result.FromJsonString(resultMsg);
        DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->ExecuteIntentDone(
            requestCode, resultCode, result);
    }
}

void RemoteIntentResultCallback::OnLinkDisconnected(uint64_t requestCode, int32_t reason)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "OnLinkDisconnected requestCode=%{public}" PRIu64 ", reason=%{public}d", requestCode, reason);
    AppExecFwk::InsightIntentExecuteResult errorResult{};
        errorResult.innerErr = AbilityRuntime::InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED;
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->ExecuteIntentDone(
        requestCode, ERR_INTENT_DEVICE_DISCONNECTED, errorResult);
}
}  // namespace AAFwk
}  // namespace OHOS
