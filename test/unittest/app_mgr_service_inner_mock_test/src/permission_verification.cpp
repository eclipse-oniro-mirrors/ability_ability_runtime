/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "permission_verification.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
int PermissionVerification::flag = 0;
bool PermissionVerification::hasSuperviseKiaServicePermission = false;

bool PermissionVerification::IsSACall()
{
    return (flag == FLAG::IS_SA_CALL);
}

bool PermissionVerification::VerifySuperviseKiaServicePermission()
{
    if (IsSACall() && hasSuperviseKiaServicePermission) {
        TAG_LOGD(AAFwkTag::DEFAULT, "Permission granted");
        return true;
    }
    TAG_LOGE(AAFwkTag::DEFAULT, "Permission denied");
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
