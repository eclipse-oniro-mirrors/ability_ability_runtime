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

#ifndef POLICY_INFO_H
#define POLICY_INFO_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace AccessControl {
namespace SandboxManager {
typedef enum PolicyType {
    UNKNOWN = 0,
    SELF_PATH = 1,
    AUTHORIZATION_PATH = 2,
    OTHERS_PATH = 3,
} PolicyType;

struct PolicyInfo final {
public:
    std::string path;
    uint64_t mode;
    PolicyType type = PolicyType::UNKNOWN;
};

struct SetInfo final {
public:
    std::string bundleName;
    uint64_t timestamp;
    int32_t userId = -1;
    SetInfo() : bundleName(""), timestamp(0), userId(-1) {}
};

const uint32_t IS_POLICY_ALLOWED_TO_BE_PRESISTED = 1 << 0;
} // namespace SandboxManager
} // namespace AccessControl
} // namespace OHOS
#endif // POLICY_INFO_H