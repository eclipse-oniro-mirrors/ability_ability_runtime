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

#ifndef MOCK_WANT_MANAGER_H
#define MOCK_WANT_MANAGER_H

#include "want.h"
#include "ability_base_error.h"
#include "mock_types.h"
#include <map>
#include <string>

struct AbilityBase_Want {
    AbilityBase_Element element;
    std::map<std::string, std::string> params;
    int flag = 0;
};

namespace OHOS {
namespace AAFwk {
class CWantManager {
public:
    static int TransformToWant(const AbilityBase_Want &cWant, bool flag, Want &abilityWant)
    {
        return g_transformResult;
    }

    static int TransformToCWantWithoutElement(const Want &want, bool flag, AbilityBase_Want &cWant)
    {
        return g_transformResult;
    }

    static int g_transformResult;
};

} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_WANT_MANAGER_H
