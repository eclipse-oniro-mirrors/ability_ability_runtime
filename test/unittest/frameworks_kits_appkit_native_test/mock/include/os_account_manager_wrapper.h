/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     htp://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_OS_ACCOUNT_MANAGER_WRAPPER_H
#define MOCK_OS_ACCOUNT_MANAGER_WRAPPER_H

#include <memory>

#include "errors.h"

namespace OHOS {
namespace AppExecFwk {
class OsAccountManagerWrapper {
public:
    static std::shared_ptr<OsAccountManagerWrapper> GetInstance();
    ErrCode GetOsAccountLocalIdFromProcess(int32_t &id);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // MOCK_OS_ACCOUNT_MANAGER_WRAPPER_H