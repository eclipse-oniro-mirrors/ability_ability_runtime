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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_EXECUTER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_EXECUTER_H
#include <gmock/gmock.h>
#include <unordered_map>
#include "ability_interceptor_interface.h"
#include "cpp/mutex.h"

namespace OHOS {
namespace AAFwk {
using InterceptorMap = std::unordered_map<std::string, std::shared_ptr<IAbilityInterceptor>>;
/**
 * @class AbilityInterceptorExecuter
 * AbilityInterceptorExecuter excute the interceptors.
 */
class MockAbilityInterceptorExecuter {
public:
    /**
     * Add Interceptor to Executer.
     *
     * @param interceptorName, interceptor name.
     * @param interceptor, interceptor handle the interception processing.
     */
    void AddInterceptor(std::string interceptorName, const std::shared_ptr<IAbilityInterceptor> &interceptor);

    /**
     * @param interceptorName, interceptor name.
     */
    void RemoveInterceptor(std::string interceptorName);

    /**
     * Excute the DoProcess of the interceptors.
     */
    ErrCode DoProcess(const AbilityInterceptorParam &param);

    void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler);
private:
    InterceptorMap GetInterceptorMapCopy();
private:
    std::mutex interceptorMapLock_;
    std::unordered_map<std::string, std::shared_ptr<IAbilityInterceptor>> interceptorMap_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_EXECUTER_H