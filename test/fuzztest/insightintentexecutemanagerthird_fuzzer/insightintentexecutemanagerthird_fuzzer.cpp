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

#include "insightintentexecutemanagerthird_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#define private public
#include "insight_intent_execute_manager.h"
#include "insight_intent_query_param.h"
#include "string_wrapper.h"
#undef private

#include "ability_fuzz_util.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
} // namespace

void ExerciseCheckEntityQueryable(FuzzedDataProvider &fdp, const std::shared_ptr<InsightIntentExecuteManager> &mgr)
{
    ExtractInsightIntentInfo intentInfo;
    AbilityFuzzUtil::GetRandomExtractInsightIntentInfo(fdp, intentInfo);
    std::string className = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    InsightIntentQueryEntityParam queryEntityParam;
    queryEntityParam.queryType_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    if (fdp.ConsumeBool()) {
        queryEntityParam.parameters_ = std::make_shared<WantParams>();
        queryEntityParam.parameters_->SetParam(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH),
            String::Box(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH)));
    }
    mgr->CheckEntityQueryable(intentInfo, className, queryEntityParam);

    queryEntityParam.queryType_ = "all";
    mgr->CheckEntityQueryable(intentInfo, className, queryEntityParam);
    queryEntityParam.queryType_ = "byProperty";
    mgr->CheckEntityQueryable(intentInfo, className, queryEntityParam);
}

void ExerciseQueryEntityWant(FuzzedDataProvider &fdp, const std::shared_ptr<InsightIntentExecuteManager> &mgr)
{
    std::shared_ptr<AppExecFwk::InsightIntentQueryParam> param =
        std::make_shared<AppExecFwk::InsightIntentQueryParam>();
    param->bundleName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param->moduleName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param->intentName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param->className_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param->userId_ = fdp.ConsumeIntegral<int32_t>();
    Want want;
    mgr->GenerateQueryEntityWant(param, want);
    mgr->GenerateQueryEntityWant(nullptr, want);

    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = fdp.ConsumeIntegral<uint64_t>();
    std::string callerBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    mgr->CheckAndUpdateQueryEntityParam(key, callerToken, param, callerBundleName);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto mgr = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    if (mgr == nullptr) {
        return false;
    }
    ExerciseCheckEntityQueryable(fdp, mgr);
    ExerciseQueryEntityWant(fdp, mgr);

    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    mgr->GetMainElementName(bundleName, moduleName, userId);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
