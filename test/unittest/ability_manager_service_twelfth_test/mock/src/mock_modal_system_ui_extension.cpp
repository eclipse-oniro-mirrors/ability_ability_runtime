/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "mock_modal_system_ui_extension.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace Rosen {

ModalSystemUiExtension::ModalSystemUiExtension()
{}

ModalSystemUiExtension::~ModalSystemUiExtension()
{}

bool ModalSystemUiExtension::CreateModalUIExtension(const AAFwk::Want &want)
{
    return AAFwk::MyFlag::retCreateModalUIExtension_;
}

bool ModalSystemUiExtension::CreateModalUIExtension(const AAFwk::Want &want, int32_t userId)
{
    (void)userId;  // Unused parameter
    return AAFwk::MyFlag::retCreateModalUIExtension_;
}

} // namespace Rosen
} // namespace OHOS
