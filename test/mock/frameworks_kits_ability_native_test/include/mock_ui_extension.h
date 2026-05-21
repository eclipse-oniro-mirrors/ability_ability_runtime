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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_UI_EXTENSION_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_UI_EXTENSION_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ui_extension.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {

class MockUIExtension : public UIExtension {
public:
    MockUIExtension() = default;
    virtual ~MockUIExtension() = default;

    MOCK_METHOD1(CreateModalUIExtension, int(const AAFwk::Want &want));
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_OHOS_ABILITY_RUNTIME_MOCK_UI_EXTENSION_H
