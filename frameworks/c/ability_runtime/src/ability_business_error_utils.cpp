/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ability_business_error_utils.h"

#include <unordered_map>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"

std::unordered_map<int32_t, AbilityRuntime_ErrorCode> g_innerToBusinessErrorCommonMap {
    { OHOS::ERR_OK, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR },
    { OHOS::AAFwk::CHECK_PERMISSION_FAILED, ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED },
    { OHOS::ERR_PERMISSION_DENIED, ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED },
    { OHOS::AAFwk::ERR_CAPABILITY_NOT_SUPPORT, ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED },
    { OHOS::AAFwk::RESOLVE_ABILITY_ERR, ABILITY_RUNTIME_ERROR_CODE_NO_SUCH_ABILITY },
    { OHOS::AAFwk::TARGET_BUNDLE_NOT_EXIST, ABILITY_RUNTIME_ERROR_CODE_NO_SUCH_ABILITY },
    { OHOS::AAFwk::ERR_NOT_ALLOW_IMPLICIT_START, ABILITY_RUNTIME_ERROR_CODE_NO_SUCH_ABILITY },
    { OHOS::AAFwk::ERR_WRONG_INTERFACE_CALL, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE },
    { OHOS::AAFwk::TARGET_ABILITY_NOT_SERVICE, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE },
    { OHOS::AAFwk::RESOLVE_CALL_ABILITY_TYPE_ERR, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE },
    { OHOS::AAFwk::ERR_CROWDTEST_EXPIRED, ABILITY_RUNTIME_ERROR_CODE_CROWDTEST_EXPIRED },
    { OHOS::ERR_WOULD_BLOCK, ABILITY_RUNTIME_ERROR_CODE_WUKONG_MODE },
    { OHOS::AAFwk::ERR_APP_CONTROLLED, ABILITY_RUNTIME_ERROR_CODE_CONTROLLED },
    { OHOS::AAFwk::ERR_EDM_APP_CONTROLLED, ABILITY_RUNTIME_ERROR_CODE_EDM_CONTROLLED },
    { OHOS::AAFwk::ERR_START_OTHER_APP_FAILED, ABILITY_RUNTIME_ERROR_CODE_CROSS_APP },
    { OHOS::AAFwk::NOT_TOP_ABILITY, ABILITY_RUNTIME_ERROR_CODE_NOT_TOP_ABILITY },
    { OHOS::AAFwk::ERR_START_OPTIONS_CHECK_FAILED, ABILITY_RUNTIME_ERROR_VISIBILITY_SETTING_DISABLED },
    { OHOS::AAFwk::ERR_UPPER_LIMIT, ABILITY_RUNTIME_ERROR_CODE_UPPER_LIMIT_REACHED },
    { OHOS::AAFwk::ERR_APP_INSTANCE_KEY_NOT_SUPPORT, ABILITY_RUNTIME_ERROR_CODE_APP_INSTANCE_KEY_NOT_SUPPORTED },
    { OHOS::AAFwk::ERR_NOT_SELF_APPLICATION, ABILITY_RUNTIME_ERROR_CODE_CROSS_APP },
    };

std::unordered_map<int32_t, AbilityRuntime_ErrorCode> g_innerToBusinessErrorApi17Map {
{ OHOS::AAFwk::ERR_MULTI_APP_NOT_SUPPORTED, ABILITY_RUNTIME_ERROR_CODE_MULTI_APP_NOT_SUPPORTED },
{ OHOS::AAFwk::ERR_INVALID_APP_INSTANCE_KEY, ABILITY_RUNTIME_ERROR_CODE_INVALID_APP_INSTANCE_KEY },
{ OHOS::AAFwk::ERR_MULTI_INSTANCE_NOT_SUPPORTED, ABILITY_RUNTIME_ERROR_MULTI_INSTANCE_NOT_SUPPORTED },
};

std::unordered_map<int32_t, AbilityRuntime_ErrorCode> g_innerToBusinessErrorApi21Map {
{ OHOS::AAFwk::ERR_ATTACH_ABILITY_THREAD_FAILED, ABILITY_RUNTIME_ERROR_CODE_START_TIMEOUT },
{ OHOS::AAFwk::ERR_CHECK_CHILD_PROCESS_FAILED, ABILITY_RUNTIME_ERROR_CODE_CHILD_PROCESS_NOT_EXIST },
};

AbilityRuntime_ErrorCode ConvertToCommonBusinessErrorCode(int32_t abilityManagerErrorCode)
{
    TAG_LOGI(AAFwkTag::APPKIT, "ability errCode:%{public}d", abilityManagerErrorCode);
    auto it = g_innerToBusinessErrorCommonMap.find(abilityManagerErrorCode);
    if (it != g_innerToBusinessErrorCommonMap.end()) {
        return it->second;
    }

    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode ConvertToAPI17BusinessErrorCode(int32_t abilityManagerErrorCode)
{
    TAG_LOGI(AAFwkTag::APPKIT, "ability errCode:%{public}d", abilityManagerErrorCode);
    auto errCode = ConvertToCommonBusinessErrorCode(abilityManagerErrorCode);
    if (errCode != ABILITY_RUNTIME_ERROR_CODE_INTERNAL) {
        return errCode;
    }

    auto it = g_innerToBusinessErrorApi17Map.find(abilityManagerErrorCode);
    if (it != g_innerToBusinessErrorApi17Map.end()) {
        return it->second;
    }

    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode ConvertToAPI21BusinessErrorCode(int32_t abilityManagerErrorCode)
{
    TAG_LOGI(AAFwkTag::APPKIT, "ability errCode:%{public}d", abilityManagerErrorCode);
    auto errCode = ConvertToAPI17BusinessErrorCode(abilityManagerErrorCode);
    if (errCode != ABILITY_RUNTIME_ERROR_CODE_INTERNAL) {
        return errCode;
    }

    auto it = g_innerToBusinessErrorApi21Map.find(abilityManagerErrorCode);
    if (it != g_innerToBusinessErrorApi21Map.end()) {
        return it->second;
    }

    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}
