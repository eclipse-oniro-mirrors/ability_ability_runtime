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

#ifndef OHOS_ABILITY_RUNTIME_BLOCK_ALL_APP_START_INTERCEPTOR
#define OHOS_ABILITY_RUNTIME_BLOCK_ALL_APP_START_INTERCEPTOR

#include <functional>
#include <memory>
#include "ability_interceptor_interface.h"

namespace OHOS {
namespace AAFwk {
struct AbilityRequest;
class BlockAllAppStartInterceptor : public IAbilityInterceptor {
public:
    BlockAllAppStartInterceptor() = default;
    ~BlockAllAppStartInterceptor() = default;
    ErrCode DoProcess(const AbilityInterceptorParam &param) override;

    void SetShouldBlockFunc(const std::function<bool()>& func);

    using IsAbilityStartedFunc = std::function<bool(AbilityRequest&, int32_t)>;
    void SetIsAbilityStartedFunc(const IsAbilityStartedFunc& func);

    ErrCode Execute();
    ErrCode Execute(AbilityRequest& abilityRequest, int32_t validUserId);
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        return;
    };

private:
    std::function<bool()> shouldBlockFunc_;
    IsAbilityStartedFunc isAbilityStartedFunc_;
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_BLOCK_ALL_APP_START_INTERCEPTOR