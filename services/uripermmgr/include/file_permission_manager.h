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

#ifndef OHOS_AAFWK_FILE_PERMISSION_MANAGER
#define OHOS_AAFWK_FILE_PERMISSION_MANAGER

#include <deque>
#include <string>
#include <vector>
#include "uri.h"

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
#include "sandbox_manager_kit.h"
#include "policy_info.h"
#else
#include "upms_policy_info.h"
#endif

namespace OHOS {
namespace AAFwk {
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
using namespace AccessControl::SandboxManager;
#endif
typedef enum OperationMode {
    READ_MODE = 1 << 0,
    WRITE_MODE = 1 << 1,
} OperationMode;

class FilePermissionManager {
public:
    static std::vector<bool>
    CheckUriPersistentPermission(std::vector<Uri> &uriVec,
                                 uint32_t callerTokenId, uint32_t flag,
                                 std::vector<PolicyInfo> &pathPolicies, const std::string &bundleName);

    static PolicyInfo GetPathPolicyInfoFromUri(Uri &uri, uint32_t flag, const std::string &bundleName = "");
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFWK_FILE_PERMISSION_MANAGER
