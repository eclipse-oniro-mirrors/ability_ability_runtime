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

#ifndef OHOS_ABILITY_RUNTIME_FUNCTION_CALL_EXECUTE_PARAM_H
#define OHOS_ABILITY_RUNTIME_FUNCTION_CALL_EXECUTE_PARAM_H

#include <string>

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

/**
 * @class FunctionCallExecuteParam
 * Parameters of ExecuteIntentByFunctionCall. The caller ability token and the
 * pre-registered execute record id are carried as dedicated fields instead of
 * being boxed into the business WantParams, so system parameters never leak
 * into the target Want.
 */
class FunctionCallExecuteParam : public Parcelable {
public:
    FunctionCallExecuteParam() = default;
    ~FunctionCallExecuteParam() override = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static FunctionCallExecuteParam *Unmarshalling(Parcel &parcel);

    std::string bundleName;
    std::string intentName;
    WantParams wantParam;
    sptr<IRemoteObject> callerAbilityToken;
    int32_t intentId = 0;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FUNCTION_CALL_EXECUTE_PARAM_H