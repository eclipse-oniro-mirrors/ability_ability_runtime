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

#ifndef MOCK_MY_FLAG_H
#define MOCK_MY_FLAG_H

#include <vector>
#include <memory>
#include "ability_info.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class MyFlag {
public:
    static int retVerifyAccessTokenId_;
    static bool retValue_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_MY_FLAG_H
