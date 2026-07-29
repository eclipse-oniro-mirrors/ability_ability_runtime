/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "keep_alive_info.h"

#include "hilog_tag_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
/**
 * @brief Validate and convert int32_t value to enum type.
 * @tparam EnumType The enum type with UNSPECIFIED as lower bound and MAX as upper sentinel.
 * @param value The int32_t value to convert.
 * @param defaultValue The default value if validation fails.
 * @param fieldName The field name for error logging.
 * @return The validated enum value.
 */
template <typename EnumType>
EnumType ValidateEnum(int32_t value, EnumType defaultValue, const char *fieldName)
{
    if (value < static_cast<int32_t>(EnumType::UNSPECIFIED) ||
        value >= static_cast<int32_t>(EnumType::MAX)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "Invalid %{public}s: %{public}d", fieldName, value);
        return defaultValue;
    }
    return static_cast<EnumType>(value);
}
} // namespace
bool KeepAliveInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = Str16ToStr8(parcel.ReadString16());
    userId = parcel.ReadInt32();

    appType = ValidateEnum<KeepAliveAppType>(parcel.ReadInt32(),
        KeepAliveAppType::UNSPECIFIED, "appType");
    setter = ValidateEnum<KeepAliveSetter>(parcel.ReadInt32(),
        KeepAliveSetter::UNSPECIFIED, "setter");
    setterId = parcel.ReadInt32();
    policy = ValidateEnum<KeepAlivePolicy>(parcel.ReadInt32(),
        KeepAlivePolicy::UNSPECIFIED, "policy");

    return true;
}

KeepAliveInfo *KeepAliveInfo::Unmarshalling(Parcel &parcel)
{
    KeepAliveInfo *info = new (std::nothrow) KeepAliveInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "info null");
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool KeepAliveInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(bundleName))) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "failed to write bundleName");
        return false;
    }
    if (!parcel.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "failed to write userId");
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(appType))) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "failed to write appType");
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(setter))) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "failed to write setter");
        return false;
    }
    if (!parcel.WriteInt32(setterId)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "failed to write setterId");
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(policy))) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "failed to write policy");
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
