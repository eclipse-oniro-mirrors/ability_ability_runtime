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

#ifndef OHOS_MODULAR_OBJECT_RDB_STORAGE_MGR_H
#define OHOS_MODULAR_OBJECT_RDB_STORAGE_MGR_H

#include <singleton.h>
#include <string>

#include "modular_object_extension_info.h"
#include "modular_object_rdb_data_mgr.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
class ModularObjectExtensionRdbStorageMgr : public std::enable_shared_from_this<ModularObjectExtensionRdbStorageMgr> {
    DECLARE_DELAYED_SINGLETON(ModularObjectExtensionRdbStorageMgr)
public:
    int32_t InsertData(const std::string &key, const std::vector<AAFwk::ModularObjectExtensionInfo> &infos,
        uint32_t versionCode);
    int32_t UpdateData(const std::string &key, const std::vector<AAFwk::ModularObjectExtensionInfo> &infos,
        uint32_t versionCode);
    int32_t DeleteData(const std::string &key);
    int32_t QueryData(const std::string &key, std::vector<AAFwk::ModularObjectExtensionInfo> &infos);
    bool QueryVersion(const std::string& key, uint32_t &versionCode);
private:
    std::string ToJsonString(const std::vector<AAFwk::ModularObjectExtensionInfo> &infos, uint32_t versionCode);
    void FromJsonString(const std::string& jsonStr, std::vector<AAFwk::ModularObjectExtensionInfo> &infos);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_MODULAR_OBJECT_RDB_STORAGE_MGR_H