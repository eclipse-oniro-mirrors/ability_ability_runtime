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

#include "app_mem_dump_info.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

bool MemDumpInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(static_cast<uint32_t>(dumpType))) {
        return false;
    }
    if (!parcel.WriteBool(needLeakobj)) {
        return false;
    }
    if (!parcel.WriteUint32(pid)) {
        return false;
    }
    if (!parcel.WriteUint32(tid)) {
        return false;
    }
    if (!parcel.WriteBool(isSync)) {
        return false;
    }
    return true;
}

MemDumpInfo *MemDumpInfo::Unmarshalling(Parcel &parcel)
{
    MemDumpInfo *info = new (std::nothrow) MemDumpInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "info nullptr");
        return nullptr;
    }

    uint32_t dumpTypeValue = 0;
    if (!parcel.ReadUint32(dumpTypeValue)) {
        delete info;
        return nullptr;
    }
    info->dumpType = static_cast<MemDumpType>(dumpTypeValue);

    if (!parcel.ReadBool(info->needLeakobj)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadUint32(info->pid)) {
        delete info;
        return nullptr;
    }
    if (info->pid == 0) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadUint32(info->tid)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadBool(info->isSync)) {
        delete info;
        return nullptr;
    }

    return info;
}

} // namespace AppExecFwk
} // namespace OHOS
