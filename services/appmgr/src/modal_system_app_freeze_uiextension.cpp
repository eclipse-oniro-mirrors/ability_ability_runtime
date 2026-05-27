/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef APP_NO_RESPONSE_DIALOG
#include "modal_system_app_freeze_uiextension.h"

#include <chrono>
#include <mutex>

#include "ability_record.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "scene_board_judgement.h"
#include "session_manager_lite.h"
#include "window_visibility_info.h"

using namespace OHOS::AAFwk;
using namespace OHOS::Rosen;

namespace OHOS {
namespace AppExecFwk {
const std::string UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
const std::string UIEXTENSION_SYS_COMMON_UI = "sysDialog/common";
const std::string APP_FREEZE_PID = "pid";
const std::string FREEZE_WINDOW_POSX = "FREEZE_WINDOW_POSX";
const std::string FREEZE_WINDOW_POSY = "FREEZE_WINDOW_POSY";
const std::string FREEZE_WINDOW_WIDTH = "FREEZE_WINDOW_WIDTH";
const std::string FREEZE_WINDOW_HEIGHT = "FREEZE_WINDOW_HEIGHT";
const std::string START_BUNDLE_NAME = "startBundleName";
const std::string APP_FREEZE_TOKEN = "freezeToken";
const std::string APP_FREEZE_EVENT_NAME = "eventName";
const std::string APP_FREEZE_RUNNING_UNIQUE_ID = "appRunningUniqueId";
constexpr int32_t INVALID_USERID = -1;
constexpr int32_t MESSAGE_PARCEL_KEY_SIZE = 3;
constexpr uint32_t COMMAND_START_DIALOG = 1;
constexpr char INVALID_PID[] = "-1";
constexpr uint64_t TIMEOUT_INTERVAL_MS = 8000;

ModalSystemAppFreezeUIExtension &ModalSystemAppFreezeUIExtension::GetInstance()
{
    static ModalSystemAppFreezeUIExtension instance;
    return instance;
}

ModalSystemAppFreezeUIExtension::~ModalSystemAppFreezeUIExtension()
{}

void ModalSystemAppFreezeUIExtension::ProcessAppFreeze(bool focusFlag, const FaultData &faultData, std::string pid,
    std::string bundleName, std::function<void()> callback)
{
    const std::string SCENE_BAOARD_NAME = "com.ohos.sceneboard";
    if ((bundleName == SCENE_BAOARD_NAME || faultData.waitSaveState) && callback) {
        callback();
        return;
    }
    FaultDataType faultType = faultData.faultType;
    std::string name = faultData.errorObject.name;
    if (name == AppFreezeType::THREAD_BLOCK_6S || name == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S) {
        return;
    }
    bool isAppFreezeDialog = name == AppFreezeType::APP_INPUT_BLOCK || name == AppFreezeType::BUSINESS_INPUT_BLOCK;
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "ProcessAppFreeze %{public}s is %{public}s pid:%{public}s eventName:%{public}s", bundleName.c_str(),
        focusFlag ? "focus" : "not focus", pid.c_str(), name.c_str());
    if (focusFlag && isAppFreezeDialog) {
        CreateModalUIExtension(pid, bundleName, faultData);
    } else if (callback && (faultType != FaultDataType::APP_FREEZE || !isAppFreezeDialog)) {
        callback();
    }
}

bool ModalSystemAppFreezeUIExtension::CreateModalUIExtension(std::string& pid, std::string& bundleName,
    const FaultData &faultData)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Create Modal UIExtension Called");
    sptr<IRemoteObject> token;
    AAFwk::Want want;
    std::unique_lock<std::mutex> lockAssertResult(appFreezeResultMutex_);
    if (!CreateSystemDialogWant(pid, bundleName, token, want, faultData)) {
        return false;
    }
    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManagerClient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityManagerClient");
        return false;
    }
    auto result = IN_PROCESS_CALL(abilityManagerClient->StartExtensionAbility(want, token));
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartExtensionAbility failed, result = %{public}d", result);
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "success, result = %{public}d", result);
    return true;
}

bool ModalSystemAppFreezeUIExtension::CreateSystemDialogWant(
    std::string& pid, std::string& bundleName, sptr<IRemoteObject> token, AAFwk::Want &want,
    const FaultData &faultData)
{
    want.SetElementName(APP_NO_RESPONSE_BUNDLENAME, APP_NO_RESPONSE_ABILITY);
    want.SetParam(UIEXTENSION_TYPE_KEY, UIEXTENSION_SYS_COMMON_UI);
    want.SetParam(APP_FREEZE_PID, pid);
    want.SetParam(START_BUNDLE_NAME, bundleName);
    want.SetParam(APP_FREEZE_EVENT_NAME, faultData.errorObject.name);
    want.SetParam(APP_FREEZE_RUNNING_UNIQUE_ID, faultData.appRunningUniqueId);

    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    if (!sceneSessionManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sceneSessionManager is null proxy!");
        return false;
    }
    auto ret = static_cast<int>(sceneSessionManager->GetFocusSessionToken(token));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get focus session token err: %{public}d", ret);
        return false;
    }
    want.SetParam(APP_FREEZE_TOKEN, token);
    std::vector<sptr<Rosen::WindowVisibilityInfo>> infos;
    ret = static_cast<int>(sceneSessionManager->GetVisibilityWindowInfo(infos));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get visibility window info err: %{public}d", ret);
        return false;
    }

    int32_t posX = 0;
    int32_t posY = 0;
    uint32_t width = 10;
    uint32_t height  = 10;
    int32_t focusPid = -1;
    for (const auto &info : infos) {
        if (info != nullptr && info->IsFocused()) {
            posX = info->rect_.posX_;
            posY = info->rect_.posY_;
            width = info->rect_.width_;
            height = info->rect_.height_;
            focusPid = info->pid_;
            break;
        }
    }
    if ((focusPid == -1) || (std::to_string(focusPid) != pid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fucused window pid is %{public}d, not freeze pid!", focusPid);
        return false;
    }
    want.SetParam(FREEZE_WINDOW_POSX, std::to_string(posX));
    want.SetParam(FREEZE_WINDOW_POSY, std::to_string(posY));
    want.SetParam(FREEZE_WINDOW_WIDTH, std::to_string(width));
    want.SetParam(FREEZE_WINDOW_HEIGHT, std::to_string(height));
    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // APP_NO_RESPONSE_DIALOG
