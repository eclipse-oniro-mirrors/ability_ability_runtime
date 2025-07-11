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

#ifndef OHOS_ABILITY_RUNTIME_CONNECTION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CONNECTION_MANAGER_H

#include "ability_connection.h"
#include "element_name.h"
#include "want.h"
#include "oh_mock_utils.h"

namespace OHOS {
namespace AbilityRuntime {
class ConnectionManager {
public:
    static ConnectionManager& GetInstance()
    {
        static ConnectionManager instance;
        return instance;
    }

    void ReportConnectionLeakEvent(const int pid, const int tid) {}

    OH_MOCK_METHOD(bool, ConnectionManager, DisconnectNonexistentService, const AppExecFwk::ElementName&,
        const sptr<AbilityConnection>);

    OH_MOCK_METHOD(bool, ConnectionManager, RemoveConnection, const sptr<AbilityConnection>);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CONNECTION_MANAGER_H
