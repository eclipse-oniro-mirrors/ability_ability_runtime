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

#include "mock_app_mgr_service_inner.h"

#include "ability_manager_errors.h"
#include "app_utils.h"
#include "constants.h"
#include "hilog_tag_wrapper.h"
#include "permission_verification.h"
#include "window_manager.h"

namespace OHOS {
namespace AppExecFwk {
int32_t AppMgrServiceInner::MakeKiaProcess(std::shared_ptr<AAFwk::Want> want, bool &isKia,
    std::string &watermarkBusinessName, bool &isWatermarkEnabled,
    bool &isFileUri, std::string &processName)
{
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation()) {
        return ERR_OK;
    }
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
#ifdef INCLUDE_ZURI
    isFileUri = !want->GetUriString().empty() && want->GetUri().GetScheme() == "file";
#endif
    if (isFileUri && kiaInterceptor_ != nullptr) {
        auto resultCode = kiaInterceptor_->OnIntercept(*want);
        watermarkBusinessName = want->GetStringParam(KEY_WATERMARK_BUSINESS_NAME);
        isWatermarkEnabled = want->GetBoolParam(KEY_IS_WATERMARK_ENABLED, false);
        TAG_LOGI(AAFwkTag::APPMGR, "After calling kiaInterceptor_->OnIntercept,"
            "resultCode=%{public}d,watermarkBusinessName=%{private}s,isWatermarkEnabled=%{private}d",
            resultCode, watermarkBusinessName.c_str(),
            static_cast<int>(isWatermarkEnabled));
        isKia = (resultCode == ERR_OK && !watermarkBusinessName.empty() && isWatermarkEnabled);
        if (isKia) {
            processName += "_KIA";
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::ProcessKia(bool isKia, std::shared_ptr<AppRunningRecord> appRecord,
    const std::string& watermarkBusinessName, bool isWatermarkEnabled)
{
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation() || !isKia) {
        return ERR_OK;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "Openning KIA file, start setting watermark");
    int32_t resultCode = static_cast<int32_t>(WindowManager::GetInstance(appRecord->GetUserId()).SetProcessWatermark(
        appRecord->GetPid(), watermarkBusinessName, isWatermarkEnabled));
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "setting watermark fails with result code:%{public}d", resultCode);
        return resultCode;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "setting watermark succeeds, start setting snapshot skip");
    resultCode = static_cast<int32_t>(WindowManager::GetInstance(appRecord->GetUserId()).SkipSnapshotForAppProcess(
        appRecord->GetPid(), isWatermarkEnabled));
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "setting snapshot skip fails with result code:%{public}d", resultCode);
        return resultCode;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "setting snapshot skip succeeds");
    return ERR_OK;
}

int AppMgrServiceInner::RegisterKiaInterceptor(const sptr<IKiaInterceptor> &interceptor)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation() ||
        !AAFwk::PermissionVerification::GetInstance()->VerifySuperviseKiaServicePermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "no kia permission.");
        return ERR_PERMISSION_DENIED;
    }
    if (interceptor == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "interceptor is nullptr.");
        return ERR_INVALID_VALUE;
    }
    kiaInterceptor_ = interceptor;
    return ERR_OK;
}

int32_t AppMgrServiceInner::CheckIsKiaProcess(pid_t pid, bool &isKia)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation() ||
        !AAFwk::PermissionVerification::GetInstance()->VerifySuperviseKiaServicePermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "no kia permission.");
        return ERR_PERMISSION_DENIED;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return appRunningManager_->CheckIsKiaProcess(pid, isKia);
}
}  // namespace AppExecFwk
}  // namespace OHOS
