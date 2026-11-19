/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_APPLICATION_CONTEXT_FFI_H
#define OHOS_ABILITY_RUNTIME_CJ_APPLICATION_CONTEXT_FFI_H

#include <cstdint>
#include <shared_mutex>

#include "application_context.h"
#include "cj_ability_lifecycle_callback.h"
#include "cj_application_context.h"
#include "cj_application_state_change_callback.h"
#include "cj_common_ffi.h"
#include "cj_environment_callback.h"
#include "cj_macro.h"
#include "cj_utils_ffi.h"
#include "cj_want_ffi.h"
#include "ffi_remote_data.h"
#include "running_process_info.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace ApplicationContextCJ {

struct CApplicationInfo {
    const char *name;
    const char *bundleName;
};

extern "C" {
CJ_EXPORT int64_t FFIGetArea(int64_t id);
CJ_EXPORT CApplicationInfo *FFICJApplicationInfo(int64_t id);
CJ_EXPORT int32_t FfiCJApplicationContextOnOnEnvironment(int64_t id, void (*cfgCallback)(CConfiguration),
                                                         void (*memCallback)(int32_t), int32_t *errCode);
CJ_EXPORT int32_t FfiCJApplicationContextOnOnEnvironmentV2(int64_t id, void (*cfgCallback)(CConfigurationV2),
                                                           void (*memCallback)(int32_t), int32_t *errCode);
CJ_EXPORT int32_t FfiCJApplicationContextOnOnAbilityLifecycle(int64_t id, CArrI64 cFuncIds, int32_t *errCode);
CJ_EXPORT int32_t FfiCJApplicationContextOnOnApplicationStateChange(int64_t id, void (*foregroundCallback)(void),
                                                                    void (*backgroundCallback)(void), int32_t *errCode);
CJ_EXPORT void FfiCJApplicationContextOnOff(int64_t id, const char *type, int32_t callbackId, int32_t *errCode);
CJ_EXPORT void FfiCJApplicationContextSetFont(int64_t id, const char *font, int32_t *errCode);
CJ_EXPORT void FfiCJApplicationContextSetLanguage(int64_t id, const char *language, int32_t *errCode);
CJ_EXPORT void FfiCJApplicationContextSetColorMode(int64_t id, int32_t colorMode, int32_t *errCode);
CJ_EXPORT CArrProcessInformation FfiCJApplicationContextGetRunningProcessInformation(int64_t id, int32_t *errCode);
CJ_EXPORT void FfiCJApplicationContextKillAllProcesses(int64_t id, bool clearPageStack, int32_t *errCode);
CJ_EXPORT int32_t FfiCJApplicationContextGetCurrentAppCloneIndex(int64_t id, int32_t *errCode);
CJ_EXPORT void FfiCJApplicationContextRestartApp(int64_t id, WantHandle want, int32_t *errCode);
CJ_EXPORT void FfiCJApplicationContextClearUpApplicationData(int64_t id, int32_t *errCode);
CJ_EXPORT void FfiCJApplicationContextSetSupportedProcessCache(int64_t id, bool isSupported, int32_t *errCode);
CJ_EXPORT int32_t FfiCJApplicationContextSetFontSizeScale(int64_t id, double fontSizeScale);
};
} // namespace ApplicationContextCJ
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CJ_APPLICATION_CONTEXT_H