/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_FREEZE_UTIL_H
#define OHOS_ABILITY_RUNTIME_FREEZE_UTIL_H

#include <list>
#include <mutex>
#include <unordered_map>

#include "iremote_object.h"

namespace OHOS::AbilityRuntime {
class FreezeUtil {
public:
    enum class TimeoutState {
        UNKNOWN = 0,
        LOAD,
        FOREGROUND,
        BACKGROUND,
        CONNECT,
        BY_CALL
    };

    struct LifecycleFlow {
        sptr<IRemoteObject> token;
        TimeoutState state = TimeoutState::UNKNOWN;
    };

    FreezeUtil& operator=(const FreezeUtil&) = delete;
    FreezeUtil(const FreezeUtil&) = delete;
    virtual ~FreezeUtil() = default;
    static FreezeUtil& GetInstance();

    void AddLifecycleEvent(sptr<IRemoteObject> token, const std::string &entry);
    bool AppendLifecycleEvent(sptr<IRemoteObject> token, const std::string &entry);
    bool IsFrozen(pid_t pid);
    std::string GetLifecycleEvent(sptr<IRemoteObject> token);
    void DeleteLifecycleEvent(sptr<IRemoteObject> token);

    void AddAppLifecycleEvent(pid_t pid, const std::string &entry);
    void DeleteAppLifecycleEvent(pid_t pid);
    std::string GetAppLifecycleEvent(pid_t pid);
private:
    FreezeUtil() = default;

    class RemoteObjHash {
    public:
        size_t operator() (const sptr<IRemoteObject> &obj) const
        {
            return std::hash<IRemoteObject*>()(obj.GetRefPtr());
        }
    };

    std::mutex mutex_;
    std::unordered_map<sptr<IRemoteObject>, std::list<std::string>, RemoteObjHash> lifecycleFlow_;
    std::unordered_map<pid_t, std::list<std::string>> appLifeCycleFlow_;
};
}  // namespace OHOS::AbilityRuntime
#endif  // OHOS_ABILITY_RUNTIME_FREEZE_UTIL_H