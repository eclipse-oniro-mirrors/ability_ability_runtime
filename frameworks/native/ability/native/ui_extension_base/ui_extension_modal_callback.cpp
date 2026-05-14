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

#include "ui_extension_modal_callback.h"
#include "ui_extension_context.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* EMBEDDABLE_SERVICE_EXIT = "ohos.param.exitEmbeddableUIExtension";
}

void UIExtensionModalCallback::OnRelease()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtensionModalCallback::OnRelease, sessionId: %{public}d", sessionId_);

    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnRelease");
        return;
    }

    context->EraseUIExtension(sessionId_);
    TAG_LOGI(AAFwkTag::UI_EXT, "Erased modal UIExtension on release: %{public}d", sessionId_);

#ifdef SUPPORT_SCREEN
    // Close the modal UIExtension
    if (uiContent_ != nullptr) {
        uiContent_->CloseModalUIExtension(sessionId_);
        TAG_LOGI(AAFwkTag::UI_EXT, "Closed modal UIExtension: %{public}d", sessionId_);
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent_ in OnRelease");
    }
#endif // SUPPORT_SCREEN
}

void UIExtensionModalCallback::OnError()
{
    TAG_LOGE(AAFwkTag::UI_EXT, "UIExtensionModalCallback::OnError, sessionId: %{public}d", sessionId_);

    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnError");
        return;
    }

    // Erase from context's map
    context->EraseUIExtension(sessionId_);

#ifdef SUPPORT_SCREEN
    // Close the modal UIExtension on error
    if (uiContent_ != nullptr) {
        uiContent_->CloseModalUIExtension(sessionId_);
        TAG_LOGI(AAFwkTag::UI_EXT, "Closed modal UIExtension on error: %{public}d", sessionId_);
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent_ in OnError");
    }
#endif // SUPPORT_SCREEN
}

void UIExtensionModalCallback::OnDestroy()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtensionModalCallback::OnDestroy, sessionId: %{public}d", sessionId_);

    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnDestroy");
        return;
    }

    // Only erase from context's map
    // UIExtension is already closed by the system
    context->EraseUIExtension(sessionId_);
    TAG_LOGI(AAFwkTag::UI_EXT, "Erased modal UIExtension on destroy: %{public}d", sessionId_);
}

void UIExtensionModalCallback::OnReceive(const AAFwk::WantParams& data)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtensionModalCallback::OnReceive, sessionId: %{public}d", sessionId_);

    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnReceive");
        return;
    }

    if (data.HasParam(EMBEDDABLE_SERVICE_EXIT)) {
        bool shouldExit = data.GetIntParam(EMBEDDABLE_SERVICE_EXIT, 0);
        if (shouldExit == 1) {
            TAG_LOGI(AAFwkTag::UI_EXT, "Modal dialog notified embeddable exit, sessionId: %{public}d", sessionId_);
            context->TerminateSelfWithAnimation(nullptr);
            return;
        }
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
