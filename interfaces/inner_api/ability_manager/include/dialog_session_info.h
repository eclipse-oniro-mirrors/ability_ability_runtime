/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DIALOG_SESSION_INFO_H
#define OHOS_ABILITY_RUNTIME_DIALOG_SESSION_INFO_H

#include <string>
#include <vector>

#include "application_info.h"
#include "json_serializer.h"
#include "parcel.h"
#include "refbase.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AAFwk {
struct DialogAbilityInfo {
    bool visible = true;
    int32_t bundleIconId = 0;
    int32_t bundleLabelId = 0;
    int32_t abilityIconId = 0;
    int32_t abilityLabelId = 0;
    int32_t appIndex = 0;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string codePath = "";
    std::string installSource = "";
    AppExecFwk::MultiAppModeData multiAppMode;

    std::string GetURI() const;
    bool ParseURI(const std::string &uri);
    void Split(const std::string &str, const std::string &delim, std::vector<std::string> &vec);
};

struct DialogSessionInfo : public Parcelable {
    DialogAbilityInfo callerAbilityInfo;
    std::vector<DialogAbilityInfo> targetAbilityInfos;
    AAFwk::WantParams parameters;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static DialogSessionInfo *Unmarshalling(Parcel &parcel);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DIALOG_SESSION_INFO_H
