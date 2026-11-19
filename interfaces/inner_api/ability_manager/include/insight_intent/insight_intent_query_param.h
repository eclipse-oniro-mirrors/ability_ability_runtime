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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_QUERY_PARAM_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_QUERY_PARAM_H

#include <string>

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;
class InsightIntentQueryEntityParam {
public:
    std::string queryType_;
    std::shared_ptr<WantParams> parameters_;
};

class InsightIntentQueryParam : public Parcelable {
    const int32_t DEFAULT_INVAL_VALUE = -1;
public:
    InsightIntentQueryParam() = default;
    ~InsightIntentQueryParam() = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static InsightIntentQueryParam *Unmarshalling(Parcel &parcel);

    std::string bundleName_;
    std::string moduleName_;
    std::string intentName_;
    std::string className_;
    InsightIntentQueryEntityParam queryEntityParam_;
    
    int32_t userId_ = DEFAULT_INVAL_VALUE;
    uint64_t intentId_ = 0;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_QUERY_PARAM_H
