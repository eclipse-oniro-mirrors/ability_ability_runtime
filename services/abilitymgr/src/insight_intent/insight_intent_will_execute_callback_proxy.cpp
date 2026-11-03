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

#include "insight_intent_will_execute_callback_proxy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
void InsightIntentWillExecuteCallbackProxy::OnWillInsightIntentExecute(
    const AppExecFwk::InsightIntentExecutionInfo &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "will execute callback proxy called, intent:%{public}s", info.intentName.c_str());
    MessageParcel data;
    if (!data.WriteInterfaceToken(IInsightIntentWillExecuteCallback::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::INTENT, "interface token write failed");
        return;
    }
    if (!data.WriteParcelable(&info)) {
        TAG_LOGE(AAFwkTag::INTENT, "execution info Parcelable write failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null remote");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = remote->SendRequest(ON_WILL_INSIGHT_INTENT_EXECUTE, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "SendRequest fail, error: %{public}d", error);
    }
}
} // namespace AAFwk
} // namespace OHOS