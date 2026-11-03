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

#ifndef OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_CALLBACK_H

#include <ani.h>

#include "ui_extension_callback.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsUIExtensionCallback : public UIExtensionCallback {
public:
    explicit EtsUIExtensionCallback(ani_vm *vm) : vm_(vm) {}
    ~EtsUIExtensionCallback() override;
    void OnError(int32_t number) override;
    void OnAbilityByTypeResult(int32_t errorCode) override;
    void OnResult(int32_t resultCode, const AAFwk::Want &want) override;
    void SetEtsCallbackObject(ani_object aniObject);
    void SetCompletionHandler(ani_env *env, ani_object completionHandler);
    void SetAsyncCallback(ani_env *env, ani_object asyncCallback);
    void OnRequestSuccess(const std::string& name) override;
    void OnRequestFailure(const std::string& name, int32_t failureCode, const std::string& failureMessage) override;

private:
    void InvokeAsyncCallback(int32_t errCode);
    void CallObjectMethod(const char *name, const char *signature, ...);
    ani_env* GetAniEnv();

    ani_vm *vm_ = nullptr;
    ani_ref callback_ = nullptr;
    ani_ref completionHandler_ = nullptr;
    ani_ref asyncCallback_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_CALLBACK_H
