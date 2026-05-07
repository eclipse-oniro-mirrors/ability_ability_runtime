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

#include "c_modular_object_utils.h"

#include "ability_business_error_utils.h"
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "native_extension/context_impl.h"
#include "securec.h"
#include "want_manager.h"
#include "want_utils.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AbilityRuntime {
AbilityRuntime_ErrorCode CModularObjectUtils::ConvertConnectBusinessErrorCode(int32_t errCode)
{
    switch (errCode) {
        case ABILITY_VISIBLE_FALSE_DENY_REQUEST:
            return ABILITY_RUNTIME_ERROR_CODE_VISIBILITY_VERIFICATION_FAILED;
        case ERR_STATIC_CFG_PERMISSION:
            return ABILITY_RUNTIME_ERROR_CODE_STATIC_CFG_PERMISSION;
        case ERR_CROSS_USER:
            return ABILITY_RUNTIME_ERROR_CODE_CROSS_USER_OPERATION;
        case ERR_FREQ_START_ABILITY:
            return ABILITY_RUNTIME_ERROR_CODE_UPPER_RATE_LIMIT;
        case ERR_MODULAR_OBJECT_DISABLED:
            return ABILITY_RUNTIME_ERROR_CODE_NO_SUCH_ABILITY;
        case ERR_NO_RUNNING_ABILITIES_WITH_UI:
            return ABILITY_RUNTIME_ERROR_CODE_NO_RUNNING_ABILITIES_WITH_UI;
        case ERR_INVALID_DISTRIBUTION_TYPE:
            return ABILITY_RUNTIME_ERROR_CODE_INVALID_DISTRIBUTION_TYPE;
        default:
            return ConvertToCommonBusinessErrorCode(errCode);
    }
}

bool CModularObjectUtils::BuildElement(const AppExecFwk::ElementName &elementName, AbilityBase_Element &element)
{
    element.bundleName = nullptr;
    element.moduleName = nullptr;
    element.abilityName = nullptr;
    if (!CopyToCString(elementName.GetBundleName(), element.bundleName)) {
        return false;
    }
    if (!CopyToCString(elementName.GetModuleName(), element.moduleName)) {
        delete[] element.bundleName;
        element.bundleName = nullptr;
        return false;
    }
    if (!CopyToCString(elementName.GetAbilityName(), element.abilityName)) {
        delete[] element.bundleName;
        delete[] element.moduleName;
        element.bundleName = nullptr;
        element.moduleName = nullptr;
        return false;
    }
    return true;
}

void CModularObjectUtils::DestroyElement(AbilityBase_Element &element)
{
    delete[] element.bundleName;
    delete[] element.moduleName;
    delete[] element.abilityName;
    element.bundleName = nullptr;
    element.moduleName = nullptr;
    element.abilityName = nullptr;
}

bool CModularObjectUtils::CopyToCString(const std::string &src, char *&dst)
{
    dst = new (std::nothrow) char[src.size() + 1];
    if (dst == nullptr) {
        return false;
    }
    if (strcpy_s(dst, src.size() + 1, src.c_str()) != EOK) {
        delete[] dst;
        dst = nullptr;
        return false;
    }
    return true;
}

AbilityRuntime_ErrorCode CModularObjectUtils::TransformWant(AbilityBase_Want *want, AAFwk::Want &abilityWant)
{
    auto ret = CheckWant(want);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid want");
        return ret;
    }
    auto errCode = CWantManager::TransformToWant(*want, false, abilityWant);
    if (errCode != ABILITY_BASE_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "transform want failed: %{public}d", errCode);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode CModularObjectUtils::CheckContextAndToken(AbilityRuntime_ContextHandle context,
    sptr<IRemoteObject> &token)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto contextPtr = context->context.lock();
    if (contextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context not exist");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    token = contextPtr->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null token");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void CModularObjectUtils::NotifyFailed(std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> state,
    int32_t businessErrorCode)
{
    if (state == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "state null");
        return;
    }
    OH_AbilityRuntime_ConnectOptions_OnFailedCallback callback = nullptr;
    OH_AbilityRuntime_ConnectOptions *owner = nullptr;
    {
        std::lock_guard<std::mutex> guard(state->mutex);
        if (!state->alive) {
            return;
        }
        callback = state->onFailedCallback;
        owner = state->owner;
    }
    if (callback != nullptr) {
        callback(owner, static_cast<AbilityRuntime_ErrorCode>(businessErrorCode));
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
