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

#ifndef MOCK_BUNDLE_MGR_STUB_H
#define MOCK_BUNDLE_MGR_STUB_H

#include <vector>
#include "bundlemgr/bundle_mgr_interface.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {

class MockBundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    MockBundleMgrStub() = default;
    ~MockBundleMgrStub() override = default;

    sptr<IRemoteObject> AsObject() override
    {
        return this;
    }

    ErrCode QueryAbilityInfosV9(
        const Want &want, int32_t flags, int32_t userId, std::vector<AbilityInfo> &abilityInfos) override
    {
        abilityInfos = mockAbilityInfos;
        return mockQueryAbilityInfosV9Ret;
    }

    static ErrCode mockQueryAbilityInfosV9Ret;
    static std::vector<AbilityInfo> mockAbilityInfos;
};

ErrCode MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;
std::vector<AbilityInfo> MockBundleMgrStub::mockAbilityInfos;
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_BUNDLE_MGR_STUB_H
