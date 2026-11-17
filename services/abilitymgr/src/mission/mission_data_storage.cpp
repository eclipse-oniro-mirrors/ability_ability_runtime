/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "mission_data_storage.h"
#include <cstdio>
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "image_packer.h"
#include "image_source.h"
#include "media_errors.h"
#include "mission_info_mgr.h"
#ifdef SUPPORT_GRAPHICS
#include <cstdio>
#include "securec.h"
#endif

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* IMAGE_FORMAT = "image/jpeg";
constexpr uint8_t IMAGE_QUALITY = 75;
}
#ifdef SUPPORT_GRAPHICS
constexpr int32_t RGB888_PIXEL_BYTES = 3;
const mode_t MODE = 0770;
#endif

MissionDataStorage::MissionDataStorage(int userId)
{
    userId_ = userId;
}

MissionDataStorage::~MissionDataStorage()
{}

bool MissionDataStorage::LoadAllMissionInfo(std::list<InnerMissionInfo> &missionInfoList)
{
    std::vector<std::string> fileNameVec;
    std::vector<int32_t> tempMissions;
    std::string dirPath = GetMissionDataDirPath();
    OHOS::GetDirFiles(dirPath, fileNameVec);

    for (auto fileName : fileNameVec) {
        if (!CheckFileNameValid(fileName)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "file name %{public}s invalid", fileName.c_str());
            continue;
        }

        std::string content;
        bool loadFile = OHOS::LoadStringFromFile(fileName, content);
        if (!loadFile) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load %{public}s fail", fileName.c_str());
            continue;
        }

        InnerMissionInfo misssionInfo;
        if (!misssionInfo.FromJsonStr(content)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "fail. file: %{public}s", fileName.c_str());
            continue;
        }
        if (misssionInfo.isTemporary) {
            tempMissions.push_back(misssionInfo.missionInfo.id);
            continue;
        }
        missionInfoList.push_back(misssionInfo);
    }

    for (auto missionId : tempMissions) {
        DeleteMissionInfo(missionId);
    }
    return true;
}

void MissionDataStorage::SaveMissionInfo(const InnerMissionInfo &missionInfo)
{
    std::string filePath = GetMissionDataFilePath(missionInfo.missionInfo.id);
    std::string dirPath = OHOS::ExtractFilePath(filePath);
    if (!OHOS::FileExists(dirPath)) {
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "create dir %{public}s fail", dirPath.c_str());
            return;
        }
#ifdef SUPPORT_GRAPHICS
        chmod(dirPath.c_str(), MODE);
#endif // SUPPORT_GRAPHICS
    }

    std::string jsonStr = missionInfo.ToJsonStr();
    bool saveMissionFile = OHOS::SaveStringToFile(filePath, jsonStr, true);
    if (!saveMissionFile) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "save %{public}s fail", filePath.c_str());
    }
}

void MissionDataStorage::DeleteMissionInfo(int missionId)
{
    std::string filePath = GetMissionDataFilePath(missionId);
    bool removeMissionFile = OHOS::RemoveFile(filePath);
    if (!removeMissionFile) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remove %{public}s fail", filePath.c_str());
        return;
    }
    DeleteMissionSnapshot(missionId);
}

void MissionDataStorage::SaveMissionSnapshot(int32_t missionId, const MissionSnapshot& missionSnapshot)
{
#ifdef SUPPORT_SCREEN
    TAG_LOGI(AAFwkTag::ABILITYMGR, "snapshot: save snapshot from cache, missionId = %{public}d", missionId);
    SaveCachedSnapshot(missionId, missionSnapshot);
    SaveSnapshotFile(missionId, missionSnapshot);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "delete snapshot from cache, missionId = %{public}d", missionId);
    DeleteCachedSnapshot(missionId);
#endif
}

void MissionDataStorage::DeleteMissionSnapshot(int32_t missionId)
{
#ifdef SUPPORT_SCREEN
    DeleteMissionSnapshot(missionId, false);
    DeleteMissionSnapshot(missionId, true);
#endif
}

bool MissionDataStorage::GetMissionSnapshot(int32_t missionId, MissionSnapshot& missionSnapshot, bool isLowResolution)
{
#ifdef SUPPORT_SCREEN
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (GetCachedSnapshot(missionId, missionSnapshot)) {
        if (isLowResolution) {
            missionSnapshot.snapshot = GetReducedPixelMap(missionSnapshot.snapshot);
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "GetMissionSnapshot from cache, missionId = %{public}d", missionId);
        return true;
    }

    auto pixelMap = GetPixelMap(missionId, isLowResolution);
    if (!pixelMap) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: GetPixelMap fail", __func__);
        return false;
    }
    missionSnapshot.snapshot = std::move(pixelMap);
#endif
    return true;
}

std::string MissionDataStorage::GetMissionDataDirPath() const
{
    return std::string(TASK_DATA_FILE_BASE_PATH) + "/" + std::to_string(userId_) + "/"
        + std::string(MISSION_DATA_FILE_PATH);
}

std::string MissionDataStorage::GetMissionDataFilePath(int missionId)
{
    return GetMissionDataDirPath() + "/"
        + MISSION_JSON_FILE_PREFIX + "_" + std::to_string(missionId) + JSON_FILE_SUFFIX;
}

std::string MissionDataStorage::GetMissionSnapshotPath(int32_t missionId, bool isLowResolution) const
{
    std::string filePath = GetMissionDataDirPath() + FILE_SEPARATOR + MISSION_JSON_FILE_PREFIX +
        UNDERLINE_SEPARATOR + std::to_string(missionId);
    if (isLowResolution) {
        filePath = filePath + UNDERLINE_SEPARATOR + LOW_RESOLUTION_FLAG;
    }
    filePath = filePath + JPEG_FILE_SUFFIX;
    return filePath;
}

bool MissionDataStorage::CheckFileNameValid(const std::string &fileName)
{
    std::string fileNameExcludePath = OHOS::ExtractFileName(fileName);
    if (fileNameExcludePath.find(MISSION_JSON_FILE_PREFIX) != 0) {
        return false;
    }

    if (fileNameExcludePath.find("_") != std::string(MISSION_JSON_FILE_PREFIX).length()) {
        return false;
    }

    if (fileNameExcludePath.find(JSON_FILE_SUFFIX) != fileNameExcludePath.length()
        - std::string(JSON_FILE_SUFFIX).length()) {
        return false;
    }

    size_t missionIdLength = fileNameExcludePath.find(JSON_FILE_SUFFIX) - fileNameExcludePath.find("_") - 1;
    std::string missionId = fileNameExcludePath.substr(fileNameExcludePath.find("_") + 1, missionIdLength);
    for (auto ch : missionId) {
        if (!isdigit(ch)) {
            return false;
        }
    }

    return true;
}

#ifdef SUPPORT_SCREEN
void MissionDataStorage::SaveSnapshotFile(int32_t missionId, const MissionSnapshot& missionSnapshot)
{
    SaveSnapshotFile(missionId, missionSnapshot.snapshot, missionSnapshot.isPrivate, false);
    SaveSnapshotFile(missionId, GetReducedPixelMap(missionSnapshot.snapshot), missionSnapshot.isPrivate, true);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->CompleteSaveSnapshot(missionId);
}

void MissionDataStorage::SaveSnapshotFile(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap>& snapshot,
    bool isPrivate, bool isLowResolution)
{
    if (!snapshot) {
        return;
    }

    std::string filePath = GetMissionSnapshotPath(missionId, isLowResolution);
    std::string dirPath = OHOS::ExtractFilePath(filePath);
    if (!OHOS::FileExists(dirPath)) {
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "create dir %{public}s fail", dirPath.c_str());
            return;
        }
        chmod(dirPath.c_str(), MODE);
    }

    if (isPrivate) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "snapshot: the param isPrivate is true.");
        int32_t width = snapshot->GetWidth();
        int32_t height = snapshot->GetHeight();
        if (width <= 0 || height <= 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid snapshot size, width: %{public}d, height: %{public}d",
                width, height);
            return;
        }
        size_t dataLength = static_cast<size_t>(width) * static_cast<size_t>(height) * RGB888_PIXEL_BYTES;
        uint8_t* data = (uint8_t*) malloc(dataLength);
        if (data == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "malloc fail");
            return;
        }
        if (memset_s(data, dataLength, 0xff, dataLength) == EOK) {
            Media::SourceOptions sourceOptions;
            uint32_t errCode = 0;
            auto imageSource = Media::ImageSource::CreateImageSource(data, dataLength, sourceOptions, errCode);
            if (errCode != OHOS::Media::SUCCESS || imageSource == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "CreateImageSource fail, errCode = %{public}d", errCode);
                free(data);
                return;
            }
            WriteToJpeg(filePath, *imageSource);
        }
        free(data);
    } else {
        WriteToJpeg(filePath, *snapshot);
    }
}

std::shared_ptr<OHOS::Media::PixelMap> MissionDataStorage::GetReducedPixelMap(
    const std::shared_ptr<OHOS::Media::PixelMap>& snapshot)
{
    if (!snapshot) {
        return nullptr;
    }

    OHOS::Media::InitializationOptions options;
    options.size.width = snapshot->GetWidth() / SCALE;
    options.size.height = snapshot->GetHeight() / SCALE;
    std::unique_ptr<OHOS::Media::PixelMap> reducedPixelMap = OHOS::Media::PixelMap::Create(*snapshot, options);
    return std::shared_ptr<OHOS::Media::PixelMap>(reducedPixelMap.release());
}

bool MissionDataStorage::GetCachedSnapshot(int32_t missionId, MissionSnapshot& missionSnapshot)
{
    std::lock_guard<ffrt::mutex> lock(cachedPixelMapMutex_);
    auto pixelMap = cachedPixelMap_.find(missionId);
    if (pixelMap != cachedPixelMap_.end()) {
        missionSnapshot.snapshot = pixelMap->second;
        return true;
    }
    return false;
}

bool MissionDataStorage::SaveCachedSnapshot(int32_t missionId, const MissionSnapshot& missionSnapshot)
{
    std::lock_guard<ffrt::mutex> lock(cachedPixelMapMutex_);
    auto result = cachedPixelMap_.insert_or_assign(missionId, missionSnapshot.snapshot);
    if (!result.second) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "save fail, missionId = %{public}d", missionId);
        return false;
    }
    return true;
}

bool MissionDataStorage::DeleteCachedSnapshot(int32_t missionId)
{
    std::lock_guard<ffrt::mutex> lock(cachedPixelMapMutex_);
    auto result = cachedPixelMap_.erase(missionId);
    if (result != 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "delete fail, missionId = %{public}d", missionId);
        return false;
    }
    return true;
}

void MissionDataStorage::DeleteMissionSnapshot(int32_t missionId, bool isLowResolution)
{
    std::string filePath = GetMissionSnapshotPath(missionId, isLowResolution);
    if (!OHOS::FileExists(filePath)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "fail, file: %{public}s",
            filePath.c_str());
        return;
    }
    bool removeResult = OHOS::RemoveFile(filePath);
    if (!removeResult) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remove %{public}s fail", filePath.c_str());
    }
}

std::shared_ptr<Media::PixelMap> MissionDataStorage::GetSnapshot(int missionId, bool isLowResolution) const
{
    auto pixelMapPtr = GetPixelMap(missionId, isLowResolution);
    if (!pixelMapPtr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: GetPixelMap fail", __func__);
        return nullptr;
    }
    return std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
}

std::unique_ptr<uint8_t[]> MissionDataStorage::ReadFileToBuffer(const std::string &filePath, size_t &bufferSize) const
{
    struct stat statbuf;
    int ret = stat(filePath.c_str(), &statbuf);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, ret:%{public}d", ret);
        return nullptr;
    }
    bufferSize = static_cast<size_t>(statbuf.st_size);
    std::string realPath;
    if (!OHOS::PathToRealPath(filePath, realPath)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PathToRealPath fail, file path=%{public}s",
            filePath.c_str());
        return nullptr;
    }

    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(bufferSize);
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "buffer null");
        return nullptr;
    }

    FILE *fp = fopen(realPath.c_str(), "rb");
    if (fp == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, real path=%{public}s", realPath.c_str());
        return nullptr;
    }
    fseek(fp, 0, SEEK_END);
    size_t fileSize = static_cast<size_t>(ftell(fp));
    fseek(fp, 0, SEEK_SET);
    if (bufferSize < fileSize) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "buffer size:(%{public}zu) is smaller than file size:(%{public}zu)", bufferSize,
            fileSize);
        fclose(fp);
        return nullptr;
    }
    size_t retSize = std::fread(buffer.get(), 1, fileSize, fp);
    if (retSize != fileSize) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read file result size = %{public}zu, size = %{public}zu",
            retSize, fileSize);
        fclose(fp);
        return nullptr;
    }
    fclose(fp);
    return buffer;
}

std::unique_ptr<Media::PixelMap> MissionDataStorage::GetPixelMap(int missionId, bool isLowResolution) const
{
    std::string filePath = GetMissionSnapshotPath(missionId, isLowResolution);
    if (!OHOS::FileExists(filePath)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "storage snapshot not exists, missionId = %{public}d", missionId);
        return nullptr;
    }
    uint32_t errCode = 0;
    
    size_t bufferSize = 0;
    const std::string fileName = filePath;
    std::unique_ptr<uint8_t[]> buffer = MissionDataStorage::ReadFileToBuffer(fileName, bufferSize);
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "buffer null");
        return nullptr;
    }
    Media::SourceOptions sourceOptions;
    auto imageSource = Media::ImageSource::CreateImageSource(buffer.get(), bufferSize, sourceOptions, errCode);
    if (errCode != OHOS::Media::SUCCESS || imageSource == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, null or errCode = %{public}d", errCode);
        return nullptr;
    }
    Media::DecodeOptions decodeOptions;
    decodeOptions.allocatorType = Media::AllocatorType::SHARE_MEM_ALLOC;
    auto pixelMapPtr = imageSource->CreatePixelMap(decodeOptions, errCode);
    if (errCode != OHOS::Media::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, errCode = %{public}d", errCode);
        return nullptr;
    }
    return pixelMapPtr;
}

template<typename T>
void MissionDataStorage::WriteToJpeg(const std::string &filePath, T &snapshot) const
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "file:%{public}s", filePath.c_str());
    OHOS::Media::PackOption option;
    option.format = IMAGE_FORMAT;
    option.quality = IMAGE_QUALITY;
    Media::ImagePacker imagePacker;
    uint32_t err = imagePacker.StartPacking(filePath, option);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail. %{public}d", err);
        return;
    }
    err = imagePacker.AddImage(snapshot);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail. %{public}d", err);
        return;
    }
    int64_t packedSize = 0;
    imagePacker.FinalizePacking(packedSize);
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
