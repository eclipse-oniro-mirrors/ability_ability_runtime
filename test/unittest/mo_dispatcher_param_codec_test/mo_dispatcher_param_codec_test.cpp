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
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "message_parcel.h"
#include "mo_dispatcher_param_codec.h"
#include "mo_dispatcher_types.h"
#include "modular_object_dispatcher.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

namespace {

// ---- Helper functions for constructing test data ----

// Create a simple MoTypeInfo for a basic type
std::shared_ptr<MoTypeInfo> MakeTypeInfo(OH_AbilityRuntime_ModObjDispatcher_ValueType vt)
{
    auto info = std::make_shared<MoTypeInfo>();
    info->vt = vt;
    return info;
}

// Create a MoMethodMeta with specified parameter types and return type
MoMethodMeta MakeMethodMeta(const std::vector<std::shared_ptr<MoTypeInfo>>& paramTypes,
    std::shared_ptr<MoTypeInfo> returnType = nullptr)
{
    MoMethodMeta meta;
    meta.interfaceName = "ITestInterface";
    meta.name = "testMethod";
    meta.memberId = 1;
    meta.ipcCode = 100;
    meta.oneway = false;
    meta.returnType = returnType;
    for (size_t i = 0; i < paramTypes.size(); i++) {
        MoMethodParamMeta param;
        param.name = "param" + std::to_string(i);
        param.typeInfo = paramTypes[i];
        meta.params.push_back(param);
    }
    return meta;
}

// Create a Variant with vt=I32
OH_AbilityRuntime_ModObjDispatcher_Variant MakeI32Variant(int32_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32;
    v.u.i32Val = val;
    return v;
}

// Create a Variant with vt=I64
OH_AbilityRuntime_ModObjDispatcher_Variant MakeI64Variant(int64_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64;
    v.u.i64Val = val;
    return v;
}

// Create a Variant with vt=STRING (caller must free bstrVal if non-null)
OH_AbilityRuntime_ModObjDispatcher_Variant MakeStringVariant(const char* val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    if (val != nullptr) {
        size_t len = strlen(val);
        v.u.bstrVal = static_cast<char*>(std::malloc(len + 1));
        if (v.u.bstrVal != nullptr) {
            (void)strcpy_s(v.u.bstrVal, len + 1, val);
        }
    }
    return v;
}

// Create a Variant with vt=BOOL
OH_AbilityRuntime_ModObjDispatcher_Variant MakeBoolVariant(bool val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL;
    v.u.boolVal = val;
    return v;
}

// Create a Variant with vt=F64
OH_AbilityRuntime_ModObjDispatcher_Variant MakeF64Variant(double val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64;
    v.u.f64Val = val;
    return v;
}

// Create a Variant with vt=F32
OH_AbilityRuntime_ModObjDispatcher_Variant MakeF32Variant(float val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32;
    v.u.f32Val = val;
    return v;
}

// Create a Variant with vt=I8
OH_AbilityRuntime_ModObjDispatcher_Variant MakeI8Variant(int8_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8;
    v.u.i8Val = val;
    return v;
}

// Create a Variant with vt=I16
OH_AbilityRuntime_ModObjDispatcher_Variant MakeI16Variant(int16_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16;
    v.u.i16Val = val;
    return v;
}

// Create a Variant with vt=U8
OH_AbilityRuntime_ModObjDispatcher_Variant MakeU8Variant(uint8_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8;
    v.u.u8Val = val;
    return v;
}

// Create a Variant with vt=U16
OH_AbilityRuntime_ModObjDispatcher_Variant MakeU16Variant(uint16_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16;
    v.u.u16Val = val;
    return v;
}

// Create a Variant with vt=U32
OH_AbilityRuntime_ModObjDispatcher_Variant MakeU32Variant(uint32_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32;
    v.u.u32Val = val;
    return v;
}

// Create a Variant with vt=U64
OH_AbilityRuntime_ModObjDispatcher_Variant MakeU64Variant(uint64_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64;
    v.u.u64Val = val;
    return v;
}

// Create a Variant with vt=ENUM
OH_AbilityRuntime_ModObjDispatcher_Variant MakeEnumVariant(int32_t val)
{
    OH_AbilityRuntime_ModObjDispatcher_Variant v;
    (void)memset_s(&v, sizeof(v), 0, sizeof(v));
    v.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM;
    v.u.enumVal = val;
    return v;
}

// RAII wrapper to clean up a Variant's heap resources
struct VariantGuard {
    OH_AbilityRuntime_ModObjDispatcher_Variant* v_;
    explicit VariantGuard(OH_AbilityRuntime_ModObjDispatcher_Variant* v) : v_(v) {}
    ~VariantGuard()
    {
        if (v_ != nullptr) {
            if (v_->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING && v_->u.bstrVal != nullptr) {
                std::free(v_->u.bstrVal);
                v_->u.bstrVal = nullptr;
            }
            v_->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
        }
    }
};

} // anonymous namespace

class MoDispatcherParamCodecTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

// ============================================================
// MarshalCallRequest tests
// ============================================================

// Test 1: null inputParams -> PARAM_INVALID
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_NullInputParams, TestSize.Level1)
{
    MoMethodMeta meta = MakeMethodMeta({});
    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, nullptr, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// Test 2: null rgvarg -> PARAM_INVALID
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_NullRgvarg, TestSize.Level1)
{
    MoMethodMeta meta = MakeMethodMeta({});
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = nullptr;
    inputParams.cArgs = 0;
    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// Test 3: count mismatch (more args than params) -> TYPE_MISMATCH
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_CountMismatch, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    MoMethodMeta meta = MakeMethodMeta({typeI32}); // expects 1 param

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI32Variant(42);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 2; // but claims 2 args

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
}

// Test 4: type mismatch variant (I64 value for I32 param) -> TYPE_MISMATCH
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_TypeMismatch, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    MoMethodMeta meta = MakeMethodMeta({typeI32});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI64Variant(42LL);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
}

// Test 5: success with I32 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_I32, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    MoMethodMeta meta = MakeMethodMeta({typeI32});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI32Variant(12345);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Verify the data was written correctly
    int32_t val = parcel.ReadInt32();
    EXPECT_EQ(val, 12345);
}

// Test 6: success with I64 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_I64, TestSize.Level1)
{
    auto typeI64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64);
    MoMethodMeta meta = MakeMethodMeta({typeI64});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI64Variant(9876543210LL);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    int64_t val = parcel.ReadInt64();
    EXPECT_EQ(val, 9876543210LL);
}

// Test 7: success with STRING param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_String, TestSize.Level1)
{
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    MoMethodMeta meta = MakeMethodMeta({typeStr});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeStringVariant("hello world");
    VariantGuard guard(&arg);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    const char* val = parcel.ReadCString();
    ASSERT_NE(val, nullptr);
    EXPECT_STREQ(val, "hello world");
}

// Test 8: success with BOOL param (true)
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_BoolTrue, TestSize.Level1)
{
    auto typeBool = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL);
    MoMethodMeta meta = MakeMethodMeta({typeBool});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeBoolVariant(true);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    int8_t val = parcel.ReadInt8();
    EXPECT_EQ(val, 1);
}

// Test 9: success with BOOL param (false)
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_BoolFalse, TestSize.Level1)
{
    auto typeBool = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL);
    MoMethodMeta meta = MakeMethodMeta({typeBool});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeBoolVariant(false);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    int8_t val = parcel.ReadInt8();
    EXPECT_EQ(val, 0);
}

// Test 10: success with F64 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_F64, TestSize.Level1)
{
    auto typeF64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);
    MoMethodMeta meta = MakeMethodMeta({typeF64});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeF64Variant(3.14159265358979);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    double val = parcel.ReadDouble();
    EXPECT_DOUBLE_EQ(val, 3.14159265358979);
}

// Test 11: success with F32 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_F32, TestSize.Level1)
{
    auto typeF32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32);
    MoMethodMeta meta = MakeMethodMeta({typeF32});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeF32Variant(2.718f);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    float val = parcel.ReadFloat();
    EXPECT_FLOAT_EQ(val, 2.718f);
}

// Test 12: success with I8 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_I8, TestSize.Level1)
{
    auto typeI8 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8);
    MoMethodMeta meta = MakeMethodMeta({typeI8});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI8Variant(-42);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    int8_t val = parcel.ReadInt8();
    EXPECT_EQ(val, -42);
}

// Test 13: success with I16 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_I16, TestSize.Level1)
{
    auto typeI16 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16);
    MoMethodMeta meta = MakeMethodMeta({typeI16});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI16Variant(1234);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    int16_t val = parcel.ReadInt16();
    EXPECT_EQ(val, 1234);
}

// Test 14: success with U8 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_U8, TestSize.Level1)
{
    auto typeU8 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8);
    MoMethodMeta meta = MakeMethodMeta({typeU8});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeU8Variant(200);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint8_t val = static_cast<uint8_t>(parcel.ReadInt8());
    EXPECT_EQ(val, 200);
}

// Test 15: success with U16 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_U16, TestSize.Level1)
{
    auto typeU16 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16);
    MoMethodMeta meta = MakeMethodMeta({typeU16});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeU16Variant(60000);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint16_t val = static_cast<uint16_t>(parcel.ReadInt16());
    EXPECT_EQ(val, 60000);
}

// Test 16: success with U32 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_U32, TestSize.Level1)
{
    auto typeU32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32);
    MoMethodMeta meta = MakeMethodMeta({typeU32});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeU32Variant(3000000000U);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint32_t val = static_cast<uint32_t>(parcel.ReadInt32());
    EXPECT_EQ(val, 3000000000U);
}

// Test 17: success with U64 param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_U64, TestSize.Level1)
{
    auto typeU64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64);
    MoMethodMeta meta = MakeMethodMeta({typeU64});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeU64Variant(18446744073709551615ULL);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    uint64_t val = static_cast<uint64_t>(parcel.ReadInt64());
    EXPECT_EQ(val, 18446744073709551615ULL);
}

// Test 18: success with ENUM param
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_Enum, TestSize.Level1)
{
    auto typeEnum = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM);
    MoMethodMeta meta = MakeMethodMeta({typeEnum});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeEnumVariant(7);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    int32_t val = parcel.ReadInt32();
    EXPECT_EQ(val, 7);
}

// Test 19: success with multiple params
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_MultipleParams, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    auto typeF64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);
    MoMethodMeta meta = MakeMethodMeta({typeI32, typeStr, typeF64});

    OH_AbilityRuntime_ModObjDispatcher_Variant args[3];
    args[0] = MakeI32Variant(100);
    args[1] = MakeStringVariant("multi param test");
    args[2] = MakeF64Variant(1.23456789);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = args;
    inputParams.cArgs = 3;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Read back and verify in order
    EXPECT_EQ(parcel.ReadInt32(), 100);
    EXPECT_STREQ(parcel.ReadCString(), "multi param test");
    EXPECT_DOUBLE_EQ(parcel.ReadDouble(), 1.23456789);

    // Cleanup string
    std::free(args[1].u.bstrVal);
}

// Test 20: success with EMPTY type param (no data written)
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_EmptyType, TestSize.Level1)
{
    auto typeEmpty = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    MoMethodMeta meta = MakeMethodMeta({typeEmpty});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

// Test 21: success with VOID type param (no data written)
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_VoidType, TestSize.Level1)
{
    auto typeVoid = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID);
    MoMethodMeta meta = MakeMethodMeta({typeVoid});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID;
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

// Test 22: zero args, zero params -> NO_ERROR
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_ZeroArgs, TestSize.Level1)
{
    MoMethodMeta meta = MakeMethodMeta({});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 0;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

// Test 23: string variant with null bstrVal (should write empty string)
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_StringNull, TestSize.Level1)
{
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    MoMethodMeta meta = MakeMethodMeta({typeStr});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
    arg.u.bstrVal = nullptr;
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    const char* val = parcel.ReadCString();
    ASSERT_NE(val, nullptr);
    EXPECT_STREQ(val, "");
}

// Test 24: param with null typeInfo -> treated as VT_EMPTY
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_NullTypeInfoParam, TestSize.Level1)
{
    MoMethodMeta meta;
    meta.interfaceName = "ITest";
    meta.name = "test";
    meta.memberId = 1;
    meta.ipcCode = 1;
    meta.oneway = false;
    meta.returnType = nullptr;
    MoMethodParamMeta param;
    param.name = "p";
    param.typeInfo = nullptr; // null typeInfo
    meta.params.push_back(param);

    OH_AbilityRuntime_ModObjDispatcher_Variant arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

// Test 25: Marshal with empty string value
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_EmptyString, TestSize.Level1)
{
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    MoMethodMeta meta = MakeMethodMeta({typeStr});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeStringVariant("");
    VariantGuard guard(&arg);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    const char* val = parcel.ReadCString();
    ASSERT_NE(val, nullptr);
    EXPECT_STREQ(val, "");
}

// ============================================================
// UnmarshalCallResult tests
// ============================================================

// Test 26: null result -> PARAM_INVALID
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_NullResult, TestSize.Level1)
{
    MoMethodMeta meta = MakeMethodMeta({});
    MessageParcel reply;
    reply.WriteInt32(0); // error code
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// Test 27: void return type -> NO_ERROR, result->vt = VT_VOID
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_VoidReturnType, TestSize.Level1)
{
    auto voidType = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID);
    MoMethodMeta meta = MakeMethodMeta({}, voidType);

    MessageParcel reply;
    reply.WriteInt32(0); // method error code

    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID);
}

// Test 28: null returnType -> NO_ERROR, result->vt = VT_VOID
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_NullReturnType, TestSize.Level1)
{
    MoMethodMeta meta = MakeMethodMeta({}, nullptr); // null return type

    MessageParcel reply;
    reply.WriteInt32(0); // method error code

    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID);
}

// Test 29: I32 round-trip (marshal then unmarshal)
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_I32RoundTrip, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // Marshal: write I32 value
    MessageParcel dataParcel;
    int32_t expectedVal = -99999;
    ASSERT_TRUE(dataParcel.WriteInt32(expectedVal));

    // Unmarshal: set up reply parcel with errCode + value
    MessageParcel reply;
    reply.WriteInt32(0); // method error code
    reply.WriteInt32(expectedVal);

    MoMethodMeta meta = MakeMethodMeta({}, typeI32);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(result.u.i32Val, expectedVal);
}

// Test 30: I64 round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_I64RoundTrip, TestSize.Level1)
{
    auto typeI64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64);

    MessageParcel reply;
    reply.WriteInt32(0); // method error code
    reply.WriteInt64(123456789012345LL);

    MoMethodMeta meta = MakeMethodMeta({}, typeI64);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64);
    EXPECT_EQ(result.u.i64Val, 123456789012345LL);
}

// Test 31: STRING round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_StringRoundTrip, TestSize.Level1)
{
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteCString("test string data");

    MoMethodMeta meta = MakeMethodMeta({}, typeStr);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    ASSERT_NE(result.u.bstrVal, nullptr);
    EXPECT_STREQ(result.u.bstrVal, "test string data");

    // Cleanup allocated string
    std::free(result.u.bstrVal);
}

// Test 32: BOOL round-trip (true)
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_BoolTrueRoundTrip, TestSize.Level1)
{
    auto typeBool = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt8(1); // true

    MoMethodMeta meta = MakeMethodMeta({}, typeBool);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL);
    EXPECT_EQ(result.u.boolVal, true);
}

// Test 33: BOOL round-trip (false)
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_BoolFalseRoundTrip, TestSize.Level1)
{
    auto typeBool = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt8(0); // false

    MoMethodMeta meta = MakeMethodMeta({}, typeBool);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL);
    EXPECT_EQ(result.u.boolVal, false);
}

// Test 34: F64 round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_F64RoundTrip, TestSize.Level1)
{
    auto typeF64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteDouble(2.718281828459045);

    MoMethodMeta meta = MakeMethodMeta({}, typeF64);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);
    EXPECT_DOUBLE_EQ(result.u.f64Val, 2.718281828459045);
}

// Test 35: F32 round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_F32RoundTrip, TestSize.Level1)
{
    auto typeF32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteFloat(1.5f);

    MoMethodMeta meta = MakeMethodMeta({}, typeF32);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32);
    EXPECT_FLOAT_EQ(result.u.f32Val, 1.5f);
}

// Test 36: I8 round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_I8RoundTrip, TestSize.Level1)
{
    auto typeI8 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt8(-99);

    MoMethodMeta meta = MakeMethodMeta({}, typeI8);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8);
    EXPECT_EQ(result.u.i8Val, -99);
}

// Test 37: I16 round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_I16RoundTrip, TestSize.Level1)
{
    auto typeI16 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt16(-32000);

    MoMethodMeta meta = MakeMethodMeta({}, typeI16);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16);
    EXPECT_EQ(result.u.i16Val, -32000);
}

// Test 38: U8 round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_U8RoundTrip, TestSize.Level1)
{
    auto typeU8 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt8(static_cast<int8_t>(250));

    MoMethodMeta meta = MakeMethodMeta({}, typeU8);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8);
    EXPECT_EQ(result.u.u8Val, 250);
}

// Test 39: U16 round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_U16RoundTrip, TestSize.Level1)
{
    auto typeU16 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt16(static_cast<int16_t>(65000));

    MoMethodMeta meta = MakeMethodMeta({}, typeU16);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16);
    EXPECT_EQ(result.u.u16Val, 65000);
}

// Test 40: U32 round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_U32RoundTrip, TestSize.Level1)
{
    auto typeU32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt32(static_cast<int32_t>(4000000000U));

    MoMethodMeta meta = MakeMethodMeta({}, typeU32);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32);
    EXPECT_EQ(result.u.u32Val, 4000000000U);
}

// Test 41: U64 round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_U64RoundTrip, TestSize.Level1)
{
    auto typeU64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt64(static_cast<int64_t>(18000000000000000000ULL));

    MoMethodMeta meta = MakeMethodMeta({}, typeU64);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64);
    EXPECT_EQ(result.u.u64Val, 18000000000000000000ULL);
}

// Test 42: ENUM round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_EnumRoundTrip, TestSize.Level1)
{
    auto typeEnum = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt32(42);

    MoMethodMeta meta = MakeMethodMeta({}, typeEnum);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM);
    EXPECT_EQ(result.u.enumVal, 42);
}

// Test 43: pMethodErrCode non-null receives error code
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_MethodErrCode, TestSize.Level1)
{
    auto voidType = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID);
    MoMethodMeta meta = MakeMethodMeta({}, voidType);

    MessageParcel reply;
    reply.WriteInt32(42); // method error code = 42

    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    int32_t methodErr = 0;
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, &methodErr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(methodErr, 42);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID);
}

// Test 44: pMethodErrCode null (should not crash)
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_NullMethodErrCode, TestSize.Level1)
{
    auto voidType = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID);
    MoMethodMeta meta = MakeMethodMeta({}, voidType);

    MessageParcel reply;
    reply.WriteInt32(99);

    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID);
}

// Test 45: EMPTY return type -> NO_ERROR, result->vt = VT_EMPTY
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_EmptyReturnType, TestSize.Level1)
{
    auto emptyType = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY);
    MoMethodMeta meta = MakeMethodMeta({}, emptyType);

    MessageParcel reply;
    reply.WriteInt32(0);

    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID);
}

// ============================================================
// Full round-trip tests (Marshal then Unmarshal)
// ============================================================

// Test 46: full I32 round-trip through Marshal + Unmarshal
HWTEST_F(MoDispatcherParamCodecTest, FullRoundTrip_I32, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    // --- Marshal side ---
    MoMethodMeta marshalMeta = MakeMethodMeta({typeI32}, typeI32);
    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI32Variant(77777);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel dataParcel;
    auto marshalRet = ModObjDispatcherParamCodec::MarshalCallRequest(marshalMeta, &inputParams, dataParcel);
    EXPECT_EQ(marshalRet, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Verify the marshaled data
    int32_t marshaledVal = dataParcel.ReadInt32();
    EXPECT_EQ(marshaledVal, 77777);

    // --- Unmarshal side ---
    MessageParcel reply;
    reply.WriteInt32(0); // method error code
    reply.WriteInt32(77777); // return value

    MoMethodMeta unmarshalMeta = MakeMethodMeta({}, typeI32);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    int32_t errCode = -1;
    auto unmarshalRet = ModObjDispatcherParamCodec::UnmarshalCallResult(
        unmarshalMeta, reply, &result, &errCode);
    EXPECT_EQ(unmarshalRet, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(errCode, 0);
    EXPECT_EQ(result.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    EXPECT_EQ(result.u.i32Val, 77777);
}

// Test 47: full STRING round-trip
HWTEST_F(MoDispatcherParamCodecTest, FullRoundTrip_String, TestSize.Level1)
{
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);

    // --- Marshal side ---
    MoMethodMeta marshalMeta = MakeMethodMeta({typeStr}, typeStr);
    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeStringVariant("round trip string");
    VariantGuard guard(&arg);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel dataParcel;
    auto marshalRet = ModObjDispatcherParamCodec::MarshalCallRequest(marshalMeta, &inputParams, dataParcel);
    EXPECT_EQ(marshalRet, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // --- Unmarshal side ---
    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteCString("round trip string");

    MoMethodMeta unmarshalMeta = MakeMethodMeta({}, typeStr);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto unmarshalRet = ModObjDispatcherParamCodec::UnmarshalCallResult(
        unmarshalMeta, reply, &result, nullptr);
    EXPECT_EQ(unmarshalRet, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(result.u.bstrVal, nullptr);
    EXPECT_STREQ(result.u.bstrVal, "round trip string");
    std::free(result.u.bstrVal);
}

// Test 48: full multi-param round-trip (I32 + STRING + F64)
HWTEST_F(MoDispatcherParamCodecTest, FullRoundTrip_MultiParam, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    auto typeF64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);

    MoMethodMeta marshalMeta = MakeMethodMeta({typeI32, typeStr, typeF64});
    OH_AbilityRuntime_ModObjDispatcher_Variant args[3];
    args[0] = MakeI32Variant(-12345);
    args[1] = MakeStringVariant("multi param round trip");
    args[2] = MakeF64Variant(6.62607015e-34);

    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = args;
    inputParams.cArgs = 3;

    MessageParcel dataParcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(marshalMeta, &inputParams, dataParcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    // Verify marshaled data
    EXPECT_EQ(dataParcel.ReadInt32(), -12345);
    EXPECT_STREQ(dataParcel.ReadCString(), "multi param round trip");
    EXPECT_DOUBLE_EQ(dataParcel.ReadDouble(), 6.62607015e-34);

    std::free(args[1].u.bstrVal);
}

// Test 49: MarshalCallRequest with non-zero method error code in unmarshal
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_NonZeroMethodErrCode, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    MessageParcel reply;
    reply.WriteInt32(-1); // non-zero method error code
    reply.WriteInt32(88888);

    MoMethodMeta meta = MakeMethodMeta({}, typeI32);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    int32_t errCode = 0;
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, &errCode);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(errCode, -1);
    EXPECT_EQ(result.u.i32Val, 88888);
}

// Test 50: Negative I32 value round-trip
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_NegativeI32, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    MoMethodMeta meta = MakeMethodMeta({typeI32});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI32Variant(-2147483647);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    EXPECT_EQ(parcel.ReadInt32(), -2147483647);
}

// Test 51: Zero I32 value
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_ZeroI32, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    MoMethodMeta meta = MakeMethodMeta({typeI32});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI32Variant(0);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    EXPECT_EQ(parcel.ReadInt32(), 0);
}

// Test 52: F64 zero round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_F64Zero, TestSize.Level1)
{
    auto typeF64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteDouble(0.0);

    MoMethodMeta meta = MakeMethodMeta({}, typeF64);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_DOUBLE_EQ(result.u.f64Val, 0.0);
}

// Test 53: Large string round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_LargeString, TestSize.Level1)
{
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    std::string largeStr(4096, 'A'); // 4KB string

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteCString(largeStr.c_str());

    MoMethodMeta meta = MakeMethodMeta({}, typeStr);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ASSERT_NE(result.u.bstrVal, nullptr);
    EXPECT_STREQ(result.u.bstrVal, largeStr.c_str());
    std::free(result.u.bstrVal);
}

// Test 54: I64 max value round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_I64MaxValue, TestSize.Level1)
{
    auto typeI64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt64(INT64_MAX);

    MoMethodMeta meta = MakeMethodMeta({}, typeI64);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.u.i64Val, INT64_MAX);
}

// Test 55: I64 min value round-trip
HWTEST_F(MoDispatcherParamCodecTest, UnmarshalCallResult_I64MinValue, TestSize.Level1)
{
    auto typeI64 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64);

    MessageParcel reply;
    reply.WriteInt32(0);
    reply.WriteInt64(INT64_MIN);

    MoMethodMeta meta = MakeMethodMeta({}, typeI64);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(result.u.i64Val, INT64_MIN);
}

// Test 56: Verify count mismatch with fewer args than params
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_FewerArgsThanParams, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    MoMethodMeta meta = MakeMethodMeta({typeI32, typeStr}); // expects 2 params

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI32Variant(42);
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1; // only 1 arg provided

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
}

// Test 57: Marshal + Unmarshal with method returning non-zero error code and I32 result
HWTEST_F(MoDispatcherParamCodecTest, FullRoundTrip_MethodErrorWithResult, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);

    MessageParcel reply;
    reply.WriteInt32(100); // non-zero method error
    reply.WriteInt32(55555);

    MoMethodMeta meta = MakeMethodMeta({}, typeI32);
    OH_AbilityRuntime_ModObjDispatcher_Variant result;
    (void)memset_s(&result, sizeof(result), 0, sizeof(result));
    int32_t errCode = 0;
    auto ret = ModObjDispatcherParamCodec::UnmarshalCallResult(meta, reply, &result, &errCode);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(errCode, 100);
    EXPECT_EQ(result.u.i32Val, 55555);
}

// Test 58: BOOL type mismatch (sending I32 when BOOL expected)
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_BoolTypeMismatch, TestSize.Level1)
{
    auto typeBool = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL);
    MoMethodMeta meta = MakeMethodMeta({typeBool});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeI32Variant(1); // I32 instead of BOOL
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
}

// Test 59: STRING type mismatch (sending BOOL when STRING expected)
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_StringTypeMismatch, TestSize.Level1)
{
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    MoMethodMeta meta = MakeMethodMeta({typeStr});

    OH_AbilityRuntime_ModObjDispatcher_Variant arg = MakeBoolVariant(true); // BOOL instead of STRING
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = &arg;
    inputParams.cArgs = 1;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
}

// Test 60: Multiple type mismatches across params
HWTEST_F(MoDispatcherParamCodecTest, MarshalCallRequest_MultiParamTypeMismatch, TestSize.Level1)
{
    auto typeI32 = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
    auto typeStr = MakeTypeInfo(OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
    MoMethodMeta meta = MakeMethodMeta({typeI32, typeStr});

    OH_AbilityRuntime_ModObjDispatcher_Variant args[2];
    args[0] = MakeI32Variant(42); // correct
    args[1] = MakeI32Variant(99); // wrong: I32 instead of STRING
    OH_AbilityRuntime_ModObjDispatcher_InputParams inputParams;
    inputParams.rgvarg = args;
    inputParams.cArgs = 2;

    MessageParcel parcel;
    auto ret = ModObjDispatcherParamCodec::MarshalCallRequest(meta, &inputParams, parcel);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH);
}
