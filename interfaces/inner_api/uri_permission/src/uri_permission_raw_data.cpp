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

#include "ipc_types.h"
#include "securec.h"
#include "uri_permission_raw_data.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr uint32_t MAX_RAW_DATA_SIZE = 128 * 1024 * 1024; // 128M
}

int32_t UriPermissionRawData::RawDataCpy(const void* readdata)
{
    if (readdata == nullptr || size == 0 || size > MAX_RAW_DATA_SIZE) {
        return ERR_INVALID_DATA;
    }
    void* newData = malloc(size);
    if (newData == nullptr) {
        return ERR_INVALID_DATA;
    }
    if (memcpy_s(newData, size, readdata, size) != EOK) {
        free(newData);
        return ERR_INVALID_DATA;
    }
    if (data != nullptr && isMalloc) {
        free(const_cast<void*>(data));
    }
    data = newData;
    isMalloc = true;
    return ERR_NONE;
}

UriPermissionRawData::~UriPermissionRawData()
{
    if (data != nullptr && isMalloc) {
        free(const_cast<void*>(data));
        isMalloc = false;
        data = nullptr;
    }
}
} // namespace AAFwk
} // namespace OHOS