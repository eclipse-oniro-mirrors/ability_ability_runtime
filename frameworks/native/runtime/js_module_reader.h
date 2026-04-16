/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H
#define OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H

#include <sstream>
#include <string>

#include "js_module_searcher.h"
#include "extractor.h"

using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AbilityRuntime {
class JsModuleReader final : private JsModuleSearcher {
public:
    static constexpr char ABS_CODE_PATH[] = "/data/storage/el1/";
    static constexpr char ABS_DATA_CODE_PATH[] = "/data/app/el1/bundle/public/";
    static constexpr char BUNDLE[] = "bundle/";
    static constexpr char MERGE_ABC_PATH[] = "ets/modules.abc";
    static constexpr char SYS_ABS_CODE_PATH[] = "/system/app/appServiceFwk/";
    static constexpr char SHARED_FILE_SUFFIX[] = ".hsp";
    JsModuleReader(const std::string& bundleName, const std::string& hapPath, bool isFormRender = false);
    ~JsModuleReader() = default;

    JsModuleReader(const JsModuleReader&) = default;
    JsModuleReader(JsModuleReader&&) = default;
    JsModuleReader& operator=(const JsModuleReader&) = default;
    JsModuleReader& operator=(JsModuleReader&&) = default;

    bool operator()(const std::string& inputPath, uint8_t **buff, size_t *buffSize, std::string& errorMsg) const;
    static std::string GetPresetAppHapPath(const std::string& inputPath, const std::string& bundleName);
    static void GetHapPathList(const std::string &bundleName, std::vector<std::string> &hapList);

private:
    std::string GetAppHspPath(const std::string& inputPath, bool& needFindPluginHsp, std::string& errorMsg) const;
    std::string GetCommonAppHspPath(const std::string& inputPath, bool& needFindPluginHsp,
        std::string& errorMsg) const;
    std::string GetFormAppHspPath(const std::string& inputPath, bool& needFindPluginHsp, std::string& errorMsg) const;
    std::string GetModuleName(const std::string& inputPath) const;
    std::string GetPluginHspPath(const std::string& inputPath) const;
    static std::string GetOtherHspPath(const std::string& bundleName, const std::string& moduleName,
        const std::string& inputPath, bool& needFindPluginHsp, std::string& errorMsg);
    static std::string GetPresetAppHapPath(const std::string& inputPath, const std::string& bundleName,
        bool& needFindPluginHsp, std::string& errorMsg);

    bool isSystemPath_ = false;
    bool isFormRender_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H
