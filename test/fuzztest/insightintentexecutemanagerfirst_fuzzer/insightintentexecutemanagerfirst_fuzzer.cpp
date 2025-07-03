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

#include "insightintentexecutemanagerfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "insight_intent_execute_manager.h"
#include "insight_intent_execute_param.h"
#undef private

#include "ability_fuzz_util.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    uint64_t key;
    sptr<IRemoteObject> callerToken;
    std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> param =
        std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    std::string callerBundleName;
    bool ignoreAbilityName;
    Want want;
    ExtractInsightIntentInfo info;
    ExtractInsightIntentGenericInfo decoratorInfo;
    FuzzedDataProvider fdp(data, size);
    key = fdp.ConsumeIntegral<uint64_t>();
    callerBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    ignoreAbilityName = fdp.ConsumeBool();
    AbilityFuzzUtil::GetRandomExtractInsightIntentInfo(fdp, info);
    AbilityFuzzUtil::GetRandomExtractInsightIntentGenericInfo(fdp, decoratorInfo);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateParam(key, callerToken,
        param, callerBundleName, ignoreAbilityName);
    ExecuteMode executeMode = UI_ABILITY_FOREGROUND;
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateWant(want, executeMode,
        callerBundleName);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->UpdateEntryDecoratorParams(want, executeMode);
    executeMode = UI_ABILITY_BACKGROUND;
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateWant(want, executeMode,
        callerBundleName);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->UpdateEntryDecoratorParams(want, executeMode);
    executeMode = UI_EXTENSION_ABILITY;
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateWant(want, executeMode,
        callerBundleName);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->UpdateEntryDecoratorParams(want, executeMode);
    executeMode = SERVICE_EXTENSION_ABILITY;
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateWant(want, executeMode,
        callerBundleName);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->UpdateEntryDecoratorParams(want, executeMode);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->AddWantUirsAndFlagsFromParam(param, want);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->UpdateFuncDecoratorParams(param, info, want);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->GetMainElementName(param);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->UpdatePageDecoratorParams(param, info, want);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->UpdateEntryDecoratorParams(param, info, want);
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateDecoratorParams(param,
        decoratorInfo, want);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}