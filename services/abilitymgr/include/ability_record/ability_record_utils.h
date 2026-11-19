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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RECORD_UTILS_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RECORD_UTILS_H

#include <cinttypes>
#include <memory>

#include "ability_token_stub.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityRecord;

// new version
enum ResolveResultType {
    OK_NO_REMOTE_OBJ = 0,
    OK_HAS_REMOTE_OBJ,
    NG_INNER_ERROR,
};

enum class AbilityWindowState {
    FOREGROUND = 0,
    BACKGROUND,
    TERMINATE,
    FOREGROUNDING,
    BACKGROUNDING,
    TERMINATING
};

enum class AbilityVisibilityState {
    INITIAL = 0,
    FOREGROUND_HIDE,
    FOREGROUND_SHOW,
    UNSPECIFIED,
};

enum class AbilityRecordType {
    BASE_ABILITY,
    UI_ABILITY,
    MISSION_ABILITY,
    EXTENSION_ABILITY,
};

/**
 * @class Token
 * Token is identification of ability and used to interact with kit and wms.
 */
class Token : public AbilityTokenStub {
public:
    explicit Token(std::weak_ptr<AbilityRecord> abilityRecord);
    virtual ~Token();

    std::shared_ptr<AbilityRecord> GetAbilityRecord() const;
    static std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(sptr<IRemoteObject> token);

private:
    std::weak_ptr<AbilityRecord> abilityRecord_;  // ability of this token
};

/**
 * @class AbilityResult
 * Record requestCode of for-result start mode and result.
 */
class AbilityResult {
public:
    AbilityResult() = default;
    AbilityResult(int requestCode, int resultCode, const Want &resultWant)
        : requestCode_(requestCode), resultCode_(resultCode), resultWant_(resultWant)
    {}
    virtual ~AbilityResult()
    {}

    int requestCode_ = -1;  // requestCode of for-result start mode
    int resultCode_ = -1;   // resultCode of for-result start mode
    Want resultWant_;       // for-result start mode ability will send the result to caller
};

struct LaunchDebugInfo {
public:
    void Update(const Want &want);

    std::string perfCmd;
    bool isDebugAppSet = false;
    bool isNativeDebugSet = false;
    bool isPerfCmdSet = false;
    bool debugApp = false;
    bool nativeDebug = false;
};

struct ForegroundOptions {
    uint32_t sceneFlag = 0;
    bool isShellCall = false;
    bool isStartupHide = false;
    std::string targetGrantBundleName;
    pid_t callingPid = -1;
    pid_t selfPid = -1;
    int32_t requestCode = -1;
    uint64_t loadAbilityCallbackId = 0;
    int32_t callerTypeForAnco = 0;
    uint32_t realCallerTokenId = 0;
    int32_t callerUid = -1;
    std::string callerBundleName;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif