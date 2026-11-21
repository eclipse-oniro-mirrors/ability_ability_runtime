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

#ifndef MOCK_OHOS_ROSEN_MOCK_SESSION_MANAGER_SERVICE_H
#define MOCK_OHOS_ROSEN_MOCK_SESSION_MANAGER_SERVICE_H

#include "errors.h"
#include "iremote_object.h"
#include "refbase.h"

namespace OHOS {
namespace Rosen {

struct WindowCreateParams;
class ISceneSessionManagerLite;

class MockSessionManagerService {
public:
    static ErrCode verifyParamsResult_;

    static MockSessionManagerService& GetInstance()
    {
        static MockSessionManagerService instance;
        return instance;
    }

    bool SetSessionManagerService(const sptr<IRemoteObject>& sessionManagerService)
    {
        return false;
    }

    void NotifyNotKillService() {}

    ErrCode VerifyParamsForStartAbilityToBottom(const sptr<WindowCreateParams>& windowCreateParams);

    sptr<ISceneSessionManagerLite> GetSceneSessionManagerLiteBySA(int32_t userId);
};

} // namespace Rosen
} // namespace OHOS
#endif // MOCK_OHOS_ROSEN_MOCK_SESSION_MANAGER_SERVICE_H
