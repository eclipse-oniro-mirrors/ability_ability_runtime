/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_CONTEXT_H

#include <cstdint>

#include "cj_common_ffi.h"
#include "ffi_remote_data.h"
#include "hap_module_info.h"

namespace OHOS {
namespace AbilityRuntime {

class Context;

class CJAbilityStageContext : public FFI::FFIData {
public:
    explicit CJAbilityStageContext(std::weak_ptr<AbilityRuntime::Context> &&abilityStageContext)
        :abilityStageContext_(std::move(abilityStageContext)){};

    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo();
    std::shared_ptr<Context> GetContext()
    {
        return abilityStageContext_.lock();
    }

private:
    std::weak_ptr<AbilityRuntime::Context> abilityStageContext_;
};
}
}
#endif //OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_CONTEXT_H