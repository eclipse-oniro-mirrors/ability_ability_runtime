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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H

#include <memory>

#include "ability_info.h"
#include "ability_manager_client.h"
#include "in_process_call_wrapper.h"
#include "iremote_object.h"
#include "session_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityRecord;

/**
 * @class Token
 * Token is identification of ability and used to interact with kit and wms.
 */
class Token {
public:
    explicit Token();
    virtual ~Token();

    static std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token);
};

/**
 * @class AbilityRecord
 * AbilityRecord records ability info and states and used to schedule ability life.
 */
class AbilityRecord : public std::enable_shared_from_this<AbilityRecord> {
public:
    AbilityRecord();

    ~AbilityRecord();

    /**
     * get ability's info.
     *
     * @return ability info.
     */
    const AppExecFwk::AbilityInfo &GetAbilityInfo() const;

    const Want &GetWant() const;

    sptr<SessionInfo> GetSessionInfo() const;

    int32_t GetMissionId() const;

    int32_t CreateModalUIExtension(const Want &want);

public:
    AppExecFwk::AbilityInfo abilityInfo;
    Want want_;
    sptr<SessionInfo> sessionInfo_ = nullptr;
    int32_t missionId_ = -1;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
