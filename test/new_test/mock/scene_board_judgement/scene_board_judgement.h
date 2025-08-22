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

#ifndef MOCK_OHOS_ROSEN_WINDOW_SCENE_SCENE_BOARD_JUDGEMENT_H
#define MOCK_OHOS_ROSEN_WINDOW_SCENE_SCENE_BOARD_JUDGEMENT_H

#include "oh_mock_utils.h"

namespace OHOS {
namespace Rosen {
class SceneBoardJudgement final {
public:
    OH_MOCK_METHOD_WITH_DECORATOR(static, bool, SceneBoardJudgement, IsSceneBoardEnabled);
};
} // namespace Rosen
} // namespace OHOS
#endif