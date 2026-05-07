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

#include "ets_insight_intent_executor_instance.h"

#include <cstddef>
#include <dlfcn.h>

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const char *ETS_ANI_LIBNAME = "libinsight_intent_executor_ani.z.so";
using CreateETSInsightIntentFunc = InsightIntentExecutor *(*)(OHOS::AbilityRuntime::Runtime &);

InsightIntentExecutor *CreateEtsInsightIntentBySymbol(
    Runtime &runtime, const char *funcName, CreateETSInsightIntentFunc &createFunc)
{
    if (createFunc != nullptr) {
        return createFunc(runtime);
    }
    auto handle = dlopen(ETS_ANI_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "dlopen failed %{public}s, %{public}s", ETS_ANI_LIBNAME, dlerror());
        return nullptr;
    }
    auto symbol = dlsym(handle, funcName);
    if (symbol == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "dlsym failed %{public}s, %{public}s", funcName, dlerror());
        dlclose(handle);
        return nullptr;
    }
    createFunc = reinterpret_cast<CreateETSInsightIntentFunc>(symbol);
    return createFunc(runtime);
}
CreateETSInsightIntentFunc g_etsCreateExecutorFunc = nullptr;
CreateETSInsightIntentFunc g_etsCreateEntryFunc = nullptr;
CreateETSInsightIntentFunc g_etsCreateFuncFunc = nullptr;
CreateETSInsightIntentFunc g_etsCreatePageFunc = nullptr;
CreateETSInsightIntentFunc g_etsCreateQueryEntityFunc = nullptr;
} // namespace

InsightIntentExecutor *CreateETSInsightIntentExecutor(Runtime &runtime)
{
    return CreateEtsInsightIntentBySymbol(runtime, "OHOS_ETS_Insight_Intent_Executor_Create", g_etsCreateExecutorFunc);
}

InsightIntentExecutor *CreateETSInsightIntentEntry(Runtime &runtime)
{
    return CreateEtsInsightIntentBySymbol(runtime, "OHOS_ETS_Insight_Intent_Entry_Create", g_etsCreateEntryFunc);
}

InsightIntentExecutor *CreateETSInsightIntentFunc(Runtime &runtime)
{
    return CreateEtsInsightIntentBySymbol(runtime, "OHOS_ETS_Insight_Intent_Func_Create", g_etsCreateFuncFunc);
}

InsightIntentExecutor *CreateETSInsightIntentPage(Runtime &runtime)
{
    return CreateEtsInsightIntentBySymbol(runtime, "OHOS_ETS_Insight_Intent_Page_Create", g_etsCreatePageFunc);
}

InsightIntentExecutor *CreateETSInsightIntentQueryEntity(Runtime &runtime)
{
    return CreateEtsInsightIntentBySymbol(runtime, "OHOS_ETS_Insight_Intent_QueryEntity_Create",
        g_etsCreateQueryEntityFunc);
}
} // namespace AbilityRuntime
} // namespace OHOS