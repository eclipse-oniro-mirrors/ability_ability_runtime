/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_STARTUP_INFO_H
#define OHOS_ABILITY_RUNTIME_AUTO_STARTUP_INFO_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @struct AutoStartupInfo
 * Defines auto startup info.
 */
struct AutoStartupInfo : public Parcelable {
public:
    std::string bundleName;
    std::string abilityName;
    std::string moduleName;
    std::string abilityTypeName;
    std::string accessTokenId;
    int32_t appCloneIndex = 0;
    int32_t userId = -1;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static AutoStartupInfo *Unmarshalling(Parcel &parcel);
};

struct AutoStartupStatus {
    int32_t code;
    bool isAutoStartup;
    bool isEdmForce;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_STARTUP_INFO_H