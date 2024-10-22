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

#ifndef OHOS_ABILITY_RUNTIME_WINDOW_OPTIONS_UTILS_H
#define OHOS_ABILITY_RUNTIME_WINDOW_OPTIONS_UTILS_H

#include "ability_info.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "start_options.h"
#include "string_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class WindowOptionsUtils
 * provides window options utilities.
 */
class WindowOptionsUtils final {
public:
    /**
     * SetWindowPositionAndSize, set window position and size.
     *
     * @param want The want param.
     * @param callerToken The caller token.
     * @param startOptions The start options.
     */
    static void SetWindowPositionAndSize(Want& want,
        const sptr<IRemoteObject>& callerToken, const StartOptions& startOptions);

    /**
     * WindowModeMap, get window mode map.
     *
     * @param windowMode The window mode.
     * @return The pair of the window mode result and the supported window mode.
     */
    static std::pair<bool, AppExecFwk::SupportWindowMode> WindowModeMap(int32_t windowMode);

    /**
     * UpdateWantToSetDisplayID, update want to set display id.
     *
     * @param want The want param.
     * @param callerToken The caller token.
     */
    static void UpdateWantToSetDisplayID(Want &want, const sptr<IRemoteObject> &callerToken);

    /**
     * UpdateStartOptionsToSetDisplayID, update start options to set display id.
     *
     * @param startOptions The start options.
     * @param callerToken The caller token.
     */
    static void UpdateStartOptionsToSetDisplayID(StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_WINDOW_OPTIONS_UTILS_H
