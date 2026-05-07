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

#ifndef MOCK_ABILITY_MANAGER_ERRORS_H
#define MOCK_ABILITY_MANAGER_ERRORS_H

#include <cstdint>

using ErrCode = int32_t;

constexpr int32_t SUBSYS_AAFWK = 4;
constexpr int32_t ABILITY_MODULE_TYPE_SERVICE = 0;

inline constexpr ErrCode ErrCodeOffset(int32_t subsysId, int32_t moduleId)
{
    return ((subsysId << 20) | (moduleId << 16));
}

constexpr ErrCode AAFWK_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_AAFWK, ABILITY_MODULE_TYPE_SERVICE);

enum {
    RESOLVE_ABILITY_ERR = AAFWK_SERVICE_ERR_OFFSET,
    INNER_ERR = AAFWK_SERVICE_ERR_OFFSET + 100,
    ERR_CAPABILITY_NOT_SUPPORT = AAFWK_SERVICE_ERR_OFFSET + 125,
    ERR_PERMISSION_DENIED = AAFWK_SERVICE_ERR_OFFSET + 6,
    ERR_OK = 0,
};

enum {
    NOT_TOP_ABILITY = 0x500001,
};

constexpr ErrCode ERR_MODULAR_OBJECT_DISABLED = 2099412;
constexpr ErrCode ERR_NO_RUNNING_ABILITIES_WITH_UI = 2099413;
constexpr ErrCode ERR_INVALID_DISTRIBUTION_TYPE = 2099414;
constexpr ErrCode ERR_FREQ_START_ABILITY = 2098012;
constexpr ErrCode ABILITY_VISIBLE_FALSE_DENY_REQUEST = 2097179;

#endif // MOCK_ABILITY_MANAGER_ERRORS_H
