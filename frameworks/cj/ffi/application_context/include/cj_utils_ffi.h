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

#ifndef OHOS_ABILITY_RUNTIME_CJ_UTILS_FFI_H
#define OHOS_ABILITY_RUNTIME_CJ_UTILS_FFI_H

#include <string>

#include "configuration.h"

namespace OHOS {
namespace AbilityRuntime {

#define CJ_CONFIGURATION_COMMON_FIELDS \
    char* language;                    \
    int32_t colorMode;                 \
    int32_t direction;                 \
    int32_t screenDensity;             \
    int32_t displayId;                 \
    bool hasPointerDevice;             \
    double fontSizeScale;              \
    double fontWeightScale;            \
    char* mcc;                         \
    char* mnc

struct CConfiguration {
    CJ_CONFIGURATION_COMMON_FIELDS;
};

struct CConfigurationV2 {
    CJ_CONFIGURATION_COMMON_FIELDS;
    char* fontId;
    char* locale;
};

#undef CJ_CONFIGURATION_COMMON_FIELDS

struct CArrString {
    char** head;
    int64_t size;
};

struct CProcessInformation {
    int32_t pid;
    int32_t uid;
    char* processName;
    CArrString bundleNames;
    int32_t state;
    int32_t bundleType;
    int32_t appCloneIndex;
};

struct CArrProcessInformation {
    CProcessInformation* head;
    int64_t size;
};

CConfiguration CreateCConfiguration(const OHOS::AppExecFwk::Configuration &configuration);
void FreeCConfiguration(CConfiguration *configuration);
CConfigurationV2 CreateCConfigurationV2(const OHOS::AppExecFwk::Configuration &configuration);
void FreeCConfigurationV2(CConfigurationV2 *configuration);
}
}

// The return variable needs free in CJ.
char* CreateCStringFromString(const std::string& source);
char** VectorToCArrString(const std::vector<std::string>& vec);

#endif // OHOS_ABILITY_RUNTIME_CJ_UTILS_FFI_H
