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

#ifndef MOCK_MODAL_SYSTEM_UI_EXTENSION_H
#define MOCK_MODAL_SYSTEM_UI_EXTENSION_H

#include "want.h"

namespace OHOS {
namespace Rosen {
class ModalSystemUiExtension {
public:
    ModalSystemUiExtension();
    ~ModalSystemUiExtension();

    bool CreateModalUIExtension(const AAFwk::Want &want);
    bool CreateModalUIExtension(const AAFwk::Want &want, int32_t userId);
};
} // namespace Rosen
} // namespace OHOS

#endif // MOCK_MODAL_SYSTEM_UI_EXTENSION_H
