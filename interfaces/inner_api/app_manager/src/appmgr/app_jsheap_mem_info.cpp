/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "app_jsheap_mem_info.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool JsHeapDumpInfo::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteUint32(pid) && parcel.WriteUint32(tid)
        && parcel.WriteBool(needGc) && parcel.WriteBool(needSnapshot)
        && parcel.WriteBool(needLeakobj) && parcel.WriteBool(needBinary)
        && parcel.WriteBool(needClearNodeIdCache));
}

JsHeapDumpInfo *JsHeapDumpInfo::Unmarshalling(Parcel &parcel)
{
    JsHeapDumpInfo *info = new (std::nothrow) JsHeapDumpInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "info nullptr");
        return nullptr;
    }
    info->pid = parcel.ReadUint32();
    info->tid = parcel.ReadUint32();
    info->needGc = parcel.ReadBool();
    info->needSnapshot = parcel.ReadBool();
    info->needLeakobj = parcel.ReadBool();
    info->needBinary = parcel.ReadBool();
    info->needClearNodeIdCache = parcel.ReadBool();
    return info;
}
} // namespace AppExecFwk
} // namespace OHOS
