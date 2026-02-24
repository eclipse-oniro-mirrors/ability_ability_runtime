/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "js_worker.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <vector>

#include "bundle_info.h"
#include "bundle_mgr_helper.h"
#include "bundle_mgr_proxy.h"
#include "connect_server_manager.h"
#include "console.h"
#ifdef SUPPORT_SCREEN
#include "core/common/container_scope.h"
#include "declarative_module_preloader.h"
#endif
#include "replace_intl_module.h"

#include "extractor.h"
#include "file_mapper.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "js_runtime_common.h"
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "refbase.h"
#include "singleton.h"
#include "syscap_ts.h"
#include "system_ability_definition.h"
#include "js_environment_impl.h"
#include "worker_info.h"
#ifdef SUPPORT_SCREEN
using OHOS::Ace::ContainerScope;
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t ASSET_FILE_MAX_SIZE = 32 * 1024 * 1024;
constexpr int32_t API8 = 8;
constexpr int32_t API12 = 12;
const std::string BUNDLE_NAME_FLAG = "@bundle:";
const std::string CACHE_DIRECTORY = "el2";
const std::string RESTRICTED_PREFIX_PATH = "abcs/";
const int PATH_THREE = 3;
#ifdef APP_USE_ARM
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_inspector.z.so";
#elif defined(APP_USE_X86_64)
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_inspector.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_inspector.z.so";
#endif

bool g_jsFramework = false;
std::mutex g_mutex;
}

void InitWorkerFunc(NativeEngine* nativeEngine)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    if (nativeEngine == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null nativeEngine");
        return;
    }

    napi_value globalObj = nullptr;
    HandleScope handleScope(reinterpret_cast<napi_env>(nativeEngine));
    napi_get_global(reinterpret_cast<napi_env>(nativeEngine), &globalObj);
    if (globalObj == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null globalObj");
        return;
    }

    OHOS::Global::I18n::ReplaceIntlModule(reinterpret_cast<napi_env>(nativeEngine));
    InitSyscapModule(reinterpret_cast<napi_env>(nativeEngine));
    OHOS::JsSysModule::Console::InitConsoleModule(reinterpret_cast<napi_env>(nativeEngine));
#ifdef SUPPORT_SCREEN
    OHOS::Ace::DeclarativeModulePreloader::PreloadWorker(*nativeEngine);
#endif
    auto arkNativeEngine = static_cast<ArkNativeEngine*>(nativeEngine);
    // load jsfwk
    if (g_jsFramework && !arkNativeEngine->ExecuteJsBin("/system/etc/strip.native.min.abc")) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "load jsFramework failed");
    }

    if (JsRuntimeCommon::GetInstance().IsDebugMode()) {
        const std::string threadName = "workerThread";
        napi_status errCode = JsRuntimeCommon::GetInstance().StartDebugMode(nativeEngine, threadName);
        if (errCode != napi_status::napi_ok) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "start debug mode failed");
        }
    }
}

void OffWorkerFunc(NativeEngine* nativeEngine)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    if (nativeEngine == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null nativeEngine");
        return;
    }

    if (JsRuntimeCommon::GetInstance().IsDebugMode()) {
        napi_status errCode = JsRuntimeCommon::GetInstance().StopDebugMode(nativeEngine);
        if (errCode != napi_status::napi_ok) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "stop debug mode failed");
        }
    }
}


using Extractor = AbilityBase::Extractor;
using ExtractorUtil = AbilityBase::ExtractorUtil;
using FileMapper = AbilityBase::FileMapper;
using FileMapperType = AbilityBase::FileMapperType;
using IBundleMgr = AppExecFwk::IBundleMgr;

void ReleaseWorkerSafeMemFunc(void* mapper)
{
    TAG_LOGI(AAFwkTag::JSRUNTIME, "called");
    if (mapper) {
        FileMapper* myMapper = static_cast<FileMapper*>(mapper);
        myMapper->SetAutoReleaseMem(true);
        delete myMapper;
    }
}

std::string AssetHelper::NormalizedFileName(const std::string& fileName) const
{
    std::string normalizedFilePath;
    size_t index = 0;
    index = fileName.find_last_of(".");
    // 1.1 end with file name
    // 1.2 end with file name and file type
    if (index == std::string::npos) {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "uri end without file type");
        normalizedFilePath = fileName + ".abc";
    } else {
        TAG_LOGD(AAFwkTag::JSRUNTIME, "uri end with file type");
        normalizedFilePath = fileName.substr(0, index) + ".abc";
    }
    return normalizedFilePath;
}

AssetHelper::AssetHelper(std::shared_ptr<JsEnv::WorkerInfo> workerInfo) : workerInfo_(workerInfo)
{
    panda::panda_file::StringPacProtect codePath = panda::panda_file::StringPacProtect(workerInfo_->codePath);
    if (!(codePath.GetOriginString()).empty() && (codePath.GetOriginString()).back() != '/') {
        (workerInfo_->codePath).Append('/');
    }
}

AssetHelper::~AssetHelper()
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "destroyed");
    if (file_ != nullptr) {
        fclose(file_);
        file_ = nullptr;
    }
}

void AssetHelper::operator()(const std::string& uri, uint8_t** buff, size_t* buffSize, std::vector<uint8_t>& content,
    std::string& ami, bool& useSecureMem, void** mapper, bool isRestricted)
{
    if (uri.empty() || buff == nullptr || buffSize == nullptr || workerInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Input params invalid");
        return;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "RegisterAssetFunc called, uri: %{private}s", uri.c_str());
    std::string realPath;
    std::string filePath;
    useSecureMem = false;

    // 1. compilemode is jsbundle
    // 2. compilemode is esmodule
    if (workerInfo_->isBundle) {
        // the @bundle:bundlename/modulename only exist in esmodule.
        // 1.1 start with /modulename
        // 1.2 start with ../
        // 1.3 start with @namespace [not support]
        // 1.4 start with modulename
        TAG_LOGD(AAFwkTag::JSRUNTIME, "esmodule mode");
        if (uri.find_first_of("/") == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with /modulename");
            realPath = uri.substr(1);
        } else if (uri.find("../") == 0 && !GetIsStageModel()) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with ../");
            realPath = uri.substr(PATH_THREE);
        } else if (uri.find_first_of("@") == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with @namespace");
            realPath = uri.substr(uri.find_first_of("/") + 1);
        } else {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with modulename");
            realPath = uri;
        }

        filePath = NormalizedFileName(realPath);
        TAG_LOGI(AAFwkTag::JSRUNTIME, "filePath %{private}s", filePath.c_str());

        if (!GetIsStageModel()) {
            GetAmi(ami, filePath);
        } else {
            ami = (workerInfo_->codePath).GetOriginString() + filePath;
        }

        TAG_LOGD(AAFwkTag::JSRUNTIME, "Get asset, ami: %{private}s", ami.c_str());
        if (ami.find(CACHE_DIRECTORY) != std::string::npos) {
            if (!ReadAmiData(ami, buff, buffSize, content, useSecureMem, isRestricted, mapper)) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "Get buffer by ami failed");
            }
        } else if (!ReadFilePathData(filePath, buff, buffSize, content, useSecureMem, isRestricted, mapper)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "Get buffer by filepath failed");
        }
    } else {
        // 2.1 start with @bundle:bundlename/modulename
        // 2.2 start with /modulename
        // 2.3 start with @namespace
        // 2.4 start with modulename
        TAG_LOGD(AAFwkTag::JSRUNTIME, "esmodule mode");
        if (uri.find(BUNDLE_NAME_FLAG) == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with @bundle:");
            size_t fileNamePos = uri.find_last_of("/");
            realPath = uri.substr(fileNamePos + 1);
            if (realPath.find_last_of(".") != std::string::npos) {
                ami = NormalizedFileName(uri);
            } else {
                ami = uri;
            }
            TAG_LOGD(AAFwkTag::JSRUNTIME, "Get asset, ami: %{private}s", ami.c_str());
            return;
        } else if (uri.find_first_of("/") == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with /modulename");
            realPath = uri.substr(1);
        } else if (uri.find_first_of("@") == 0) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with @namespace");
            realPath = workerInfo_->moduleName + uri;
        } else {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "uri start with modulename");
            realPath = uri;
        }

        filePath = NormalizedFileName(realPath);
        // for safe reason, filePath must starts with 'abcs/' in restricted env
        if (isRestricted && filePath.find(RESTRICTED_PREFIX_PATH)
            && (static_cast<int32_t>(workerInfo_->apiTargetVersion.GetOriginPointer())) >= API12) {
            filePath = RESTRICTED_PREFIX_PATH + filePath;
        }
        ami = (workerInfo_->codePath).GetOriginString() + filePath;
        TAG_LOGD(AAFwkTag::JSRUNTIME, "Get asset, ami: %{private}s", ami.c_str());
        if (ami.find(CACHE_DIRECTORY) != std::string::npos) {
            if (!ReadAmiData(ami, buff, buffSize, content, useSecureMem, isRestricted, mapper)) {
                TAG_LOGE(AAFwkTag::JSRUNTIME, "Get buffer by ami failed");
            }
        } else if (!ReadFilePathData(filePath, buff, buffSize, content, useSecureMem, isRestricted, mapper)) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "Get buffer by filepath failed");
        }
    }
}

bool AssetHelper::GetSafeData(const std::string& ami, uint8_t** buff, size_t* buffSize, void** mapper)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "called");
    std::string resolvedPath;
    resolvedPath.reserve(PATH_MAX);
    resolvedPath.resize(PATH_MAX - 1);
    if (realpath(ami.c_str(), &(resolvedPath[0])) == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Realpath file %{private}s caught error: %{public}d", ami.c_str(), errno);
        return false;
    }
    FILE *fileF = fopen(resolvedPath.c_str(), "r");
    if (fileF == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Open file %{private}s caught error: %{public}d", resolvedPath.c_str(), errno);
        return false;
    }
    int fd = fileno(fileF);
    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get fstat of file %{private}s caught error: %{public}d", resolvedPath.c_str(),
            errno);
        fclose(fileF);
        return false;
    }

    std::unique_ptr<FileMapper> fileMapper = std::make_unique<FileMapper>();
    if (fileMapper == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null fileMapper");
        fclose(fileF);
        return false;
    }

    auto result = fileMapper->CreateFileMapper(resolvedPath, false, fd, 0, statbuf.st_size, FileMapperType::SAFE_ABC);
    if (!result) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Create file %{private}s mapper failed", resolvedPath.c_str());
        fclose(fileF);
        return false;
    }

    *buff = fileMapper->GetDataPtr();
    *buffSize = fileMapper->GetDataLen();
    *mapper = fileMapper.release();
    if (file_ != nullptr) {
        fclose(file_);
        file_ = nullptr;
    }
    file_ = fileF;
    return true;
}

bool AssetHelper::ReadAmiData(const std::string& ami, uint8_t** buff, size_t* buffSize, std::vector<uint8_t>& content,
    bool& useSecureMem, bool isRestricted, void** mapper)
{
    // Current function is a private, validity of workerInfo_ has been checked by caller.
    int32_t apiTargetVersion = static_cast<int32_t>(workerInfo_->apiTargetVersion.GetOriginPointer());
    bool apiSatisfy = apiTargetVersion == 0 || apiTargetVersion > API8;
    if (GetIsStageModel() && !isRestricted && apiSatisfy) {
        if (apiTargetVersion >= API12) {
            useSecureMem = true;
            return GetSafeData(ami, buff, buffSize, mapper);
        } else if (GetSafeData(ami, buff, buffSize, mapper)) {
            useSecureMem = true;
            return true;
        } else {
            // If api version less than 12 and get secure mem failed, try get normal mem.
            TAG_LOGW(AAFwkTag::JSRUNTIME, "file %{private}s", ami.c_str());
        }
    }

    char path[PATH_MAX];
    if (realpath(ami.c_str(), path) == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Realpath file %{private}s caught error: %{public}d", ami.c_str(), errno);
        return false;
    }

    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.is_open()) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "open file %{private}s failed", ami.c_str());
        return false;
    }

    auto fileLen = stream.tellg();
    if (!workerInfo_->isDebugVersion && fileLen > ASSET_FILE_MAX_SIZE) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "File too large");
        return false;
    }

    if (fileLen <= 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Invalid file length");
        return false;
    }

    content.resize(fileLen);
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(content.data()), content.size());
    return true;
}

bool AssetHelper::ReadFilePathData(const std::string& filePath, uint8_t** buff, size_t* buffSize,
    std::vector<uint8_t>& content, bool& useSecureMem, bool isRestricted, void** mapper)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null bundleMgrHelper");
        return false;
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto getInfoResult = bundleMgrHelper->GetBundleInfoForSelf(
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo);
    if (getInfoResult != 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "GetBundleInfoForSelf failed");
        return false;
    }
    if (bundleInfo.hapModuleInfos.size() == 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "Get hapModuleInfo of bundleInfo failed");
        return false;
    }

    std::string newHapPath;
    size_t pos = filePath.find('/');
    if (!GetIsStageModel()) {
        newHapPath = (workerInfo_->hapPath).GetOriginString();
    } else {
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.moduleName == filePath.substr(0, pos)) {
                newHapPath = hapModuleInfo.hapPath;
                break;
            }
        }
    }
    TAG_LOGD(AAFwkTag::JSRUNTIME, "HapPath: %{private}s", newHapPath.c_str());
    bool newCreate = false;
    std::string loadPath = ExtractorUtil::GetLoadFilePath(newHapPath);
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate);
    if (extractor == nullptr) {
        TAG_LOGW(AAFwkTag::JSRUNTIME, "LoadPath %{private}s GetExtractor failed", loadPath.c_str());
        return false;
    }
    std::unique_ptr<uint8_t[]> dataPtr = nullptr;
    std::string realfilePath;
    size_t fileLen = 0;
    if (!GetIsStageModel()) {
        bool flag = false;
        for (const auto& basePath : workerInfo_->assetBasePathStr) {
            realfilePath = basePath + filePath;
            TAG_LOGD(AAFwkTag::JSRUNTIME, "realfilePath: %{private}s", realfilePath.c_str());
            if (extractor->ExtractToBufByName(realfilePath, dataPtr, fileLen)) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "ExtractToBufByName error");
            return flag;
        }
    } else {
        realfilePath = filePath.substr(pos + 1);
        TAG_LOGD(AAFwkTag::JSRUNTIME, "realfilePath: %{private}s", realfilePath.c_str());
        int32_t apiTargetVersion = static_cast<int32_t>(workerInfo_->apiTargetVersion.GetOriginPointer());
        bool apiSatisfy = apiTargetVersion == 0 || apiTargetVersion > API8;
        if (GetIsStageModel() && !isRestricted && apiSatisfy && !extractor->IsHapCompress(realfilePath)) {
            TAG_LOGD(AAFwkTag::JSRUNTIME, "Use secure mem.");
            auto safeData = extractor->GetSafeData(realfilePath);
            if (apiTargetVersion >= API12) {
                useSecureMem = true;
                if (safeData == nullptr) {
                    TAG_LOGW(AAFwkTag::JSRUNTIME, "null safeData file %{private}s", filePath.c_str());
                    return false;
                }
                *buff = safeData->GetDataPtr();
                *buffSize = safeData->GetDataLen();
                *mapper = safeData.release();
                return true;
            } else if (safeData != nullptr) {
                useSecureMem = true;
                *buff = safeData->GetDataPtr();
                *buffSize = safeData->GetDataLen();
                *mapper = safeData.release();
                return true;
            } else {
                // If api version less than 12 and get secure mem failed, try get normal mem.
                TAG_LOGW(AAFwkTag::JSRUNTIME, "file %{private}s", filePath.c_str());
            }
        }
        if (!extractor->ExtractToBufByName(realfilePath, dataPtr, fileLen)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "get mergeAbc fileBuffer failed");
            return false;
        }
    }

    if (!workerInfo_->isDebugVersion && fileLen > ASSET_FILE_MAX_SIZE) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "file too large");
        return false;
    }

    content.assign(dataPtr.get(), dataPtr.get() + fileLen);
    return true;
}

void AssetHelper::GetAmi(std::string& ami, const std::string& filePath)
{
    size_t slashPos = filePath.find_last_of("/");
    std::string fileName = filePath.substr(slashPos + 1);
    std::string path = filePath.substr(0, slashPos + 1);

    std::string loadPath = ExtractorUtil::GetLoadFilePath((workerInfo_->hapPath).GetOriginString());
    bool newCreate = false;
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(loadPath, newCreate);
    if (extractor == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "loadPath %{private}s GetExtractor failed", loadPath.c_str());
        return;
    }
    std::vector<std::string> files;
    for (const auto& basePath : workerInfo_->assetBasePathStr) {
        std::string assetPath = basePath + path;
        TAG_LOGI(AAFwkTag::JSRUNTIME, "assetPath: %{private}s", assetPath.c_str());
        bool res = extractor->IsDirExist(assetPath);
        if (!res) {
            continue;
        }
        res = extractor->GetFileList(assetPath, files);
        if (!res) {
            continue;
        }
    }

    std::string targetFilePath;
    bool flag = false;
    for (const auto& file : files) {
        size_t filePos = file.find_last_of("/");
        if (filePos != std::string::npos) {
            if (file.substr(filePos + 1) == fileName) {
                targetFilePath = path + fileName;
                flag = true;
                break;
            }
        }
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "targetFilePath %{private}s", targetFilePath.c_str());

    if (!flag) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "get targetFilePath failed");
        return;
    }

    for (const auto& basePath : workerInfo_->assetBasePathStr) {
        std::string filePathName = basePath + targetFilePath;
        bool hasFile = extractor->HasEntry(filePathName);
        if (hasFile) {
            ami = (workerInfo_->hapPath).GetOriginString() + "/" + filePathName;
            return;
        }
    }
}

bool AssetHelper::GetIsStageModel()
{
    bool stageModule = workerInfo_->isStageModel.GetBool();
    TAG_LOGI(AAFwkTag::JSRUNTIME, "stageModule: %{public}d", stageModule);
    return stageModule;
}

int32_t GetContainerId()
{
#ifdef SUPPORT_SCREEN
    int32_t scopeId = ContainerScope::CurrentId();
    return scopeId;
#else
    constexpr int32_t containerScopeDefaultId = 0;
    return containerScopeDefaultId;
#endif
}
void UpdateContainerScope(int32_t id)
{
#ifdef SUPPORT_SCREEN
ContainerScope::UpdateCurrent(id);
#endif
}
void RestoreContainerScope(int32_t id)
{
#ifdef SUPPORT_SCREEN
ContainerScope::UpdateCurrent(-1);
#endif
}

void SetJsFramework()
{
    g_jsFramework = true;
}
} // namespace AbilityRuntime
} // namespace OHOS
