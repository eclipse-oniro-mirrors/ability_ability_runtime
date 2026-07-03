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

#ifndef MOCK_MY_FLAG_H
#define MOCK_MY_FLAG_H

#include <memory>
#include <vector>

namespace OHOS {
namespace AAFwk {
class UIAbilityLifecycleManager;
class AbilityRecord;

class MyFlag {
public:
    static MyFlag &GetInstance()
    {
        static MyFlag instance;
        return instance;
    };

public:
    bool retCheckSpecificSystemAbilityAccessPermission_ = false;
    bool isScbEnabled_ = false;
    bool retCheckCallingTokenId_ = false;
    std::shared_ptr<UIAbilityLifecycleManager> uiManager_ = nullptr;
    bool isCallerInStatusBar_ = false;
    std::vector<std::shared_ptr<AbilityRecord>> abilityRecords_;
    bool isStartSelfUIAbility_ = false;
    bool isHiddenStart_ = false;
    int32_t retHiddenStartSupported_ = 0;
    bool isStartOptionsWithProcessOptions_ = false;
    int32_t setTargetCloneIndexCallCount_ = 0;
    int32_t resolveTargetAppCloneIndexCallCount_ = 0;
    int32_t resolveTargetAppCloneIndexUserId_ = -1;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_MY_FLAG_H
