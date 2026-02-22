/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "hisysevent_report.h"

#include <cstring>
#include <cstdlib>
#include <memory>
#include "securec.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
void HisyseventReport::InsertParam(const char* name, bool value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_BOOL,
        .v = { .b = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, int8_t value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_INT8,
        .v = { .i8 = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, uint8_t value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_UINT8,
        .v = { .ui8 = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, int16_t value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_INT16,
        .v = { .i16 = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, uint16_t value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_UINT16,
        .v = { .ui16 = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, int32_t value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_INT32,
        .v = { .i32 = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, uint32_t value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_UINT32,
        .v = { .ui32 = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, int64_t value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_INT64,
        .v = { .i64 = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, uint64_t value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_UINT64,
        .v = { .ui64 = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, float value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_FLOAT,
        .v = { .f = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, double value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_DOUBLE,
        .v = { .d = value},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, char* value)
{
    if (!name || !value || length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    int32_t len = std::strlen(value) + 1;
    auto buffer = std::make_unique<char[]>(len);
    int32_t ret = strcpy_s(buffer.get(), len, value);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "InsertParam err %{public}d", ret);
        return;
    }

    HiSysEventParam param = {
        .t = HISYSEVENT_STRING,
        .v = { .s = buffer.get()},
        .arraySize = 0,
    };
    SetParamName(param, name);
    params_[pos_++] = param;

    paramBuffers_.emplace_back(std::move(buffer));
}

void HisyseventReport::InsertParam(const char* name, const std::vector<int32_t> &value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_INT32_ARRAY,
        .v = { .array = nullptr},
        .arraySize = 0,
    };

    if (!value.empty()) {
        param.v.array = static_cast<void*>(const_cast<int32_t*>(value.data()));
        param.arraySize = value.size();
    }
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, const std::vector<uint64_t> &value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_UINT64_ARRAY,
        .v = { .array = nullptr},
        .arraySize = 0,
    };

    if (!value.empty()) {
        param.v.array = static_cast<void*>(const_cast<uint64_t*>(value.data()));
        param.arraySize = value.size();
    }
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, std::vector<char*> &value)
{
    if (length_ <= pos_) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return;
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_STRING_ARRAY,
        .v = { .array = nullptr},
        .arraySize = 0,
    };

    if (!value.empty()) {
        param.v.array = static_cast<void*>(value.data());
        param.arraySize = value.size();
    }
    SetParamName(param, name);
    params_[pos_++] = param;
}

void HisyseventReport::InsertParam(const char* name, std::string value)
{
    this->InsertParam(name, const_cast<char *>(value.c_str()));
}

void HisyseventReport::InsertParam(const char* name, const char* value)
{
    this->InsertParam(name, const_cast<char *>(value));
}

void HisyseventReport::SetParamName(HiSysEventParam& param, const char* name)
{
    int32_t ret = strcpy_s(param.name, sizeof(param.name), name);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "SetParamName err %{public}d", ret);
    }
}

int32_t HisyseventReport::Report(const char* domain, const char* event, HiSysEventEventType type)
{
    if (params_ == nullptr || length_ == 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
        return -1;
    }
    return OH_HiSysEvent_Write(domain, event, type, params_, pos_);
}
}
}