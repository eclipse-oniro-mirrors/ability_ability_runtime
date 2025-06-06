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

#ifndef OHOS_ABILITY_RUNTIME_MEMORY_LEVEL_INFO_H
#define OHOS_ABILITY_RUNTIME_MEMORY_LEVEL_INFO_H

#include <string>
#include <map>
#include <unistd.h>

#include "app_mem_info.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {

class MemoryLevelInfo : public Parcelable {
public:
    MemoryLevelInfo() = default;
    MemoryLevelInfo(const std::map<pid_t, MemoryLevel> &procLevelMap);
    
    virtual bool Marshalling(Parcel &parcel) const override;

    static MemoryLevelInfo *Unmarshalling(Parcel &parcel);

    const std::map<pid_t, MemoryLevel> &GetProcLevelMap() const;

private:
    bool ReadFromParcel(Parcel &parcel);

    std::map<pid_t, MemoryLevel> procLevelMap_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MEMORY_LEVEL_INFO_H