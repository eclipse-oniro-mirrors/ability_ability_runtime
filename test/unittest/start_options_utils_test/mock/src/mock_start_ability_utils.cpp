/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "start_ability_utils.h"

#include "mock_my_flag.h"

namespace OHOS {
namespace AAFwk {

bool StartAbilityUtils::GetAppIndex(const Want &want, sptr<IRemoteObject> callerToken, int32_t &appIndex)
{
    appIndex = 0;
    return true;
}

void StartAbilityUtils::SetTargetCloneIndexInSameBundle(const Want &want, sptr<IRemoteObject> callerToken)
{
    ++MyFlag::GetInstance().setTargetCloneIndexCallCount_;
}

void StartAbilityUtils::ResolveTargetAppCloneIndex(const Want &want, sptr<IRemoteObject> callerToken, int32_t userId)
{
    ++MyFlag::GetInstance().resolveTargetAppCloneIndexCallCount_;
    MyFlag::GetInstance().resolveTargetAppCloneIndexUserId_ = userId;
}
}
}
