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

#include "ability_manager_xcollie.h"

#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace AbilityRuntime {
AbilityManagerXCollie::AbilityManagerXCollie(const std::string &tag, uint32_t timeoutSeconds)
{
    id_ = HiviewDFX::XCollie::GetInstance().SetTimer(tag, timeoutSeconds, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_LOG);
}

AbilityManagerXCollie::~AbilityManagerXCollie()
{
    CancelAbilityManagerXCollie();
}

void AbilityManagerXCollie::CancelAbilityManagerXCollie()
{
    if (!isCanceled_) {
        HiviewDFX::XCollie::GetInstance().CancelTimer(id_);
        isCanceled_ = true;
    }
}
}
}