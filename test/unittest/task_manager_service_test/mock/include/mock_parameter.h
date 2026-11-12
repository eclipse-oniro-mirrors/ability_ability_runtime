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

#ifndef MOCK_PARAMETER_H
#define MOCK_PARAMETER_H

namespace OHOS {
namespace system {
// Mirrors DEVELOPER_MODE_STATE in app_mgr_service.cpp.
constexpr const char *MOCK_DEVELOPER_MODE_KEY = "const.security.developermode.state";

// Test seam for the developer-mode gate of the --task register/unregister hooks.
void SetMockDeveloperMode(bool enabled);
} // namespace system
} // namespace OHOS
#endif // MOCK_PARAMETER_H
