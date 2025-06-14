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

#ifndef MOCK_MY_STATUS_H
#define MOCK_MY_STATUS_H

#include <cinttypes>

namespace OHOS {
namespace AppExecFwk {
class MyStatus {
public:
    static MyStatus& GetInstance();
    ~MyStatus() = default;
    int32_t statusValue_ = 0;
    bool instanceStatus_ = true;
private:
    MyStatus() = default;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // MOCK_MY_STATUS_H