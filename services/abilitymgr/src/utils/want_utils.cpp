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

#include "utils/want_utils.h"

#include "ability_util.h"
#include "in_process_call_wrapper.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t CONVERT_CALLBACK_TIMEOUT_SECONDS = 2; // 2s

int32_t WantUtils::GetCallerBundleName(std::string &callerBundleName)
{
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, ERR_INVALID_VALUE);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    return IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerUid, callerBundleName));
}

int32_t  WantUtils::ConvertToExplicitWant(Want& want)
{
    int32_t retCode = ERR_OK;
#ifdef APP_DOMAIN_VERIFY_ENABLED
    bool isUsed = false;
    ffrt::condition_variable callbackDoneCv;
    ffrt::mutex callbackDoneMutex;
    ConvertCallbackTask task = [&retCode, &isUsed, &callbackDoneCv, &callbackDoneMutex,
        &convertedWant = want](int resultCode, AAFwk::Want& want) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "in convert callback task, resultCode=%{public}d,want=%{private}s",
            resultCode, want.ToString().c_str());
        retCode = resultCode;
        convertedWant = want;
        {
            std::lock_guard<ffrt::mutex> lock(callbackDoneMutex);
            isUsed = true;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "start notify");
        callbackDoneCv.notify_all();
        TAG_LOGI(AAFwkTag::ABILITYMGR, "convert callback task finished");
    };
    sptr<ConvertCallbackImpl> callbackTask = new ConvertCallbackImpl(std::move(task));
    sptr<OHOS::AppDomainVerify::IConvertCallback> callback = callbackTask;
    AppDomainVerify::AppDomainVerifyMgrClient::GetInstance()->ConvertToExplicitWant(want, callback);
    auto condition = [&isUsed] { return isUsed; };
    std::unique_lock<ffrt::mutex> lock(callbackDoneMutex);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "wait for condition");
    if (!callbackDoneCv.wait_for(lock, std::chrono::seconds(CONVERT_CALLBACK_TIMEOUT_SECONDS), condition)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert callback timeout");
        callbackTask->Cancel();
        retCode = ERR_TIMED_OUT;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "finish wait condition");
#endif
    return retCode;
}
}  // namespace AAFwk
}  // namespace OHOS
