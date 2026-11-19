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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_RESULT_H
#define OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_RESULT_H

#include <string>
#include <vector>

#include "parcel.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {

using WantParams = OHOS::AAFwk::WantParams;

struct SkillExecuteResult : public Parcelable {
public:
    int32_t code = 0;
    std::shared_ptr<WantParams> result = nullptr;
    std::vector<std::string> uris;
    uint32_t flags = 0;

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static SkillExecuteResult *Unmarshalling(Parcel &parcel);
};

} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_RESULT_H
