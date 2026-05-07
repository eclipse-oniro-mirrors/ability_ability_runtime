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

#ifndef MOCK_MODULAR_OBJECT_RDB_STORAGE_MGR_H
#define MOCK_MODULAR_OBJECT_RDB_STORAGE_MGR_H

#include <string>
#include <vector>
#include "mock_flag.h"
#include "modular_object_extension_info.h"

namespace OHOS {
namespace AbilityRuntime {

class ModularObjectExtensionRdbStorageMgr
    : public std::enable_shared_from_this<ModularObjectExtensionRdbStorageMgr> {
    DECLARE_DELAYED_SINGLETON(ModularObjectExtensionRdbStorageMgr);
public:
    int32_t QueryData(const std::string &key, std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
    {
        if (MockFlag::queryDataRet != 0) {
            return MockFlag::queryDataRet;
        }
        if (MockFlag::extensionFound) {
            AAFwk::ModularObjectExtensionInfo info;
            info.bundleName = "com.test.bundle";
            info.abilityName = "TestAbility";
            info.isDisabled = MockFlag::extensionDisabled;
            infos.push_back(info);
        }
        return 0;
    }
    int32_t InsertOrUpdateData(const std::string &key,
        const std::vector<AAFwk::ModularObjectExtensionInfo> &infos, uint32_t versionCode)
    {
        return 0;
    }
    int32_t DeleteData(const std::string &key) { return 0; }
    bool QueryVersion(const std::string &key, uint32_t &versionCode) { return false; }
};

} // namespace AbilityRuntime
} // namespace OHOS

// Bring into AAFwk namespace for source compatibility
namespace OHOS {
namespace AAFwk {
using AbilityRuntime::ModularObjectExtensionRdbStorageMgr;
} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_MODULAR_OBJECT_RDB_STORAGE_MGR_H
