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

#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "modal_system_dialog/modal_system_dialog_ui_extension.h"
#include "parameters.h"
#include "utils/modal_system_dialog_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
constexpr const char* RESOURCE_ID_GENERAL_GET_IT = "general_get_it";
constexpr const char* RESOURCE_ID_DEVELOPER_DIALOG_TITLE = "lab_developer_mode_tips";
constexpr const char* RESOURCE_ID_DEVELOPER_DIALOG_CONTENT = "cont_developer_mode_tips";
}

bool ModalSystemDialogUtil::CheckDebugAppNotInDeveloperMode(const AppExecFwk::ApplicationInfo &applicationInfo)
{
    if (applicationInfo.appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG &&
        !system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not developer mode.");
        return true;
    }
    return false;
}

void ModalSystemDialogUtil::ShowDeveloperModeDialog(
    const std::string &bundleName, const std::string &moduleName, uint32_t labelId, int32_t userId)
{
    auto bms = AAFwk::AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER(bms);
    std::string labelString = IN_PROCESS_CALL(bms->GetStringById(bundleName, moduleName, labelId, userId));

    nlohmann::json infoObject;
    infoObject["appName"] = labelString.c_str();
    infoObject["title"] = RESOURCE_ID_DEVELOPER_DIALOG_TITLE;
    infoObject["content"] = RESOURCE_ID_DEVELOPER_DIALOG_CONTENT;
    infoObject["button"] = RESOURCE_ID_GENERAL_GET_IT;
    infoObject["isFormat"] = true;

    nlohmann::json param;
    param["extraInfo"] = infoObject;

    std::string paramStr = param.dump();
    auto connection = std::make_shared<ModalSystemDialogUIExtension>();
    if (connection && connection->CreateModalUIExtension(paramStr)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create modal ui extension failed");
    }
}

}  // namespace AbilityRuntime
}  // namespace OHOS
