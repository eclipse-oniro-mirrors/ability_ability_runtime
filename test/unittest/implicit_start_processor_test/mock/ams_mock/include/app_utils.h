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

#ifndef OHOS_ABILITY_RUNTIME_APP_UTILS_H
#define OHOS_ABILITY_RUNTIME_APP_UTILS_H

#include "mock_app_utils_status.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AppUtils
 * Test double that shadows services/common/include/app_utils.h for targets compiling
 * implicit_start_processor.cpp locally. The real implementation lives in the shared
 * libapp_util.z.so and resolves system::GetBoolParameter from libbegetutil, so the
 * parameter mock compiled into this test binary cannot intercept it. Shadowing the
 * header keeps the selector-position query deterministic and per-test controllable.
 */
class AppUtils {
public:
    /**
     * GetInstance, get an instance of AppUtils.
     *
     * @return An instance of AppUtils.
     */
    static AppUtils &GetInstance()
    {
        static AppUtils instance;
        return instance;
    }

    /**
     * IsSelectorDialogDefaultPossion, whether the selector dialog stays in the default
     * (phone) position. Mirrors the on-device default of
     * persist.sys.abilityms.selector_dialog_possion (unset -> true -> phone layout).
     * Tests exercising the PC "more apps" wildcard flow switch the flag via
     * MockAppUtilsStatus.
     */
    bool IsSelectorDialogDefaultPossion()
    {
        return MockAppUtilsStatus::isSelectorDialogDefaultPossion_;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_UTILS_H
