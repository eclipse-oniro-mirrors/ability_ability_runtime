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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_ARGS_FFI_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_ARGS_FFI_H

#include <map>
#include <string>

#include "want.h"
#include "ability_delegator_registry.h"
#include "cj_macro.h"
#include "cj_common_ffi.h"
#include "ffi_remote_data.h"

namespace OHOS {
namespace AbilityDelegatorArgsCJ {

class CJAbilityDelegatorArgs : public FFI::FFIData {
    DECL_TYPE(CJAbilityDelegatorArgs, OHOS::FFI::FFIData)
public:
    explicit CJAbilityDelegatorArgs(const std::shared_ptr<AppExecFwk::AbilityDelegatorArgs>& abilityDelegatorArgs)
        : delegatorArgs_(abilityDelegatorArgs) {};
    std::string GetTestBundleName();
    std::map<std::string, std::string> GetTestParam();
    std::string GetTestCaseName();
    std::string GetTestRunnerClassName();
private:
    std::shared_ptr<AppExecFwk::AbilityDelegatorArgs> delegatorArgs_;
};

struct CRecord {
    CArrString keys;
    CArrString values;
};

extern "C" {
CJ_EXPORT int64_t FfiAbilityDelegatorRegistryGetArguments();
CJ_EXPORT char* FfiAbilityDelegatorArgsGetTestBundleName(int64_t id, int32_t *errCode);
CJ_EXPORT CRecord FfiAbilityDelegatorArgsGetTestParam(int64_t id, int32_t *errCode);
CJ_EXPORT char* FfiAbilityDelegatorArgsGetTestCaseName(int64_t id, int32_t *errCode);
CJ_EXPORT char* FfiAbilityDelegatorArgsGetTestRunnerClassName(int64_t id, int32_t *errCode);
};
}
}
#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_ARGS_FFI_H