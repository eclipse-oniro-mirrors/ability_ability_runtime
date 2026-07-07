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

#include "app_jshandle_map_info.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool JsHandleMapInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteUint32(pid) && parcel.WriteUint32(tid);
}

JsHandleMapInfo *JsHandleMapInfo::Unmarshalling(Parcel &parcel)
{
    JsHandleMapInfo *info = new (std::nothrow) JsHandleMapInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "info nullptr");
        return nullptr;
    }
    if (!parcel.ReadUint32(info->pid) || !parcel.ReadUint32(info->tid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read param fail");
        delete info;
        return nullptr;
    }
    return info;
}
} // namespace AppExecFwk
} // namespace OHOS