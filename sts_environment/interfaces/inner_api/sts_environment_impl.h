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

#ifndef OHOS_ABILITY_STS_ENVIRONMENT_STS_ENVIRONMENT_IMPL_H
#define OHOS_ABILITY_STS_ENVIRONMENT_STS_ENVIRONMENT_IMPL_H

#include <string>
#include "event_handler.h"
// #include "native_engine/native_engine.h"

// #include "native_engine/native_engine.h"

// #include "data_protect.h"

namespace OHOS {
namespace StsEnv {
class StsEnvironmentImpl {
public:
    StsEnvironmentImpl() {}
    virtual ~StsEnvironmentImpl() {}
    virtual void PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime) = 0;
    virtual void PostSyncTask(const std::function<void()>& task, const std::string& name) = 0;
    virtual void RemoveTask(const std::string& name) = 0;
    virtual bool InitLoop(bool isStage) = 0;
    virtual void DeInitLoop() = 0;
    virtual bool ReInitUVLoop() = 0;
};
} // namespace StsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_STS_ENVIRONMENT_STS_ENVIRONMENT_IMPL_H