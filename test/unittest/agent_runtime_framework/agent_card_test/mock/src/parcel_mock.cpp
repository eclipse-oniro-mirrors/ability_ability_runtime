/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "parcel.h"

#include "parcel_mock.h"

namespace OHOS {
namespace {
size_t g_readableBytes = 0;
int32_t g_readInt32Value = 0;
}

namespace ParcelMock {
void Reset()
{
    g_readableBytes = 0;
    g_readInt32Value = 0;
}

void SetReadableBytes(size_t readableBytes)
{
    g_readableBytes = readableBytes;
}

void SetReadInt32Value(int32_t readInt32Value)
{
    g_readInt32Value = readInt32Value;
}
} // namespace ParcelMock

bool Parcel::WriteInt32(int32_t value)
{
    if (value == 0) {
        return false;
    }
    return true;
}

bool Parcel::WriteUint32(uint32_t value)
{
    if (value == 0) {
        return false;
    }
    return true;
}

bool Parcel::WriteString(const std::string& value)
{
    if (value == "test") {
        return false;
    }
    return true;
}

bool Parcel::WriteParcelable(const Parcelable* parcelable)
{
    if (parcelable == nullptr) {
        return false;
    }
    return true;
}

int32_t Parcel::ReadInt32()
{
    return g_readInt32Value;
}

uint32_t Parcel::ReadUint32()
{
    return 0;
}

const std::string Parcel::ReadString()
{
    return "test1";
}

size_t Parcel::GetReadableBytes() const
{
    return g_readableBytes;
}

bool Parcel::WriteBool(bool value)
{
    return value;
}

bool Parcel::WriteStringVector(const std::vector<std::string>& value)
{
    if (value.size() == 0) {
        return false;
    }
    return true;
}

bool Parcel::ReadBool()
{
    return true;
}

bool Parcel::ReadStringVector(std::vector<std::string>* value)
{
    if (value == nullptr || value->size() == 0) {
        return false;
    }
    return true;
}
} // namespace OHOS
