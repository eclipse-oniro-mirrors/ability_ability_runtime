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

#include "skill/skill_execute_callback_proxy.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

void SkillExecuteCallbackProxy::OnExecuteDone(const std::string &requestCode, int32_t resultCode,
    const AppExecFwk::SkillExecuteResult &result)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "skill execute callback proxy, requestCode:%{public}s code:%{public}d",
        requestCode.c_str(), resultCode);
    MessageParcel data;
    if (!data.WriteInterfaceToken(ISkillExecuteCallback::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }
    if (!data.WriteString(requestCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write requestCode failed");
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write resultCode failed");
        return;
    }
    if (!data.WriteParcelable(&result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t error = remote->SendRequest(ON_SKILL_EXECUTE_DONE, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest fail, error:%{public}d", error);
    }
}
} // namespace AAFwk
} // namespace OHOS
