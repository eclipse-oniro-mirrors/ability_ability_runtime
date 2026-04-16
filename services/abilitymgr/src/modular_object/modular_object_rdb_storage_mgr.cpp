/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#include <sstream>

#include "modular_object_rdb_data_mgr.h"
#include "modular_object_rdb_storage_mgr.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* MODULAR_OBJECT_EXTENSION_INFO = "ModularObjectExtensionInfo";
constexpr const char* MODULAR_OBJECT_BUNDLE_VERSION = "ModularObjectBundleVersion";
}

ModularObjectExtensionRdbStorageMgr::ModularObjectExtensionRdbStorageMgr()
{
}

ModularObjectExtensionRdbStorageMgr::~ModularObjectExtensionRdbStorageMgr()
{
    TAG_LOGD(AAFwkTag::EXT, "ModularObjectExtensionRdbStorageMgr is deleted");
}

int32_t ModularObjectExtensionRdbStorageMgr::InsertData(
    const std::string& key, const std::vector<AAFwk::ModularObjectExtensionInfo> &infos, uint32_t versionCode)
{
    if (infos.empty()) {
        return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->DeleteData(key);
    }
    std::string jsonString = ToJsonString(infos, versionCode);
    return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->InsertData(key, jsonString);
}

int32_t ModularObjectExtensionRdbStorageMgr::UpdateData(
    const std::string& key, const std::vector<AAFwk::ModularObjectExtensionInfo> &infos, uint32_t versionCode)
{
    std::string jsonString = ToJsonString(infos, versionCode);
    return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->UpdateData(key, jsonString);
}

int32_t ModularObjectExtensionRdbStorageMgr::DeleteData(const std::string& key)
{
    return DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->DeleteData(key);
}

int32_t ModularObjectExtensionRdbStorageMgr::QueryData(const std::string& key,
    std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    std::string jsonString;
    DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->QueryData(key, jsonString);
    FromJsonString(jsonString, infos);
    return NativeRdb::E_OK;
}

bool ModularObjectExtensionRdbStorageMgr::QueryVersion(const std::string& key, uint32_t &versionCode)
{
    std::string jsonString;
    auto ret = DelayedSingleton<ModularObjectExtensionRdbDataMgr>::GetInstance()->QueryData(key, jsonString);
    if (ret != NativeRdb::E_OK) {
        return false;
    }

    nlohmann::json jsonObject = nlohmann::json::parse(jsonString, nullptr, false);
    if (jsonObject.is_discarded()) {
        return false;
    }

    if (jsonObject.contains(MODULAR_OBJECT_BUNDLE_VERSION) && jsonObject[MODULAR_OBJECT_BUNDLE_VERSION].is_number()) {
        versionCode = jsonObject[MODULAR_OBJECT_BUNDLE_VERSION].get<uint32_t>();
        return true;
    }

    return false;
}

std::string ModularObjectExtensionRdbStorageMgr::ToJsonString(
    const std::vector<AAFwk::ModularObjectExtensionInfo> &infos, uint32_t versionCode)
{
    nlohmann::json jsonNodes = nlohmann::json::array();
    auto size = infos.size();
    for (size_t i = 0; i < size; i++) {
        jsonNodes.emplace_back(infos[i].ToJson());
    }
    nlohmann::json jsonObject {
        {MODULAR_OBJECT_EXTENSION_INFO, jsonNodes},
        {MODULAR_OBJECT_BUNDLE_VERSION, versionCode},
    };
    return jsonObject.dump();
}

void ModularObjectExtensionRdbStorageMgr::FromJsonString(const std::string& jsonStr,
    std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        return;
    }
    if (jsonObject.contains(MODULAR_OBJECT_EXTENSION_INFO) && jsonObject[MODULAR_OBJECT_EXTENSION_INFO].is_array()) {
        for (const auto& item : jsonObject[MODULAR_OBJECT_EXTENSION_INFO]) {
            if (item.is_object()) {
                infos.emplace_back(AAFwk::ModularObjectExtensionInfo::FromJson(item));
            }
        }
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
