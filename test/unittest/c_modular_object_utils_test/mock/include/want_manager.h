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

#ifndef WANT_MANAGER_H
#define WANT_MANAGER_H

#include "want.h"

namespace OHOS {
namespace AAFwk {

class Want {};

class CWantManager {
public:
    static AbilityBase_ErrorCode TransformToWant(AbilityBase_Want &cWant, bool flag, Want &abilityWant);
};

} // namespace AAFwk
} // namespace OHOS

#endif // WANT_MANAGER_H
