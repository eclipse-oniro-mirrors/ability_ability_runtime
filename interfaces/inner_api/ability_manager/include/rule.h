/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RULE_H
#define OHOS_ABILITY_RUNTIME_RULE_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace AbilityRuntime {
enum class RuleType {
    ALLOW = 0,
    NOT_ALLOW,
    USER_SELECTION,
    NOT_ALLOW_FROZEN
};

/**
 * @struct Rule
 * Defines Rule.
 */
struct Rule : public Parcelable {
    RuleType type = RuleType::ALLOW;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static Rule *Unmarshalling(Parcel &parcel);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_RULE_H