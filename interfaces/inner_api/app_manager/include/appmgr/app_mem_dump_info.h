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

#ifndef OHOS_ABILITY_RUNTIME_APP_MEM_DUMP_INFO_H
#define OHOS_ABILITY_RUNTIME_APP_MEM_DUMP_INFO_H

#include "parcel.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
enum class MemDumpType : uint32_t {
    INVALID = 0,
    NATIVE = 1,
    JSVM = 2,
    KMP_KOTLIN = 3,
};

struct MemDumpInfo : public Parcelable {
    MemDumpType dumpType = MemDumpType::INVALID;
    bool needLeakobj = false;
    bool needRaw = false;
    uint32_t pid = 0;
    uint32_t tid = 0;
    bool isSync = false;
    bool mayReportToOEM = false;
    virtual bool Marshalling(Parcel &parcel) const override;
    static MemDumpInfo *Unmarshalling(Parcel &parcel);
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_APP_MEM_DUMP_INFO_H
