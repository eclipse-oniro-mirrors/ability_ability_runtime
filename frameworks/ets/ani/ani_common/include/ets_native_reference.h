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

#ifndef OHOS_ABILITY_RUNTIME_ETS_NATIVE_REFERENCE_H
#define OHOS_ABILITY_RUNTIME_ETS_NATIVE_REFERENCE_H

#include <string>
#include "ani.h"

namespace OHOS {
namespace AppExecFwk {
struct ETSNativeReference {
    ani_class aniCls = nullptr;
    ani_object aniObj = nullptr;
    ani_ref aniRef = nullptr;
};
}
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_NATIVE_REFERENCE_H
