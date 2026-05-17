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

// Helper: register a simple struct metadata for testing
static void RegisterTestStructMetadata()
{
    std::vector<MoStructMeta> structs;
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
    ModObjDispatcherComplexTypeManager::RegisterStructMetadata(structs);
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
    OH_AbilityRuntime_ModObjDispatcher_StructHandle handle = nullptr;
    auto ret = ModObjDispatcherComplexTypeManager::StructCreate("MyStruct", &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(handle, nullptr);

    ModObjDispatcherComplexTypeManager::StructRelease(&handle);
    EXPECT_EQ(handle, nullptr);
}

HWTEST_F(ModObjDispatcherComplexTypeTest, StructGetName_Success, TestSize.Level1)
{
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

} // namespace AbilityRuntime
} // namespace OHOS
