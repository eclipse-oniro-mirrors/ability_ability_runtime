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

#ifndef OHOS_ABILITY_RUNTIME_HYBRID_JS_MODULE_READER_H
#define OHOS_ABILITY_RUNTIME_HYBRID_JS_MODULE_READER_H

#include <sstream>
#include <string>

#include "js_module_searcher.h"
#include "extractor.h"

using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AbilityRuntime {
class HybridJsModuleReader final : private JsModuleSearcher {
public:
    static constexpr char ABS_CODE_PATH[] = "/data/storage/el1/";
    static constexpr char ABS_DATA_CODE_PATH[] = "/data/app/el1/bundle/public/";
    static constexpr char MERGE_ABC_PATH[] = "ets/modules.abc";
    static constexpr char SYS_ABS_CODE_PATH[] = "/system/app/appServiceFwk/";
    static constexpr char SHARED_FILE_SUFFIX[] = ".hsp";
    static constexpr char ABILITY_FILE_SUFFIX[] = ".hap";
    HybridJsModuleReader(const std::string& bundleName, const std::string& hapPath, bool isFormRender = false);
    ~HybridJsModuleReader() = default;

    HybridJsModuleReader(const HybridJsModuleReader&) = default;
    HybridJsModuleReader(HybridJsModuleReader&&) = default;
    HybridJsModuleReader& operator=(const HybridJsModuleReader&) = default;
    HybridJsModuleReader& operator=(HybridJsModuleReader&&) = default;

    bool operator()(const std::string& inputPath, uint8_t **buff, size_t *buffSize, std::string& errorMsg) const;
    static std::string GetPresetAppHapPath(const std::string& inputPath, const std::string& bundleName);

private:
    std::string GetAppPath(const std::string& inputPath, const std::string& suffix,
        bool& needFindPluginHsp) const;
    std::string GetCommonAppPath(const std::string& inputPath, const std::string& suffix,
        bool& needFindPluginHsp) const;
    std::string GetFormAppPath(const std::string& inputPath, const std::string& suffix,
        bool& needFindPluginHsp) const;
    std::string GetModuleName(const std::string& inputPath) const;
    std::string GetPluginHspPath(const std::string& inputPath) const;
    std::shared_ptr<Extractor> GetExtractor(const std::string& inputPath, std::string& errorMsg) const;
    static std::string GetOtherHspPath(const std::string& bundleName, const std::string& moduleName,
        const std::string& inputPath, bool& needFindPluginHsp);
    static std::string GetPresetAppHapPath(const std::string& inputPath, const std::string& bundleName,
        bool& needFindPluginHsp);

    bool isSystemPath_ = false;
    bool isFormRender_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_HYBRID_JS_MODULE_READER_H
