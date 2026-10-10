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

#ifndef OHOS_INNER_API_APP_MANAGER_TASK_MGR_ATTRIBUTION_RECORD_H
#define OHOS_INNER_API_APP_MANAGER_TASK_MGR_ATTRIBUTION_RECORD_H

#include <cstdint>
#include <sys/types.h>

#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
// Which tool chain attributed a process into a task. Each tool passes its own type when
// calling RegisterManagedProcess; new tools extend this list.
enum class AttributionTool : int32_t {
    UNKNOWN = 0,
    INSIGHT = 1,
    CLI = 2,
    A2A = 3,
    ABILITY = 4,
};

// A process attributed into a task by a tool chain. This is the IPC contract of the
// RegisterManagedProcess entry and the stored entry of TaskRecord::attributions:
// the record carries the most basic attribution information, and each tool may derive
// its own struct on top of it to prepare richer, tool-specific context. Note that the
// IPC boundary transfers the base fields only (pid and tool; the server stamps the
// attribution time), so derived fields are for in-process preparation and must be
// flattened into base fields or a future extension field before crossing IPC.
struct AttributionRecord : public Parcelable {
    pid_t pid = -1;
    AttributionTool tool = AttributionTool::UNKNOWN;
    int64_t attributedTimeMillis = 0;  // server-authoritative, set at registration

    bool Marshalling(Parcel &parcel) const override;
    static AttributionRecord *Unmarshalling(Parcel &parcel);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_INNER_API_APP_MANAGER_TASK_MGR_ATTRIBUTION_RECORD_H
