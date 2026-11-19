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

#include "cj_photo_editor_extension_impl.h"
#include "hilog_tag_wrapper.h"
#include "cj_ui_extension_content_session.h"

namespace OHOS {
namespace AbilityRuntime {
CJPhotoEditorExtensionImpl::CJPhotoEditorExtensionImpl(const std::unique_ptr<Runtime> &runtime)
    : CJUIExtensionBase(runtime)
{}

void CJPhotoEditorExtensionImpl::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    CJUIExtensionBase::OnForeground(want, sessionInfo);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null sessionInfo");
        return;
    }
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiExtensionComponentIdSet_.find(componentId) == uiExtensionComponentIdSet_.end()) {
        OnStartContentEditing(want, sessionInfo);
        uiExtensionComponentIdSet_.emplace(componentId);
    }
}

void CJPhotoEditorExtensionImpl::OnStartContentEditing(const AAFwk::Want &want,
                                                       const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CJPhotoEditorExtensionImpl want: (%{public}s), begin", want.ToUri().c_str());

    std::string imageUri = want.GetStringParam("ability.params.stream");
    if (imageUri.empty()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "empty imageUri");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "CJPhotoEditorExtensionImpl imageUri: (%{public}s), begin", imageUri.c_str());
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    auto newWant = std::make_shared<AAFwk::Want>(want);
    context_->SetWant(newWant);
    if (cjContext_ != nullptr) {
        cjContext_->SetWant(newWant);
    }

    auto it = contentSessions_.find(sessionInfo->uiExtensionComponentId);
    if (it == contentSessions_.end() || it->second == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "contentSessions_ not found or null");
        return;
    }

    cjObj.OnStartContentEditing(imageUri, want, contentSessions_[sessionInfo->uiExtensionComponentId]->GetID());
}
} // namespace AbilityRuntime
} // namespace OHOS
