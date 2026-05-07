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
#include <gmock/gmock.h>

#include <cstring>
#include <memory>
#include <string>
#include <securec.h>

#include "ability_manager_errors.h"
#include "ability_runtime_common.h"
#include "c_modular_object_utils.h"
#include "element_name.h"
#include "iremote_object.h"
#include "iremote_broker.h"
#include "message_parcel.h"
#include "message_option.h"
#include "mock_context_base.h"
#include "mock_my_flag.h"
#include "native_extension/context_impl.h"
#include "want_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

// Complete the forward-declared AbilityBase_Want at global scope
struct AbilityBase_Want {
    AbilityBase_Element element;
    int dummy = 0;
};

namespace OHOS {
namespace AbilityRuntime {
namespace {

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"mock_descriptor") {}
    ~MockRemoteObject() = default;

    int32_t GetObjectRefCount() override { return 0; }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        (void)code;
        (void)data;
        (void)reply;
        (void)option;
        return 0;
    }
    bool IsProxyObject() const override { return true; }
    bool CheckObjectLegality() const override { return true; }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        (void)recipient;
        return true;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        (void)recipient;
        return true;
    }
    bool Marshalling(Parcel &parcel) const override
    {
        (void)parcel;
        return true;
    }
    sptr<IRemoteBroker> AsInterface() override { return nullptr; }
    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        (void)fd;
        (void)args;
        return 0;
    }
    std::u16string GetObjectDescriptor() const { return std::u16string(); }
};

} // namespace

class CModularObjectUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override
    {
        MyFlag::retCheckWant = 0;
        MyFlag::retTransformToWant = 0;
        MyFlag::retConvertToCommonBusinessErrorCode = 0;
    }
    void TearDown() override {}
};

// ==================== ConvertConnectBusinessErrorCode ====================

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_001 start";
    // ABILITY_VISIBLE_FALSE_DENY_REQUEST -> VISIBILITY_VERIFICATION_FAILED
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_VISIBILITY_VERIFICATION_FAILED);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_001 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_002 start";
    // ERR_STATIC_CFG_PERMISSION -> STATIC_CFG_PERMISSION
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_STATIC_CFG_PERMISSION);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_STATIC_CFG_PERMISSION);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_002 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_003 start";
    // ERR_CROSS_USER -> CROSS_USER_OPERATION
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_CROSS_USER);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CROSS_USER_OPERATION);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_003 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_004 start";
    // ERR_INVALID_DISTRIBUTION_TYPE -> INVALID_DISTRIBUTION_TYPE
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_INVALID_DISTRIBUTION_TYPE);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INVALID_DISTRIBUTION_TYPE);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_004 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_005 start";
    // ERR_FREQ_START_ABILITY -> UPPER_RATE_LIMIT
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_FREQ_START_ABILITY);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_UPPER_RATE_LIMIT);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_005 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_006 start";
    // ERR_REACH_UPPER_LIMIT -> falls to default -> ConvertToCommonBusinessErrorCode
    MyFlag::retConvertToCommonBusinessErrorCode = ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED;
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_REACH_UPPER_LIMIT);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_006 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_007 start";
    // ERR_UPPER_LIMIT -> falls to default -> ConvertToCommonBusinessErrorCode
    MyFlag::retConvertToCommonBusinessErrorCode = 0;
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_UPPER_LIMIT);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_007 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_008 start";
    // ERR_MODULAR_OBJECT_DISABLED -> NO_SUCH_ABILITY
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_MODULAR_OBJECT_DISABLED);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_SUCH_ABILITY);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_008 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_009 start";
    // ERR_NO_RUNNING_ABILITIES_WITH_UI -> NO_RUNNING_ABILITIES_WITH_UI
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_NO_RUNNING_ABILITIES_WITH_UI);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_RUNNING_ABILITIES_WITH_UI);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_009 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_010 start";
    // default case -> calls ConvertToCommonBusinessErrorCode
    MyFlag::retConvertToCommonBusinessErrorCode = ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED;
    int32_t unknownErrCode = 9999999;
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(unknownErrCode);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_010 end";
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_011 start";
    // default case with mock returning default value
    MyFlag::retConvertToCommonBusinessErrorCode = 0;
    int32_t unknownErrCode = 1;
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(unknownErrCode);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
    GTEST_LOG_(INFO) << "ConvertConnectBusinessErrorCode_011 end";
}

// ==================== CopyToCString ====================

HWTEST_F(CModularObjectUtilsTest, CopyToCString_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyToCString_001 start";
    // success: normal string
    char *dst = nullptr;
    std::string src = "hello";
    bool ret = CModularObjectUtils::CopyToCString(src, dst);
    EXPECT_TRUE(ret);
    ASSERT_NE(dst, nullptr);
    EXPECT_STREQ(dst, "hello");
    delete[] dst;
    GTEST_LOG_(INFO) << "CopyToCString_001 end";
}

HWTEST_F(CModularObjectUtilsTest, CopyToCString_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyToCString_002 start";
    // success: empty string
    char *dst = nullptr;
    std::string src = "";
    bool ret = CModularObjectUtils::CopyToCString(src, dst);
    EXPECT_TRUE(ret);
    ASSERT_NE(dst, nullptr);
    EXPECT_STREQ(dst, "");
    delete[] dst;
    GTEST_LOG_(INFO) << "CopyToCString_002 end";
}

HWTEST_F(CModularObjectUtilsTest, CopyToCString_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyToCString_003 start";
    // success: string with special characters
    char *dst = nullptr;
    std::string src = "com.example.test/Module:Ability";
    bool ret = CModularObjectUtils::CopyToCString(src, dst);
    EXPECT_TRUE(ret);
    ASSERT_NE(dst, nullptr);
    EXPECT_STREQ(dst, src.c_str());
    delete[] dst;
    GTEST_LOG_(INFO) << "CopyToCString_003 end";
}

// ==================== BuildElement ====================

HWTEST_F(CModularObjectUtilsTest, BuildElement_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildElement_001 start";
    // success: all fields populated
    ElementName elementName("", "com.test.bundle", "com.test.module", "TestAbility");
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    bool ret = CModularObjectUtils::BuildElement(elementName, element);
    EXPECT_TRUE(ret);
    ASSERT_NE(element.bundleName, nullptr);
    EXPECT_STREQ(element.bundleName, "com.test.bundle");
    ASSERT_NE(element.moduleName, nullptr);
    EXPECT_STREQ(element.moduleName, "com.test.module");
    ASSERT_NE(element.abilityName, nullptr);
    EXPECT_STREQ(element.abilityName, "TestAbility");
    CModularObjectUtils::DestroyElement(element);
    GTEST_LOG_(INFO) << "BuildElement_001 end";
}

HWTEST_F(CModularObjectUtilsTest, BuildElement_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildElement_002 start";
    // success: fields with empty strings
    ElementName elementName("", "", "", "");
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    bool ret = CModularObjectUtils::BuildElement(elementName, element);
    EXPECT_TRUE(ret);
    ASSERT_NE(element.bundleName, nullptr);
    EXPECT_STREQ(element.bundleName, "");
    ASSERT_NE(element.moduleName, nullptr);
    EXPECT_STREQ(element.moduleName, "");
    ASSERT_NE(element.abilityName, nullptr);
    EXPECT_STREQ(element.abilityName, "");
    CModularObjectUtils::DestroyElement(element);
    GTEST_LOG_(INFO) << "BuildElement_002 end";
}

// ==================== DestroyElement ====================

HWTEST_F(CModularObjectUtilsTest, DestroyElement_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DestroyElement_001 start";
    // destroy populated element
    AbilityBase_Element element;
    element.bundleName = new char[5];
    (void)strcpy_s(element.bundleName, 5, "test");
    element.moduleName = new char[4];
    (void)strcpy_s(element.moduleName, 4, "mod");
    element.abilityName = new char[4];
    (void)strcpy_s(element.abilityName, 4, "abc");

    CModularObjectUtils::DestroyElement(element);
    EXPECT_EQ(element.bundleName, nullptr);
    EXPECT_EQ(element.moduleName, nullptr);
    EXPECT_EQ(element.abilityName, nullptr);
    GTEST_LOG_(INFO) << "DestroyElement_001 end";
}

HWTEST_F(CModularObjectUtilsTest, DestroyElement_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DestroyElement_002 start";
    // destroy element with null fields (no crash)
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    CModularObjectUtils::DestroyElement(element);
    EXPECT_EQ(element.bundleName, nullptr);
    EXPECT_EQ(element.moduleName, nullptr);
    EXPECT_EQ(element.abilityName, nullptr);
    GTEST_LOG_(INFO) << "DestroyElement_002 end";
}

// ==================== NotifyFailed ====================

// Global callback tracking for NotifyFailed tests
namespace {
OH_AbilityRuntime_ConnectOptions *g_capturedOwner = nullptr;
AbilityRuntime_ErrorCode g_capturedErrorCode = ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
int32_t g_callbackCallCount = 0;

void ResetCallbackState()
{
    g_capturedOwner = nullptr;
    g_capturedErrorCode = ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    g_callbackCallCount = 0;
}

void MockOnFailedCallback(OH_AbilityRuntime_ConnectOptions *owner, AbilityRuntime_ErrorCode code)
{
    g_capturedOwner = owner;
    g_capturedErrorCode = code;
    g_callbackCallCount++;
}
} // namespace

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyFailed_001 start";
    // state is nullptr -> early return, no crash
    ResetCallbackState();
    CModularObjectUtils::NotifyFailed(nullptr, ERR_MODULAR_OBJECT_DISABLED);
    EXPECT_EQ(g_callbackCallCount, 0);
    GTEST_LOG_(INFO) << "NotifyFailed_001 end";
}

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyFailed_002 start";
    // state alive is false -> early return
    ResetCallbackState();
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = false;
    state->onFailedCallback = MockOnFailedCallback;
    CModularObjectUtils::NotifyFailed(state, ERR_MODULAR_OBJECT_DISABLED);
    EXPECT_EQ(g_callbackCallCount, 0);
    GTEST_LOG_(INFO) << "NotifyFailed_002 end";
}

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyFailed_003 start";
    // callback is nullptr -> no call
    ResetCallbackState();
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->onFailedCallback = nullptr;
    CModularObjectUtils::NotifyFailed(state, ERR_MODULAR_OBJECT_DISABLED);
    EXPECT_EQ(g_callbackCallCount, 0);
    GTEST_LOG_(INFO) << "NotifyFailed_003 end";
}

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyFailed_004 start";
    // callback is non-null -> callback invoked with correct error code
    ResetCallbackState();
    OH_AbilityRuntime_ConnectOptions owner;
    owner.state = nullptr;

    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->owner = &owner;
    state->onFailedCallback = MockOnFailedCallback;

    CModularObjectUtils::NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED);
    EXPECT_EQ(g_callbackCallCount, 1);
    EXPECT_EQ(g_capturedOwner, &owner);
    EXPECT_EQ(g_capturedErrorCode, ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "NotifyFailed_004 end";
}

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyFailed_005 start";
    // verify error code is cast correctly from int32_t
    ResetCallbackState();
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->owner = nullptr;
    state->onFailedCallback = MockOnFailedCallback;

    int32_t businessCode = ABILITY_RUNTIME_ERROR_CODE_INVALID_DISTRIBUTION_TYPE;
    CModularObjectUtils::NotifyFailed(state, businessCode);
    EXPECT_EQ(g_callbackCallCount, 1);
    EXPECT_EQ(g_capturedErrorCode, ABILITY_RUNTIME_ERROR_CODE_INVALID_DISTRIBUTION_TYPE);
    GTEST_LOG_(INFO) << "NotifyFailed_005 end";
}

// ==================== TransformWant ====================

HWTEST_F(CModularObjectUtilsTest, TransformWant_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransformWant_001 start";
    // CheckWant returns error -> propagate error
    MyFlag::retCheckWant = ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    AbilityBase_Want want;
    want.element = {nullptr, nullptr, nullptr};
    AAFwk::Want abilityWant;
    auto ret = CModularObjectUtils::TransformWant(reinterpret_cast<AbilityBase_Want *>(&want), abilityWant);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "TransformWant_001 end";
}

HWTEST_F(CModularObjectUtilsTest, TransformWant_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransformWant_002 start";
    // CheckWant succeeds, TransformToWant returns error
    MyFlag::retCheckWant = 0;
    MyFlag::retTransformToWant = ABILITY_BASE_ERROR_CODE_PARAM_INVALID;
    AbilityBase_Want want;
    want.element = {nullptr, nullptr, nullptr};
    AAFwk::Want abilityWant;
    auto ret = CModularObjectUtils::TransformWant(reinterpret_cast<AbilityBase_Want *>(&want), abilityWant);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "TransformWant_002 end";
}

HWTEST_F(CModularObjectUtilsTest, TransformWant_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransformWant_003 start";
    // success path
    MyFlag::retCheckWant = 0;
    MyFlag::retTransformToWant = 0;
    AbilityBase_Want want;
    want.element = {nullptr, nullptr, nullptr};
    AAFwk::Want abilityWant;
    auto ret = CModularObjectUtils::TransformWant(reinterpret_cast<AbilityBase_Want *>(&want), abilityWant);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    GTEST_LOG_(INFO) << "TransformWant_003 end";
}

// ==================== CheckContextAndToken ====================

HWTEST_F(CModularObjectUtilsTest, CheckContextAndToken_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckContextAndToken_001 start";
    // context is nullptr
    sptr<IRemoteObject> token;
    auto ret = CModularObjectUtils::CheckContextAndToken(nullptr, token);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "CheckContextAndToken_001 end";
}

HWTEST_F(CModularObjectUtilsTest, CheckContextAndToken_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckContextAndToken_002 start";
    // context->context.lock() returns nullptr (expired weak_ptr)
    AbilityRuntime_Context context;
    context.type = 0;
    // weak_ptr is default-constructed (expired)
    context.context = std::weak_ptr<ContextBase>();

    sptr<IRemoteObject> token;
    auto ret = CModularObjectUtils::CheckContextAndToken(&context, token);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST);
    GTEST_LOG_(INFO) << "CheckContextAndToken_002 end";
}

HWTEST_F(CModularObjectUtilsTest, CheckContextAndToken_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckContextAndToken_003 start";
    // GetToken returns nullptr
    auto mockContext = std::make_shared<MockContext>();
    EXPECT_CALL(*mockContext, GetToken()).Times(1).WillOnce(Return(sptr<IRemoteObject>(nullptr)));

    AbilityRuntime_Context context;
    context.type = 0;
    context.context = mockContext;

    sptr<IRemoteObject> token;
    auto ret = CModularObjectUtils::CheckContextAndToken(&context, token);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST);
    GTEST_LOG_(INFO) << "CheckContextAndToken_003 end";
}

HWTEST_F(CModularObjectUtilsTest, CheckContextAndToken_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckContextAndToken_004 start";
    // success path: GetToken returns valid token
    sptr<IRemoteObject> expectedToken = sptr<IRemoteObject>(new (std::nothrow) MockRemoteObject());
    ASSERT_NE(expectedToken, nullptr);
    auto mockContext = std::make_shared<MockContext>();
    EXPECT_CALL(*mockContext, GetToken()).Times(1).WillOnce(Return(expectedToken));

    AbilityRuntime_Context context;
    context.type = 0;
    context.context = mockContext;

    sptr<IRemoteObject> token;
    auto ret = CModularObjectUtils::CheckContextAndToken(&context, token);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(token.GetRefPtr(), expectedToken.GetRefPtr());
    GTEST_LOG_(INFO) << "CheckContextAndToken_004 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
