/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_window.h"
#include "ability.h"
#include "ability_handler.h"
#include "hilog_tag_wrapper.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace AppExecFwk {
AbilityWindow::AbilityWindow()
{}

AbilityWindow::~AbilityWindow()
{}

/**
 * @brief Init the AbilityWindow object.
 *
 * @param handler The EventHandler of the Ability the AbilityWindow belong.
 */
void AbilityWindow::Init(const std::shared_ptr<AbilityHandler> &handler, const std::shared_ptr<Ability> &ability)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    handler_ = handler;
    ability_ = std::weak_ptr<IAbilityEvent>(ability);
    windowScene_ = std::make_shared<Rosen::WindowScene>();
}

bool AbilityWindow::InitWindow(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
    sptr<Rosen::IWindowLifeCycle> &listener, int32_t displayId, const sptr<Rosen::WindowOption> &option, bool isPrivacy)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (windowScene_ == nullptr) {
        windowScene_ = std::make_shared<Rosen::WindowScene>();
    }
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    auto sessionToken = GetSessionToken();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionToken != nullptr) {
        ret = windowScene_->Init(displayId, abilityContext, listener, option, sessionToken);
    } else {
        ret = windowScene_->Init(displayId, abilityContext, listener, option);
    }
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "init window scene failed");
        return false;
    }

    auto window = windowScene_->GetMainWindow();
    if (!window) {
        TAG_LOGE(AAFwkTag::ABILITY, "null window");
        return false;
    }

    if (isPrivacy) {
        window->SetSystemPrivacyMode(true);
    }

    isWindowAttached = true;
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    return true;
}

/**
 * @brief Called when this ability is background.
 *
 */
void AbilityWindow::OnPostAbilityBackground(uint32_t sceneFlag)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (!isWindowAttached) {
        TAG_LOGE(AAFwkTag::ABILITY, "window not attached");
        return;
    }

    if (windowScene_) {
        TAG_LOGD(AAFwkTag::ABILITY, "sceneFlag:%{public}d", sceneFlag);
        windowScene_->GoBackground(sceneFlag);
        TAG_LOGD(AAFwkTag::ABILITY, "go back end");
    }

    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

/**
 * @brief Called when this ability is foreground.
 *
 */
void AbilityWindow::OnPostAbilityForeground(uint32_t sceneFlag)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (!isWindowAttached) {
        TAG_LOGE(AAFwkTag::ABILITY, "window not attached");
        return;
    }

    if (windowScene_) {
        TAG_LOGD(AAFwkTag::ABILITY, "sceneFlag:%{public}d", sceneFlag);
        windowScene_->GoForeground(sceneFlag);
        TAG_LOGD(AAFwkTag::ABILITY, "go fore end");
    }

    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

/**
 * @brief Called when this ability is stopped.
 *
 */
void AbilityWindow::OnPostAbilityStop()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (!isWindowAttached) {
        TAG_LOGE(AAFwkTag::ABILITY, "window not attached");
        return;
    }

    if (windowScene_) {
        windowScene_->GoDestroy();
    }
    isWindowAttached = false;
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

/**
 * @brief Get the window belong to the ability.
 *
 * @return Returns a Window object pointer.
 */
const sptr<Rosen::Window> AbilityWindow::GetWindow()
{
    if (!isWindowAttached) {
        TAG_LOGE(AAFwkTag::ABILITY, "window not attached");
        return nullptr;
    }
    return windowScene_ ? windowScene_->GetMainWindow() : nullptr;
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityWindow::SetMissionLabel(const std::string &label)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    auto window = GetWindow();
    if (!window) {
        TAG_LOGE(AAFwkTag::ABILITY, "get window failed");
        return -1;
    }

    auto ret = window->SetAPPWindowLabel(label);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "errCode:%{public}d", ret);
        return -1;
    }

    return ERR_OK;
}

ErrCode AbilityWindow::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    auto window = GetWindow();
    if (!window) {
        TAG_LOGE(AAFwkTag::ABILITY, "get window failed");
        return -1;
    }

    auto ret = window->SetAPPWindowIcon(icon);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "errCode:%{public}d", ret);
        return -1;
    }

    return ERR_OK;
}
#endif

void AbilityWindow::SetSessionToken(sptr<IRemoteObject> sessionToken)
{
    std::lock_guard lock(sessionTokenMutex_);
    sessionToken_ = sessionToken;
}

sptr<IRemoteObject> AbilityWindow::GetSessionToken()
{
    std::lock_guard lock(sessionTokenMutex_);
    return sessionToken_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
