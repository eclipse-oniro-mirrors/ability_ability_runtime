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

#ifndef OHOS_ABILITY_RUNTIME_EXIT_INFO_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_EXIT_INFO_DATA_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "running_process_info.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
struct ExitCacheInfo {
    AppExecFwk::RunningProcessInfo exitInfo = {};
    std::string bundleName = "";
    std::vector<std::string> abilityNames;
    std::vector<std::string> uiExtensionNames;
};

class ExitInfoDataManager {
public:
    static ExitInfoDataManager &GetInstance()
    {
        static ExitInfoDataManager instance;
        return instance;
    }

    virtual ~ExitInfoDataManager() = default;

    bool AddExitInfo(uint32_t accessTokenId, ExitCacheInfo &cacheInfo);

    bool DeleteExitInfo(uint32_t accessTokenId);

    bool GetExitInfo(uint32_t accessTokenId, ExitCacheInfo &cacheInfo);

    bool GetExitInfo(int32_t pid, int32_t uid, uint32_t &accessTokenId, ExitCacheInfo &cacheInfo);

    bool IsExitInfoExist(uint32_t accessTokenId);

private:
    std::mutex mutex_;
    ExitInfoDataManager() = default;
    DISALLOW_COPY_AND_MOVE(ExitInfoDataManager);

private:
    std::map<uint32_t, ExitCacheInfo> exitCacheInfos_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXIT_INFO_DATA_MANAGER_H
