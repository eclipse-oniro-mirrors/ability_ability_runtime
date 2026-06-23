/*
* Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <limits>
#include "dataobs_mgr_changeinfo.h"
#include "dataobs_utils.h"
#include "datashare_log.h"
#include "securec.h"

namespace OHOS {
namespace AAFwk {
using namespace DataShare;
using Value = std::variant<std::monostate, int64_t, double, std::string, bool, std::vector<uint8_t>>;
using VBucket = std::map<std::string, Value>;
using VBuckets = std::vector<VBucket>;
bool ChangeInfo::Marshalling(const ChangeInfo &input, MessageParcel &parcel)
{
    if (!parcel.WriteUint32(static_cast<uint32_t>(input.changeType_))) {
        return false;
    }

    if (input.uris_.size() > std::numeric_limits<uint32_t>::max() ||
        !parcel.WriteUint32(static_cast<uint32_t>(input.uris_.size()))) {
        return false;
    }

    for (auto const &uri : input.uris_) {
        if (!parcel.WriteString(uri.ToString())) {
            return false;
        }
    }

    if (!parcel.WriteUint32(input.size_)) {
        return false;
    }

    if (!(input.size_ == 0 || parcel.WriteBuffer(input.data_, input.size_))) {
        return false;
    }

    if (!DataObsUtils::Marshal(parcel, input.valueBuckets_)) {
        return false;
    }
    return true;
}

bool ChangeInfo::Unmarshalling(ChangeInfo &output, MessageParcel &parcel)
{
    uint32_t changeType;
    if (!parcel.ReadUint32(changeType)) {
        LOG_ERROR("Failed to read changeType from parcel.");
        return false;
    }

    uint32_t len = 0;
    if (!parcel.ReadUint32(len)) {
        LOG_ERROR("Failed to read uris size from parcel.");
        return false;
    }
    if (len > LIST_MAX_COUNT) {
        LOG_ERROR("Uris size exceeds LIST_MAX_COUNT.");
        return false;
    }

    std::list<Uri> uris;
    for (uint32_t i = 0; i < len; i++) {
        Uri uri = Uri(parcel.ReadString());
        if (uri.ToString().empty()) {
            LOG_ERROR("The count:%{public}d uri is empty.", i);
            return false;
        }
        uris.emplace_back(std::move(uri));
    }

    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        LOG_ERROR("Failed to read size from parcel.");
        return false;
    }

    const uint8_t *data = (size > 0 && size <= ChangeInfo::MAX_DATA_SIZE) ? parcel.ReadBuffer(size) : nullptr;
    if (size > 0 && data == nullptr) {
        LOG_ERROR("Failed to read buffer from parcel. size: %{public}u", size);
        return false;
    }
    VBuckets buckets;
    if (!(DataObsUtils::Unmarshal(parcel, buckets))) {
        LOG_ERROR("Failed to unmarshall valueBuckets from parcel.");
        return false;
    }
    output.changeType_ = static_cast<ChangeType>(changeType);
    std::swap(output.uris_, uris);
    output.data_ = const_cast<uint8_t*>(data);
    output.size_ = size;
    output.valueBuckets_ = std::move(buckets);
    return true;
}
} // namespace AAFwk
} // namespace OHOS