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

#ifndef OHOS_ABILITY_RUNTIME_ETS_AUTO_SAVE_REQUEST_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_ETS_AUTO_SAVE_REQUEST_CALLBACK_H

#include "ets_native_reference.h"
#include "save_request_callback_interface.h"

namespace OHOS {
namespace AutoFillManagerEts {
using AutoFillManagerFunc = std::function<void(int32_t)>;
class EtsAutoSaveRequestCallback : public AbilityRuntime::ISaveRequestCallback,
    public std::enable_shared_from_this<EtsAutoSaveRequestCallback> {
public:
    EtsAutoSaveRequestCallback(ani_vm *vm, int32_t instanceId, AutoFillManagerFunc autoFillManagerFunc);
    virtual ~EtsAutoSaveRequestCallback();

    void Register(ani_object object);
    void OnSaveRequestSuccess() override;
    void OnSaveRequestFailed() override;

private:
    void ETSCallFunction(const std::string &methodName);
    bool IsEtsCallbackEquals(std::shared_ptr<AppExecFwk::ETSNativeReference> callback, ani_object object);
    ani_env *GetAniEnv();

    ani_vm *vm_ = nullptr;
    std::shared_ptr<AppExecFwk::ETSNativeReference> callback_;
    int32_t instanceId_ = -1;
    AutoFillManagerFunc autoFillManagerFunc_ = nullptr;
};
} // namespace AutoFillManagerEts
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_AUTO_SAVE_REQUEST_CALLBACK_H