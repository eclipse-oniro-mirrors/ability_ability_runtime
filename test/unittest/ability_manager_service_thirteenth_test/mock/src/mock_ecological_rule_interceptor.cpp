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

#include "mock_ecological_rule_interceptor.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {

ErrCode EcologicalRuleInterceptor::DoProcess(const AbilityInterceptorParam &param)
{
    return ERR_OK;
}

bool EcologicalRuleInterceptor::DoProcess(Want &want, int32_t userId)
{
    return true;
}

ErrCode EcologicalRuleInterceptor::QueryAtomicServiceStartupRule(Want &want, sptr<IRemoteObject> callerToken,
    int32_t userId, AtomicServiceStartupRule &rule, sptr<Want> &replaceWant)
{
    return MyStatus::GetInstance().eriQueryAtomicServiceStartupRule_;
}

void EcologicalRuleInterceptor::GetEcologicalTargetInfo(const Want &want,
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo, ErmsCallerInfo &callerInfo)
{
}

void EcologicalRuleInterceptor::GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId,
    const sptr<IRemoteObject> &callerToken)
{
}

void EcologicalRuleInterceptor::InitErmsCallerInfo(const Want &want,
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    ErmsCallerInfo &callerInfo, int32_t userId, const sptr<IRemoteObject> &callerToken)
{
}

int32_t EcologicalRuleInterceptor::GetAppTypeByBundleType(int32_t bundleType)
{
    return 0;
}
} // namespace AAFwk
} // namespace OHOS
