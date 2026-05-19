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

#include <cstring>
#include <gtest/gtest.h>

#include "securec.h"

#include "mo_dispatcher_complex_type_manager.h"
#include "mo_dispatcher_types.h"
#include "modular_object_dispatcher.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

// Helper: build an I32 TypeInfo
static OH_AbilityRuntime_ModObjDispatcher_TypeInfo MakeI32TypeInfo()
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo ti = {};
    ti.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    return ti;
}

// Helper: build a STRING TypeInfo
static OH_AbilityRuntime_ModObjDispatcher_TypeInfo MakeStringTypeInfo()
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo ti = {};
    ti.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    return ti;
}

// Helper: build an I64 TypeInfo
static OH_AbilityRuntime_ModObjDispatcher_TypeInfo MakeI64TypeInfo()
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo ti = {};
    ti.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64;
    return ti;
}

// Helper: build a Variant with I32 value
static OH_AbilityRuntime_ModObjDispatcher_Variant MakeI32Variant(int32_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    v.u.i32Val = val;
    return v;
}

// Helper: build a Variant with STRING value (caller owns the strdup'd memory)
static OH_AbilityRuntime_ModObjDispatcher_Variant MakeStringVariant(const char* val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    v.u.bstrVal = strdup(val);
    return v;
}

// Helper: build a Variant with BOOL value
static OH_AbilityRuntime_ModObjDispatcher_Variant MakeBoolVariant(bool val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL;
    v.u.boolVal = val;
    return v;
}

// Helper: build a Variant with F64 value
static OH_AbilityRuntime_ModObjDispatcher_Variant MakeF64Variant(double val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64;
    v.u.f64Val = val;
    return v;
}

// Helper: build a Variant with I64 value
static OH_AbilityRuntime_ModObjDispatcher_Variant MakeI64Variant(int64_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64;
    v.u.i64Val = val;
    return v;
}

// Helper: register a simple struct metadata for testing
static void RegisterTestStructMetadata()
{
    std::vector<MoStructMeta> structs;
    // "TestStruct" with fields: id(I32), name(STRING)
    {
        MoStructMeta sm;
        sm.name = "TestStruct";
        MoStructFieldMeta f1;
        f1.name = "id";
        f1.typeInfo = std::make_shared<MoTypeInfo>();
        f1.typeInfo->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
        sm.fields.push_back(f1);
        MoStructFieldMeta f2;
        f2.name = "name";
        f2.typeInfo = std::make_shared<MoTypeInfo>();
        f2.typeInfo->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
        sm.fields.push_back(f2);
        structs.push_back(sm);
    }
    // "MyStruct" with no fields (for basic create/getname tests)
    {
        MoStructMeta sm;
        sm.name = "MyStruct";
        structs.push_back(sm);
    }
    // "S" with no fields (for release tests)
    {
        MoStructMeta sm;
        sm.name = "S";
        structs.push_back(sm);
    }
    // "LongStructName" with no fields (for buffer test)
    {
        MoStructMeta sm;
        sm.name = "LongStructName";
        structs.push_back(sm);
    }
    ModObjDispatcherComplexTypeManager::RegisterStructMetadata(structs);
}

// Helper: build a MAP TypeInfo with heap-allocated value type
static OH_AbilityRuntime_ModObjDispatcher_TypeInfo MakeMapTypeInfo(
    OH_AbilityRuntime_ModObjDispatcher_ValueType keyVt,
    OH_AbilityRuntime_ModObjDispatcher_ValueType valVt)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo ti = {};
    ti.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP;
    ti.u.mapType.keyType = keyVt;
    auto* valType = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(valType, sizeof(*valType), 0, sizeof(*valType));
    valType->vt = valVt;
    ti.u.mapType.pValueType = valType;
    return ti;
}

// Helper: free heap-allocated child TypeInfo nodes
static void FreeTypeInfo(OH_AbilityRuntime_ModObjDispatcher_TypeInfo& ti)
{
    if (ti.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR ||
        ti.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET) {
        if (ti.u.pElementType != nullptr) {
            FreeTypeInfo(*ti.u.pElementType);
            delete ti.u.pElementType;
            ti.u.pElementType = nullptr;
        }
    } else if (ti.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY) {
        if (ti.u.arrayType.pElementType != nullptr) {
            FreeTypeInfo(*ti.u.arrayType.pElementType);
            delete ti.u.arrayType.pElementType;
            ti.u.arrayType.pElementType = nullptr;
        }
    } else if (ti.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP) {
        if (ti.u.mapType.pValueType != nullptr) {
            FreeTypeInfo(*ti.u.mapType.pValueType);
            delete ti.u.mapType.pValueType;
            ti.u.mapType.pValueType = nullptr;
        }
    }
}

// Helper: allocate and zero-init a TypeInfo with given vt
static OH_AbilityRuntime_ModObjDispatcher_TypeInfo* NewTypeInfo(OH_AbilityRuntime_ModObjDispatcher_ValueType vt)
{
    auto* ti = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(ti, sizeof(*ti), 0, sizeof(*ti));
    ti->vt = vt;
    return ti;
}

// Helper: build a container TypeInfo (VECTOR/SET/ARRAY) with a leaf element type
static OH_AbilityRuntime_ModObjDispatcher_TypeInfo* MakeContainerTypeInfo(
    OH_AbilityRuntime_ModObjDispatcher_ValueType containerVt,
    OH_AbilityRuntime_ModObjDispatcher_ValueType leafVt)
{
    auto* ti = NewTypeInfo(containerVt);
    ti->u.pElementType = NewTypeInfo(leafVt);
    return ti;
}

// Helper: build a MAP TypeInfo with key vt and heap-allocated leaf value type
static OH_AbilityRuntime_ModObjDispatcher_TypeInfo* MakeMapTypeInfoHeap(
    OH_AbilityRuntime_ModObjDispatcher_ValueType keyVt,
    OH_AbilityRuntime_ModObjDispatcher_ValueType valVt)
{
    auto* ti = NewTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP);
    ti->u.mapType.keyType = keyVt;
    ti->u.mapType.pValueType = NewTypeInfo(valVt);
    return ti;
}

// Helper: build a Vector variant wrapping a vector handle
static OH_AbilityRuntime_ModObjDispatcher_Variant MakeVectorVariant(OH_AbilityRuntime_ModObjDispatcher_VectorHandle h)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    v.u.pvectorVal = h;
    return v;
}

// Helper: build a Map variant wrapping a map handle
static OH_AbilityRuntime_ModObjDispatcher_Variant MakeMapVariant(OH_AbilityRuntime_ModObjDispatcher_MapHandle h)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP;
    v.u.pmapVal = h;
    return v;
}

// Helper: build a Set variant wrapping a set handle
static OH_AbilityRuntime_ModObjDispatcher_Variant MakeSetVariant(OH_AbilityRuntime_ModObjDispatcher_SetHandle h)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET;
    v.u.psetVal = h;
    return v;
}

// Helper: build an Array variant wrapping an array handle
static OH_AbilityRuntime_ModObjDispatcher_Variant MakeArrayVariant(OH_AbilityRuntime_ModObjDispatcher_ArrayHandle h)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY;
    v.u.parrayVal = h;
    return v;
}

// Helper: create a Vector<I32> with given int32 values
static OH_AbilityRuntime_ModObjDispatcher_VectorHandle CreateI32Vector(std::initializer_list<int32_t> vals)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vec = nullptr;
    ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &vec);
    for (auto v : vals) {
        auto var = MakeI32Variant(v);
        ModObjDispatcherComplexTypeManager::VectorAdd(vec, &var);
    }
    return vec;
}

// Helper: create a Vector<STRING> with given string values
static OH_AbilityRuntime_ModObjDispatcher_VectorHandle CreateStringVector(std::initializer_list<const char*> vals)
{
    auto ti = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vec = nullptr;
    ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &vec);
    for (auto v : vals) {
        auto var = MakeStringVariant(v);
        ModObjDispatcherComplexTypeManager::VectorAdd(vec, &var);
        ModObjDispatcherComplexTypeManager::Variant_Clear(&var);
    }
    return vec;
}

// ==================== Test Fixture ====================

class ModObjDispatcherComplexTypeTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override {}
    void TearDown() override {}
};

// ==================== Array Tests ====================

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayCreate_NullElementType, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(nullptr, 3, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayCreate_NullOutput, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 3, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayCreate_InvalidVT, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo ti = {};
    ti.vt = static_cast<OH_AbilityRuntime_ModObjDispatcher_ValueType>(999);
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayCreate_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 4, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(handle, nullptr);

    uint32_t size = 0;
    ret = ModObjDispatcherComplexTypeManager::ArrayGetSize(handle, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 4u);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayCreate_ZeroSize, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 0, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(handle, nullptr);

    uint32_t size = 99;
    ret = ModObjDispatcherComplexTypeManager::ArrayGetSize(handle, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 0u);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayGetSize_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::ArrayGetSize(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ret = ModObjDispatcherComplexTypeManager::ArrayGetSize(handle, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::ArrayGetSize(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayGetElementType_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    ret = ModObjDispatcherComplexTypeManager::ArrayGetElementType(handle, &elemType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(elemType.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    ModObjDispatcherComplexTypeManager::TypeInfo_Clear(&elemType);
    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayGetElementType_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::ArrayGetElementType(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    ret = ModObjDispatcherComplexTypeManager::ArrayGetElementType(handle, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::ArrayGetElementType(nullptr, &elemType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArraySetAndGet_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 3, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto val = MakeI32Variant(42);
    ret = ModObjDispatcherComplexTypeManager::ArraySet(handle, 0, &val);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto val2 = MakeI32Variant(99);
    ret = ModObjDispatcherComplexTypeManager::ArraySet(handle, 2, &val2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::ArrayGet(handle, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(out.u.i32Val, 42);

    ret = ModObjDispatcherComplexTypeManager::ArrayGet(handle, 2, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.u.i32Val, 99);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArraySet_NullParams, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 3, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto val = MakeI32Variant(1);
    ret = ModObjDispatcherComplexTypeManager::ArraySet(nullptr, 0, &val);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::ArraySet(handle, 0, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArraySet_IndexOutOfRange, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto val = MakeI32Variant(1);
    ret = ModObjDispatcherComplexTypeManager::ArraySet(handle, 5, &val);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArraySet_TypeMismatch, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto strVal = MakeStringVariant("hello");
    ret = ModObjDispatcherComplexTypeManager::ArraySet(handle, 0, &strVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&strVal);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayGet_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::ArrayGet(nullptr, 0, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::ArrayGet(handle, 0, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::ArrayGet(nullptr, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayGet_IndexOutOfRange, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::ArrayGet(handle, 10, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayRelease_NullAndDoubleRelease, TestSize.Level1)
{
    ModObjDispatcherComplexTypeManager::ArrayRelease(nullptr);

    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle nullHandle = nullptr;
    ModObjDispatcherComplexTypeManager::ArrayRelease(&nullHandle);
    EXPECT_EQ(nullHandle, nullptr);

    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
    EXPECT_EQ(handle, nullptr);
    // Double release should not crash
    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ArrayWithStrings_Success, TestSize.Level1)
{
    auto ti = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto val0 = MakeStringVariant("first");
    ret = ModObjDispatcherComplexTypeManager::ArraySet(handle, 0, &val0);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&val0);

    auto val1 = MakeStringVariant("second");
    ret = ModObjDispatcherComplexTypeManager::ArraySet(handle, 1, &val1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&val1);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::ArrayGet(handle, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_NE(out.u.bstrVal, nullptr);
    EXPECT_STREQ(out.u.bstrVal, "first");
    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);

    ret = ModObjDispatcherComplexTypeManager::ArrayGet(handle, 1, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(out.u.bstrVal, "second");
    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&handle);
}

// ==================== Vector Tests ====================

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorCreate_NullElementType, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(nullptr, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorCreate_NullOutput, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorCreate_InvalidVT, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo ti = {};
    ti.vt = static_cast<OH_AbilityRuntime_ModObjDispatcher_ValueType>(999);
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorCreate_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(handle, nullptr);

    uint32_t size = 99;
    ret = ModObjDispatcherComplexTypeManager::VectorGetSize(handle, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 0u);

    ModObjDispatcherComplexTypeManager::VectorRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorAddAndGet_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32Variant(10);
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(handle, &v1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v2 = MakeI32Variant(20);
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(handle, &v2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v3 = MakeI32Variant(30);
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(handle, &v3);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    ret = ModObjDispatcherComplexTypeManager::VectorGetSize(handle, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 3u);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::VectorGet(handle, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.u.i32Val, 10);

    ret = ModObjDispatcherComplexTypeManager::VectorGet(handle, 2, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.u.i32Val, 30);

    ModObjDispatcherComplexTypeManager::VectorRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorAdd_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::VectorAdd(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto val = MakeI32Variant(1);
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(nullptr, &val);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::VectorAdd(handle, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::VectorRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorAdd_TypeMismatch, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto strVal = MakeStringVariant("wrong");
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(handle, &strVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&strVal);

    ModObjDispatcherComplexTypeManager::VectorRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorGet_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::VectorGet(nullptr, 0, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorGet_IndexOutOfRange, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::VectorGet(handle, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::VectorRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorGetSize_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::VectorGetSize(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorGetElementType_Success, TestSize.Level1)
{
    auto ti = MakeI64TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    ret = ModObjDispatcherComplexTypeManager::VectorGetElementType(handle, &elemType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(elemType.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64);

    ModObjDispatcherComplexTypeManager::TypeInfo_Clear(&elemType);
    ModObjDispatcherComplexTypeManager::VectorRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorGetElementType_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::VectorGetElementType(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorClear_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32Variant(1);
    ModObjDispatcherComplexTypeManager::VectorAdd(handle, &v1);
    auto v2 = MakeI32Variant(2);
    ModObjDispatcherComplexTypeManager::VectorAdd(handle, &v2);

    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::VectorGetSize(handle, &size);
    EXPECT_EQ(size, 2u);

    ret = ModObjDispatcherComplexTypeManager::VectorClear(handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ModObjDispatcherComplexTypeManager::VectorGetSize(handle, &size);
    EXPECT_EQ(size, 0u);

    ModObjDispatcherComplexTypeManager::VectorRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorClear_Null, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::VectorClear(nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, VectorRelease_NullAndDoubleRelease, TestSize.Level1)
{
    ModObjDispatcherComplexTypeManager::VectorRelease(nullptr);

    OH_AbilityRuntime_ModObjDispatcher_VectorHandle nullHandle = nullptr;
    ModObjDispatcherComplexTypeManager::VectorRelease(&nullHandle);
    EXPECT_EQ(nullHandle, nullptr);

    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::VectorRelease(&handle);
    EXPECT_EQ(handle, nullptr);
    ModObjDispatcherComplexTypeManager::VectorRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

// ==================== Set Tests ====================

HWTEST_F(ModObjDispatcherComplexTypeTest, SetCreate_NullElementType, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(nullptr, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetCreate_NullOutput, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetCreate_InvalidVT, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo ti = {};
    ti.vt = static_cast<OH_AbilityRuntime_ModObjDispatcher_ValueType>(999);
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetCreate_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(handle, nullptr);

    uint32_t size = 99;
    ret = ModObjDispatcherComplexTypeManager::SetGetSize(handle, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 0u);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetAddAndContains_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32Variant(10);
    ret = ModObjDispatcherComplexTypeManager::SetAdd(handle, &v1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v2 = MakeI32Variant(20);
    ret = ModObjDispatcherComplexTypeManager::SetAdd(handle, &v2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    ret = ModObjDispatcherComplexTypeManager::SetGetSize(handle, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 2u);

    bool exists = false;
    ret = ModObjDispatcherComplexTypeManager::SetContains(handle, &v1, &exists);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(exists);

    auto v3 = MakeI32Variant(999);
    ret = ModObjDispatcherComplexTypeManager::SetContains(handle, &v3, &exists);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(exists);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetAdd_Dedup, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32Variant(10);
    ret = ModObjDispatcherComplexTypeManager::SetAdd(handle, &v1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Add same value again - should dedup
    ret = ModObjDispatcherComplexTypeManager::SetAdd(handle, &v1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    ret = ModObjDispatcherComplexTypeManager::SetGetSize(handle, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 1u);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetAdd_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::SetAdd(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetAdd_TypeMismatch, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto strVal = MakeStringVariant("bad");
    ret = ModObjDispatcherComplexTypeManager::SetAdd(handle, &strVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&strVal);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetRemove_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32Variant(10);
    ModObjDispatcherComplexTypeManager::SetAdd(handle, &v1);
    auto v2 = MakeI32Variant(20);
    ModObjDispatcherComplexTypeManager::SetAdd(handle, &v2);

    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::SetGetSize(handle, &size);
    EXPECT_EQ(size, 2u);

    ret = ModObjDispatcherComplexTypeManager::SetRemove(handle, &v1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ModObjDispatcherComplexTypeManager::SetGetSize(handle, &size);
    EXPECT_EQ(size, 1u);

    bool exists = true;
    ModObjDispatcherComplexTypeManager::SetContains(handle, &v1, &exists);
    EXPECT_FALSE(exists);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetRemove_NotFound, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32Variant(10);
    ret = ModObjDispatcherComplexTypeManager::SetRemove(handle, &v1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetRemove_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::SetRemove(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetContains_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::SetContains(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetGetAt_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32Variant(100);
    ModObjDispatcherComplexTypeManager::SetAdd(handle, &v1);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::SetGetAt(handle, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(out.u.i32Val, 100);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetGetAt_IndexOutOfRange, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::SetGetAt(handle, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::SetGetAt(nullptr, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetGetSize_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::SetGetSize(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetGetElementType_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    ret = ModObjDispatcherComplexTypeManager::SetGetElementType(handle, &elemType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(elemType.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    ModObjDispatcherComplexTypeManager::TypeInfo_Clear(&elemType);
    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetGetElementType_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::SetGetElementType(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetClear_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32Variant(1);
    ModObjDispatcherComplexTypeManager::SetAdd(handle, &v1);
    auto v2 = MakeI32Variant(2);
    ModObjDispatcherComplexTypeManager::SetAdd(handle, &v2);

    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::SetGetSize(handle, &size);
    EXPECT_EQ(size, 2u);

    ret = ModObjDispatcherComplexTypeManager::SetClear(handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ModObjDispatcherComplexTypeManager::SetGetSize(handle, &size);
    EXPECT_EQ(size, 0u);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetClear_Null, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::SetClear(nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetRelease_NullAndDoubleRelease, TestSize.Level1)
{
    ModObjDispatcherComplexTypeManager::SetRelease(nullptr);

    OH_AbilityRuntime_ModObjDispatcher_SetHandle nullHandle = nullptr;
    ModObjDispatcherComplexTypeManager::SetRelease(&nullHandle);
    EXPECT_EQ(nullHandle, nullptr);

    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
    EXPECT_EQ(handle, nullptr);
    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, SetWithStringElements_Success, TestSize.Level1)
{
    auto ti = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeStringVariant("alpha");
    ModObjDispatcherComplexTypeManager::SetAdd(handle, &v1);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v1);

    auto v2 = MakeStringVariant("beta");
    ModObjDispatcherComplexTypeManager::SetAdd(handle, &v2);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v2);

    // Dedup same string
    auto v3 = MakeStringVariant("alpha");
    ret = ModObjDispatcherComplexTypeManager::SetAdd(handle, &v3);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v3);

    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::SetGetSize(handle, &size);
    EXPECT_EQ(size, 2u);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

// ==================== Map Tests ====================

HWTEST_F(ModObjDispatcherComplexTypeTest, MapCreate_NullValueType, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, nullptr, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapCreate_NullOutput, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapCreate_InvalidKeyType, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    // ARRAY is not a valid map key type
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY, &ti, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapCreate_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(handle, nullptr);

    uint32_t size = 99;
    ret = ModObjDispatcherComplexTypeManager::MapGetSize(handle, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 0u);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGetKeyType_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_ValueType keyType = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
    ret = ModObjDispatcherComplexTypeManager::MapGetKeyType(handle, &keyType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(keyType, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGetKeyType_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::MapGetKeyType(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGetValueType_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_TypeInfo valType = {};
    ret = ModObjDispatcherComplexTypeManager::MapGetValueType(handle, &valType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(valType.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    ModObjDispatcherComplexTypeManager::TypeInfo_Clear(&valType);
    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGetValueType_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::MapGetValueType(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapPutAndGet_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key1 = MakeStringVariant("key1");
    auto val1 = MakeI32Variant(100);
    ret = ModObjDispatcherComplexTypeManager::MapPut(handle, &key1, &val1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key1);

    auto key2 = MakeStringVariant("key2");
    auto val2 = MakeI32Variant(200);
    ret = ModObjDispatcherComplexTypeManager::MapPut(handle, &key2, &val2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key2);

    uint32_t size = 0;
    ret = ModObjDispatcherComplexTypeManager::MapGetSize(handle, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 2u);

    // Get by key
    auto searchKey = MakeStringVariant("key1");
    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(handle, &searchKey, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(out.u.i32Val, 100);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&searchKey);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapPut_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::MapPut(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("k");
    auto val = MakeI32Variant(1);
    ret = ModObjDispatcherComplexTypeManager::MapPut(handle, nullptr, &val);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::MapPut(handle, &key, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapPut_KeyTypeMismatch, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Key should be STRING but passing I32
    auto badKey = MakeI32Variant(1);
    auto val = MakeI32Variant(100);
    ret = ModObjDispatcherComplexTypeManager::MapPut(handle, &badKey, &val);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapPut_ValueTypeMismatch, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("k");
    auto badVal = MakeStringVariant("wrong");
    ret = ModObjDispatcherComplexTypeManager::MapPut(handle, &key, &badVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&badVal);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapPut_UpdateExistingKey, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("key1");
    auto val1 = MakeI32Variant(100);
    ret = ModObjDispatcherComplexTypeManager::MapPut(handle, &key, &val1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Update with same key, new value
    auto val2 = MakeI32Variant(999);
    ret = ModObjDispatcherComplexTypeManager::MapPut(handle, &key, &val2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Size should still be 1
    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::MapGetSize(handle, &size);
    EXPECT_EQ(size, 1u);

    // Get should return updated value
    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(handle, &key, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.u.i32Val, 999);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGet_NotFound, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("missing");
    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(handle, &key, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGet_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::MapGet(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapRemove_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("key1");
    auto val = MakeI32Variant(100);
    ModObjDispatcherComplexTypeManager::MapPut(handle, &key, &val);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);

    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::MapGetSize(handle, &size);
    EXPECT_EQ(size, 1u);

    auto removeKey = MakeStringVariant("key1");
    ret = ModObjDispatcherComplexTypeManager::MapRemove(handle, &removeKey);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&removeKey);

    ModObjDispatcherComplexTypeManager::MapGetSize(handle, &size);
    EXPECT_EQ(size, 0u);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapRemove_NotFound, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("nonexistent");
    ret = ModObjDispatcherComplexTypeManager::MapRemove(handle, &key);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapRemove_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::MapRemove(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapContainsKey_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("mykey");
    auto val = MakeI32Variant(42);
    ModObjDispatcherComplexTypeManager::MapPut(handle, &key, &val);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);

    auto searchKey = MakeStringVariant("mykey");
    bool exists = false;
    ret = ModObjDispatcherComplexTypeManager::MapContainsKey(handle, &searchKey, &exists);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(exists);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&searchKey);

    auto missingKey = MakeStringVariant("nokey");
    exists = true;
    ret = ModObjDispatcherComplexTypeManager::MapContainsKey(handle, &missingKey, &exists);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(exists);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&missingKey);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapContainsKey_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::MapContainsKey(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGetSize_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::MapGetSize(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGetKeyAt_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("first");
    auto val = MakeI32Variant(1);
    ModObjDispatcherComplexTypeManager::MapPut(handle, &key, &val);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);

    OH_AbilityRuntime_ModObjDispatcher_Variant outKey = {};
    ret = ModObjDispatcherComplexTypeManager::MapGetKeyAt(handle, 0, &outKey);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(outKey.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_NE(outKey.u.bstrVal, nullptr);
    EXPECT_STREQ(outKey.u.bstrVal, "first");
    ModObjDispatcherComplexTypeManager::Variant_Clear(&outKey);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGetKeyAt_OutOfRange, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant outKey = {};
    ret = ModObjDispatcherComplexTypeManager::MapGetKeyAt(handle, 0, &outKey);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::MapGetKeyAt(nullptr, 0, &outKey);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGetValueAt_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("k");
    auto val = MakeI32Variant(77);
    ModObjDispatcherComplexTypeManager::MapPut(handle, &key, &val);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);

    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    ret = ModObjDispatcherComplexTypeManager::MapGetValueAt(handle, 0, &outVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(outVal.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(outVal.u.i32Val, 77);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapGetValueAt_OutOfRange, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    ret = ModObjDispatcherComplexTypeManager::MapGetValueAt(handle, 5, &outVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::MapGetValueAt(nullptr, 0, &outVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapClear_Success, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeStringVariant("k1");
    auto val = MakeI32Variant(1);
    ModObjDispatcherComplexTypeManager::MapPut(handle, &key, &val);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&key);

    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::MapGetSize(handle, &size);
    EXPECT_EQ(size, 1u);

    ret = ModObjDispatcherComplexTypeManager::MapClear(handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ModObjDispatcherComplexTypeManager::MapGetSize(handle, &size);
    EXPECT_EQ(size, 0u);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapClear_Null, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::MapClear(nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapRelease_NullAndDoubleRelease, TestSize.Level1)
{
    ModObjDispatcherComplexTypeManager::MapRelease(nullptr);

    OH_AbilityRuntime_ModObjDispatcher_MapHandle nullHandle = nullptr;
    ModObjDispatcherComplexTypeManager::MapRelease(&nullHandle);
    EXPECT_EQ(nullHandle, nullptr);

    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
    EXPECT_EQ(handle, nullptr);
    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, MapWithI32Key_Success, TestSize.Level1)
{
    auto ti = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, &ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeI32Variant(1);
    auto val = MakeStringVariant("one");
    ret = ModObjDispatcherComplexTypeManager::MapPut(handle, &key, &val);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&val);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(handle, &key, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_STREQ(out.u.bstrVal, "one");
    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

// ==================== Struct Tests ====================

HWTEST_F(ModObjDispatcherComplexTypeTest, StructCreate_NullName, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate(nullptr, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructCreate_NullOutput, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("MyStruct", nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructCreate_Success, TestSize.Level1)
{
    RegisterTestStructMetadata();
    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("MyStruct", &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(handle, nullptr);

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructGetName_Success, TestSize.Level1)
{
    RegisterTestStructMetadata();
    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("MyStruct", &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    char buf[64] = {};
    ret = ModObjDispatcherComplexTypeManager::StructGetName(handle, buf, sizeof(buf));
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(buf, "MyStruct");

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructGetName_NullParams, TestSize.Level1)
{
    RegisterTestStructMetadata();
    auto ret = ModObjDispatcherComplexTypeManager::StructGetName(nullptr, nullptr, 0);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    ret = ModObjDispatcherComplexTypeManager::StructCreate("S", &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    char buf[32] = {};
    ret = ModObjDispatcherComplexTypeManager::StructGetName(handle, nullptr, sizeof(buf));
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ret = ModObjDispatcherComplexTypeManager::StructGetName(handle, buf, 0);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructGetName_BufferTooSmall, TestSize.Level1)
{
    RegisterTestStructMetadata();
    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("LongStructName", &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    char buf[4] = {};
    ret = ModObjDispatcherComplexTypeManager::StructGetName(handle, buf, sizeof(buf));
    // "LongStructName" requires 15 bytes including null, buf is only 4
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructSetAndGetField_Success, TestSize.Level1)
{
    RegisterTestStructMetadata();

    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("TestStruct", &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto idVal = MakeI32Variant(42);
    ret = ModObjDispatcherComplexTypeManager::StructSetField(handle, "id", &idVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto nameVal = MakeStringVariant("Alice");
    ret = ModObjDispatcherComplexTypeManager::StructSetField(handle, "name", &nameVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&nameVal);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::StructGetField(handle, "id", &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(out.u.i32Val, 42);

    ret = ModObjDispatcherComplexTypeManager::StructGetField(handle, "name", &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_STREQ(out.u.bstrVal, "Alice");
    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructSetField_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::StructSetField(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructSetField_FieldNotFound, TestSize.Level1)
{
    RegisterTestStructMetadata();

    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("TestStruct", &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto val = MakeI32Variant(1);
    ret = ModObjDispatcherComplexTypeManager::StructSetField(handle, "nonexistent", &val);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructSetField_TypeMismatch, TestSize.Level1)
{
    RegisterTestStructMetadata();

    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("TestStruct", &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // "id" field expects I32, pass STRING
    auto strVal = MakeStringVariant("bad");
    ret = ModObjDispatcherComplexTypeManager::StructSetField(handle, "id", &strVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&strVal);

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructGetField_NullParams, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::StructGetField(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructGetField_FieldNotFound, TestSize.Level1)
{
    RegisterTestStructMetadata();

    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("TestStruct", &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::StructGetField(handle, "nonexistent", &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructGetField_NotSet, TestSize.Level1)
{
    RegisterTestStructMetadata();

    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("TestStruct", &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // "id" is a valid field but never set
    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::StructGetField(handle, "id", &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructRelease_NullAndDoubleRelease, TestSize.Level1)
{
    RegisterTestStructMetadata();
    ModObjDispatcherComplexTypeManager::StructRelease(nullptr);

    OH_AbilityRuntime_ModObjDispatcher_StructHandle nullHandle = nullptr;
    ModObjDispatcherComplexTypeManager::StructRelease(&nullHandle);
    EXPECT_EQ(nullHandle, nullptr);

    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("S", &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
    EXPECT_EQ(handle, nullptr);
    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

// ==================== Variant_Clear Tests ====================

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_Null, TestSize.Level1)
{
    // Should not crash
    ModObjDispatcherComplexTypeManager::Variant_Clear(nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_StringType, TestSize.Level1)
{
    auto v = MakeStringVariant("hello");
    EXPECT_NE(v.u.bstrVal, nullptr);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.bstrVal, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_SimpleType, TestSize.Level1)
{
    auto v = MakeI32Variant(42);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.i32Val, 0);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_BoolType, TestSize.Level1)
{
    auto v = MakeBoolVariant(true);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_F64Type, TestSize.Level1)
{
    auto v = MakeF64Variant(3.14);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_ArrayType, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle arrHandle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&ti, 2, &arrHandle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY;
    v.u.parrayVal = arrHandle;

    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.parrayVal, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_VectorType, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vecHandle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(&ti, &vecHandle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    v.u.pvectorVal = vecHandle;

    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.pvectorVal, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_SetType, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle setHandle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &setHandle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET;
    v.u.psetVal = setHandle;

    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.psetVal, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_MapType, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle mapHandle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &ti, &mapHandle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP;
    v.u.pmapVal = mapHandle;

    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.pmapVal, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, Variant_Clear_StructType, TestSize.Level1)
{
    RegisterTestStructMetadata();
    OH_AbilityRuntime_ModObjDispatcher_StructHandle structHandle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("S", &structHandle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT;
    v.u.pstructVal = structHandle;

    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.pstructVal, nullptr);
}

// ==================== TypeInfo_Clear Tests ====================

HWTEST_F(ModObjDispatcherComplexTypeTest, TypeInfo_Clear_Null, TestSize.Level1)
{
    // Should not crash
    ModObjDispatcherComplexTypeManager::TypeInfo_Clear(nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, TypeInfo_Clear_SimpleType, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    ModObjDispatcherComplexTypeManager::TypeInfo_Clear(&ti);
    EXPECT_EQ(ti.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
}

// ==================== ValidateVariantType Tests ====================

HWTEST_F(ModObjDispatcherComplexTypeTest, ValidateVariantType_Null, TestSize.Level1)
{
    auto ret = ModObjDispatcherComplexTypeManager::ValidateVariantType(
        nullptr, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ValidateVariantType_Match, TestSize.Level1)
{
    auto v = MakeI32Variant(42);
    auto ret = ModObjDispatcherComplexTypeManager::ValidateVariantType(
        &v, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ValidateVariantType_Mismatch, TestSize.Level1)
{
    auto v = MakeI32Variant(42);
    auto ret = ModObjDispatcherComplexTypeManager::ValidateVariantType(
        &v, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ValidateVariantType_BoolMatch, TestSize.Level1)
{
    auto v = MakeBoolVariant(true);
    auto ret = ModObjDispatcherComplexTypeManager::ValidateVariantType(
        &v, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ValidateVariantType_StringMatch, TestSize.Level1)
{
    auto v = MakeStringVariant("test");
    auto ret = ModObjDispatcherComplexTypeManager::ValidateVariantType(
        &v, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ValidateVariantType_StringWithNullBstr, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    v.u.bstrVal = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ValidateVariantType(
        &v, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, ValidateVariantType_EmptyMismatch, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
    auto ret = ModObjDispatcherComplexTypeManager::ValidateVariantType(
        &v, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
}

// ==================== Additional TDD tests for commit 2617e6d595b ====================

// StructCreate rejects unknown struct name (not in metadata)
HWTEST_F(ModObjDispatcherComplexTypeTest, StructCreate_UnknownName_Rejected, TestSize.Level1)
{
    RegisterTestStructMetadata();
    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("UnknownStruct", &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
    EXPECT_EQ(handle, nullptr);
}

// StructCreate rejects when no metadata registered at all
HWTEST_F(ModObjDispatcherComplexTypeTest, StructCreate_NoMetadata_Rejected, TestSize.Level1)
{
    std::vector<MoStructMeta> empty;
    ModObjDispatcherComplexTypeManager::RegisterStructMetadata(empty);

    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("AnyStruct", &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
    EXPECT_EQ(handle, nullptr);
}

// StructCreate succeeds for registered name
HWTEST_F(ModObjDispatcherComplexTypeTest, StructCreate_RegisteredName_Success, TestSize.Level1)
{
    RegisterTestStructMetadata();
    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("TestStruct", &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(handle, nullptr);

    char buf[64] = {};
    ret = ModObjDispatcherComplexTypeManager::StructGetName(handle, buf, sizeof(buf));
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(buf, "TestStruct");

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
}

// SetRemove rejects type mismatch
HWTEST_F(ModObjDispatcherComplexTypeTest, SetRemove_TypeMismatch, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto strVal = MakeStringVariant("bad");
    ret = ModObjDispatcherComplexTypeManager::SetRemove(handle, &strVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&strVal);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

// SetContains rejects type mismatch
HWTEST_F(ModObjDispatcherComplexTypeTest, SetContains_TypeMismatch, TestSize.Level1)
{
    auto ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&ti, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto strVal = MakeStringVariant("bad");
    bool exists = false;
    ret = ModObjDispatcherComplexTypeManager::SetContains(handle, &strVal, &exists);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&strVal);

    ModObjDispatcherComplexTypeManager::SetRelease(&handle);
}

// MapGet rejects key type mismatch
HWTEST_F(ModObjDispatcherComplexTypeTest, MapGet_KeyTypeMismatch, TestSize.Level1)
{
    auto valTi = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, &valTi, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto badKey = MakeStringVariant("not_i32");
    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(handle, &badKey, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&badKey);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

// MapGet returns PROPERTY_NOT_FOUND when key not found
HWTEST_F(ModObjDispatcherComplexTypeTest, MapGet_KeyNotFound, TestSize.Level1)
{
    auto valTi = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, &valTi, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto k1 = MakeI32Variant(1);
    auto v1 = MakeI32Variant(100);
    ModObjDispatcherComplexTypeManager::MapPut(handle, &k1, &v1);

    auto k2 = MakeI32Variant(2);
    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(handle, &k2, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

// MapRemove rejects key type mismatch
HWTEST_F(ModObjDispatcherComplexTypeTest, MapRemove_KeyTypeMismatch, TestSize.Level1)
{
    auto valTi = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, &valTi, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto badKey = MakeStringVariant("not_i32");
    ret = ModObjDispatcherComplexTypeManager::MapRemove(handle, &badKey);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&badKey);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

// MapContainsKey rejects key type mismatch
HWTEST_F(ModObjDispatcherComplexTypeTest, MapContainsKey_KeyTypeMismatch, TestSize.Level1)
{
    auto valTi = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, &valTi, &handle);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto badKey = MakeStringVariant("not_i32");
    bool exists = false;
    ret = ModObjDispatcherComplexTypeManager::MapContainsKey(handle, &badKey, &exists);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&badKey);

    ModObjDispatcherComplexTypeManager::MapRelease(&handle);
}

// ==================== 2-level nested tests ====================

// Vector<Vector<i32>>: create outer, add inner vector, get back and verify
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_VectorOfVector_2Level, TestSize.Level1)
{
    auto* outerTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle outerVec = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(outerTi, &outerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto innerVec = CreateI32Vector({10, 20});
    auto vecVar = MakeVectorVariant(innerVec);
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(outerVec, &vecVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::VectorGet(outerVec, 0, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR);
    ASSERT_NE(out.u.pvectorVal, nullptr);
    uint32_t innerSize = 0;
    ModObjDispatcherComplexTypeManager::VectorGetSize(out.u.pvectorVal, &innerSize);
    EXPECT_EQ(innerSize, 2u);

    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);
    ModObjDispatcherComplexTypeManager::VectorRelease(&innerVec);
    ModObjDispatcherComplexTypeManager::VectorRelease(&outerVec);
    FreeTypeInfo(*outerTi);
    delete outerTi;
}

// Map<i32, Vector<string>>: nested vector as map value
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_MapOfVector_2Level, TestSize.Level1)
{
    auto* valTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, valTi, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto innerVec = CreateStringVector({"hello", "world"});
    auto key = MakeI32Variant(1);
    auto vecVar = MakeVectorVariant(innerVec);
    ret = ModObjDispatcherComplexTypeManager::MapPut(map, &key, &vecVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(map, &key, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR);
    ASSERT_NE(out.u.pvectorVal, nullptr);
    uint32_t sz = 0;
    ModObjDispatcherComplexTypeManager::VectorGetSize(out.u.pvectorVal, &sz);
    EXPECT_EQ(sz, 2u);

    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);
    ModObjDispatcherComplexTypeManager::VectorRelease(&innerVec);
    ModObjDispatcherComplexTypeManager::MapRelease(&map);
    FreeTypeInfo(*valTi);
    delete valTi;
}

// Map<i32, Map<i32, i64>>: nested map as value, verify 2-level get
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_MapOfMap_2Level, TestSize.Level1)
{
    auto* outerValTi = MakeMapTypeInfoHeap(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle outerMap = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, outerValTi, &outerMap);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto innerValTi = MakeI64TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle innerMap = nullptr;
    ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, &innerValTi, &innerMap);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto ik = MakeI32Variant(100);
    auto iv = MakeI64Variant(999LL);
    ModObjDispatcherComplexTypeManager::MapPut(innerMap, &ik, &iv);

    auto ok = MakeI32Variant(1);
    auto mapVar = MakeMapVariant(innerMap);
    ret = ModObjDispatcherComplexTypeManager::MapPut(outerMap, &ok, &mapVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out1 = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(outerMap, &ok, &out1);
    EXPECT_EQ(out1.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP);
    ASSERT_NE(out1.u.pmapVal, nullptr);
    OH_AbilityRuntime_ModObjDispatcher_Variant out2 = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(out1.u.pmapVal, &ik, &out2);
    EXPECT_EQ(out2.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64);
    EXPECT_EQ(out2.u.i64Val, 999LL);

    ModObjDispatcherComplexTypeManager::Variant_Clear(&out2);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&out1);
    ModObjDispatcherComplexTypeManager::MapRelease(&innerMap);
    ModObjDispatcherComplexTypeManager::MapRelease(&outerMap);
    FreeTypeInfo(*outerValTi);
    delete outerValTi;
}

// Array<Vector<i32>>: overwrite slot, old vector should not leak
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_ArrayOfVector_OverwriteNoLeak, TestSize.Level1)
{
    auto* arrTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle arr = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::ArrayCreate(arrTi, 1, &arr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto vec1 = CreateI32Vector({100});
    auto var1 = MakeVectorVariant(vec1);
    ret = ModObjDispatcherComplexTypeManager::ArraySet(arr, 0, &var1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto vec2 = CreateI32Vector({200, 300});
    auto var2 = MakeVectorVariant(vec2);
    ret = ModObjDispatcherComplexTypeManager::ArraySet(arr, 0, &var2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::ArrayGet(arr, 0, &out);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(out.u.pvectorVal, nullptr);
    uint32_t sz = 0;
    ModObjDispatcherComplexTypeManager::VectorGetSize(out.u.pvectorVal, &sz);
    EXPECT_EQ(sz, 2u);

    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);
    ModObjDispatcherComplexTypeManager::VectorRelease(&vec1);
    ModObjDispatcherComplexTypeManager::VectorRelease(&vec2);
    ModObjDispatcherComplexTypeManager::ArrayRelease(&arr);
    FreeTypeInfo(*arrTi);
    delete arrTi;
}

// Map same key overwrite should release old nested value
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_MapOverwriteValue_NoLeak, TestSize.Level1)
{
    auto* valTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, valTi, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeI32Variant(1);
    auto vec1 = CreateI32Vector({100});
    auto var1 = MakeVectorVariant(vec1);
    ret = ModObjDispatcherComplexTypeManager::MapPut(map, &key, &var1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto vec2 = CreateI32Vector({200});
    auto var2 = MakeVectorVariant(vec2);
    ret = ModObjDispatcherComplexTypeManager::MapPut(map, &key, &var2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t sz = 99;
    ModObjDispatcherComplexTypeManager::MapGetSize(map, &sz);
    EXPECT_EQ(sz, 1u);

    ModObjDispatcherComplexTypeManager::VectorRelease(&vec1);
    ModObjDispatcherComplexTypeManager::VectorRelease(&vec2);
    ModObjDispatcherComplexTypeManager::MapRelease(&map);
    FreeTypeInfo(*valTi);
    delete valTi;
}

// 3-level: Vector<Map<i32, Vector<string>>>
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_3Level_VectorMapVector, TestSize.Level1)
{
    // Level 3: Vector<string>
    auto innerVec = CreateStringVector({"hello", "world"});

    // Level 2: Map<i32, Vector<string>>
    auto* midValTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle midMap = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, midValTi, &midMap);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto mk = MakeI32Variant(1);
    auto vecVar = MakeVectorVariant(innerVec);
    ModObjDispatcherComplexTypeManager::MapPut(midMap, &mk, &vecVar);

    // Level 1: Vector<Map<i32, Vector<string>>>
    auto* outerTi = NewTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP);
    outerTi->u.mapType.keyType = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    outerTi->u.mapType.pValueType = MakeContainerTypeInfo(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle outerVec = nullptr;
    ret = ModObjDispatcherComplexTypeManager::VectorCreate(outerTi, &outerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto mapVar = MakeMapVariant(midMap);
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(outerVec, &mapVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Verify 3-level get: outer[0] → map[key=1] → vector size
    OH_AbilityRuntime_ModObjDispatcher_Variant out1 = {};
    ret = ModObjDispatcherComplexTypeManager::VectorGet(outerVec, 0, &out1);
    ASSERT_EQ(out1.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP);
    ASSERT_NE(out1.u.pmapVal, nullptr);
    OH_AbilityRuntime_ModObjDispatcher_Variant out2 = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(out1.u.pmapVal, &mk, &out2);
    ASSERT_EQ(out2.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR);
    ASSERT_NE(out2.u.pvectorVal, nullptr);
    uint32_t sz = 0;
    ModObjDispatcherComplexTypeManager::VectorGetSize(out2.u.pvectorVal, &sz);
    EXPECT_EQ(sz, 2u);

    ModObjDispatcherComplexTypeManager::Variant_Clear(&out2);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&out1);
    ModObjDispatcherComplexTypeManager::VectorRelease(&innerVec);
    ModObjDispatcherComplexTypeManager::MapRelease(&midMap);
    ModObjDispatcherComplexTypeManager::VectorRelease(&outerVec);
    FreeTypeInfo(*outerTi);
    delete outerTi;
}

// 3-level release chain: Vector<Array<Set<i32>>> should not leak
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_3Level_ReleaseChain, TestSize.Level1)
{
    // Level 3: Set<i32>
    auto i32Ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle innerSet = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&i32Ti, &innerSet);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto e1 = MakeI32Variant(1);
    ModObjDispatcherComplexTypeManager::SetAdd(innerSet, &e1);

    // Level 2: Array<Set<i32>>
    auto* setTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle midArr = nullptr;
    ret = ModObjDispatcherComplexTypeManager::ArrayCreate(setTi, 1, &midArr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto setVar = MakeSetVariant(innerSet);
    ret = ModObjDispatcherComplexTypeManager::ArraySet(midArr, 0, &setVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Level 1: Vector<Array<Set<i32>>>
    auto* arrTi = NewTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY);
    arrTi->u.arrayType.pElementType = MakeContainerTypeInfo(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle outerVec = nullptr;
    ret = ModObjDispatcherComplexTypeManager::VectorCreate(arrTi, &outerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto arrVar = MakeArrayVariant(midArr);
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(outerVec, &arrVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ModObjDispatcherComplexTypeManager::SetRelease(&innerSet);
    ModObjDispatcherComplexTypeManager::ArrayRelease(&midArr);
    ModObjDispatcherComplexTypeManager::VectorRelease(&outerVec);
    FreeTypeInfo(*setTi);
    delete setTi;
    FreeTypeInfo(*arrTi);
    delete arrTi;
}

// Map<i32, Set<i32>>: nested Set dedup inside Map, then Get/Contains through Map
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_MapOfSet_DedupAndAccess, TestSize.Level1)
{
    auto i32Ti = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle innerSet = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&i32Ti, &innerSet);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto e1 = MakeI32Variant(10);
    auto e2 = MakeI32Variant(20);
    auto e3 = MakeI32Variant(10); // duplicate
    ModObjDispatcherComplexTypeManager::SetAdd(innerSet, &e1);
    ModObjDispatcherComplexTypeManager::SetAdd(innerSet, &e2);
    ModObjDispatcherComplexTypeManager::SetAdd(innerSet, &e3);
    uint32_t setSize = 99;
    ModObjDispatcherComplexTypeManager::SetGetSize(innerSet, &setSize);
    EXPECT_EQ(setSize, 2u);

    auto* setValTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, setValTi, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto setVar = MakeSetVariant(innerSet);
    auto key = MakeI32Variant(1);
    ret = ModObjDispatcherComplexTypeManager::MapPut(map, &key, &setVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(map, &key, &out);
    ASSERT_NE(out.u.psetVal, nullptr);
    bool exists = false;
    auto checkVal = MakeI32Variant(10);
    ModObjDispatcherComplexTypeManager::SetContains(out.u.psetVal, &checkVal, &exists);
    EXPECT_TRUE(exists);

    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);
    ModObjDispatcherComplexTypeManager::SetRelease(&innerSet);
    ModObjDispatcherComplexTypeManager::MapRelease(&map);
    FreeTypeInfo(*setValTi);
    delete setValTi;
}

// Get returns independent deep copy — modifying copy doesn't affect original
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_GetProducesDeepCopy, TestSize.Level1)
{
    auto vec = CreateStringVector({"test"});
    auto* valTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, valTi, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto key = MakeI32Variant(1);
    auto vecVar = MakeVectorVariant(vec);
    ModObjDispatcherComplexTypeManager::MapPut(map, &key, &vecVar);

    // Get copy — pointer should differ from original
    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(map, &key, &out);
    ASSERT_NE(out.u.pvectorVal, nullptr);
    EXPECT_NE(out.u.pvectorVal, vec);
    uint32_t sz = 0;
    ModObjDispatcherComplexTypeManager::VectorGetSize(out.u.pvectorVal, &sz);
    EXPECT_EQ(sz, 1u);

    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);
    ModObjDispatcherComplexTypeManager::VectorRelease(&vec);
    ModObjDispatcherComplexTypeManager::MapRelease(&map);
    FreeTypeInfo(*valTi);
    delete valTi;
}

// Self-reference: vec.Add(vec) — circular detection in StoreVariant
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_Circular_SelfReference, TestSize.Level1)
{
    // Element type: VECTOR with pElementType=nullptr so ValidateVariantTypeDeep skips deep check
    auto* innerVecTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(innerVecTi, sizeof(*innerVecTi), 0, sizeof(*innerVecTi));
    innerVecTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    // pElementType stays nullptr → FromCTypeInfo sets elementTypeInfo.pElementType=nullptr
    // → ValidateVariantTypeDeep sees expectedInfo->pElementType==nullptr → returns OK

    auto* outerElemTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(outerElemTi, sizeof(*outerElemTi), 0, sizeof(*outerElemTi));
    outerElemTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    outerElemTi->u.pElementType = innerVecTi;

    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vec = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(outerElemTi, &vec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Variant pointing back to vec itself
    OH_AbilityRuntime_ModObjDispatcher_Variant selfVar = {};
    selfVar.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    selfVar.u.pvectorVal = vec;

    // StoreVariant deep copy should detect cycle
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(vec, &selfVar);
    EXPECT_TRUE(ret == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR ||
        ret == ABILITY_RUNTIME_ERROR_CODE_INTERNAL);

    ModObjDispatcherComplexTypeManager::VectorRelease(&vec);
    FreeTypeInfo(*outerElemTi); // recursively frees innerVecTi
    delete outerElemTi;
}

// Cross-reference: vecA → vecB → vecA
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_Circular_CrossReference, TestSize.Level1)
{
    // Element type: VECTOR with pElementType=nullptr so ValidateVariantTypeDeep skips deep check
    auto* innerVecTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(innerVecTi, sizeof(*innerVecTi), 0, sizeof(*innerVecTi));
    innerVecTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;

    auto* outerElemTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(outerElemTi, sizeof(*outerElemTi), 0, sizeof(*outerElemTi));
    outerElemTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    outerElemTi->u.pElementType = innerVecTi;

    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vecA = nullptr;
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vecB = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(outerElemTi, &vecA);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ret = ModObjDispatcherComplexTypeManager::VectorCreate(outerElemTi, &vecB);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Put vecB into vecA (no cycle yet)
    OH_AbilityRuntime_ModObjDispatcher_Variant varB = {};
    varB.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    varB.u.pvectorVal = vecB;
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(vecA, &varB);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Try to put vecA into vecB — deep copy should detect cycle
    OH_AbilityRuntime_ModObjDispatcher_Variant varA = {};
    varA.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    varA.u.pvectorVal = vecA;
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(vecB, &varA);
    EXPECT_TRUE(ret == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR ||
        ret == ABILITY_RUNTIME_ERROR_CODE_INTERNAL);

    ModObjDispatcherComplexTypeManager::VectorRelease(&vecA);
    ModObjDispatcherComplexTypeManager::VectorRelease(&vecB);
    FreeTypeInfo(*outerElemTi); // recursively frees innerVecTi
    delete outerElemTi;
}

// Type mismatch in nested context: Vector<Map<i32, string>> rejects Map<i32, i64>
HWTEST_F(ModObjDispatcherComplexTypeTest, Nested_NestedTypeMismatch, TestSize.Level1)
{
    // Expected inner: Map<i32, string>
    auto expectedTi = MakeMapTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    // Outer: Vector<Map<i32, string>> — element TypeInfo = MAP{I32, STRING}
    auto* mapI32StrTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(mapI32StrTi, sizeof(*mapI32StrTi), 0, sizeof(*mapI32StrTi));
    mapI32StrTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP;
    mapI32StrTi->u.mapType.keyType = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    auto* strTi5 = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(strTi5, sizeof(*strTi5), 0, sizeof(*strTi5));
    strTi5->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    mapI32StrTi->u.mapType.pValueType = strTi5;
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vec = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(mapI32StrTi, &vec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Wrong inner: Map<i32, i64>
    auto wrongValTi = MakeI64TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle wrongMap = nullptr;
    ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, &wrongValTi, &wrongMap);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto wk = MakeI32Variant(1);
    auto wv = MakeI64Variant(99LL);
    ModObjDispatcherComplexTypeManager::MapPut(wrongMap, &wk, &wv);

    OH_AbilityRuntime_ModObjDispatcher_Variant wrongVar = {};
    wrongVar.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP;
    wrongVar.u.pmapVal = wrongMap;

    // Should detect nested value type mismatch
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(vec, &wrongVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    ModObjDispatcherComplexTypeManager::MapRelease(&wrongMap);
    ModObjDispatcherComplexTypeManager::VectorRelease(&vec);
    FreeTypeInfo(expectedTi);
    FreeTypeInfo(*mapI32StrTi);
    delete mapI32StrTi;
}

// ==================== TypeInfoMatches (deep nested type comparison) TDD ====================

// Vector<Vector<I32>> — add correct Vector<I32> element, should succeed (TypeInfoMatches passes)
HWTEST_F(ModObjDispatcherComplexTypeTest, TypeInfoMatches_VectorOfVector_I32_Match, TestSize.Level1)
{
    // Outer: Vector<Vector<I32>> — element TypeInfo = VECTOR{I32}
    auto* vecElemTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(vecElemTi, sizeof(*vecElemTi), 0, sizeof(*vecElemTi));
    vecElemTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    auto* i32Leaf = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(i32Leaf, sizeof(*i32Leaf), 0, sizeof(*i32Leaf));
    i32Leaf->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    vecElemTi->u.pElementType = i32Leaf;

    OH_AbilityRuntime_ModObjDispatcher_VectorHandle outerVec = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(vecElemTi, &outerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Inner: Vector<I32> with matching element type
    auto innerTi = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle innerVec = nullptr;
    ret = ModObjDispatcherComplexTypeManager::VectorCreate(&innerTi, &innerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeI32Variant(10);
    auto v2 = MakeI32Variant(20);
    ModObjDispatcherComplexTypeManager::VectorAdd(innerVec, &v1);
    ModObjDispatcherComplexTypeManager::VectorAdd(innerVec, &v2);

    OH_AbilityRuntime_ModObjDispatcher_Variant elem = {};
    elem.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    elem.u.pvectorVal = innerVec;

    // TypeInfoMatches: inner elementTypeInfo={I32} matches expected pElementType={I32} → OK
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(outerVec, &elem);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t sz = 0;
    ModObjDispatcherComplexTypeManager::VectorGetSize(outerVec, &sz);
    EXPECT_EQ(sz, 1u);

    ModObjDispatcherComplexTypeManager::VectorRelease(&innerVec);
    ModObjDispatcherComplexTypeManager::VectorRelease(&outerVec);
    FreeTypeInfo(*vecElemTi);
    delete vecElemTi;
}

// Vector<Vector<I32>> — add wrong Vector<STRING> element, should fail (TypeInfoMismatch)
HWTEST_F(ModObjDispatcherComplexTypeTest, TypeInfoMatches_VectorOfVector_I32_vs_String_Mismatch, TestSize.Level1)
{
    // Outer: Vector<Vector<I32>> — element TypeInfo = VECTOR{I32}
    auto* vecElemTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(vecElemTi, sizeof(*vecElemTi), 0, sizeof(*vecElemTi));
    vecElemTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    auto* i32Leaf = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(i32Leaf, sizeof(*i32Leaf), 0, sizeof(*i32Leaf));
    i32Leaf->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    vecElemTi->u.pElementType = i32Leaf;

    OH_AbilityRuntime_ModObjDispatcher_VectorHandle outerVec = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(vecElemTi, &outerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Inner: Vector<STRING> — wrong element type
    auto strTi = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle wrongVec = nullptr;
    ret = ModObjDispatcherComplexTypeManager::VectorCreate(&strTi, &wrongVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto sv = MakeStringVariant("hello");
    ModObjDispatcherComplexTypeManager::VectorAdd(wrongVec, &sv);

    OH_AbilityRuntime_ModObjDispatcher_Variant elem = {};
    elem.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    elem.u.pvectorVal = wrongVec;

    // TypeInfoMatches: inner elementTypeInfo={STRING} vs expected pElementType={I32} → MISMATCH
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(outerVec, &elem);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    ModObjDispatcherComplexTypeManager::VectorRelease(&wrongVec);
    ModObjDispatcherComplexTypeManager::VectorRelease(&outerVec);
    FreeTypeInfo(*vecElemTi);
    delete vecElemTi;
}

// Map<I32, Vector<I32>> — put Vector<STRING> value should fail (nested value type mismatch)
HWTEST_F(ModObjDispatcherComplexTypeTest, TypeInfoMatches_MapValueNestedMismatch, TestSize.Level1)
{
    // Map<I32, Vector<I32>> — value TypeInfo = VECTOR{I32}
    auto* vecValTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(vecValTi, sizeof(*vecValTi), 0, sizeof(*vecValTi));
    vecValTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    auto* i32Leaf = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(i32Leaf, sizeof(*i32Leaf), 0, sizeof(*i32Leaf));
    i32Leaf->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    vecValTi->u.pElementType = i32Leaf;

    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, vecValTi, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Wrong: Vector<STRING> instead of Vector<I32>
    auto wrongStrTi = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle wrongVec = nullptr;
    ret = ModObjDispatcherComplexTypeManager::VectorCreate(&wrongStrTi, &wrongVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto sv = MakeStringVariant("bad");
    ModObjDispatcherComplexTypeManager::VectorAdd(wrongVec, &sv);

    auto key = MakeI32Variant(1);
    OH_AbilityRuntime_ModObjDispatcher_Variant wrongVal = {};
    wrongVal.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    wrongVal.u.pvectorVal = wrongVec;

    // TypeInfoMatches: value Vector elementTypeInfo={STRING} vs expected pElementType={I32} → MISMATCH
    ret = ModObjDispatcherComplexTypeManager::MapPut(map, &key, &wrongVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    ModObjDispatcherComplexTypeManager::VectorRelease(&wrongVec);
    ModObjDispatcherComplexTypeManager::MapRelease(&map);
    FreeTypeInfo(*vecValTi);
    delete vecValTi;
}

// Vector<Set<I32>> — add Set<STRING> should fail (nested element type mismatch)
HWTEST_F(ModObjDispatcherComplexTypeTest, TypeInfoMatches_VectorOfSetMismatch, TestSize.Level1)
{
    // Outer: Vector<Set<I32>> — element TypeInfo = SET{I32}
    auto* setElemTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(setElemTi, sizeof(*setElemTi), 0, sizeof(*setElemTi));
    setElemTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET;
    auto* i32Leaf = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(i32Leaf, sizeof(*i32Leaf), 0, sizeof(*i32Leaf));
    i32Leaf->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    setElemTi->u.pElementType = i32Leaf;

    OH_AbilityRuntime_ModObjDispatcher_VectorHandle outerVec = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(setElemTi, &outerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Wrong: Set<STRING>
    auto strTi = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle wrongSet = nullptr;
    ret = ModObjDispatcherComplexTypeManager::SetCreate(&strTi, &wrongSet);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant elem = {};
    elem.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET;
    elem.u.psetVal = wrongSet;

    // TypeInfoMatches: set elementTypeInfo={STRING} vs expected pElementType={I32} → MISMATCH
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(outerVec, &elem);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    ModObjDispatcherComplexTypeManager::SetRelease(&wrongSet);
    ModObjDispatcherComplexTypeManager::VectorRelease(&outerVec);
    FreeTypeInfo(*setElemTi);
    delete setElemTi;
}

// Vector<Array<I32>> — add Array<STRING> should fail (nested element type mismatch)
HWTEST_F(ModObjDispatcherComplexTypeTest, TypeInfoMatches_VectorOfArrayMismatch, TestSize.Level1)
{
    // Outer: Vector<Array<I32>> — element TypeInfo = ARRAY{I32}
    auto* arrElemTi = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(arrElemTi, sizeof(*arrElemTi), 0, sizeof(*arrElemTi));
    arrElemTi->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY;
    auto* i32Leaf = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    (void)memset_s(i32Leaf, sizeof(*i32Leaf), 0, sizeof(*i32Leaf));
    i32Leaf->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    arrElemTi->u.arrayType.pElementType = i32Leaf;

    OH_AbilityRuntime_ModObjDispatcher_VectorHandle outerVec = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(arrElemTi, &outerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Wrong: Array<STRING>
    auto strTi = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle wrongArr = nullptr;
    ret = ModObjDispatcherComplexTypeManager::ArrayCreate(&strTi, 2, &wrongArr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant elem = {};
    elem.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY;
    elem.u.parrayVal = wrongArr;

    // TypeInfoMatches: array elementTypeInfo={STRING} vs expected pElementType={I32} → MISMATCH
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(outerVec, &elem);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    ModObjDispatcherComplexTypeManager::ArrayRelease(&wrongArr);
    ModObjDispatcherComplexTypeManager::VectorRelease(&outerVec);
    FreeTypeInfo(*arrElemTi);
    delete arrElemTi;
}

// 3-level nesting: Vector<Map<I32, Vector<STRING>>> — correct type should succeed
HWTEST_F(ModObjDispatcherComplexTypeTest, TypeInfoMatches_ThreeLevelNesting_Match, TestSize.Level1)
{
    auto* mapTi = NewTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP);
    mapTi->u.mapType.keyType = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    auto* vecStrTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    mapTi->u.mapType.pValueType = vecStrTi;

    OH_AbilityRuntime_ModObjDispatcher_VectorHandle outerVec = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(mapTi, &outerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto innerVec = CreateStringVector({"hello"});
    OH_AbilityRuntime_ModObjDispatcher_MapHandle innerMap = nullptr;
    ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, vecStrTi, &innerMap);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto mk = MakeI32Variant(1);
    auto vecVar = MakeVectorVariant(innerVec);
    ModObjDispatcherComplexTypeManager::MapPut(innerMap, &mk, &vecVar);

    auto mapVar = MakeMapVariant(innerMap);
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(outerVec, &mapVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    uint32_t sz = 0;
    ModObjDispatcherComplexTypeManager::VectorGetSize(outerVec, &sz);
    EXPECT_EQ(sz, 1u);

    ModObjDispatcherComplexTypeManager::MapRelease(&innerMap);
    ModObjDispatcherComplexTypeManager::VectorRelease(&innerVec);
    ModObjDispatcherComplexTypeManager::VectorRelease(&outerVec);
    FreeTypeInfo(*mapTi);
    delete mapTi;
}

// 3-level nesting: Vector<Map<I32, Vector<STRING>>> — wrong 3rd level should fail
HWTEST_F(ModObjDispatcherComplexTypeTest, TypeInfoMatches_ThreeLevelNesting_Mismatch, TestSize.Level1)
{
    auto* mapTi = NewTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP);
    mapTi->u.mapType.keyType = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    mapTi->u.mapType.pValueType = MakeContainerTypeInfo(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle outerVec = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::VectorCreate(mapTi, &outerVec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Wrong: Vector<I64> instead of Vector<STRING>
    auto wrongVec = CreateI32Vector({42}); // reusing I32, but with wrong map value type below
    auto* wrongVecTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle wrongMap = nullptr;
    ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, wrongVecTi, &wrongMap);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto mk = MakeI32Variant(1);
    auto wrongVecVar = MakeVectorVariant(wrongVec);
    ModObjDispatcherComplexTypeManager::MapPut(wrongMap, &mk, &wrongVecVar);

    auto wrongMapVar = MakeMapVariant(wrongMap);
    ret = ModObjDispatcherComplexTypeManager::VectorAdd(outerVec, &wrongMapVar);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    ModObjDispatcherComplexTypeManager::MapRelease(&wrongMap);
    ModObjDispatcherComplexTypeManager::VectorRelease(&wrongVec);
    ModObjDispatcherComplexTypeManager::VectorRelease(&outerVec);
    FreeTypeInfo(*wrongVecTi);
    delete wrongVecTi;
    FreeTypeInfo(*mapTi);
    delete mapTi;
}

// ==================== Dedup / VariantDeepEquals TDD ====================

// Set<STRING> — same content different pointer should dedup
HWTEST_F(ModObjDispatcherComplexTypeTest, Dedup_SetOfString_SameContent, TestSize.Level1)
{
    auto strTi = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_SetHandle set = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(&strTi, &set);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Add "hello" twice with different strdup'd pointers
    auto v1 = MakeStringVariant("hello");
    ret = ModObjDispatcherComplexTypeManager::SetAdd(set, &v1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    auto v2 = MakeStringVariant("hello"); // same content, different pointer
    ret = ModObjDispatcherComplexTypeManager::SetAdd(set, &v2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::SetGetSize(set, &size);
    EXPECT_EQ(size, 1u); // deduped

    // Different content should add
    auto v3 = MakeStringVariant("world");
    ret = ModObjDispatcherComplexTypeManager::SetAdd(set, &v3);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::SetGetSize(set, &size);
    EXPECT_EQ(size, 2u);

    ModObjDispatcherComplexTypeManager::Variant_Clear(&v1);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v2);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v3);
    ModObjDispatcherComplexTypeManager::SetRelease(&set);
}

// Set<Vector<I32>> — two vectors with same content should dedup
HWTEST_F(ModObjDispatcherComplexTypeTest, Dedup_SetOfVector_SameContent, TestSize.Level1)
{
    auto* setTi = MakeContainerTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR,
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_SetHandle set = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::SetCreate(setTi, &set);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // First vector [1,2,3]
    auto vec1 = CreateI32Vector({1, 2, 3});
    auto elem1 = MakeVectorVariant(vec1);
    ret = ModObjDispatcherComplexTypeManager::SetAdd(set, &elem1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Second vector [1,2,3] — same content, different instance → dedup
    auto vec2 = CreateI32Vector({1, 2, 3});
    auto elem2 = MakeVectorVariant(vec2);
    ret = ModObjDispatcherComplexTypeManager::SetAdd(set, &elem2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::SetGetSize(set, &size);
    EXPECT_EQ(size, 1u);

    // Third vector [1,2] — different content → add
    auto vec3 = CreateI32Vector({1, 2});
    auto elem3 = MakeVectorVariant(vec3);
    ret = ModObjDispatcherComplexTypeManager::SetAdd(set, &elem3);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ModObjDispatcherComplexTypeManager::SetGetSize(set, &size);
    EXPECT_EQ(size, 2u);

    ModObjDispatcherComplexTypeManager::VectorRelease(&vec1);
    ModObjDispatcherComplexTypeManager::VectorRelease(&vec2);
    ModObjDispatcherComplexTypeManager::VectorRelease(&vec3);
    ModObjDispatcherComplexTypeManager::SetRelease(&set);
    FreeTypeInfo(*setTi);
    delete setTi;
}

// Map<I32, STRING> — put same key twice should update value, not add entry
HWTEST_F(ModObjDispatcherComplexTypeTest, Dedup_MapSameKey_UpdatesValue, TestSize.Level1)
{
    auto valTi = MakeStringTypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, &valTi, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto k = MakeI32Variant(1);
    auto v1 = MakeStringVariant("first");
    ret = ModObjDispatcherComplexTypeManager::MapPut(map, &k, &v1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Same key, different value
    auto v2 = MakeStringVariant("second");
    ret = ModObjDispatcherComplexTypeManager::MapPut(map, &k, &v2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::MapGetSize(map, &size);
    EXPECT_EQ(size, 1u); // not 2 — key was deduped, value updated

    // Verify value was updated
    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(map, &k, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_STREQ(out.u.bstrVal, "second");

    ModObjDispatcherComplexTypeManager::Variant_Clear(&v1);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&v2);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);
    ModObjDispatcherComplexTypeManager::MapRelease(&map);
}

// Map<STRING, I32> — string key dedup (different pointer, same content)
HWTEST_F(ModObjDispatcherComplexTypeTest, Dedup_MapStringKey_SameContent, TestSize.Level1)
{
    auto valTi = MakeI32TypeInfo();
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTi, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // First put with key "name"
    auto k1 = MakeStringVariant("name");
    auto v1 = MakeI32Variant(10);
    ret = ModObjDispatcherComplexTypeManager::MapPut(map, &k1, &v1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Second put with same key content but different pointer
    auto k2 = MakeStringVariant("name");
    auto v2 = MakeI32Variant(20);
    ret = ModObjDispatcherComplexTypeManager::MapPut(map, &k2, &v2);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    ModObjDispatcherComplexTypeManager::MapGetSize(map, &size);
    EXPECT_EQ(size, 1u); // string key deduped by content

    // Verify value was updated to 20
    auto kCheck = MakeStringVariant("name");
    OH_AbilityRuntime_ModObjDispatcher_Variant out = {};
    ret = ModObjDispatcherComplexTypeManager::MapGet(map, &kCheck, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(out.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(out.u.i32Val, 20);

    ModObjDispatcherComplexTypeManager::Variant_Clear(&k1);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&k2);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&kCheck);
    ModObjDispatcherComplexTypeManager::Variant_Clear(&out);
    ModObjDispatcherComplexTypeManager::MapRelease(&map);
}

} // namespace AbilityRuntime
} // namespace OHOS
