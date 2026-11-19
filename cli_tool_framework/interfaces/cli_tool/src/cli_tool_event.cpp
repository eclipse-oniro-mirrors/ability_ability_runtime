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

#include "cli_tool_event.h"

namespace OHOS {
namespace CliTool {
bool CliToolEvent::Marshalling(Parcel &parcel) const
{
    return parcel.WriteString(type) &&
           parcel.WriteString(eventData) &&
           parcel.WriteInt32(exitCode) &&
           parcel.WriteInt64(timestamp);
}

CliToolEvent *CliToolEvent::Unmarshalling(Parcel &parcel)
{
    auto *event = new (std::nothrow) CliToolEvent();
    if (event == nullptr) {
        return nullptr;
    }

    if (!parcel.ReadString(event->type) ||
        !parcel.ReadString(event->eventData) ||
        !parcel.ReadInt32(event->exitCode) ||
        !parcel.ReadInt64(event->timestamp)) {
        delete event;
        return nullptr;
    }

    return event;
}
} // namespace CliTool
} // namespace OHOS