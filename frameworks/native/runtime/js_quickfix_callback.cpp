/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_quickfix_callback.h"

#include "file_mapper.h"
#include "file_path_utils.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
    constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
}

bool JsQuickfixCallback::operator()(std::string baseFileName, std::string &patchFileName,
                                    uint8_t **patchBuffer, size_t &patchSize)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "baseFileName: %{private}s", baseFileName.c_str());
    auto position = baseFileName.find(".abc");
    if (position == std::string::npos) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, ".abc not found");
        return false;
    }
    int baseFileNameLen = static_cast<int>(baseFileName.length());
    int prefixLen = strlen(BUNDLE_INSTALL_PATH);
    int suffixLen = strlen(MERGE_ABC_PATH);
    int moduleLen = baseFileNameLen - prefixLen - suffixLen;
    if (moduleLen < 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "invalid moduleLen");
        return false;
    }
    std::string moduleName = baseFileName.substr(prefixLen, moduleLen);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "moduleName: %{private}s", moduleName.c_str());

    auto it = moduleAndHqfPath_.find(moduleName);
    if (it == moduleAndHqfPath_.end()) {
        return false;
    }

    std::string hqfFile = it->second;
    std::string resolvedHqfFile(AbilityBase::GetLoadPath(hqfFile));
    TAG_LOGD(AAFwkTag::JSRUNTIME, "hqfFile: %{private}s, resolvedHqfFile: %{private}s", hqfFile.c_str(),
        resolvedHqfFile.c_str());

    auto data = JsRuntime::GetSafeData(resolvedHqfFile, patchFileName);
    if (data == nullptr) {
        if (patchFileName.empty()) {
            TAG_LOGI(AAFwkTag::JSRUNTIME, "No need to load patch cause no ets. path: %{private}s",
                resolvedHqfFile.c_str());
            return true;
        }
        return false;
    }
    *patchBuffer = data->GetDataPtr();
    TAG_LOGD(AAFwkTag::JSRUNTIME, "patchFileName: %{private}s", patchFileName.c_str());
    patchSize = data->GetDataLen();
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
