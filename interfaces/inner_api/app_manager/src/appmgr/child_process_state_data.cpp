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

#include "child_process_state_data.h"

#include "hilog_tag_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {
bool ChildProcessStateData::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteInt32(pid) && parcel.WriteInt32(uid) && parcel.WriteInt32(callingPid) &&
        parcel.WriteInt32(callingUid) && parcel.WriteInt32(state));
}

bool ChildProcessStateData::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid read failed.");
        return false;
    }
    if (!parcel.ReadInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "uid read failed.");
        return false;
    }
    if (!parcel.ReadInt32(callingPid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "callingPid read failed.");
        return false;
    }
    if (!parcel.ReadInt32(callingUid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "callingUid read failed.");
        return false;
    }
    if (!parcel.ReadInt32(state)) {
        TAG_LOGE(AAFwkTag::APPMGR, "state read failed.");
        return false;
    }
    return true;
};

ChildProcessStateData *ChildProcessStateData::Unmarshalling(Parcel &parcel)
{
    ChildProcessStateData *childProcessStateData = new (std::nothrow) ChildProcessStateData();
    if (childProcessStateData && !childProcessStateData->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "childProcessStateData failed, because ReadFromParcel failed");
        delete childProcessStateData;
        childProcessStateData = nullptr;
    }
    return childProcessStateData;
}
}
}