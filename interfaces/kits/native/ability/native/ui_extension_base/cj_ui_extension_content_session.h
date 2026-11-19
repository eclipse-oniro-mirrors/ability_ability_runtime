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

#ifndef OHOS_ABILITY_RUNTIME_CJ_UI_EXTENSION_CONTENT_SESSION_H
#define OHOS_ABILITY_RUNTIME_CJ_UI_EXTENSION_CONTENT_SESSION_H

#include "cj_ui_extension_callback.h"
#include "ffi_remote_data.h"
#include "session_info.h"
#include "start_options.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
class CJUIExtensionContentSession : public FFI::FFIData {
    DECL_TYPE(CJUIExtensionContentSession, OHOS::FFI::FFIData)
public:
    CJUIExtensionContentSession(sptr<AAFwk::SessionInfo> sessionInfo,
        sptr<Rosen::Window> uiWindow, std::weak_ptr<AbilityRuntime::Context> context);

    virtual ~CJUIExtensionContentSession() = default;

    static sptr<CJUIExtensionContentSession> Create(sptr<AAFwk::SessionInfo> sessionInfo,
        sptr<Rosen::Window> uiWindow, std::weak_ptr<AbilityRuntime::Context> context);

    int32_t LoadContent(const std::string& path);
    int32_t TerminateSelf();
    int32_t TerminateSelfWithResult(AAFwk::Want* want, int32_t resultCode);
    int32_t SetWindowPrivacyMode(bool isPrivacyMode);
    int32_t StartAbilityByType(const std::string &type, AAFwk::WantParams &wantParam,
        const std::shared_ptr<CjUIExtensionCallback> &uiExtensionCallbacks);

#ifdef SUPPORT_SCREEN
private:
    void InitDisplayId(AAFwk::Want &want);
#endif
private:
    sptr<AAFwk::SessionInfo> sessionInfo_;
    sptr<Rosen::Window> uiWindow_;
    std::weak_ptr<AbilityRuntime::Context> context_;
    bool isFirstTriggerBindModal_ {true};
};

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CJ_UI_EXTENSION_CONTENT_SESSION_H
