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

#include "ets_ability_stage_instance.h"

#include <cstddef>
#include <dlfcn.h>

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const char *ETS_ANI_LIBNAME = "libability_stage_ani.z.so";
const char *ETS_ANI_CREATE_FUNC = "OHOS_ETS_Ability_Stage_Create";
using CreateETSAbilityStageFunc = AbilityStage*(*)(const std::unique_ptr<Runtime>&,
    const AppExecFwk::HapModuleInfo&);
CreateETSAbilityStageFunc g_etsCreateFunc = nullptr;
}

AbilityStage *CreateETSAbilityStage(const std::unique_ptr<Runtime> &runtime,
    const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    if (g_etsCreateFunc != nullptr) {
        return g_etsCreateFunc(runtime, hapModuleInfo);
    }
    auto handle = dlopen(ETS_ANI_LIBNAME, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "dlopen failed %{public}s, %{public}s", ETS_ANI_LIBNAME, dlerror());
        return nullptr;
    }
    auto symbol = dlsym(handle, ETS_ANI_CREATE_FUNC);
    if (symbol == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "dlsym failed %{public}s, %{public}s", ETS_ANI_CREATE_FUNC, dlerror());
        dlclose(handle);
        return nullptr;
    }
    g_etsCreateFunc = reinterpret_cast<CreateETSAbilityStageFunc>(symbol);
    return g_etsCreateFunc(runtime, hapModuleInfo);
}
} // namespace AbilityRuntime
} // namespace OHOS