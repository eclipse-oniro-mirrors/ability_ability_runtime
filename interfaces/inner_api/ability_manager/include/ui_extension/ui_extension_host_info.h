/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_HOST_INFO_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_HOST_INFO_H

#include "element_name.h"
#include "parcel.h"

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionHostInfo : public Parcelable {
public:
    UIExtensionHostInfo() = default;
    virtual ~UIExtensionHostInfo() = default;

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static UIExtensionHostInfo *Unmarshalling(Parcel &parcel);

    AppExecFwk::ElementName elementName_;
    std::string sessionName;
    int32_t persistentId_ = 0;
    std::string moduleName;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_HOST_INFO_H
