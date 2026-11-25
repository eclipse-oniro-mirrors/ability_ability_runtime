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

#include "pending_want_common_event.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
PendingWantCommonEvent::PendingWantCommonEvent()
{}

void PendingWantCommonEvent::SetWantParams(const WantParams &wantParams)
{
    std::lock_guard<std::mutex> lock(mutex_);
    wantParams_ = wantParams;
}

void PendingWantCommonEvent::SetFinishedReceiver(const sptr<IWantReceiver> &finishedReceiver)
{
    std::lock_guard<std::mutex> lock(mutex_);
    finishedReceiver_ = finishedReceiver;
}

void PendingWantCommonEvent::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    sptr<IWantReceiver> finishedReceiver = nullptr;
    WantParams wantParams;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        finishedReceiver = finishedReceiver_;
        wantParams = wantParams_;
    }
    if (finishedReceiver != nullptr) {
        TAG_LOGI(AAFwkTag::WANTAGENT, "begin");
        finishedReceiver->PerformReceive(data.GetWant(), data.GetCode(), "", wantParams, false, false, 0);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
