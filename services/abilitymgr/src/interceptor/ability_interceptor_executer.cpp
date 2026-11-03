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

#include "interceptor/ability_interceptor_executer.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AAFwk {
void AbilityInterceptorExecuter::AddInterceptor(std::string interceptorName,
    const std::shared_ptr<IAbilityInterceptor> &interceptor)
{
    std::lock_guard lock(interceptorListLock_);
    if (interceptor == nullptr) {
        return;
    }
    for (auto it = interceptorList_.begin(); it != interceptorList_.end(); ++it) {
        if (it->first == interceptorName) {
            interceptorList_.erase(it);
            break;
        }
    }
    interceptorList_.emplace_back(interceptorName, interceptor);
}

void AbilityInterceptorExecuter::RemoveInterceptor(std::string interceptorName)
{
    std::lock_guard lock(interceptorListLock_);
    for (auto it = interceptorList_.begin(); it != interceptorList_.end(); ++it) {
        if (it->first == interceptorName) {
            interceptorList_.erase(it);
            break;
        }
    }
}

bool AbilityInterceptorExecuter::HasInterceptor(std::string interceptorName)
{
    std::lock_guard lock(interceptorListLock_);
    for (const auto &item : interceptorList_) {
        if (item.first == interceptorName) {
            return true;
        }
    }
    return false;
}

ErrCode AbilityInterceptorExecuter::DoProcess(AbilityInterceptorParam &param)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int32_t result = ERR_OK;
    auto interceptorList = GetInterceptorListCopy();
    auto item = interceptorList.begin();
    while (item != interceptorList.end()) {
        if ((*item).second == nullptr) {
            item++;
            continue;
        }
        result = (*item).second->DoProcess(param);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "DoProcess err: %{public}s_%{public}d", (*item).first.c_str(), result);
            break;
        } else {
            item++;
        }
    }
    return result;
}

InterceptorList AbilityInterceptorExecuter::GetInterceptorListCopy()
{
    std::lock_guard lock(interceptorListLock_);
    return interceptorList_;
}
} // namespace AAFwk
} // namespace OHOS