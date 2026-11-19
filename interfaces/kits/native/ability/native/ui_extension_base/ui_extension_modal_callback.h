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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_MODAL_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_MODAL_CALLBACK_H

#include "hilog_tag_wrapper.h"
#include <memory>

#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {

class UIExtensionContext;

/**
 * @brief Callback class for modal UIExtension lifecycle management in UIExtension context.
 * Similar to DialogUIExtensionCallback but does not depend on IAbilityCallback.
 * Uses weak_ptr to avoid dangling pointer issues (P1 fix).
 */
class UIExtensionModalCallback {
public:
    UIExtensionModalCallback() = default;
    ~UIExtensionModalCallback() = default;

    /**
     * @brief Set the session ID for this modal UIExtension.
     * @param sessionId The session ID.
     */
    void SetSessionId(int32_t sessionId) { sessionId_ = sessionId; }

#ifdef SUPPORT_SCREEN
    /**
     * @brief Set the UIContent for this modal UIExtension.
     * @param uiContent Pointer to the UIContent.
     */
    void SetUIContent(Ace::UIContent* uiContent) { uiContent_ = uiContent; }
#endif // SUPPORT_SCREEN

    /**
     * @brief Set the UIExtensionContext for this modal UIExtension.
     * @param context Weak pointer to the UIExtensionContext (P1 fix: use weak_ptr).
     */
    void SetUIExtensionContext(const std::weak_ptr<UIExtensionContext>& context) { contextWeak_ = context; }

    /**
     * @brief Called when the modal UIExtension is released.
     * Will close the modal UIExtension and erase it from context's map.
     */
    void OnRelease();

    /**
     * @brief Called when an error occurs in the modal UIExtension.
     * Will close the modal UIExtension and erase it from context's map.
     */
    void OnError();

    /**
     * @brief Called when the modal UIExtension is destroyed.
     * Will only erase it from context's map (UIExtension already closed by system).
     */
    void OnDestroy();

    /**
     * @brief Called when the modal UIExtension receives data.
     * @param data The received WantParams data.
     */
    void OnReceive(const AAFwk::WantParams& data);

private:
    int32_t sessionId_ = 0;
#ifdef SUPPORT_SCREEN
    Ace::UIContent* uiContent_ = nullptr;
#endif // SUPPORT_SCREEN
    std::weak_ptr<UIExtensionContext> contextWeak_;  // P1 fix: use weak_ptr instead of raw pointer
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_MODAL_CALLBACK_H
