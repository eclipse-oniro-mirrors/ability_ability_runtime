/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H

#include "gmock/gmock.h"
#include "app_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "param.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::AppExecFwk;
class MockAppMgrClient : public AppMgrClient {
public:
    MockAppMgrClient() {};
    virtual ~MockAppMgrClient() {};

    virtual AppMgrResultCode LoadAbility(const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo,
        const AAFwk::Want &want, AbilityRuntime::LoadParam loadParam)
    {
        TAG_LOGI(AAFwkTag::TEST, "MockAppMgrClient LoadAbility enter.");
        token_ = loadParam.token;
        return AppMgrResultCode::RESULT_OK;
    }

    sptr<IRemoteObject> GetToken()
    {
        return token_;
    };

private:
    sptr<IRemoteObject> token_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H
