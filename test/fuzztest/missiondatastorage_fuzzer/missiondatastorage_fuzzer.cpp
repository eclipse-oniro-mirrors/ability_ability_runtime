/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "missiondatastorage_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "mission_data_storage.h"
#undef private

#include "securec.h"
#include "ability_record.h"
#ifdef SUPPORT_SCREEN
#include "pixel_map.h"
#endif //SUPPORT_SCREEN

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr uint8_t ENABLE = 2;
} // namespace

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = int32Param;
    std::list<InnerMissionInfo> missionInfoList;
    missionDataStorage->SaveMissionInfo(missionInfo);
    missionDataStorage->LoadAllMissionInfo(missionInfoList);
    missionDataStorage->DeleteMissionInfo(intParam);
    missionDataStorage->DeleteMissionInfo(intParam + 1);
    MissionSnapshot missionSnapshot;
    missionDataStorage->SaveMissionSnapshot(int32Param, missionSnapshot);
    missionDataStorage->DeleteMissionSnapshot(int32Param);
    bool boolParam = *data % ENABLE;
    missionDataStorage->GetMissionSnapshot(int32Param, missionSnapshot, boolParam);
    missionDataStorage->GetMissionSnapshotPath(int32Param, boolParam);

#ifdef SUPPORT_SCREEN
    std::shared_ptr<OHOS::Media::PixelMap> snapshot = nullptr;
    missionDataStorage->GetReducedPixelMap(snapshot);
    snapshot = std::make_shared<Media::PixelMap>();
    missionDataStorage->GetReducedPixelMap(snapshot);
    missionDataStorage->GetSnapshot(int32Param, boolParam);
    missionDataStorage->GetPixelMap(int32Param, boolParam);
    std::string stringParam("mission_788529156.json");
    size_t bufferSize;
    missionDataStorage->ReadFileToBuffer(stringParam, bufferSize);
    missionDataStorage->GetCachedSnapshot(int32Param, missionSnapshot);
    missionDataStorage->SaveSnapshotFile(int32Param, missionSnapshot);
    missionSnapshot.snapshot = std::make_shared<Media::PixelMap>();
    missionDataStorage->SaveCachedSnapshot(int32Param, missionSnapshot);
    missionDataStorage->DeleteCachedSnapshot(int32Param);
    missionDataStorage->DeleteMissionSnapshot(int32Param);
    missionDataStorage->SaveSnapshotFile(int32Param, missionSnapshot);
#endif

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}