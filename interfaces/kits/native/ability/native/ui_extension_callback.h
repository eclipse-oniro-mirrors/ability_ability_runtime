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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_CALLBACK_H

#include <string>

#include "want.h"

namespace OHOS {
namespace Ace {
class UIContent;
}
namespace AbilityRuntime {
class UIExtensionCallback {
public:
    UIExtensionCallback() = default;
    virtual ~UIExtensionCallback() = default;
    virtual void OnError(int32_t number) = 0;
    virtual void OnAbilityByTypeResult(int32_t errorCode) = 0;
    virtual void OnResult(int32_t resultCode, const AAFwk::Want &want) = 0;
    void OnRelease(int32_t code);
    void SetSessionId(int32_t sessionId);
    void SetUIContent(Ace::UIContent *uiContent);
    void CloseModalUIExtension();
    virtual void OnRequestSuccess(const std::string& name);
    virtual void OnRequestFailure(const std::string& name, int32_t failureCode, const std::string& failureMessage);
private:
    int32_t sessionId_ = 0;
    Ace::UIContent *uiContent_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_CALLBACK_H
