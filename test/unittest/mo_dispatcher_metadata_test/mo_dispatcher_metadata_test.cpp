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

#define private public
#include "mo_dispatcher_metadata_manager.h"
#undef private

#include "mo_dispatcher_complex_type_manager.h"
#include "modular_object_dispatcher.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

static const char* VALID_TLB_JSON = R"({
    "version": "1.0.0",
    "enums": [
        {
            "name": "TestEnum",
            "memberId": 100,
            "values": [
                {"name": "VAL_A", "value": 0, "memberId": 101},
                {"name": "VAL_B", "value": 1, "memberId": 102}
            ]
        }
    ],
    "structs": [
        {
            "name": "TestStruct",
            "memberId": 200,
            "fields": [
                {"name": "field1", "memberId": 201, "type_info": {"type": "i32"}},
                {"name": "field2", "memberId": 202, "type_info": {"type": "string"}}
            ]
        }
    ],
    "interfaces": [
        {
            "name": "ITestService",
            "descriptor": "ohos.test.ITestService",
            "memberId": 300,
            "interface_type": 1,
            "methods": [
                {
                    "name": "TestMethod",
                    "memberId": 301,
                    "code": 1,
                    "oneway": false,
                    "return_type": {"type": "i32"},
                    "parameters": [
                        {"name": "param1", "memberId": 302, "type_info": {"type": "string"}}
                    ]
                }
            ]
        },
        {
            "name": "ITestCallback",
            "descriptor": "ohos.test.ITestCallback",
            "memberId": 400,
            "interface_type": 2,
            "methods": []
        }
    ]
})";

class MoDispatcherMetadataTest : public testing::Test {
public:
    ModObjDispatcherMetadataManager mgr_;
    void SetUp() override
    {
        auto ret = mgr_.ParseMetadata(VALID_TLB_JSON);
        ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
        mgr_.loaded_ = true;
        ModObjDispatcherComplexTypeManager::RegisterStructMetadata(mgr_.structs_);
    }
    void TearDown() override {}
};

// ---- EnsureLoaded ----
HWTEST_F(MoDispatcherMetadataTest, EnsureLoaded_NullProxy_0100, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.EnsureLoaded(nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ---- ParseMetadata ----
HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_ValidJson_0100, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(VALID_TLB_JSON), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(m.version_, "1.0.0");
    EXPECT_EQ(m.interfaces_.size(), 2u);
    EXPECT_EQ(m.enums_.size(), 1u);
    EXPECT_EQ(m.structs_.size(), 1u);
    EXPECT_EQ(m.mainServiceInterface_, "ITestService");
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_InvalidJson_0200, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata("not json"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_EmptyJson_0300, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata("{}"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_NoMainService_0400, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({"interfaces":[{"name":"I","memberId":1,"interface_type":0}]})"),
        ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_DuplicateMainService_0500, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[
            {"name":"A","memberId":1,"interface_type":1},
            {"name":"B","memberId":2,"interface_type":1}
        ]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_InvalidInterfaceType_0600, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","memberId":1,"interface_type":3}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_EnumZeroMemberId_0700, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "enums":[{"name":"E"}],
        "interfaces":[{"name":"I","memberId":1,"interface_type":1}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_StructZeroMemberId_0800, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "structs":[{"name":"S"}],
        "interfaces":[{"name":"I","memberId":1,"interface_type":1}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_InterfaceZeroMemberId_0900, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","interface_type":1}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_MethodZeroMemberId_1000, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","memberId":1,"interface_type":1,
            "methods":[{"name":"M"}]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_DuplicateMemberIds_1100, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "enums":[{"name":"E1","memberId":100}],
        "structs":[{"name":"S1","memberId":100}],
        "interfaces":[{"name":"I","memberId":1,"interface_type":1}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

// ---- QueryMainServiceInterfaceMemberIds ----
HWTEST_F(MoDispatcherMetadataTest, QueryMemberIds_NullNames_1200, TestSize.Level1)
{
    uint32_t id = 0;
    EXPECT_EQ(mgr_.QueryMainServiceInterfaceMemberIds(nullptr, 1, &id), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, QueryMemberIds_NullMemberIds_1300, TestSize.Level1)
{
    const char* names[] = {"TestMethod"};
    EXPECT_EQ(mgr_.QueryMainServiceInterfaceMemberIds(names, 1, nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, QueryMemberIds_NullNameElement_1400, TestSize.Level1)
{
    const char* names[] = {nullptr};
    uint32_t id = 0;
    EXPECT_EQ(mgr_.QueryMainServiceInterfaceMemberIds(names, 1, &id), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, QueryMemberIds_NotFound_1500, TestSize.Level1)
{
    const char* names[] = {"NotExist"};
    uint32_t id = 0;
    EXPECT_EQ(mgr_.QueryMainServiceInterfaceMemberIds(names, 1, &id), ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, QueryMemberIds_Success_1600, TestSize.Level1)
{
    const char* names[] = {"TestMethod"};
    uint32_t id = 0;
    EXPECT_EQ(mgr_.QueryMainServiceInterfaceMemberIds(names, 1, &id), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(id, 301u);
}

// ---- GetMethodMeta ----
HWTEST_F(MoDispatcherMetadataTest, GetMethodMeta_NullParam_1700, TestSize.Level1)
{
    EXPECT_EQ(mgr_.GetMethodMeta(301, nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodMeta_NotFound_1800, TestSize.Level1)
{
    MoMethodMeta meta;
    EXPECT_EQ(mgr_.GetMethodMeta(9999, &meta), ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodMeta_Success_1900, TestSize.Level1)
{
    MoMethodMeta meta;
    EXPECT_EQ(mgr_.GetMethodMeta(301, &meta), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(meta.name, "TestMethod");
    EXPECT_EQ(meta.params.size(), 1u);
}

// ---- GetVersion ----
HWTEST_F(MoDispatcherMetadataTest, GetVersion_NullParam_2000, TestSize.Level1)
{
    EXPECT_EQ(mgr_.GetVersion(nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetVersion_Success_2100, TestSize.Level1)
{
    std::string ver;
    EXPECT_EQ(mgr_.GetVersion(&ver), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(ver, "1.0.0");
}

// ---- GetMainServiceInterfaceName ----
HWTEST_F(MoDispatcherMetadataTest, GetMainServiceName_NullParam_2200, TestSize.Level1)
{
    EXPECT_EQ(mgr_.GetMainServiceInterfaceName(nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetMainServiceName_Success_2300, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetMainServiceInterfaceName(&name), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(name, "ITestService");
}

// ---- Not-loaded state tests ----
HWTEST_F(MoDispatcherMetadataTest, NotLoaded_GetVersion_2400, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    std::string ver;
    EXPECT_EQ(m.GetVersion(&ver), ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
}

HWTEST_F(MoDispatcherMetadataTest, NotLoaded_GetInterfaceCount_2500, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    uint32_t cnt = 0;
    EXPECT_EQ(m.GetInterfaceCount(&cnt), ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
}

HWTEST_F(MoDispatcherMetadataTest, NotLoaded_GetEnumCount_2600, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    uint32_t cnt = 0;
    EXPECT_EQ(m.GetEnumCount(&cnt), ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
}

HWTEST_F(MoDispatcherMetadataTest, NotLoaded_GetStructCount_2700, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    uint32_t cnt = 0;
    EXPECT_EQ(m.GetStructCount(&cnt), ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
}

// ---- GetInterfaceCount/Name ----
HWTEST_F(MoDispatcherMetadataTest, GetInterfaceCount_Null_2800, TestSize.Level1)
{
    EXPECT_EQ(mgr_.GetInterfaceCount(nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetInterfaceCount_Success_2900, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetInterfaceCount(&cnt), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(cnt, 2u);
}

HWTEST_F(MoDispatcherMetadataTest, GetInterfaceName_Null_3000, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetInterfaceName(0, nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetInterfaceName_OutOfRange_3100, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetInterfaceName(99, &name), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetInterfaceName_Success_3200, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetInterfaceName(0, &name), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(name, "ITestService");
}

// ---- GetInterfaceIsCallback ----
HWTEST_F(MoDispatcherMetadataTest, GetIsCallback_Null_3300, TestSize.Level1)
{
    EXPECT_EQ(mgr_.GetInterfaceIsCallback("ITestCallback", nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetIsCallback_EmptyName_3400, TestSize.Level1)
{
    bool cb = false;
    EXPECT_EQ(mgr_.GetInterfaceIsCallback("", &cb), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetIsCallback_NotFound_3500, TestSize.Level1)
{
    bool cb = false;
    EXPECT_EQ(mgr_.GetInterfaceIsCallback("NotExist", &cb), ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, GetIsCallback_Callback_3600, TestSize.Level1)
{
    bool cb = false;
    EXPECT_EQ(mgr_.GetInterfaceIsCallback("ITestCallback", &cb), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(cb);
}

HWTEST_F(MoDispatcherMetadataTest, GetIsCallback_NotCallback_3700, TestSize.Level1)
{
    bool cb = true;
    EXPECT_EQ(mgr_.GetInterfaceIsCallback("ITestService", &cb), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(cb);
}

// ---- GetInterfaceDescriptor ----
HWTEST_F(MoDispatcherMetadataTest, GetDescriptor_Null_3800, TestSize.Level1)
{
    std::u16string desc;
    EXPECT_EQ(mgr_.GetInterfaceDescriptor("ITestService", nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetDescriptor_EmptyName_3900, TestSize.Level1)
{
    std::u16string desc;
    EXPECT_EQ(mgr_.GetInterfaceDescriptor("", &desc), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetDescriptor_Success_4000, TestSize.Level1)
{
    std::u16string desc;
    EXPECT_EQ(mgr_.GetInterfaceDescriptor("ITestService", &desc), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_FALSE(desc.empty());
}

// ---- GetMethodCount/Name/MemberId/ReturnType/ParamCount/ParamType/ParamName ----
HWTEST_F(MoDispatcherMetadataTest, GetMethodCount_Null_4100, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetMethodCount(nullptr, &cnt), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodCount_EmptyName_4200, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetMethodCount("", &cnt), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodCount_NotFound_4300, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetMethodCount("NoExist", &cnt), ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodCount_Success_4400, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetMethodCount("ITestService", &cnt), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(cnt, 1u);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodName_Null_4500, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetMethodName("ITestService", 0, nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodName_OutOfRange_4600, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetMethodName("ITestService", 99, &name), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodName_Success_4700, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetMethodName("ITestService", 0, &name), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(name, "TestMethod");
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodMemberId_Null_4800, TestSize.Level1)
{
    EXPECT_EQ(mgr_.GetMethodMemberId("ITestService", "TestMethod", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodMemberId_NotFound_4900, TestSize.Level1)
{
    uint32_t id = 0;
    EXPECT_EQ(mgr_.GetMethodMemberId("ITestService", "NoMethod", &id),
        ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodMemberId_Success_5000, TestSize.Level1)
{
    uint32_t id = 0;
    EXPECT_EQ(mgr_.GetMethodMemberId("ITestService", "TestMethod", &id),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(id, 301u);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodReturnType_Null_5100, TestSize.Level1)
{
    EXPECT_EQ(mgr_.GetMethodReturnType("ITestService", "TestMethod", nullptr),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodReturnType_Success_5200, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo rt = {};
    EXPECT_EQ(mgr_.GetMethodReturnType("ITestService", "TestMethod", &rt),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(rt.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodParamCount_Success_5300, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetMethodParamCount("ITestService", "TestMethod", &cnt),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(cnt, 1u);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodParamType_OutOfRange_5400, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo pt = {};
    EXPECT_EQ(mgr_.GetMethodParamType("ITestService", "TestMethod", 99, &pt),
        ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodParamType_Success_5500, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo pt = {};
    EXPECT_EQ(mgr_.GetMethodParamType("ITestService", "TestMethod", 0, &pt),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(pt.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING);
}

HWTEST_F(MoDispatcherMetadataTest, GetMethodParamName_Success_5600, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetMethodParamName("ITestService", "TestMethod", 0, &name),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(name, "param1");
}

// ---- GetEnumCount/Name/ValueCount/ValueName/Value ----
HWTEST_F(MoDispatcherMetadataTest, GetEnumCount_Null_5700, TestSize.Level1)
{
    EXPECT_EQ(mgr_.GetEnumCount(nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumCount_Success_5800, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetEnumCount(&cnt), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(cnt, 1u);
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumName_Null_5900, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetEnumName(0, nullptr), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumName_OutOfRange_6000, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetEnumName(99, &name), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumName_Success_6100, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetEnumName(0, &name), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(name, "TestEnum");
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumValueCount_NotFound_6200, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetEnumValueCount("NoEnum", &cnt), ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumValueCount_Success_6300, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetEnumValueCount("TestEnum", &cnt), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(cnt, 2u);
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumValueName_OutOfRange_6400, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetEnumValueName("TestEnum", 99, &name), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumValueName_Success_6500, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetEnumValueName("TestEnum", 0, &name), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(name, "VAL_A");
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumValue_NotFoundEnum_6600, TestSize.Level1)
{
    int32_t val = 0;
    EXPECT_EQ(mgr_.GetEnumValue("NoEnum", "VAL_A", &val), ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumValue_NotFoundValue_6700, TestSize.Level1)
{
    int32_t val = 0;
    EXPECT_EQ(mgr_.GetEnumValue("TestEnum", "NoVal", &val), ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, GetEnumValue_Success_6800, TestSize.Level1)
{
    int32_t val = 0;
    EXPECT_EQ(mgr_.GetEnumValue("TestEnum", "VAL_B", &val), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(val, 1);
}

// ---- GetStructCount/Name/FieldCount/FieldName/FieldType ----
HWTEST_F(MoDispatcherMetadataTest, GetStructCount_Success_6900, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetStructCount(&cnt), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(cnt, 1u);
}

HWTEST_F(MoDispatcherMetadataTest, GetStructName_OutOfRange_7000, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetStructName(99, &name), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetStructName_Success_7100, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetStructName(0, &name), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(name, "TestStruct");
}

HWTEST_F(MoDispatcherMetadataTest, GetStructFieldCount_NotFound_7200, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetStructFieldCount("NoStruct", &cnt), ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, GetStructFieldCount_Success_7300, TestSize.Level1)
{
    uint32_t cnt = 0;
    EXPECT_EQ(mgr_.GetStructFieldCount("TestStruct", &cnt), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(cnt, 2u);
}

HWTEST_F(MoDispatcherMetadataTest, GetStructFieldName_OutOfRange_7400, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetStructFieldName("TestStruct", 99, &name), ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, GetStructFieldName_Success_7500, TestSize.Level1)
{
    std::string name;
    EXPECT_EQ(mgr_.GetStructFieldName("TestStruct", 0, &name), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(name, "field1");
}

HWTEST_F(MoDispatcherMetadataTest, GetStructFieldType_NotFound_7600, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo ft = {};
    EXPECT_EQ(mgr_.GetStructFieldType("TestStruct", "NoField", &ft),
        ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND);
}

HWTEST_F(MoDispatcherMetadataTest, GetStructFieldType_Success_7700, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo ft = {};
    EXPECT_EQ(mgr_.GetStructFieldType("TestStruct", "field1", &ft),
        ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(ft.vt, OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32);
}

// ---- ParseTypeInfoFromJson edge cases ----
HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_UnresolvedIdlType_7800, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "structs":[{"name":"S","memberId":200,"fields":[
            {"name":"f","memberId":201,"type_info":{"type":"struct","idl_type":"Undeclared"}}
        ]}],
        "interfaces":[{"name":"I","memberId":300,"interface_type":1,"methods":[]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_MapType_7900, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","memberId":300,"interface_type":1,"methods":[
            {"name":"M","memberId":301,"code":1,"return_type":{"type":"void"},
             "parameters":[{"name":"p","memberId":302,
                "type_info":{"type":"map","key_type":{"type":"string"},"value_type":{"type":"i32"}}}]}
        ]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_ArrayType_8000, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","memberId":300,"interface_type":1,"methods":[
            {"name":"M","memberId":301,"code":1,"return_type":{"type":"void"},
             "parameters":[{"name":"p","memberId":302,
                "type_info":{"type":"array","value_type":{"type":"i32"},"size":5}}]}
        ]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_VectorType_8100, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","memberId":300,"interface_type":1,"methods":[
            {"name":"M","memberId":301,"code":1,"return_type":{"type":"void"},
             "parameters":[{"name":"p","memberId":302,
                "type_info":{"type":"vector","value_type":{"type":"i32"}}}]}
        ]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_SetType_8200, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","memberId":300,"interface_type":1,"methods":[
            {"name":"M","memberId":301,"code":1,"return_type":{"type":"void"},
             "parameters":[{"name":"p","memberId":302,
                "type_info":{"type":"set","value_type":{"type":"string"}}}]}
        ]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_UnknownType_8300, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","memberId":300,"interface_type":1,"methods":[
            {"name":"M","memberId":301,"code":1,"return_type":{"type":"void"},
             "parameters":[{"name":"p","memberId":302,"type_info":{"type":"unknown_type_xyz"}}]}
        ]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_MapMissingKeyType_8400, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","memberId":300,"interface_type":1,"methods":[
            {"name":"M","memberId":301,"code":1,"return_type":{"type":"void"},
             "parameters":[{"name":"p","memberId":302,"type_info":{"type":"map"}}]}
        ]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_MapKeyTypeNotSimple_8500, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"I","memberId":300,"interface_type":1,"methods":[
            {"name":"M","memberId":301,"code":1,"return_type":{"type":"void"},
             "parameters":[{"name":"p","memberId":302,
                "type_info":{"type":"map","key_type":{"type":"map","key_type":{"type":"i32"},"value_type":{"type":"i32"}},"value_type":{"type":"i32"}}}]}
        ]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_METADATA_INVALID);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_EnumIdlType_8600, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "enums":[{"name":"Color","memberId":100,"values":[{"name":"Red","value":0,"memberId":101}]}],
        "interfaces":[{"name":"I","memberId":300,"interface_type":1,"methods":[
            {"name":"M","memberId":301,"code":1,"return_type":{"type":"void"},
             "parameters":[{"name":"p","memberId":302,"type_info":{"type":"enum","idl_type":"Color"}}]}
        ]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

HWTEST_F(MoDispatcherMetadataTest, ParseMetadata_InterfaceIdlType_8700, TestSize.Level1)
{
    ModObjDispatcherMetadataManager m;
    EXPECT_EQ(m.ParseMetadata(R"({
        "interfaces":[{"name":"IMyService","memberId":300,"interface_type":1,"methods":[
            {"name":"M","memberId":301,"code":1,"return_type":{"type":"void"},
             "parameters":[{"name":"p","memberId":302,
                "type_info":{"type":"interface","idl_type":"IMyCallback"}}]}
        ]},
        {"name":"IMyCallback","memberId":400,"interface_type":2,"methods":[]}]
    })"), ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

} // namespace AbilityRuntime
} // namespace OHOS
