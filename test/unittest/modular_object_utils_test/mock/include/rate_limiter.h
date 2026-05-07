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

#ifndef MOCK_RATE_LIMITER_H
#define MOCK_RATE_LIMITER_H

#include "mock_flag.h"

namespace OHOS {
namespace AAFwk {
class RateLimiter {
public:
    static RateLimiter &GetInstance()
    {
        static RateLimiter instance;
        return instance;
    }
    bool CheckModularObjectLimit(int32_t uid)
    {
        return MockFlag::modularObjectLimited;
    }
};
} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_RATE_LIMITER_H
