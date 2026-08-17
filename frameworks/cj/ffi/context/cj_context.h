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

#ifndef OHOS_ABILITY_RUNTIME_CJ_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_CJ_CONTEXT_H

#include <memory>

#include "ffi_remote_data.h"

namespace OHOS {
namespace AbilityRuntime {
class Context;
}
namespace FfiContext {
using namespace OHOS::AbilityRuntime;

class CJContext : public FFI::FFIData {
    DECL_TYPE(CJContext, OHOS::FFI::FFIData)
public:
    explicit CJContext(std::shared_ptr<AbilityRuntime::Context> context)
        : context_(context) {};
    std::shared_ptr<AbilityRuntime::Context> GetContext()
    {
        return context_;
    }
private:
    std::shared_ptr<AbilityRuntime::Context> context_;
};
}
}
#endif // OHOS_ABILITY_RUNTIME_CJ_CONTEXT_H