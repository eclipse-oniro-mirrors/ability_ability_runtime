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

#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include <string>

#include "modular_object_dispatcher.h"

using namespace testing;
using namespace testing::ext;

namespace {

// Helper: create a simple TypeInfo for primitive types
OH_AbilityRuntime_MoDispatcher_TypeInfo MakeTypeInfo(OH_AbilityRuntime_MoDispatcher_ValueType vt)
{
    OH_AbilityRuntime_MoDispatcher_TypeInfo info;
    std::memset(&info, 0, sizeof(info));
    info.vt = vt;
    return info;
}

// Helper: create an i32 Variant
OH_AbilityRuntime_MoDispatcher_Variant MakeI32(int32_t val)
{
    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32;
    v.u.i32Val = val;
    return v;
}

// Helper: create a string Variant (caller owns the malloc'd string)
OH_AbilityRuntime_MoDispatcher_Variant MakeString(const char* val)
{
    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING;
    size_t len = strlen(val);
    char* buf = static_cast<char*>(std::malloc(len + 1));
    std::memcpy(buf, val, len + 1);
    v.u.bstrVal = buf;
    return v;
}

// Helper: create a bool Variant
OH_AbilityRuntime_MoDispatcher_Variant MakeBool(bool val)
{
    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL;
    v.u.boolVal = val;
    return v;
}

// Helper: create an i64 Variant
OH_AbilityRuntime_MoDispatcher_Variant MakeI64(int64_t val)
{
    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64;
    v.u.i64Val = val;
    return v;
}

// Helper: create an f64 Variant
OH_AbilityRuntime_MoDispatcher_Variant MakeF64(double val)
{
    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64;
    v.u.f64Val = val;
    return v;
}

// Helper: create an enum Variant
OH_AbilityRuntime_MoDispatcher_Variant MakeEnum(int32_t val)
{
    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM;
    v.u.enumVal = val;
    return v;
}

} // namespace

// ==================== Array Tests ====================

class MoDispatcherArrayTest : public ::testing::Test {
protected:
    void TearDown() override
    {
        if (array_ != nullptr) {
            OH_AbilityRuntime_MoDispatcher_Array_Release(&array_);
        }
    }
    OH_AbilityRuntime_MoDispatcher_ArrayHandle array_ = nullptr;
};

// Normal: create array, set/get elements
HWTEST_F(MoDispatcherArrayTest, CreateAndSetGet_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    auto ret = OH_AbilityRuntime_MoDispatcher_Array_Create(&typeInfo, 3, &array_);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(array_, nullptr);

    // Check size
    uint32_t size = 0;
    ret = OH_AbilityRuntime_MoDispatcher_Array_GetSize(array_, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 3u);

    // Set elements
    auto v0 = MakeI32(10);
    auto v1 = MakeI32(20);
    auto v2 = MakeI32(30);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_Set(array_, 0, &v0), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_Set(array_, 1, &v1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_Set(array_, 2, &v2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Get elements
    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    ret = OH_AbilityRuntime_MoDispatcher_Array_Get(array_, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.u.i32Val, 10);

    ret = OH_AbilityRuntime_MoDispatcher_Array_Get(array_, 2, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.u.i32Val, 30);
}

// Normal: get element type
HWTEST_F(MoDispatcherArrayTest, GetElementType_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    auto ret = OH_AbilityRuntime_MoDispatcher_Array_Create(&typeInfo, 2, &array_);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_MoDispatcher_TypeInfo elemType;
    std::memset(&elemType, 0, sizeof(elemType));
    ret = OH_AbilityRuntime_MoDispatcher_Array_GetElementType(array_, &elemType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(elemType.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
}

// Normal: resize array
HWTEST_F(MoDispatcherArrayTest, Resize_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    auto ret = OH_AbilityRuntime_MoDispatcher_Array_Create(&typeInfo, 2, &array_);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v = MakeI32(100);
    OH_AbilityRuntime_MoDispatcher_Array_Set(array_, 0, &v);

    ret = OH_AbilityRuntime_MoDispatcher_Array_Resize(array_, 5);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    OH_AbilityRuntime_MoDispatcher_Array_GetSize(array_, &size);
    EXPECT_EQ(size, 5u);

    // Original value preserved
    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Array_Get(array_, 0, &out);
    EXPECT_EQ(out.u.i32Val, 100);

    // New slot is default (i32 = 0)
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Array_Get(array_, 4, &out);
    EXPECT_EQ(out.u.i32Val, 0);
}

// Abnormal: null params
HWTEST_F(MoDispatcherArrayTest, NullParams_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    // null ppArray
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_Create(&typeInfo, 2, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    // null elementType
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_Create(nullptr, 2, &array_),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// Abnormal: out-of-bounds set/get
HWTEST_F(MoDispatcherArrayTest, OutOfBounds_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Array_Create(&typeInfo, 2, &array_);

    auto v = MakeI32(1);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_Set(array_, 5, &v),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_Get(array_, 5, &out),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// Abnormal: type mismatch on Set
HWTEST_F(MoDispatcherArrayTest, TypeMismatch_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Array_Create(&typeInfo, 2, &array_);

    auto strVar = MakeString("hello");
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_Set(array_, 0, &strVar),
        ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    std::free(strVar.u.bstrVal);
}

// Abnormal: null handle operations
HWTEST_F(MoDispatcherArrayTest, NullHandle_001, TestSize.Level1)
{
    uint32_t size = 99;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_GetSize(nullptr, &size),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_Resize(nullptr, 10),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_MoDispatcher_TypeInfo elemType;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Array_GetElementType(nullptr, &elemType),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// Normal: release null ppArray is safe
HWTEST_F(MoDispatcherArrayTest, ReleaseNull_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_Array_Release(nullptr);
    OH_AbilityRuntime_MoDispatcher_ArrayHandle nullHandle = nullptr;
    OH_AbilityRuntime_MoDispatcher_Array_Release(&nullHandle);
    // Should not crash
    SUCCEED();
}

// ==================== Vector Tests ====================

class MoDispatcherVectorTest : public ::testing::Test {
protected:
    void TearDown() override
    {
        if (vector_ != nullptr) {
            OH_AbilityRuntime_MoDispatcher_Vector_Release(&vector_);
        }
    }
    OH_AbilityRuntime_MoDispatcher_VectorHandle vector_ = nullptr;
};

HWTEST_F(MoDispatcherVectorTest, CreateAndAddGet_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    auto ret = OH_AbilityRuntime_MoDispatcher_Vector_Create(&typeInfo, &vector_);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(vector_, nullptr);

    auto v1 = MakeI32(100);
    auto v2 = MakeI32(200);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Vector_Add(vector_, &v1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Vector_Add(vector_, &v2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    OH_AbilityRuntime_MoDispatcher_Vector_GetSize(vector_, &size);
    EXPECT_EQ(size, 2u);

    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Vector_Get(vector_, 0, &out);
    EXPECT_EQ(out.u.i32Val, 100);
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Vector_Get(vector_, 1, &out);
    EXPECT_EQ(out.u.i32Val, 200);
}

HWTEST_F(MoDispatcherVectorTest, GetElementType_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING);
    auto ret = OH_AbilityRuntime_MoDispatcher_Vector_Create(&typeInfo, &vector_);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_MoDispatcher_TypeInfo elemType;
    std::memset(&elemType, 0, sizeof(elemType));
    ret = OH_AbilityRuntime_MoDispatcher_Vector_GetElementType(vector_, &elemType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(elemType.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING);
}

HWTEST_F(MoDispatcherVectorTest, Clear_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Vector_Create(&typeInfo, &vector_);

    auto v = MakeI32(1);
    OH_AbilityRuntime_MoDispatcher_Vector_Add(vector_, &v);

    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Vector_Clear(vector_), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 99;
    OH_AbilityRuntime_MoDispatcher_Vector_GetSize(vector_, &size);
    EXPECT_EQ(size, 0u);
}

HWTEST_F(MoDispatcherVectorTest, NullParams_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Vector_Create(&typeInfo, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Vector_Create(nullptr, &vector_),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Vector_Add(nullptr, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherVectorTest, TypeMismatch_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Vector_Create(&typeInfo, &vector_);

    auto strVar = MakeString("bad");
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Vector_Add(vector_, &strVar),
        ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    std::free(strVar.u.bstrVal);
}

HWTEST_F(MoDispatcherVectorTest, OutOfBounds_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Vector_Create(&typeInfo, &vector_);

    OH_AbilityRuntime_MoDispatcher_Variant out;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Vector_Get(vector_, 0, &out),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== Set Tests ====================

class MoDispatcherSetTest : public ::testing::Test {
protected:
    void TearDown() override
    {
        if (set_ != nullptr) {
            OH_AbilityRuntime_MoDispatcher_Set_Release(&set_);
        }
    }
    OH_AbilityRuntime_MoDispatcher_SetHandle set_ = nullptr;
};

HWTEST_F(MoDispatcherSetTest, AddRemoveContains_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    auto ret = OH_AbilityRuntime_MoDispatcher_Set_Create(&typeInfo, &set_);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32(10);
    auto v2 = MakeI32(20);
    auto v3 = MakeI32(10); // duplicate of v1

    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Add(set_, &v1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Add(set_, &v2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    // Duplicate add should succeed but not increase size
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Add(set_, &v3), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    OH_AbilityRuntime_MoDispatcher_Set_GetSize(set_, &size);
    EXPECT_EQ(size, 2u); // duplicate not added

    bool exists = false;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Contains(set_, &v1, &exists), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(exists);

    auto vNotFound = MakeI32(999);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Contains(set_, &vNotFound, &exists), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(exists);

    // Remove
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Remove(set_, &v1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    OH_AbilityRuntime_MoDispatcher_Set_GetSize(set_, &size);
    EXPECT_EQ(size, 1u);

    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Contains(set_, &v1, &exists), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(exists);
}

HWTEST_F(MoDispatcherSetTest, GetAt_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Set_Create(&typeInfo, &set_);

    auto v1 = MakeI32(10);
    auto v2 = MakeI32(20);
    OH_AbilityRuntime_MoDispatcher_Set_Add(set_, &v1);
    OH_AbilityRuntime_MoDispatcher_Set_Add(set_, &v2);

    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    auto ret = OH_AbilityRuntime_MoDispatcher_Set_GetAt(set_, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    // Value should be one of the two added
    EXPECT_TRUE(out.u.i32Val == 10 || out.u.i32Val == 20);
}

HWTEST_F(MoDispatcherSetTest, GetAt_OutOfBounds_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Set_Create(&typeInfo, &set_);

    OH_AbilityRuntime_MoDispatcher_Variant out;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_GetAt(set_, 0, &out),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherSetTest, Clear_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Set_Create(&typeInfo, &set_);

    auto v = MakeI32(1);
    OH_AbilityRuntime_MoDispatcher_Set_Add(set_, &v);

    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Clear(set_), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    uint32_t size = 99;
    OH_AbilityRuntime_MoDispatcher_Set_GetSize(set_, &size);
    EXPECT_EQ(size, 0u);
}

HWTEST_F(MoDispatcherSetTest, NullParams_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Create(&typeInfo, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Create(nullptr, &set_),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Contains(nullptr, nullptr, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherSetTest, TypeMismatch_001, TestSize.Level1)
{
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Set_Create(&typeInfo, &set_);

    auto strVar = MakeString("bad");
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Set_Add(set_, &strVar),
        ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    std::free(strVar.u.bstrVal);
}

// ==================== Map Tests ====================

class MoDispatcherMapTest : public ::testing::Test {
protected:
    void TearDown() override
    {
        if (map_ != nullptr) {
            OH_AbilityRuntime_MoDispatcher_Map_Release(&map_);
        }
    }
    OH_AbilityRuntime_MoDispatcher_MapHandle map_ = nullptr;
};

HWTEST_F(MoDispatcherMapTest, CreatePutGetRemove_001, TestSize.Level1)
{
    auto valueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING);
    auto ret = OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, &valueType, &map_);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(map_, nullptr);

    auto key1 = MakeI32(1);
    auto val1 = MakeString("one");
    auto key2 = MakeI32(2);
    auto val2 = MakeString("two");

    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Put(map_, &key1, &val1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Put(map_, &key2, &val2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    OH_AbilityRuntime_MoDispatcher_Map_GetSize(map_, &size);
    EXPECT_EQ(size, 2u);

    // Get by key
    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    ret = OH_AbilityRuntime_MoDispatcher_Map_Get(map_, &key1, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(out.u.bstrVal, "one");
    // Map_Get returns a pointer into internal storage, no need to free bstrVal

    // ContainsKey
    bool exists = false;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_ContainsKey(map_, &key1, &exists), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(exists);
    auto key3 = MakeI32(999);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_ContainsKey(map_, &key3, &exists), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(exists);

    // Remove
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Remove(map_, &key1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    OH_AbilityRuntime_MoDispatcher_Map_GetSize(map_, &size);
    EXPECT_EQ(size, 1u);

    // Get removed key -> error
    std::memset(&out, 0, sizeof(out));
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Get(map_, &key1, &out),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    std::free(val1.u.bstrVal);
    std::free(val2.u.bstrVal);
}

HWTEST_F(MoDispatcherMapTest, GetKeyAtGetValueAt_001, TestSize.Level1)
{
    auto valueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64);
    auto ret = OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, &valueType, &map_);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeI32(42);
    auto val = MakeI64(12345LL);
    OH_AbilityRuntime_MoDispatcher_Map_Put(map_, &key, &val);

    // GetKeyAt
    OH_AbilityRuntime_MoDispatcher_Variant outKey;
    std::memset(&outKey, 0, sizeof(outKey));
    ret = OH_AbilityRuntime_MoDispatcher_Map_GetKeyAt(map_, 0, &outKey);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(outKey.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    EXPECT_EQ(outKey.u.i32Val, 42);

    // GetValueAt
    OH_AbilityRuntime_MoDispatcher_Variant outVal;
    std::memset(&outVal, 0, sizeof(outVal));
    ret = OH_AbilityRuntime_MoDispatcher_Map_GetValueAt(map_, 0, &outVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(outVal.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64);
    EXPECT_EQ(outVal.u.i64Val, 12345LL);
}

HWTEST_F(MoDispatcherMapTest, GetKeyTypeGetValueType_001, TestSize.Level1)
{
    auto valueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64);
    OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING, &valueType, &map_);

    OH_AbilityRuntime_MoDispatcher_ValueType keyType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_GetKeyType(map_, &keyType), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(keyType, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING);

    OH_AbilityRuntime_MoDispatcher_TypeInfo valType;
    std::memset(&valType, 0, sizeof(valType));
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_GetValueType(map_, &valType), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(valType.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64);
}

HWTEST_F(MoDispatcherMapTest, Clear_001, TestSize.Level1)
{
    auto valueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, &valueType, &map_);

    auto key = MakeI32(1);
    auto val = MakeI32(100);
    OH_AbilityRuntime_MoDispatcher_Map_Put(map_, &key, &val);

    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Clear(map_), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    uint32_t size = 99;
    OH_AbilityRuntime_MoDispatcher_Map_GetSize(map_, &size);
    EXPECT_EQ(size, 0u);
}

HWTEST_F(MoDispatcherMapTest, NullParams_001, TestSize.Level1)
{
    auto valueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, &valueType, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, nullptr, &map_),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Put(nullptr, nullptr, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_GetKeyType(nullptr, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMapTest, KeyTypeMismatch_001, TestSize.Level1)
{
    auto valueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, &valueType, &map_);

    auto badKey = MakeString("not_i32");
    auto val = MakeI32(1);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Put(map_, &badKey, &val),
        ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    std::free(badKey.u.bstrVal);
}

HWTEST_F(MoDispatcherMapTest, ValueTypeMismatch_001, TestSize.Level1)
{
    auto valueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, &valueType, &map_);

    auto key = MakeI32(1);
    auto badVal = MakeString("not_i32");
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Map_Put(map_, &key, &badVal),
        ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    std::free(badVal.u.bstrVal);
}

HWTEST_F(MoDispatcherMapTest, PutOverwrite_001, TestSize.Level1)
{
    auto valueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, &valueType, &map_);

    auto key = MakeI32(1);
    auto val1 = MakeI32(100);
    auto val2 = MakeI32(200);
    OH_AbilityRuntime_MoDispatcher_Map_Put(map_, &key, &val1);
    OH_AbilityRuntime_MoDispatcher_Map_Put(map_, &key, &val2); // overwrite

    uint32_t size = 99;
    OH_AbilityRuntime_MoDispatcher_Map_GetSize(map_, &size);
    EXPECT_EQ(size, 1u); // still 1 entry

    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Map_Get(map_, &key, &out);
    EXPECT_EQ(out.u.i32Val, 200); // overwritten value
}

// ==================== Struct Tests ====================

class MoDispatcherStructTest : public ::testing::Test {
protected:
    void TearDown() override
    {
        if (struct_ != nullptr) {
            OH_AbilityRuntime_MoDispatcher_Struct_Release(&struct_);
        }
    }
    OH_AbilityRuntime_MoDispatcher_StructHandle struct_ = nullptr;
};

HWTEST_F(MoDispatcherStructTest, CreateSetGetField_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_MoDispatcher_Struct_Create("UserInfo", &struct_);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(struct_, nullptr);

    // Get name
    char name[64] = {0};
    ret = OH_AbilityRuntime_MoDispatcher_Struct_GetName(struct_, name, sizeof(name));
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(name, "UserInfo");

    // Set fields (struct without registered metadata accepts any field)
    auto idVal = MakeI32(42);
    ret = OH_AbilityRuntime_MoDispatcher_Struct_SetField(struct_, "id", &idVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto nameVal = MakeString("Alice");
    ret = OH_AbilityRuntime_MoDispatcher_Struct_SetField(struct_, "name", &nameVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    std::free(nameVal.u.bstrVal);

    // Get fields
    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    ret = OH_AbilityRuntime_MoDispatcher_Struct_GetField(struct_, "id", &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.u.i32Val, 42);

    std::memset(&out, 0, sizeof(out));
    ret = OH_AbilityRuntime_MoDispatcher_Struct_GetField(struct_, "name", &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(out.u.bstrVal, "Alice");
}

HWTEST_F(MoDispatcherStructTest, NullParams_001, TestSize.Level1)
{
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Struct_Create(nullptr, &struct_),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Struct_Create("Foo", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Struct_SetField(nullptr, "f", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Struct_GetField(nullptr, "f", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherStructTest, GetName_NullBuffer_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_Struct_Create("Test", &struct_);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Struct_GetName(struct_, nullptr, 64),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Struct_GetName(struct_, buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherStructTest, GetField_NotFound_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_Struct_Create("Test", &struct_);
    OH_AbilityRuntime_MoDispatcher_Variant out;
    // If no metadata registered, unknown fields are PROPERTY_NOT_FOUND
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_Struct_GetField(struct_, "nonexistent", &out),
        ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

// ==================== Variant_Clear Tests ====================

class MoDispatcherVariantClearTest : public ::testing::Test {};

HWTEST_F(MoDispatcherVariantClearTest, ClearString_001, TestSize.Level1)
{
    auto v = MakeString("hello");
    EXPECT_NE(v.u.bstrVal, nullptr);

    OH_AbilityRuntime_MoDispatcher_Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.bstrVal, nullptr);
}

HWTEST_F(MoDispatcherVariantClearTest, ClearArray_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_ArrayHandle arr = nullptr;
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Array_Create(&typeInfo, 3, &arr);

    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY;
    v.u.parrayVal = arr;

    OH_AbilityRuntime_MoDispatcher_Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.parrayVal, nullptr);
}

HWTEST_F(MoDispatcherVariantClearTest, ClearVector_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_VectorHandle vec = nullptr;
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Vector_Create(&typeInfo, &vec);

    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR;
    v.u.pvectorVal = vec;

    OH_AbilityRuntime_MoDispatcher_Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.pvectorVal, nullptr);
}

HWTEST_F(MoDispatcherVariantClearTest, ClearSet_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_SetHandle set = nullptr;
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Set_Create(&typeInfo, &set);

    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET;
    v.u.psetVal = set;

    OH_AbilityRuntime_MoDispatcher_Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.psetVal, nullptr);
}

HWTEST_F(MoDispatcherVariantClearTest, ClearMap_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_MapHandle map = nullptr;
    auto valueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Map_Create(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, &valueType, &map);

    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP;
    v.u.pmapVal = map;

    OH_AbilityRuntime_MoDispatcher_Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.pmapVal, nullptr);
}

HWTEST_F(MoDispatcherVariantClearTest, ClearStruct_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_StructHandle st = nullptr;
    OH_AbilityRuntime_MoDispatcher_Struct_Create("TestStruct", &st);

    OH_AbilityRuntime_MoDispatcher_Variant v;
    std::memset(&v, 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT;
    v.u.pstructVal = st;

    OH_AbilityRuntime_MoDispatcher_Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.pstructVal, nullptr);
}

HWTEST_F(MoDispatcherVariantClearTest, ClearPrimitive_NoOp_001, TestSize.Level1)
{
    auto v = MakeI32(42);
    OH_AbilityRuntime_MoDispatcher_Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY);
    // Should not crash
}

HWTEST_F(MoDispatcherVariantClearTest, ClearNull_NoOp_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_Variant_Clear(nullptr);
    SUCCEED();
}

// ==================== Dispatcher Null-param Tests ====================

class MoDispatcherNullTest : public ::testing::Test {};

HWTEST_F(MoDispatcherNullTest, CreateInstance_NullParams_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcherHandle disp = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_CreateInstance(nullptr, &disp),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_CreateInstance(nullptr, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherNullTest, Release_NullSafe_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_Release(nullptr);
    OH_AbilityRuntime_MoDispatcherHandle nullHandle = nullptr;
    OH_AbilityRuntime_MoDispatcher_Release(&nullHandle);
    SUCCEED();
}

HWTEST_F(MoDispatcherNullTest, HasTypeDescriptor_NullParams_001, TestSize.Level1)
{
    uint32_t pctInfo = 0;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor(nullptr, &pctInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor(nullptr, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherNullTest, GetTypeDescriptor_NullParams_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle td = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor(nullptr, &td),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor(nullptr, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherNullTest, QueryMemIDs_NullParams_001, TestSize.Level1)
{
    uint32_t memId = 0;
    const char* name = "test";
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_QueryMainServiceInterfaceMemIDsOfNames(
        nullptr, &name, 1, &memId), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_QueryMainServiceInterfaceMemIDsOfNames(
        nullptr, nullptr, 1, &memId), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherNullTest, CallMethod_NullParams_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_InputParams input;
    std::memset(&input, 0, sizeof(input));
    OH_AbilityRuntime_MoDispatcher_Variant result;
    std::memset(&result, 0, sizeof(result));
    EXPECT_EQ(OH_AbilityRuntime_MoDispatcher_CallMethod(nullptr, 1, &input, &result),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== TypeDescriptor Null-param Tests ====================

class TypeDescriptorNullTest : public ::testing::Test {};

HWTEST_F(TypeDescriptorNullTest, Release_NullSafe_001, TestSize.Level1)
{
    OH_AbilityRuntime_TypeDescriptor_Release(nullptr);
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle nullHandle = nullptr;
    OH_AbilityRuntime_TypeDescriptor_Release(&nullHandle);
    SUCCEED();
}

HWTEST_F(TypeDescriptorNullTest, GetInterfaceCount_NullParams_001, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(TypeDescriptorNullTest, GetEnumCount_NullParams_001, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumCount(nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(TypeDescriptorNullTest, GetStructCount_NullParams_001, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructCount(nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== Static Descriptor-based Method Tests ====================

class StaticDescriptorMethodTest : public ::testing::Test {};

HWTEST_F(StaticDescriptorMethodTest, GetMethodCount_NullParams_001, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodCount(nullptr, "IFoo", &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodCount("{}", nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodCount("{}", "IFoo", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodName_NullParams_001, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodName(nullptr, "IFoo", 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodName("{}", "IFoo", 0, nullptr, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodMemberId_NullParams_001, TestSize.Level1)
{
    uint32_t memId = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(nullptr, "IFoo", "bar", &memId),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodReturnType_NullParams_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_TypeInfo retType;
    std::memset(&retType, 0, sizeof(retType));
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(nullptr, "IFoo", "bar", &retType),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodParamCount_NullParams_001, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(nullptr, "IFoo", "bar", &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodParamType_NullParams_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_TypeInfo paramType;
    std::memset(&paramType, 0, sizeof(paramType));
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(nullptr, "IFoo", "bar", 0, &paramType),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodParamName_NullParams_001, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(nullptr, "IFoo", "bar", 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// Normal: valid descriptor JSON with methods
HWTEST_F(StaticDescriptorMethodTest, GetMethodCount_ValidDescriptor_001, TestSize.Level1)
{
    const char* descriptor = R"({"name":"ICalculator","methods":[{"name":"add","code":1,"dispID":10}]})";
    uint32_t count = 0;
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodCount(descriptor, "ICalculator", &count);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(count, 1u);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodName_ValidDescriptor_001, TestSize.Level1)
{
    const char* descriptor = R"({"name":"ICalculator","methods":[{"name":"add","code":1,"dispID":10}]})";
    char name[64] = {0};
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodName(descriptor, "ICalculator", 0, name, sizeof(name));
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(name, "add");
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodMemberId_ValidDescriptor_001, TestSize.Level1)
{
    const char* descriptor = R"({"name":"ICalculator","methods":[{"name":"add","code":1,"dispID":10}]})";
    uint32_t memId = 0;
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(descriptor, "ICalculator", "add", &memId);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(memId, 10u);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodName_OutOfRange_001, TestSize.Level1)
{
    const char* descriptor = R"({"name":"ICalculator","methods":[{"name":"add","code":1,"dispID":10}]})";
    char name[64] = {0};
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodName(descriptor, "ICalculator", 5, name, sizeof(name));
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodMemberId_NotFound_001, TestSize.Level1)
{
    const char* descriptor = R"({"name":"ICalculator","methods":[{"name":"add","code":1,"dispID":10}]})";
    uint32_t memId = 0;
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(descriptor, "ICalculator", "nonexistent", &memId);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodReturnType_ValidDescriptor_001, TestSize.Level1)
{
    const char* descriptor =
        R"({"name":"ICalculator","methods":[{"name":"add","code":1,"dispID":10,"return_type":{"type":"i32"}}]})";
    OH_AbilityRuntime_MoDispatcher_TypeInfo retType;
    std::memset(&retType, 0, sizeof(retType));
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(descriptor, "ICalculator", "add", &retType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(retType.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodReturnType_EnumType_001, TestSize.Level1)
{
    const char* descriptor =
        R"({"name":"ICalculator","methods":[{"name":"getStatus","code":2,"dispID":11,)"
        R"("return_type":{"type":"enum","idl_type":"StatusCode"}}]})";
    OH_AbilityRuntime_MoDispatcher_TypeInfo retType;
    std::memset(&retType, 0, sizeof(retType));
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(descriptor, "ICalculator", "getStatus", &retType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(retType.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM);
    EXPECT_NE(retType.u.idlType, nullptr);
    EXPECT_STREQ(retType.u.idlType, "StatusCode");
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodParamCount_ValidDescriptor_001, TestSize.Level1)
{
    const char* descriptor =
        R"({"name":"ICalculator","methods":[{"name":"add","code":1,"dispID":10,)"
        R"("parameters":[{"name":"a","type_info":{"type":"i32"}},{"name":"b","type_info":{"type":"i32"}}]})]})";
    uint32_t count = 0;
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(descriptor, "ICalculator", "add", &count);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(count, 2u);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodParamType_ValidDescriptor_001, TestSize.Level1)
{
    const char* descriptor =
        R"({"name":"ICalculator","methods":[{"name":"add","code":1,"dispID":10,)"
        R"("parameters":[{"name":"a","type_info":{"type":"i32"}},{"name":"b","type_info":{"type":"String"}}]})]})";
    OH_AbilityRuntime_MoDispatcher_TypeInfo paramType;
    std::memset(&paramType, 0, sizeof(paramType));
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
        descriptor, "ICalculator", "add", 0, &paramType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(paramType.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);

    std::memset(&paramType, 0, sizeof(paramType));
    ret = OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
        descriptor, "ICalculator", "add", 1, &paramType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(paramType.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING);
}

HWTEST_F(StaticDescriptorMethodTest, GetMethodParamName_ValidDescriptor_001, TestSize.Level1)
{
    const char* descriptor =
        R"({"name":"ICalculator","methods":[{"name":"add","code":1,"dispID":10,)"
        R"("parameters":[{"name":"a","type_info":{"type":"i32"}},{"name":"b","type_info":{"type":"i32"}}]})]})";
    char name[64] = {0};
    auto ret = OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
        descriptor, "ICalculator", "add", 0, name, sizeof(name));
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(name, "a");
}

// Invalid JSON
HWTEST_F(StaticDescriptorMethodTest, InvalidJSON_001, TestSize.Level1)
{
    const char* badJson = "not valid json";
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodCount(badJson, "IFoo", &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== String variant in containers ====================

class MoDispatcherStringContainerTest : public ::testing::Test {};

HWTEST_F(MoDispatcherStringContainerTest, ArrayOfString_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_ArrayHandle arr = nullptr;
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING);
    auto ret = OH_AbilityRuntime_MoDispatcher_Array_Create(&typeInfo, 2, &arr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v0 = MakeString("hello");
    auto v1 = MakeString("world");
    OH_AbilityRuntime_MoDispatcher_Array_Set(arr, 0, &v0);
    OH_AbilityRuntime_MoDispatcher_Array_Set(arr, 1, &v1);
    std::free(v0.u.bstrVal);
    std::free(v1.u.bstrVal);

    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Array_Get(arr, 0, &out);
    // Array_Get returns a pointer into internal storage
    EXPECT_STREQ(out.u.bstrVal, "hello");

    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Array_Get(arr, 1, &out);
    EXPECT_STREQ(out.u.bstrVal, "world");

    OH_AbilityRuntime_MoDispatcher_Array_Release(&arr);
}

HWTEST_F(MoDispatcherStringContainerTest, SetOfI32_GetAt_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_SetHandle set = nullptr;
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
    OH_AbilityRuntime_MoDispatcher_Set_Create(&typeInfo, &set);

    auto v1 = MakeI32(10);
    auto v2 = MakeI32(20);
    auto v3 = MakeI32(30);
    OH_AbilityRuntime_MoDispatcher_Set_Add(set, &v1);
    OH_AbilityRuntime_MoDispatcher_Set_Add(set, &v2);
    OH_AbilityRuntime_MoDispatcher_Set_Add(set, &v3);

    uint32_t size = 0;
    OH_AbilityRuntime_MoDispatcher_Set_GetSize(set, &size);
    EXPECT_EQ(size, 3u);

    // Iterate via GetAt
    for (uint32_t i = 0; i < size; i++) {
        OH_AbilityRuntime_MoDispatcher_Variant out;
        std::memset(&out, 0, sizeof(out));
        auto ret = OH_AbilityRuntime_MoDispatcher_Set_GetAt(set, i, &out);
        EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32);
        EXPECT_TRUE(out.u.i32Val == 10 || out.u.i32Val == 20 || out.u.i32Val == 30);
    }

    OH_AbilityRuntime_MoDispatcher_Set_Release(&set);
}

// ==================== Map with complex value type (Map<i32, Map<i32, i64>>) ====================

HWTEST_F(MoDispatcherStringContainerTest, NestedMapValue_001, TestSize.Level1)
{
    // Create inner map: Map<i32, i64>
    OH_AbilityRuntime_MoDispatcher_MapHandle innerMap = nullptr;
    auto innerValueType = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64);
    auto ret = OH_AbilityRuntime_MoDispatcher_Map_Create(
        OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32, &innerValueType, &innerMap);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto k = MakeI32(100);
    auto v = MakeI64(999LL);
    OH_AbilityRuntime_MoDispatcher_Map_Put(innerMap, &k, &v);

    // Verify inner map
    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Map_Get(innerMap, &k, &out);
    EXPECT_EQ(out.u.i64Val, 999LL);

    OH_AbilityRuntime_MoDispatcher_Map_Release(&innerMap);
}

// ==================== Bool and Float variants ====================

class MoDispatcherPrimitiveTypeTest : public ::testing::Test {};

HWTEST_F(MoDispatcherPrimitiveTypeTest, ArrayOfBool_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_ArrayHandle arr = nullptr;
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL);
    auto ret = OH_AbilityRuntime_MoDispatcher_Array_Create(&typeInfo, 3, &arr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v0 = MakeBool(true);
    auto v1 = MakeBool(false);
    auto v2 = MakeBool(true);
    OH_AbilityRuntime_MoDispatcher_Array_Set(arr, 0, &v0);
    OH_AbilityRuntime_MoDispatcher_Array_Set(arr, 1, &v1);
    OH_AbilityRuntime_MoDispatcher_Array_Set(arr, 2, &v2);

    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Array_Get(arr, 0, &out);
    EXPECT_EQ(out.u.boolVal, true);
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Array_Get(arr, 1, &out);
    EXPECT_EQ(out.u.boolVal, false);

    OH_AbilityRuntime_MoDispatcher_Array_Release(&arr);
}

HWTEST_F(MoDispatcherPrimitiveTypeTest, VectorOfF64_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_VectorHandle vec = nullptr;
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64);
    auto ret = OH_AbilityRuntime_MoDispatcher_Vector_Create(&typeInfo, &vec);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeF64(3.14);
    auto v2 = MakeF64(2.718);
    OH_AbilityRuntime_MoDispatcher_Vector_Add(vec, &v1);
    OH_AbilityRuntime_MoDispatcher_Vector_Add(vec, &v2);

    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Vector_Get(vec, 0, &out);
    EXPECT_DOUBLE_EQ(out.u.f64Val, 3.14);

    OH_AbilityRuntime_MoDispatcher_Vector_Release(&vec);
}

// ==================== Enum variant in container ====================

HWTEST_F(MoDispatcherPrimitiveTypeTest, VectorOfEnum_001, TestSize.Level1)
{
    OH_AbilityRuntime_MoDispatcher_VectorHandle vec = nullptr;
    auto typeInfo = MakeTypeInfo(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM);
    auto ret = OH_AbilityRuntime_MoDispatcher_Vector_Create(&typeInfo, &vec);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeEnum(0); // Success
    auto v2 = MakeEnum(1); // InvalidParam
    OH_AbilityRuntime_MoDispatcher_Vector_Add(vec, &v1);
    OH_AbilityRuntime_MoDispatcher_Vector_Add(vec, &v2);

    uint32_t size = 0;
    OH_AbilityRuntime_MoDispatcher_Vector_GetSize(vec, &size);
    EXPECT_EQ(size, 2u);

    OH_AbilityRuntime_MoDispatcher_Variant out;
    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Vector_Get(vec, 0, &out);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM);
    EXPECT_EQ(out.u.enumVal, 0);

    std::memset(&out, 0, sizeof(out));
    OH_AbilityRuntime_MoDispatcher_Vector_Get(vec, 1, &out);
    EXPECT_EQ(out.u.enumVal, 1);

    OH_AbilityRuntime_MoDispatcher_Vector_Release(&vec);
}

// ==================== Struct with registered metadata ====================

HWTEST_F(MoDispatcherPrimitiveTypeTest, StructWithMetadata_001, TestSize.Level1)
{
    // Create struct using the registered name from tlb.json
    OH_AbilityRuntime_MoDispatcher_StructHandle st = nullptr;
    auto ret = OH_AbilityRuntime_MoDispatcher_Struct_Create("UserInfo", &st);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(st, nullptr);

    char name[64] = {0};
    OH_AbilityRuntime_MoDispatcher_Struct_GetName(st, name, sizeof(name));
    EXPECT_STREQ(name, "UserInfo");

    OH_AbilityRuntime_MoDispatcher_Struct_Release(&st);
}
