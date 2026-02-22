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

#ifndef OHOS_ABILITY_RUNTIME_CALLER_RECORD_H
#define OHOS_ABILITY_RUNTIME_CALLER_RECORD_H

#include <cinttypes>
#include <memory>
#include <set>

#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityRecord;
namespace {
constexpr int32_t DEFAULT_REQUEST_CODE = -1;
}

/**
 * @struct CallerAbilityInfo
 * caller ability info.
 */
struct CallerAbilityInfo {
public:
    std::string callerNativeName;
    std::string callerBundleName;
    std::string callerAbilityName;
    
    int32_t callerTokenId = 0;
    int32_t callerUid = 0;
    int32_t callerPid = 0;
    int32_t callerAppCloneIndex = 0;
};

/**
 * @class SystemAbilityCallerRecord
 * Record system caller ability of for-result start mode and result.
 */
class SystemAbilityCallerRecord {
public:
    SystemAbilityCallerRecord(std::string &srcAbilityId, const sptr<IRemoteObject> &callerToken)
        : srcAbilityId_(srcAbilityId), callerToken_(callerToken) {}

    std::string GetSrcAbilityId()
    {
        return srcAbilityId_;
    }
    const sptr<IRemoteObject> GetCallerToken()
    {
        return callerToken_;
    }
    void SetResult(Want &want, int32_t resultCode)
    {
        resultWant_ = want;
        resultCode_ = resultCode;
    }
    Want &GetResultWant()
    {
        return resultWant_;
    }
    int32_t &GetResultCode()
    {
        return resultCode_;
    }
    /**
     * Set result to system ability.
     *
     */
    void SetResultToSystemAbility(std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord,
        Want &resultWant, int32_t resultCode);
    /**
     * Send result to system ability.
     *
     */
    void SendResultToSystemAbility(int32_t requestCode,
        const std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord,
        int32_t callerUid, uint32_t accessToken, bool schedulerdied);

private:
    Want resultWant_;
    std::string srcAbilityId_;
    sptr<IRemoteObject> callerToken_;
    int32_t resultCode_ = -1;
};

/**
 * @class CallerRecord
 * Record caller ability of for-result start mode and result.
 */
class CallerRecord {
public:
    CallerRecord() = default;
    CallerRecord(int32_t requestCode, std::weak_ptr<AbilityRecord> caller);
    CallerRecord(int32_t requestCode, std::shared_ptr<SystemAbilityCallerRecord> saCaller) : requestCode_(requestCode),
        saCaller_(saCaller)
    {}
    virtual ~CallerRecord()
    {}

    int32_t GetRequestCode()
    {
        return requestCode_;
    }
    std::shared_ptr<AbilityRecord> GetCaller()
    {
        return caller_.lock();
    }
    std::shared_ptr<SystemAbilityCallerRecord> GetSaCaller()
    {
        return saCaller_;
    }
    std::shared_ptr<CallerAbilityInfo> GetCallerInfo()
    {
        return callerInfo_;
    }
    bool IsHistoryRequestCode(int32_t requestCode)
    {
        return requestCodeSet_.count(requestCode) > 0;
    }
    void RemoveHistoryRequestCode(int32_t requestCode)
    {
        requestCodeSet_.erase(requestCode);
    }
    void AddHistoryRequestCode(int32_t requestCode)
    {
        requestCodeSet_.insert(requestCode);
    }
    void SetRequestCodeSet(const std::set<int32_t> &requestCodeSet)
    {
        requestCodeSet_ = requestCodeSet;
    }
    std::set<int32_t> GetRequestCodeSet()
    {
        return requestCodeSet_;
    }
    bool HasForResultRequestCode()
    {
        return std::any_of(requestCodeSet_.begin(), requestCodeSet_.end(),
            [](int32_t requestCode) { return requestCode != DEFAULT_REQUEST_CODE; });
    }

private:
    int32_t requestCode_ = -1;  // requestCode of for-result start mode
    std::weak_ptr<AbilityRecord> caller_;
    std::shared_ptr<SystemAbilityCallerRecord> saCaller_ = nullptr;
    std::shared_ptr<CallerAbilityInfo> callerInfo_ = nullptr;
    std::set<int32_t> requestCodeSet_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CALLER_RECORD_H