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

#ifndef MOCK_C_MODULAR_OBJECT_UTILS_H
#define MOCK_C_MODULAR_OBJECT_UTILS_H

#include <securec.h>
#include <string>

#include "element_name.h"
#include "connect_options_impl.h"

struct AbilityBase_Element {
    char *bundleName = nullptr;
    char *moduleName = nullptr;
    char *abilityName = nullptr;
};

namespace OHOS {
namespace AbilityRuntime {

class CModularObjectUtils {
public:
    static bool buildElementResult;
    static bool notifyFailedCalled;
    static int32_t notifyFailedCode;
    static int32_t convertConnectResult;

    static bool BuildElement(const AppExecFwk::ElementName &elementName, AbilityBase_Element &element)
    {
        if (!buildElementResult) {
            return false;
        }
        std::string bn = elementName.GetBundleName();
        element.bundleName = new char[bn.size() + 1];
        (void)strcpy_s(element.bundleName, bn.size() + 1, bn.c_str());
        std::string an = elementName.GetAbilityName();
        element.abilityName = new char[an.size() + 1];
        (void)strcpy_s(element.abilityName, an.size() + 1, an.c_str());
        return true;
    }

    static void DestroyElement(AbilityBase_Element &element)
    {
        delete[] element.bundleName;
        delete[] element.moduleName;
        delete[] element.abilityName;
        element.bundleName = nullptr;
        element.moduleName = nullptr;
        element.abilityName = nullptr;
    }

    static void NotifyFailed(std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> state, int32_t code)
    {
        notifyFailedCalled = true;
        notifyFailedCode = code;
        if (state == nullptr || !state->alive || state->onFailedCallback == nullptr) {
            return;
        }
        state->onFailedCallback(state->owner, static_cast<AbilityRuntime_ErrorCode>(code));
    }

    static AbilityRuntime_ErrorCode ConvertConnectBusinessErrorCode(int32_t errCode)
    {
        if (convertConnectResult != 0) {
            return static_cast<AbilityRuntime_ErrorCode>(convertConnectResult);
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_C_MODULAR_OBJECT_UTILS_H
