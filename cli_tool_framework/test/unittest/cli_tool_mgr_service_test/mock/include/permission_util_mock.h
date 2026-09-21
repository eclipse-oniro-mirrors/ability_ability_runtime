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

#ifndef OHOS_ABILITY_RUNTIME_PERMISSION_UTIL_MOCK_H
#define OHOS_ABILITY_RUNTIME_PERMISSION_UTIL_MOCK_H

namespace OHOS {
namespace CliTool {
// Test-only toggle state consumed by the mocked PermissionUtil::VerifyAccessToken
// below. Exposed as static members (instead of extern globals) so the state is
// declared in a header, defined in exactly one TU, and reachable from any test
// via PermissionUtilMock::xxx. Reset() restores the permissive default.
class PermissionUtilMock {
public:
    static bool execCliToolPermitted;
    static bool execPublicCliToolPermitted;

    static void Reset();
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PERMISSION_UTIL_MOCK_H
