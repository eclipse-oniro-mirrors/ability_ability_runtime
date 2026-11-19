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
#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H

#include <memory>
#include "ability_info.h"
#include "ability_token_stub.h"
#include "application_info.h"

namespace OHOS {
namespace AAFwk {
class AbilityRecord;
class Token : public AbilityTokenStub {
public:
    Token() {}
    virtual ~Token() {}
    static std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(sptr<IRemoteObject> token);
    static std::shared_ptr<AbilityRecord> abilityRecord;
};

class AbilityRecord : public std::enable_shared_from_this<AbilityRecord> {
public:
    AbilityRecord() {}
    ~AbilityRecord() {}
    inline int32_t GetAppIndex() const { return appIndex_; }
    inline AppExecFwk::ApplicationInfo GetApplicationInfo() const { return applicationInfo_; }
    int32_t appIndex_ = 0;
    AppExecFwk::ApplicationInfo applicationInfo_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
