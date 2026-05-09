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

#include "modular_object_manager.h"

#include "hilog_tag_wrapper.h"
#include "modular_object_rdb_storage_mgr.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AbilityRuntime {
ModularObjectManager::ModularObjectManager() = default;
ModularObjectManager::~ModularObjectManager() = default;

int32_t ModularObjectManager::QuerySelfModularObjectExtensionInfos(int32_t userId, const std::string &bundleName,
    int32_t appIndex, std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    std::string key = std::to_string(userId) + "_" + bundleName + "_" + std::to_string(appIndex);
    return DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryData(key, infos);
}
}
}
