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

#ifndef ABILITY_ECOLOGICALRULEMANAGERSERVICE_PARAM_H
#define ABILITY_ECOLOGICALRULEMANAGERSERVICE_PARAM_H

#include <string>
#include <vector>

#include "parcel.h"
#include "want.h"
#include "ability_info.h"

namespace OHOS {
namespace EcologicalRuleMgrService {
using Want = OHOS::AAFwk::Want;

struct AbilityExperienceRule : public Parcelable {
    int32_t resultCode = 10;
    std::string sceneCode = "";
    sptr<Want> replaceWant = nullptr;
    bool isBackSkuExempt = false;
    int32_t embedResultCode = 1;

    bool Marshalling(Parcel &parcel) const override;

    static AbilityExperienceRule *Unmarshalling(Parcel &parcel);
};

struct AbilityCallerInfo : public Parcelable {
    enum {
        TYPE_INVALID = 0,
        TYPE_HARMONY_APP,
        TYPE_ATOM_SERVICE,
        TYPE_QUICK_APP = 4,
        TYPE_BOXED_ATOM_SERVICE,
        TYPE_APP_SERVICE
    };

    enum {
        MODEL_STAGE = 0,
        MODEL_FA
    };

    enum {
        LINK_TYPE_INVALID = 0,
        LINK_TYPE_UNIVERSAL_LINK,
        LINK_TYPE_DEEP_LINK,
        LINK_TYPE_WEB_LINK,
        LINK_TYPE_ABILITY
    };

    std::string packageName;
    int32_t uid = 0L;
    int32_t pid = 0L;

    int32_t callerAppType = TYPE_INVALID;
    int32_t targetAppType = TYPE_INVALID;
    int32_t callerModelType = MODEL_STAGE;
    std::string targetAppDistType = "";
    std::string targetLinkFeature = "";
    int32_t targetLinkType = LINK_TYPE_INVALID;
    AppExecFwk::AbilityType callerAbilityType = AppExecFwk::AbilityType::UNKNOWN;
    int32_t embedded = 0;
    std::string callerAppProvisionType;
    std::string targetAppProvisionType;
    AppExecFwk::ExtensionAbilityType callerExtensionAbilityType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    AppExecFwk::AbilityType targetAbilityType = AppExecFwk::AbilityType::UNKNOWN;
    AppExecFwk::ExtensionAbilityType targetExtensionAbilityType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    int32_t userId = 0;
    int32_t targetApplicationReservedFlag = 0;
    bool isAsCaller = false;
    std::string targetBundleName;

    bool ReadFromParcel(Parcel &parcel);

    bool Marshalling(Parcel &parcel) const override;
    bool DoMarshallingOne(Parcel& parcel) const;
    bool DoMarshallingTwo(Parcel &parcel) const;

    static AbilityCallerInfo *Unmarshalling(Parcel &parcel);

    std::string ToString() const;
};

} // namespace EcologicalRuleMgrService
} // namespace OHOS
#endif // ABILITY_ECOLOGICALRULEMANAGERSERVICE_PARAM_H
