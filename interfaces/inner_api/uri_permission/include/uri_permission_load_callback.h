/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_LOAD_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_LOAD_CALLBACK_H

#include "iremote_object.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace AAFwk {
class UriPermissionLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    UriPermissionLoadCallback() = default;
    virtual ~UriPermissionLoadCallback() = default;

    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_URI_PERMISSION_LOAD_CALLBACK_H