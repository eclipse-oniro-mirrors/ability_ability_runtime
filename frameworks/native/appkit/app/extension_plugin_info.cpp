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

#include "extension_plugin_info.h"

#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>

#include "extension_module_loader.h"
#include "file_path_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
#ifdef APP_USE_ARM64
const std::string EXTENSION_LIB = "/system/lib64/extensionability";
#elif defined(APP_USE_X86_64)
const std::string EXTENSION_LIB = "/system/lib64/extensionability";
#else
const std::string EXTENSION_LIB = "/system/lib/extensionability";
#endif
const std::string PATH_SEPARATOR = "/";
const std::string LIB_TYPE = ".so";
constexpr char EXTENSION_PARAMS_TYPE[] = "type";
constexpr char EXTENSION_PARAMS_NAME[] = "name";

ExtensionPluginInfo::ExtensionPluginInfo()
{
}

ExtensionPluginInfo& ExtensionPluginInfo::GetInstance()
{
    static ExtensionPluginInfo instance;
    return instance;
}

void ExtensionPluginInfo::Preload()
{
    // scan all extensions in path
    std::vector<std::string> extensionFiles;
    ScanExtensions(extensionFiles);
    ParseExtensions(extensionFiles);
}

std::vector<ExtensionPluginItem> ExtensionPluginInfo::GetExtensionPlugins()
{
    return extensionPlugins_;
}

void ExtensionPluginInfo::ParseExtensions(const std::vector<std::string>& extensionFiles)
{
    if (extensionFiles.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "no extension files");
        return;
    }

    for (auto& file : extensionFiles) {
        TAG_LOGD(AAFwkTag::APPKIT, "Begin load extension file:%{public}s", file.c_str());
        std::map<std::string, std::string> params =
            AbilityRuntime::ExtensionModuleLoader::GetLoader(file.c_str()).GetParams();
        if (params.empty()) {
            TAG_LOGE(AAFwkTag::APPKIT, "no extension params");
            continue;
        }
        // get extension name and type
        std::map<std::string, std::string>::iterator it = params.find(EXTENSION_PARAMS_TYPE);
        if (it == params.end()) {
            TAG_LOGE(AAFwkTag::APPKIT, "no extension type");
            continue;
        }
        int32_t type = -1;
        try {
            type = static_cast<int32_t>(std::stoi(it->second));
        } catch (...) {
            TAG_LOGW(AAFwkTag::APPKIT, "stoi(%{public}s) failed", it->second.c_str());
            continue;
        }

        it = params.find(EXTENSION_PARAMS_NAME);
        if (it == params.end()) {
            TAG_LOGE(AAFwkTag::APPKIT, "no extension name");
            continue;
        }
        std::string extensionName = it->second;

        ExtensionPluginItem item;
        item.extensionType = type;
        item.extensionName = extensionName;
        item.extensionLibFile = file;
        auto findTask = [extensionName](ExtensionPluginItem &item) {
            return item.extensionName == extensionName;
        };
        if (find_if(extensionPlugins_.begin(), extensionPlugins_.end(), findTask) != extensionPlugins_.end()) {
            continue;
        }
        extensionPlugins_.emplace_back(item);
        TAG_LOGD(
            AAFwkTag::APPKIT, "Success load extension type: %{public}d, name:%{public}s", type, extensionName.c_str());
    }
}

bool ExtensionPluginInfo::ScanExtensions(std::vector<std::string>& files)
{
    std::string dirPath = EXTENSION_LIB;
    DIR *dirp = opendir(dirPath.c_str());
    if (dirp == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ScanDir open dir:%{public}s fail", dirPath.c_str());
        return false;
    }

    struct dirent *dirf = nullptr;
    for (;;) {
        dirf = readdir(dirp);
        if (dirf == nullptr) {
            break;
        }

        std::string currentName(dirf->d_name);
        if (currentName.compare(".") == 0 || currentName.compare("..") == 0) {
            continue;
        }

        if (CheckFileType(currentName, LIB_TYPE)) {
            files.emplace_back(dirPath + PATH_SEPARATOR + currentName);
        }
    }

    if (closedir(dirp) == -1) {
        TAG_LOGW(AAFwkTag::APPKIT, "close dir fail");
    }
    return true;
}

bool ExtensionPluginInfo::CheckFileType(const std::string& fileName, const std::string& extensionName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CheckFileType path is %{public}s, support suffix is %{public}s",
        fileName.c_str(),
        extensionName.c_str());

    if (fileName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "file name empty");
        return false;
    }

    auto position = fileName.rfind('.');
    if (position == std::string::npos) {
        TAG_LOGW(AAFwkTag::APPKIT, "filename no extension name");
        return false;
    }

    std::string suffixStr = fileName.substr(position);
    return LowerStr(suffixStr) == extensionName;
}
} // namespace AbilityRuntime
} // namespace OHOS
