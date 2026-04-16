/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DMS_UTIL_H
#define OHOS_ABILITY_RUNTIME_DMS_UTIL_H

#include "want.h"

namespace OHOS {
namespace AAFwk {
class DmsUtil {
public:

    static DmsUtil &GetInstance();

    void UpdateFlagForCollaboration(const Want &want);

    /**
     * Anonymize device ID for logging (show first 6 chars + 6 asterisks).
     * @param deviceId The original device ID.
     * @return Anonymized device ID string.
     */
    static std::string AnonymizeDeviceId(const std::string& deviceId);

private:
    DmsUtil() = default;
    ~DmsUtil() = default;
    DISALLOW_COPY_AND_MOVE(DmsUtil);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DMS_UTIL_H