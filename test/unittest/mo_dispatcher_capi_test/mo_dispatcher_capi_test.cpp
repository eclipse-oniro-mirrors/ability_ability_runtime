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
#include <cstring>

#include "modular_object_dispatcher.h"
#include "mo_dispatcher_types.h"

using namespace testing::ext;

namespace {
// Helper: create a simple TypeInfo for a basic type (e.g. I32, String)
OH_AbilityRuntime_ModObjDispatcher_TypeInfo MakeBasicTypeInfo(
    OH_AbilityRuntime_ModObjDispatcher_ValueType vt)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo info = {};
    info.vt = vt;
    return info;
}

// Helper: create a Variant of type I32
OH_AbilityRuntime_ModObjDispatcher_Variant MakeI32Variant(int32_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    v.u.i32Val = val;
    return v;
}

// Helper: create a Variant of type String (allocates heap memory via strdup)
OH_AbilityRuntime_ModObjDispatcher_Variant MakeStringVariant(const char* val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    v.u.bstrVal = strdup(val);
    return v;
}

// Helper: create a Variant of type Bool
OH_AbilityRuntime_ModObjDispatcher_Variant MakeBoolVariant(bool val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL;
    v.u.boolVal = val;
    return v;
}

// Helper: create a Variant of type F64
OH_AbilityRuntime_ModObjDispatcher_Variant MakeF64Variant(double val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64;
    v.u.f64Val = val;
    return v;
}
} // namespace

class MoDispatcherCapiTest : public testing::Test {
public:
    MoDispatcherCapiTest() = default;
    ~MoDispatcherCapiTest() override = default;
    void SetUp() override {}
    void TearDown() override {}
};

// ===================== Instance Management Null Param Tests =====================

/**
 * @tc.name: CreateMainServiceInstance_NullProxy_0100
 * @tc.desc: Test CreateMainServiceInstance with null proxy returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, CreateMainServiceInstance_NullProxy_0100, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcherHandle handle = nullptr;
    auto ret = OH_AbilityRuntime_ModObjDispatcher_CreateMainServiceInstance(nullptr, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: CreateMainServiceInstance_NullOutHandle_0200
 * @tc.desc: Test CreateMainServiceInstance with null output handle pointer returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, CreateMainServiceInstance_NullOutHandle_0200, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjDispatcher_CreateMainServiceInstance(
        reinterpret_cast<OHIPCRemoteProxy*>(0x1), nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: CreateSubInstance_NullMainDispatcher_0300
 * @tc.desc: Test CreateSubInstance with null mainServiceDispatcher returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, CreateSubInstance_NullMainDispatcher_0300, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcherHandle handle = nullptr;
    auto ret = OH_AbilityRuntime_ModObjDispatcher_CreateSubInstance(
        nullptr, reinterpret_cast<OHIPCRemoteProxy*>(0x1), &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: CreateSubInstance_NullSubProxy_0400
 * @tc.desc: Test CreateSubInstance with null subProxy returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, CreateSubInstance_NullSubProxy_0400, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcherHandle handle = nullptr;
    auto ret = OH_AbilityRuntime_ModObjDispatcher_CreateSubInstance(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcherHandle>(0x1), nullptr, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: CreateSubInstance_NullOutHandle_0500
 * @tc.desc: Test CreateSubInstance with null output handle returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, CreateSubInstance_NullOutHandle_0500, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjDispatcher_CreateSubInstance(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcherHandle>(0x1),
        reinterpret_cast<OHIPCRemoteProxy*>(0x1), nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: Release_NullPointer_0600
 * @tc.desc: Test Release with null pointer does not crash.
 */
HWTEST_F(MoDispatcherCapiTest, Release_NullPointer_0600, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_Release(nullptr);
    // Should not crash
    EXPECT_TRUE(true);
}

/**
 * @tc.name: Release_NullHandle_0700
 * @tc.desc: Test Release with null handle pointer does not crash.
 */
HWTEST_F(MoDispatcherCapiTest, Release_NullHandle_0700, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcherHandle handle = nullptr;
    OH_AbilityRuntime_ModObjDispatcher_Release(&handle);
    EXPECT_EQ(handle, nullptr);
}

// ===================== HasTypeDescriptor / GetTypeDescriptor Null Param Tests =====================

/**
 * @tc.name: HasTypeDescriptor_NullDispatcher_0800
 * @tc.desc: Test HasTypeDescriptor with null dispatcher returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, HasTypeDescriptor_NullDispatcher_0800, TestSize.Level1)
{
    uint32_t countInfo = 0;
    auto ret = OH_AbilityRuntime_ModObjDispatcher_HasTypeDescriptor(nullptr, &countInfo);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: HasTypeDescriptor_NullOutParam_0900
 * @tc.desc: Test HasTypeDescriptor with null output returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, HasTypeDescriptor_NullOutParam_0900, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjDispatcher_HasTypeDescriptor(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcherHandle>(0x1), nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: GetTypeDescriptor_NullDispatcher_1000
 * @tc.desc: Test GetTypeDescriptor with null dispatcher returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, GetTypeDescriptor_NullDispatcher_1000, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle desc = nullptr;
    auto ret = OH_AbilityRuntime_ModObjDispatcher_GetTypeDescriptor(nullptr, &desc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: GetTypeDescriptor_NullOutParam_1100
 * @tc.desc: Test GetTypeDescriptor with null output returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, GetTypeDescriptor_NullOutParam_1100, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjDispatcher_GetTypeDescriptor(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcherHandle>(0x1), nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ===================== QueryMemIDsOfNames Null Param Tests =====================

/**
 * @tc.name: QueryMemIDsOfNames_NullDispatcher_1200
 * @tc.desc: Test QueryMainServiceInterfaceMemIDsOfNames with null dispatcher.
 */
HWTEST_F(MoDispatcherCapiTest, QueryMemIDsOfNames_NullDispatcher_1200, TestSize.Level1)
{
    const char* names[] = {"method1"};
    uint32_t memId = 0;
    auto ret = OH_AbilityRuntime_ModObjDispatcher_QueryMainServiceInterfaceMemIDsOfNames(
        nullptr, names, 1, &memId);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: QueryMemIDsOfNames_NullNames_1300
 * @tc.desc: Test QueryMainServiceInterfaceMemIDsOfNames with null names array.
 */
HWTEST_F(MoDispatcherCapiTest, QueryMemIDsOfNames_NullNames_1300, TestSize.Level1)
{
    uint32_t memId = 0;
    auto ret = OH_AbilityRuntime_ModObjDispatcher_QueryMainServiceInterfaceMemIDsOfNames(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcherHandle>(0x1), nullptr, 1, &memId);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: QueryMemIDsOfNames_NullMemId_1400
 * @tc.desc: Test QueryMainServiceInterfaceMemIDsOfNames with null memId output.
 */
HWTEST_F(MoDispatcherCapiTest, QueryMemIDsOfNames_NullMemId_1400, TestSize.Level1)
{
    const char* names[] = {"method1"};
    auto ret = OH_AbilityRuntime_ModObjDispatcher_QueryMainServiceInterfaceMemIDsOfNames(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcherHandle>(0x1), names, 1, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ===================== CallMethod Null Param Tests =====================

/**
 * @tc.name: CallMethod_NullDispatcher_1500
 * @tc.desc: Test CallMethod with null dispatcher returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, CallMethod_NullDispatcher_1500, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_InputParams input = {};
    OH_AbilityRuntime_ModObjDispatcher_Variant result = {};
    auto ret = OH_AbilityRuntime_ModObjDispatcher_CallMethod(nullptr, 1, &input, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: CallMethod_NullInputParams_1600
 * @tc.desc: Test CallMethod with null input params returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, CallMethod_NullInputParams_1600, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant result = {};
    auto ret = OH_AbilityRuntime_ModObjDispatcher_CallMethod(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcherHandle>(0x1), 1, nullptr, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: CallMethod_NullResult_1700
 * @tc.desc: Test CallMethod with null result returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, CallMethod_NullResult_1700, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_InputParams input = {};
    auto ret = OH_AbilityRuntime_ModObjDispatcher_CallMethod(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcherHandle>(0x1), 1, &input, nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ===================== TypeDescriptor Null Param Tests =====================

/**
 * @tc.name: TypeDescriptor_Release_NullPointer_1800
 * @tc.desc: Test TypeDescriptor_Release with null pointer is safe.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_Release_NullPointer_1800, TestSize.Level1)
{
    OH_AbilityRuntime_TypeDescriptor_Release(nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: TypeDescriptor_Release_NullHandle_1900
 * @tc.desc: Test TypeDescriptor_Release with null handle is safe.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_Release_NullHandle_1900, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle handle = nullptr;
    OH_AbilityRuntime_TypeDescriptor_Release(&handle);
    EXPECT_EQ(handle, nullptr);
}

/**
 * @tc.name: TypeDescriptor_GetVersion_NullParams_2000
 * @tc.desc: Test GetVersion with all null params returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetVersion_NullParams_2000, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetVersion(nullptr, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetVersion(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetVersion(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetInterfaceCount_NullParams_2100
 * @tc.desc: Test GetInterfaceCount with null params returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetInterfaceCount_NullParams_2100, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetInterfaceName_NullParams_2200
 * @tc.desc: Test GetInterfaceName with null params returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetInterfaceName_NullParams_2200, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(nullptr, 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), 0, nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), 0, buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetInterfaceIsCallback_NullParams_2300
 * @tc.desc: Test GetInterfaceIsCallback with null params returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetInterfaceIsCallback_NullParams_2300, TestSize.Level1)
{
    bool isCallback = false;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(nullptr, "iface", &isCallback),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, &isCallback),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetMainServiceInterfaceName_NullParams_2400
 * @tc.desc: Test GetMainServiceInterfaceName with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetMainServiceInterfaceName_NullParams_2400, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(nullptr, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetMethodCount_NullParams_2500
 * @tc.desc: Test GetMethodCount with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetMethodCount_NullParams_2500, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodCount(nullptr, "iface", &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetMethodName_NullParams_2600
 * @tc.desc: Test GetMethodName with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetMethodName_NullParams_2600, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodName(nullptr, "iface", 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", 0, nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", 0, buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetMethodMemberId_NullParams_2700
 * @tc.desc: Test GetMethodMemberId with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetMethodMemberId_NullParams_2700, TestSize.Level1)
{
    uint32_t memId = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(nullptr, "iface", "method", &memId),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, "method", &memId),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", nullptr, &memId),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", "method", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetMethodReturnType_NullParams_2800
 * @tc.desc: Test GetMethodReturnType with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetMethodReturnType_NullParams_2800, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo typeInfo = {};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(nullptr, "iface", "method", &typeInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, "method", &typeInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", nullptr, &typeInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", "method", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetMethodParamCount_NullParams_2900
 * @tc.desc: Test GetMethodParamCount with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetMethodParamCount_NullParams_2900, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(nullptr, "iface", "method", &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, "method", &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", "method", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetMethodParamType_NullParams_3000
 * @tc.desc: Test GetMethodParamType with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetMethodParamType_NullParams_3000, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo typeInfo = {};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
        nullptr, "iface", "method", 0, &typeInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, "method", 0, &typeInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", nullptr, 0, &typeInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", "method", 0, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetMethodParamName_NullParams_3100
 * @tc.desc: Test GetMethodParamName with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetMethodParamName_NullParams_3100, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
        nullptr, "iface", "method", 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, "method", 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", nullptr, 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", "method", 0, nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "iface", "method", 0, buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ===================== Enum Query Null Param Tests =====================

/**
 * @tc.name: TypeDescriptor_GetEnumCount_NullParams_3200
 * @tc.desc: Test GetEnumCount with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetEnumCount_NullParams_3200, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumCount(nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetEnumName_NullParams_3300
 * @tc.desc: Test GetEnumName with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetEnumName_NullParams_3300, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumName(nullptr, 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), 0, nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), 0, buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetEnumValueCount_NullParams_3400
 * @tc.desc: Test GetEnumValueCount with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetEnumValueCount_NullParams_3400, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValueCount(nullptr, "MyEnum", &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValueCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValueCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyEnum", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetEnumValueName_NullParams_3500
 * @tc.desc: Test GetEnumValueName with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetEnumValueName_NullParams_3500, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(nullptr, "MyEnum", 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyEnum", 0, nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyEnum", 0, buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetEnumValue_NullParams_3600
 * @tc.desc: Test GetEnumValue with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetEnumValue_NullParams_3600, TestSize.Level1)
{
    int32_t val = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValue(nullptr, "MyEnum", "VAL1", &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValue(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, "VAL1", &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValue(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyEnum", nullptr, &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetEnumValue(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyEnum", "VAL1", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ===================== Struct Query Null Param Tests =====================

/**
 * @tc.name: TypeDescriptor_GetStructCount_NullParams_3700
 * @tc.desc: Test GetStructCount with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetStructCount_NullParams_3700, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructCount(nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetStructName_NullParams_3800
 * @tc.desc: Test GetStructName with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetStructName_NullParams_3800, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructName(nullptr, 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), 0, nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), 0, buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetStructFieldCount_NullParams_3900
 * @tc.desc: Test GetStructFieldCount with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetStructFieldCount_NullParams_3900, TestSize.Level1)
{
    uint32_t count = 0;
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(nullptr, "MyStruct", &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, &count),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyStruct", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetStructFieldName_NullParams_4000
 * @tc.desc: Test GetStructFieldName with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetStructFieldName_NullParams_4000, TestSize.Level1)
{
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(nullptr, "MyStruct", 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, 0, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyStruct", 0, nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyStruct", 0, buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

/**
 * @tc.name: TypeDescriptor_GetStructFieldType_NullParams_4100
 * @tc.desc: Test GetStructFieldType with null params.
 */
HWTEST_F(MoDispatcherCapiTest, TypeDescriptor_GetStructFieldType_NullParams_4100, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo typeInfo = {};
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(nullptr, "MyStruct", "field1", &typeInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), nullptr, "field1", &typeInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyStruct", nullptr, &typeInfo),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle>(0x1), "MyStruct", "field1", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ===================== VariantClear / TypeInfoClear Null Safety =====================

/**
 * @tc.name: VariantClear_NullParam_4200
 * @tc.desc: Test VariantClear with null pointer does not crash.
 */
HWTEST_F(MoDispatcherCapiTest, VariantClear_NullParam_4200, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_VariantClear(nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: TypeInfoClear_NullParam_4300
 * @tc.desc: Test TypeInfoClear with null pointer does not crash.
 */
HWTEST_F(MoDispatcherCapiTest, TypeInfoClear_NullParam_4300, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfoClear(nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: VariantClear_ClearsStringType_4400
 * @tc.desc: Test VariantClear properly frees string variant and resets to VT_EMPTY.
 */
HWTEST_F(MoDispatcherCapiTest, VariantClear_ClearsStringType_4400, TestSize.Level1)
{
    auto v = MakeStringVariant("hello");
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_NE(v.u.bstrVal, nullptr);

    OH_AbilityRuntime_ModObjDispatcher_VariantClear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.bstrVal, nullptr);
}

/**
 * @tc.name: VariantClear_ClearsSimpleType_4500
 * @tc.desc: Test VariantClear on a simple I32 type resets to VT_EMPTY.
 */
HWTEST_F(MoDispatcherCapiTest, VariantClear_ClearsSimpleType_4500, TestSize.Level1)
{
    auto v = MakeI32Variant(42);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    OH_AbilityRuntime_ModObjDispatcher_VariantClear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
}

/**
 * @tc.name: TypeInfoClear_ClearsIdlType_4600
 * @tc.desc: Test TypeInfoClear properly frees idlType string.
 */
HWTEST_F(MoDispatcherCapiTest, TypeInfoClear_ClearsIdlType_4600, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo info = {};
    info.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT;
    info.u.idlType = strdup("MyStruct");
    ASSERT_NE(info.u.idlType, nullptr);

    OH_AbilityRuntime_ModObjDispatcher_TypeInfoClear(&info);
    EXPECT_EQ(info.u.idlType, nullptr);
}

// ===================== Array C API Success + Null Tests =====================

/**
 * @tc.name: Array_CreateSetGetRelease_4700
 * @tc.desc: Test full lifecycle: create array of I32, set elements, get elements, release.
 */
HWTEST_F(MoDispatcherCapiTest, Array_CreateSetGetRelease_4700, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle array = nullptr;

    auto ret = OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(&typeInfo, 3, &array);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(array, nullptr);

    // Set elements
    for (uint32_t i = 0; i < 3; i++) {
        auto val = MakeI32Variant(static_cast<int32_t>(i * 10));
        ret = OH_AbilityRuntime_ModObjDispatcher_ArraySet(array, i, &val);
        EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    }

    // Get size
    uint32_t size = 0;
    ret = OH_AbilityRuntime_ModObjDispatcher_ArrayGetSize(array, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 3u);

    // Get elements
    for (uint32_t i = 0; i < 3; i++) {
        OH_AbilityRuntime_ModObjDispatcher_Variant val = {};
        ret = OH_AbilityRuntime_ModObjDispatcher_ArrayGet(array, i, &val);
        EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(val.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
        EXPECT_EQ(val.u.i32Val, static_cast<int32_t>(i * 10));
    }

    // Get element type
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    ret = OH_AbilityRuntime_ModObjDispatcher_ArrayGetElementType(array, &elemType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(elemType.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // Release
    OH_AbilityRuntime_ModObjDispatcher_ArrayRelease(&array);
    EXPECT_EQ(array, nullptr);
}

/**
 * @tc.name: Array_NullParams_4800
 * @tc.desc: Test Array C APIs with null parameters.
 */
HWTEST_F(MoDispatcherCapiTest, Array_NullParams_4800, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // ArrayCreate null params
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle array = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(nullptr, 3, &array),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(&typeInfo, 3, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // ArrayGetElementType null params
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGetElementType(nullptr, &elemType),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGetElementType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_ArrayHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // ArraySet null params
    auto val = MakeI32Variant(1);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArraySet(nullptr, 0, &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArraySet(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_ArrayHandle>(0x1), 0, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // ArrayGet null params
    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGet(nullptr, 0, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGet(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_ArrayHandle>(0x1), 0, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // ArrayGetSize null params
    uint32_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGetSize(nullptr, &size),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGetSize(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_ArrayHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // ArrayRelease null safety
    OH_AbilityRuntime_ModObjDispatcher_ArrayRelease(nullptr);
    OH_AbilityRuntime_ModObjDispatcher_ArrayRelease(&array); // array is nullptr already
    EXPECT_TRUE(true);
}

/**
 * @tc.name: Array_OutOfBounds_4900
 * @tc.desc: Test ArraySet/ArrayGet with out-of-bounds index.
 */
HWTEST_F(MoDispatcherCapiTest, Array_OutOfBounds_4900, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle array = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(&typeInfo, 2, &array),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto val = MakeI32Variant(99);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArraySet(array, 5, &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGet(array, 5, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ModObjDispatcher_ArrayRelease(&array);
}

/**
 * @tc.name: Array_TypeMismatch_5000
 * @tc.desc: Test ArraySet with wrong element type returns TYPE_MISMATCH.
 */
HWTEST_F(MoDispatcherCapiTest, Array_TypeMismatch_5000, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle array = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(&typeInfo, 2, &array),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto strVal = MakeStringVariant("wrong");
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArraySet(array, 0, &strVal),
        ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    std::free(strVal.u.bstrVal);
    OH_AbilityRuntime_ModObjDispatcher_ArrayRelease(&array);
}

// ===================== Vector C API Success + Null Tests =====================

/**
 * @tc.name: Vector_CreateAddGetClearRelease_5100
 * @tc.desc: Test full lifecycle of vector: create, add, get, size, clear, release.
 */
HWTEST_F(MoDispatcherCapiTest, Vector_CreateAddGetClearRelease_5100, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vec = nullptr;

    auto ret = OH_AbilityRuntime_ModObjDispatcher_VectorCreate(&typeInfo, &vec);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(vec, nullptr);

    // Add elements
    for (int32_t i = 0; i < 5; i++) {
        auto val = MakeI32Variant(i * 100);
        ret = OH_AbilityRuntime_ModObjDispatcher_VectorAdd(vec, &val);
        EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    }

    // Check size
    uint32_t size = 0;
    ret = OH_AbilityRuntime_ModObjDispatcher_VectorGetSize(vec, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 5u);

    // Get elements
    for (uint32_t i = 0; i < 5; i++) {
        OH_AbilityRuntime_ModObjDispatcher_Variant val = {};
        ret = OH_AbilityRuntime_ModObjDispatcher_VectorGet(vec, i, &val);
        EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
        EXPECT_EQ(val.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
        EXPECT_EQ(val.u.i32Val, static_cast<int32_t>(i * 100));
    }

    // Get element type
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    ret = OH_AbilityRuntime_ModObjDispatcher_VectorGetElementType(vec, &elemType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(elemType.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // Clear
    ret = OH_AbilityRuntime_ModObjDispatcher_VectorClear(vec);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ret = OH_AbilityRuntime_ModObjDispatcher_VectorGetSize(vec, &size);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 0u);

    // Release
    OH_AbilityRuntime_ModObjDispatcher_VectorRelease(&vec);
    EXPECT_EQ(vec, nullptr);
}

/**
 * @tc.name: Vector_NullParams_5200
 * @tc.desc: Test Vector C APIs with null parameters.
 */
HWTEST_F(MoDispatcherCapiTest, Vector_NullParams_5200, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // VectorCreate null params
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vec = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorCreate(nullptr, &vec),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorCreate(&typeInfo, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // VectorGetElementType null params
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorGetElementType(nullptr, &elemType),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorGetElementType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_VectorHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // VectorAdd null params
    auto val = MakeI32Variant(1);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorAdd(nullptr, &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorAdd(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_VectorHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // VectorGet null params
    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorGet(nullptr, 0, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorGet(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_VectorHandle>(0x1), 0, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // VectorGetSize null params
    uint32_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorGetSize(nullptr, &size),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorGetSize(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_VectorHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // VectorClear null
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorClear(nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // VectorRelease null safety
    OH_AbilityRuntime_ModObjDispatcher_VectorRelease(nullptr);
    OH_AbilityRuntime_ModObjDispatcher_VectorRelease(&vec);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: Vector_TypeMismatch_5300
 * @tc.desc: Test VectorAdd with wrong element type.
 */
HWTEST_F(MoDispatcherCapiTest, Vector_TypeMismatch_5300, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vec = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorCreate(&typeInfo, &vec),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto boolVal = MakeBoolVariant(true);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorAdd(vec, &boolVal),
        ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    OH_AbilityRuntime_ModObjDispatcher_VectorRelease(&vec);
}

// ===================== Set C API Success + Null Tests =====================

/**
 * @tc.name: Set_CreateAddContainsRemoveRelease_5400
 * @tc.desc: Test full lifecycle of set: create, add, contains, remove, getAt, clear, release.
 */
HWTEST_F(MoDispatcherCapiTest, Set_CreateAddContainsRemoveRelease_5400, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_SetHandle set = nullptr;

    auto ret = OH_AbilityRuntime_ModObjDispatcher_SetCreate(&typeInfo, &set);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(set, nullptr);

    // Add elements
    auto v1 = MakeI32Variant(10);
    auto v2 = MakeI32Variant(20);
    auto v3 = MakeI32Variant(30);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetAdd(set, &v1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetAdd(set, &v2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetAdd(set, &v3), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Duplicate add should succeed (no-op, returns NO_ERROR)
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetAdd(set, &v1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Check size (should be 3, not 4)
    uint32_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetSize(set, &size), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 3u);

    // Contains
    bool exists = false;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetContains(set, &v1, &exists), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(exists);

    auto vNotFound = MakeI32Variant(999);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetContains(set, &vNotFound, &exists), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(exists);

    // GetAt
    OH_AbilityRuntime_ModObjDispatcher_Variant atVal = {};
    ret = OH_AbilityRuntime_ModObjDispatcher_SetGetAt(set, 0, &atVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(atVal.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // Get element type
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    ret = OH_AbilityRuntime_ModObjDispatcher_SetGetElementType(set, &elemType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(elemType.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // Remove
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetRemove(set, &v2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetSize(set, &size), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 2u);

    // Contains after remove
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetContains(set, &v2, &exists), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(exists);

    // Clear
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetClear(set), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetSize(set, &size), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 0u);

    // Release
    OH_AbilityRuntime_ModObjDispatcher_SetRelease(&set);
    EXPECT_EQ(set, nullptr);
}

/**
 * @tc.name: Set_NullParams_5500
 * @tc.desc: Test Set C APIs with null parameters.
 */
HWTEST_F(MoDispatcherCapiTest, Set_NullParams_5500, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // SetCreate null params
    OH_AbilityRuntime_ModObjDispatcher_SetHandle s = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetCreate(nullptr, &s),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetCreate(&typeInfo, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // SetGetElementType null
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo elemType = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetElementType(nullptr, &elemType),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetElementType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_SetHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // SetAdd null
    auto val = MakeI32Variant(1);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetAdd(nullptr, &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetAdd(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_SetHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // SetRemove null
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetRemove(nullptr, &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetRemove(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_SetHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // SetContains null
    bool exists = false;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetContains(nullptr, &val, &exists),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetContains(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_SetHandle>(0x1), nullptr, &exists),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetContains(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_SetHandle>(0x1), &val, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // SetGetSize null
    uint32_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetSize(nullptr, &size),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetSize(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_SetHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // SetGetAt null
    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetAt(nullptr, 0, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetAt(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_SetHandle>(0x1), 0, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // SetClear null
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetClear(nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // SetRelease null safety
    OH_AbilityRuntime_ModObjDispatcher_SetRelease(nullptr);
    OH_AbilityRuntime_ModObjDispatcher_SetRelease(&s);
    EXPECT_TRUE(true);
}

// ===================== Map C API Success + Null Tests =====================

/**
 * @tc.name: Map_CreatePutGetRemoveRelease_5600
 * @tc.desc: Test full lifecycle of map: create, put, get, containsKey, remove, keyAt, valueAt, release.
 */
HWTEST_F(MoDispatcherCapiTest, Map_CreatePutGetRemoveRelease_5600, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;

    auto ret = OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTypeInfo, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(map, nullptr);

    // Put entries
    auto k1 = MakeStringVariant("key1");
    auto v1 = MakeI32Variant(100);
    auto k2 = MakeStringVariant("key2");
    auto v2 = MakeI32Variant(200);
    auto k3 = MakeStringVariant("key3");
    auto v3 = MakeI32Variant(300);

    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(map, &k1, &v1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(map, &k2, &v2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(map, &k3, &v3), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Get key type and value type
    OH_AbilityRuntime_ModObjDispatcher_ValueType keyType = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
    ret = OH_AbilityRuntime_ModObjDispatcher_MapGetKeyType(map, &keyType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(keyType, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);

    OH_AbilityRuntime_ModObjDispatcher_TypeInfo valueType = {};
    ret = OH_AbilityRuntime_ModObjDispatcher_MapGetValueType(map, &valueType);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(valueType.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // Check size
    uint32_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetSize(map, &size), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 3u);

    // ContainsKey
    bool exists = false;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapContainsKey(map, &k1, &exists),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(exists);

    auto kNotFound = MakeStringVariant("nokey");
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapContainsKey(map, &kNotFound, &exists),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(exists);

    // Get value
    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGet(map, &k1, &outVal), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(outVal.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(outVal.u.i32Val, 100);

    // MapGetKeyAt / MapGetValueAt
    OH_AbilityRuntime_ModObjDispatcher_Variant keyAt = {};
    ret = OH_AbilityRuntime_ModObjDispatcher_MapGetKeyAt(map, 0, &keyAt);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(keyAt.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_NE(keyAt.u.bstrVal, nullptr);
    OH_AbilityRuntime_ModObjDispatcher_VariantClear(&keyAt);

    OH_AbilityRuntime_ModObjDispatcher_Variant valAt = {};
    ret = OH_AbilityRuntime_ModObjDispatcher_MapGetValueAt(map, 0, &valAt);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(valAt.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // Remove
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapRemove(map, &k2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetSize(map, &size), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 2u);

    // Clear
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapClear(map), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetSize(map, &size), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 0u);

    // Release
    OH_AbilityRuntime_ModObjDispatcher_MapRelease(&map);
    EXPECT_EQ(map, nullptr);

    // Clean up string variants
    std::free(k1.u.bstrVal);
    std::free(k2.u.bstrVal);
    std::free(k3.u.bstrVal);
    std::free(kNotFound.u.bstrVal);
    // outVal is I32 (vt=I32, i32Val=100), not STRING — no bstrVal to free
}

/**
 * @tc.name: Map_NullParams_5700
 * @tc.desc: Test Map C APIs with null parameters.
 */
HWTEST_F(MoDispatcherCapiTest, Map_NullParams_5700, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // MapCreate null params
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, nullptr, &map),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTypeInfo, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapGetKeyType null
    OH_AbilityRuntime_ModObjDispatcher_ValueType keyType = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetKeyType(nullptr, &keyType),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetKeyType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapGetValueType null
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo vtype = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetValueType(nullptr, &vtype),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetValueType(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapPut null
    auto k = MakeStringVariant("k");
    auto v = MakeI32Variant(1);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(nullptr, &k, &v),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), nullptr, &v),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), &k, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapGet null
    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGet(nullptr, &k, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGet(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), nullptr, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGet(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), &k, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapRemove null
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapRemove(nullptr, &k),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapRemove(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapContainsKey null
    bool exists = false;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapContainsKey(nullptr, &k, &exists),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapContainsKey(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), nullptr, &exists),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapContainsKey(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), &k, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapGetSize null
    uint32_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetSize(nullptr, &size),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetSize(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapGetKeyAt null
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetKeyAt(nullptr, 0, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetKeyAt(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), 0, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapGetValueAt null
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetValueAt(nullptr, 0, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetValueAt(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_MapHandle>(0x1), 0, nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapClear null
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapClear(nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // MapRelease null safety
    OH_AbilityRuntime_ModObjDispatcher_MapRelease(nullptr);
    OH_AbilityRuntime_ModObjDispatcher_MapRelease(&map);
    EXPECT_TRUE(true);

    std::free(k.u.bstrVal);
}

/**
 * @tc.name: Map_InvalidKeyType_5800
 * @tc.desc: Test MapCreate with invalid key type (e.g. MAP as key) returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, Map_InvalidKeyType_5800, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    auto ret = OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP, &valTypeInfo, &map);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ===================== Struct C API Success + Null Tests =====================

/**
 * @tc.name: Struct_CreateGetNameSetGetRelease_5900
 * @tc.desc: Test full lifecycle of struct: create, get name, set field, get field, release.
 */
HWTEST_F(MoDispatcherCapiTest, Struct_CreateGetNameSetGetRelease_5900, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_StructHandle st = nullptr;

    auto ret = OH_AbilityRuntime_ModObjDispatcher_StructCreate("TestStruct", &st);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(st, nullptr);

    // Get name
    char name[64] = {0};
    ret = OH_AbilityRuntime_ModObjDispatcher_StructGetName(st, name, sizeof(name));
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_STREQ(name, "TestStruct");

    // SetField with an unknown field returns PROPERTY_NOT_FOUND (no registered metadata)
    auto val = MakeI32Variant(42);
    ret = OH_AbilityRuntime_ModObjDispatcher_StructSetField(st, "unknownField", &val);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    // GetField on unknown field returns PROPERTY_NOT_FOUND
    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    ret = OH_AbilityRuntime_ModObjDispatcher_StructGetField(st, "unknownField", &outVal);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    // Release
    OH_AbilityRuntime_ModObjDispatcher_StructRelease(&st);
    EXPECT_EQ(st, nullptr);
}

/**
 * @tc.name: Struct_NullParams_6000
 * @tc.desc: Test Struct C APIs with null parameters.
 */
HWTEST_F(MoDispatcherCapiTest, Struct_NullParams_6000, TestSize.Level1)
{
    // StructCreate null params
    OH_AbilityRuntime_ModObjDispatcher_StructHandle st = nullptr;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructCreate(nullptr, &st),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructCreate("name", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // StructGetName null params
    char buf[64] = {0};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructGetName(nullptr, buf, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructGetName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_StructHandle>(0x1), nullptr, sizeof(buf)),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructGetName(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_StructHandle>(0x1), buf, 0),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // StructSetField null params
    auto val = MakeI32Variant(1);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructSetField(nullptr, "field", &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructSetField(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_StructHandle>(0x1), nullptr, &val),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructSetField(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_StructHandle>(0x1), "field", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // StructGetField null params
    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructGetField(nullptr, "field", &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructGetField(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_StructHandle>(0x1), nullptr, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructGetField(
        reinterpret_cast<OH_AbilityRuntime_ModObjDispatcher_StructHandle>(0x1), "field", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    // StructRelease null safety
    OH_AbilityRuntime_ModObjDispatcher_StructRelease(nullptr);
    OH_AbilityRuntime_ModObjDispatcher_StructRelease(&st);
    EXPECT_TRUE(true);
}

// ===================== VariantClear with Container Types =====================

/**
 * @tc.name: VariantClear_ClearsArrayContainer_6100
 * @tc.desc: Test VariantClear properly releases an Array handle inside a Variant.
 */
HWTEST_F(MoDispatcherCapiTest, VariantClear_ClearsArrayContainer_6100, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle array = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(&typeInfo, 2, &array),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant v = {};
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY;
    v.u.parrayVal = array;

    // Clear should free the array
    OH_AbilityRuntime_ModObjDispatcher_VariantClear(&v);
    EXPECT_EQ(v.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(v.u.parrayVal, nullptr);
}

/**
 * @tc.name: VariantClear_ClearsMapContainer_6200
 * @tc.desc: Test VariantClear properly releases a Map handle inside a Variant.
 */
HWTEST_F(MoDispatcherCapiTest, VariantClear_ClearsMapContainer_6200, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTypeInfo, &map),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Add one entry
    auto k = MakeStringVariant("key");
    auto v = MakeI32Variant(123);
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(map, &k, &v), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant variant = {};
    variant.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP;
    variant.u.pmapVal = map;

    OH_AbilityRuntime_ModObjDispatcher_VariantClear(&variant);
    EXPECT_EQ(variant.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(variant.u.pmapVal, nullptr);

    std::free(k.u.bstrVal);
}

/**
 * @tc.name: VariantClear_ClearsVectorContainer_6300
 * @tc.desc: Test VariantClear properly releases a Vector handle inside a Variant.
 */
HWTEST_F(MoDispatcherCapiTest, VariantClear_ClearsVectorContainer_6300, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vec = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorCreate(&typeInfo, &vec),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant variant = {};
    variant.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    variant.u.pvectorVal = vec;

    OH_AbilityRuntime_ModObjDispatcher_VariantClear(&variant);
    EXPECT_EQ(variant.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(variant.u.pvectorVal, nullptr);
}

/**
 * @tc.name: VariantClear_ClearsSetContainer_6400
 * @tc.desc: Test VariantClear properly releases a Set handle inside a Variant.
 */
HWTEST_F(MoDispatcherCapiTest, VariantClear_ClearsSetContainer_6400, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_SetHandle set = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetCreate(&typeInfo, &set),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant variant = {};
    variant.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET;
    variant.u.psetVal = set;

    OH_AbilityRuntime_ModObjDispatcher_VariantClear(&variant);
    EXPECT_EQ(variant.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(variant.u.psetVal, nullptr);
}

/**
 * @tc.name: VariantClear_ClearsStructContainer_6500
 * @tc.desc: Test VariantClear properly releases a Struct handle inside a Variant.
 */
HWTEST_F(MoDispatcherCapiTest, VariantClear_ClearsStructContainer_6500, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_StructHandle st = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructCreate("MyStruct", &st),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant variant = {};
    variant.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT;
    variant.u.pstructVal = st;

    OH_AbilityRuntime_ModObjDispatcher_VariantClear(&variant);
    EXPECT_EQ(variant.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    EXPECT_EQ(variant.u.pstructVal, nullptr);
}

// ===================== TypeInfoClear Complex Types =====================

/**
 * @tc.name: TypeInfoClear_ClearsMapType_6600
 * @tc.desc: Test TypeInfoClear properly releases nested map type info.
 */
HWTEST_F(MoDispatcherCapiTest, TypeInfoClear_ClearsMapType_6600, TestSize.Level1)
{
    auto* valueTypeInfo = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    valueTypeInfo->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;

    OH_AbilityRuntime_ModObjDispatcher_TypeInfo mapTypeInfo = {};
    mapTypeInfo.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP;
    mapTypeInfo.u.mapType.keyType = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    mapTypeInfo.u.mapType.pValueType = valueTypeInfo;

    OH_AbilityRuntime_ModObjDispatcher_TypeInfoClear(&mapTypeInfo);
    EXPECT_EQ(mapTypeInfo.u.mapType.pValueType, nullptr);
}

/**
 * @tc.name: TypeInfoClear_ClearsArrayType_6700
 * @tc.desc: Test TypeInfoClear properly releases nested array type info.
 */
HWTEST_F(MoDispatcherCapiTest, TypeInfoClear_ClearsArrayType_6700, TestSize.Level1)
{
    auto* elemTypeInfo = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    elemTypeInfo->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;

    OH_AbilityRuntime_ModObjDispatcher_TypeInfo arrayTypeInfo = {};
    arrayTypeInfo.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY;
    arrayTypeInfo.u.arrayType.pElementType = elemTypeInfo;
    arrayTypeInfo.u.arrayType.size = 5;

    OH_AbilityRuntime_ModObjDispatcher_TypeInfoClear(&arrayTypeInfo);
    EXPECT_EQ(arrayTypeInfo.u.arrayType.pElementType, nullptr);
}

/**
 * @tc.name: TypeInfoClear_ClearsVectorType_6800
 * @tc.desc: Test TypeInfoClear properly releases nested vector type info.
 */
HWTEST_F(MoDispatcherCapiTest, TypeInfoClear_ClearsVectorType_6800, TestSize.Level1)
{
    auto* elemTypeInfo = new OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
    elemTypeInfo->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    elemTypeInfo->u.idlType = nullptr; // simple string, no idlType needed

    OH_AbilityRuntime_ModObjDispatcher_TypeInfo vecTypeInfo = {};
    vecTypeInfo.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
    vecTypeInfo.u.pElementType = elemTypeInfo;

    OH_AbilityRuntime_ModObjDispatcher_TypeInfoClear(&vecTypeInfo);
    EXPECT_EQ(vecTypeInfo.u.pElementType, nullptr);
}

// ===================== Array with String Elements =====================

/**
 * @tc.name: Array_StringElements_6900
 * @tc.desc: Test Array with string element type: create, set, get, release.
 */
HWTEST_F(MoDispatcherCapiTest, Array_StringElements_6900, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle array = nullptr;

    auto ret = OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(&typeInfo, 2, &array);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto s1 = MakeStringVariant("hello");
    auto s2 = MakeStringVariant("world");

    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArraySet(array, 0, &s1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArraySet(array, 1, &s2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant val = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGet(array, 0, &val), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(val.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    EXPECT_NE(val.u.bstrVal, nullptr);
    EXPECT_STREQ(val.u.bstrVal, "hello");

    // Clean up returned deep-copy string
    OH_AbilityRuntime_ModObjDispatcher_VariantClear(&val);

    // Clean up input strings
    std::free(s1.u.bstrVal);
    std::free(s2.u.bstrVal);

    OH_AbilityRuntime_ModObjDispatcher_ArrayRelease(&array);
}

// ===================== Map with I32 Key =====================

/**
 * @tc.name: Map_I32Key_7000
 * @tc.desc: Test Map with I32 key type.
 */
HWTEST_F(MoDispatcherCapiTest, Map_I32Key_7000, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;

    auto ret = OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32, &valTypeInfo, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto k = MakeI32Variant(1);
    auto v = MakeF64Variant(3.14);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(map, &k, &v), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGet(map, &k, &outVal), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(outVal.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);
    EXPECT_DOUBLE_EQ(outVal.u.f64Val, 3.14);

    OH_AbilityRuntime_ModObjDispatcher_MapRelease(&map);
}

// ===================== Map Put Overwrite =====================

/**
 * @tc.name: Map_PutOverwrite_7100
 * @tc.desc: Test MapPut overwrites existing key with new value.
 */
HWTEST_F(MoDispatcherCapiTest, Map_PutOverwrite_7100, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;

    auto ret = OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTypeInfo, &map);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto k = MakeStringVariant("key");
    auto v1 = MakeI32Variant(100);
    auto v2 = MakeI32Variant(200);

    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(map, &k, &v1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(map, &k, &v2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t size = 0;
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetSize(map, &size), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(size, 1u); // Overwrite, not add

    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGet(map, &k, &outVal), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(outVal.u.i32Val, 200);

    std::free(k.u.bstrVal);
    OH_AbilityRuntime_ModObjDispatcher_MapRelease(&map);
}

// ===================== Set GetAt Out Of Bounds =====================

/**
 * @tc.name: Set_GetAtOutOfBounds_7200
 * @tc.desc: Test SetGetAt with out-of-bounds index.
 */
HWTEST_F(MoDispatcherCapiTest, Set_GetAtOutOfBounds_7200, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_SetHandle set = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetCreate(&typeInfo, &set), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v = MakeI32Variant(1);
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetAdd(set, &v), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetGetAt(set, 5, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ModObjDispatcher_SetRelease(&set);
}

// ===================== Map GetKeyAt / GetValueAt Out Of Bounds =====================

/**
 * @tc.name: Map_KeyAtValueAtOutOfBounds_7300
 * @tc.desc: Test MapGetKeyAt / MapGetValueAt with out-of-bounds index.
 */
HWTEST_F(MoDispatcherCapiTest, Map_KeyAtValueAtOutOfBounds_7300, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTypeInfo, &map),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetKeyAt(map, 0, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGetValueAt(map, 0, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ModObjDispatcher_MapRelease(&map);
}

// ===================== Map Get Nonexistent Key =====================

/**
 * @tc.name: Map_GetNonexistentKey_7400
 * @tc.desc: Test MapGet with nonexistent key returns PROPERTY_NOT_FOUND.
 */
HWTEST_F(MoDispatcherCapiTest, Map_GetNonexistentKey_7400, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTypeInfo, &map),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto k = MakeStringVariant("nonexistent");
    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapGet(map, &k, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    std::free(k.u.bstrVal);
    OH_AbilityRuntime_ModObjDispatcher_MapRelease(&map);
}

// ===================== Map Remove Nonexistent Key =====================

/**
 * @tc.name: Map_RemoveNonexistentKey_7500
 * @tc.desc: Test MapRemove with nonexistent key returns PROPERTY_NOT_FOUND.
 */
HWTEST_F(MoDispatcherCapiTest, Map_RemoveNonexistentKey_7500, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTypeInfo, &map),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto k = MakeStringVariant("nonexistent");
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapRemove(map, &k),
        ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    std::free(k.u.bstrVal);
    OH_AbilityRuntime_ModObjDispatcher_MapRelease(&map);
}

// ===================== Vector Get Out Of Bounds =====================

/**
 * @tc.name: Vector_GetOutOfBounds_7600
 * @tc.desc: Test VectorGet with out-of-bounds index.
 */
HWTEST_F(MoDispatcherCapiTest, Vector_GetOutOfBounds_7600, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle vec = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorCreate(&typeInfo, &vec),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_VectorGet(vec, 0, &outVal),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ModObjDispatcher_VectorRelease(&vec);
}

// ===================== Set Remove Nonexistent =====================

/**
 * @tc.name: Set_RemoveNonexistent_7700
 * @tc.desc: Test SetRemove with nonexistent element returns PROPERTY_NOT_FOUND.
 */
HWTEST_F(MoDispatcherCapiTest, Set_RemoveNonexistent_7700, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_SetHandle set = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetCreate(&typeInfo, &set),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v = MakeI32Variant(999);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_SetRemove(set, &v),
        ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);

    OH_AbilityRuntime_ModObjDispatcher_SetRelease(&set);
}

// ===================== Struct GetName Buffer Too Small =====================

/**
 * @tc.name: Struct_GetNameBufferTooSmall_7800
 * @tc.desc: Test StructGetName with buffer too small returns PARAM_INVALID.
 */
HWTEST_F(MoDispatcherCapiTest, Struct_GetNameBufferTooSmall_7800, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_StructHandle st = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_StructCreate("ALongStructName", &st),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    char buf[4] = {0}; // Too small for "ALongStructName"
    auto ret = OH_AbilityRuntime_ModObjDispatcher_StructGetName(st, buf, sizeof(buf));
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);

    OH_AbilityRuntime_ModObjDispatcher_StructRelease(&st);
}

// ===================== Array with F64 type =====================

/**
 * @tc.name: Array_F64Elements_7900
 * @tc.desc: Test Array with F64 elements: create, set, get.
 */
HWTEST_F(MoDispatcherCapiTest, Array_F64Elements_7900, TestSize.Level1)
{
    auto typeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle array = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(&typeInfo, 2, &array),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto v1 = MakeF64Variant(1.5);
    auto v2 = MakeF64Variant(2.7);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArraySet(array, 0, &v1), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArraySet(array, 1, &v2), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    OH_AbilityRuntime_ModObjDispatcher_Variant outVal = {};
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGet(array, 0, &outVal), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_DOUBLE_EQ(outVal.u.f64Val, 1.5);

    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_ArrayGet(array, 1, &outVal), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_DOUBLE_EQ(outVal.u.f64Val, 2.7);

    OH_AbilityRuntime_ModObjDispatcher_ArrayRelease(&array);
}

// ===================== Map Type Mismatch on Put =====================

/**
 * @tc.name: Map_PutKeyTypeMismatch_8000
 * @tc.desc: Test MapPut with wrong key type returns TYPE_MISMATCH.
 */
HWTEST_F(MoDispatcherCapiTest, Map_PutKeyTypeMismatch_8000, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTypeInfo, &map),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Wrong key type: using I32 key for a STRING-key map
    auto wrongKey = MakeI32Variant(42);
    auto val = MakeI32Variant(100);
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(map, &wrongKey, &val),
        ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    OH_AbilityRuntime_ModObjDispatcher_MapRelease(&map);
}

/**
 * @tc.name: Map_PutValueTypeMismatch_8100
 * @tc.desc: Test MapPut with wrong value type returns TYPE_MISMATCH.
 */
HWTEST_F(MoDispatcherCapiTest, Map_PutValueTypeMismatch_8100, TestSize.Level1)
{
    auto valTypeInfo = MakeBasicTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    OH_AbilityRuntime_ModObjDispatcher_MapHandle map = nullptr;
    ASSERT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapCreate(
        OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING, &valTypeInfo, &map),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    auto k = MakeStringVariant("key");
    auto wrongVal = MakeStringVariant("wrong"); // Should be I32
    EXPECT_EQ(OH_AbilityRuntime_ModObjDispatcher_MapPut(map, &k, &wrongVal),
        ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);

    std::free(k.u.bstrVal);
    std::free(wrongVal.u.bstrVal);
    OH_AbilityRuntime_ModObjDispatcher_MapRelease(&map);
}
