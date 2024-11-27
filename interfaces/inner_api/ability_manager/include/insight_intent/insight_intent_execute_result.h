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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_RESULT_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_RESULT_H

#include "insight_intent_constant.h"
#include "parcel.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;
constexpr char INSIGHT_INTENT_EXECUTE_RESULT_CODE[] = "ohos.insightIntent.executeResultCode";
constexpr char INSIGHT_INTENT_EXECUTE_RESULT[] = "ohos.insightIntent.executeResult";

/**
 * @struct InsightIntentExecuteResult
 * InsightIntentExecuteResult is used to save information about execute result.
 */
struct InsightIntentExecuteResult : public Parcelable {
public:
    int32_t innerErr = AbilityRuntime::InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK;
    int32_t code = 0;
    std::shared_ptr<WantParams> result = nullptr;

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static InsightIntentExecuteResult *Unmarshalling(Parcel &parcel);
    // Check result returned by intent executor
    static bool CheckResult(std::shared_ptr<const WantParams> result);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_RESULT_H
