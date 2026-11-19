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

#include "focused_application_info.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool FocusedApplicationInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteInt32(focusedUid) && parcel.WriteString(focusedBundleName);
}

bool FocusedApplicationInfo::ReadFromParcel(Parcel &parcel)
{
    return parcel.ReadInt32(focusedUid) && parcel.ReadString(focusedBundleName);
}

FocusedApplicationInfo *FocusedApplicationInfo::Unmarshalling(Parcel &parcel)
{
    FocusedApplicationInfo *data = new (std::nothrow) FocusedApplicationInfo();
    if (data && !data->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "FocusedApplicationInfo failed, because ReadFromParcel failed");
        delete data;
        data = nullptr;
    }
    return data;
}
}  // namespace AppExecFwk
}  // namespace OHOS
