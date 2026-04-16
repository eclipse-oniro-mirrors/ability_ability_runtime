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

#include "fork_image_info.h"

namespace OHOS {
namespace AppExecFwk {
ForkImageInfo::ForkImageInfo()
{
    imageInfoId = CreateId();
}

int32_t ForkImageInfo::CreateId()
{
    static std::atomic_int id(0);
    return ++id;
}
} // namespace AppExecFwk
} // namespace OHOS