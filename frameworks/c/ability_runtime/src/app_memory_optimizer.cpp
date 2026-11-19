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

#include "app_memory_optimizer.h"

#include "ability_business_error_utils.h"
#include "app_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "singleton.h"

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_NotifyChildProcessState(
    int32_t pid, OH_AbilityRuntime_ChildProcessState state)
{
    if (state != OH_ABILITY_RUNTIME_CHILD_PROCESS_STATE_ACTIVE &&
        state != OH_ABILITY_RUNTIME_CHILD_PROCESS_STATE_INACTIVE) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "invalid state: %{public}d", static_cast<int>(state));
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ConvertToAPI21BusinessErrorCode(
        OHOS::DelayedSingleton<OHOS::AppExecFwk::AppMgrClient>::GetInstance()->NotifyChildProcessState(
            pid, static_cast<int32_t>(state)));
}

#ifdef __cplusplus
}  // extern "C"
#endif