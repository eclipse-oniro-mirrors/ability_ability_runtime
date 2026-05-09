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

#include "modular_object_extension_context.h"
#include "modular_object_extension_types.h"
#include "modular_object_extension_context_impl.h"
#include "want_manager.h"
#include "want_utils.h"
#include "start_options_impl.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

ErrCode ModularObjectExtensionContext::g_startSelfResult = ERR_OK;
ErrCode ModularObjectExtensionContext::g_startSelfWithOptionsResult = ERR_OK;
ErrCode ModularObjectExtensionContext::g_terminateResult = ERR_OK;

} // namespace AbilityRuntime
} // namespace OHOS

int OHOS::AAFwk::CWantManager::g_transformResult = 0;
AbilityRuntime_ErrorCode g_checkWantResult = ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;

namespace {
bool g_remoteStubCreateSuccess = true;
int g_remoteStubDestroyCount = 0;
OHIPCRemoteStub *g_lastDestroyedStub = nullptr;
const char *g_lastCreateDescriptor = nullptr;
OH_OnRemoteRequestCallback g_lastRequestCallback = nullptr;
OH_OnRemoteDestroyCallback g_lastDestroyCallback = nullptr;
void *g_lastRemoteStubUserData = nullptr;
OHIPCRemoteStub g_mockRemoteStub {};
} // namespace

extern "C" OHIPCRemoteStub* OH_IPCRemoteStub_Create(const char *descriptor,
    OH_OnRemoteRequestCallback requestCallback, OH_OnRemoteDestroyCallback destroyCallback, void *userData)
{
    g_lastCreateDescriptor = descriptor;
    g_lastRequestCallback = requestCallback;
    g_lastDestroyCallback = destroyCallback;
    g_lastRemoteStubUserData = userData;
    return g_remoteStubCreateSuccess ? &g_mockRemoteStub : nullptr;
}

extern "C" void OH_IPCRemoteStub_Destroy(OHIPCRemoteStub *stub)
{
    g_lastDestroyedStub = stub;
    ++g_remoteStubDestroyCount;
}

AbilityRuntime_ErrorCode CheckWant(AbilityBase_Want *want)
{
    return g_checkWantResult;
}

class ModularObjectExtensionContextCapiTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        OHOS::AbilityRuntime::ModularObjectExtensionContext::g_startSelfResult = ERR_OK;
        OHOS::AbilityRuntime::ModularObjectExtensionContext::g_startSelfWithOptionsResult = ERR_OK;
        OHOS::AbilityRuntime::ModularObjectExtensionContext::g_terminateResult = ERR_OK;
        OHOS::AAFwk::CWantManager::g_transformResult = 0;
        g_checkWantResult = ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        g_remoteStubCreateSuccess = true;
        g_remoteStubDestroyCount = 0;
        g_lastDestroyedStub = nullptr;
        g_lastCreateDescriptor = nullptr;
        g_lastRequestCallback = nullptr;
        g_lastDestroyCallback = nullptr;
        g_lastRemoteStubUserData = nullptr;
    }
    void TearDown() override {}
};

namespace {
int MockRemoteRequest(uint32_t code, const OHIPCParcel *data, OHIPCParcel *reply, void *userData)
{
    (void)code;
    (void)data;
    (void)reply;
    (void)userData;
    return 0;
}

void MockRemoteDestroy(void *userData)
{
    (void)userData;
}
} // namespace

// ==================== GetBaseContext ====================

HWTEST_F(ModularObjectExtensionContextCapiTest, GetBaseContext_NullContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBaseContext_NullContext_001 start";
    AbilityRuntime_ContextHandle baseContext = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_GetBaseContext(nullptr, &baseContext);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "GetBaseContext_NullContext_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, GetBaseContext_NullBaseContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBaseContext_NullBaseContext_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_GetBaseContext(ctx.get(), nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "GetBaseContext_NullBaseContext_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, GetBaseContext_WrongType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBaseContext_WrongType_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    AbilityRuntime_ContextHandle baseContext = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_GetBaseContext(ctx.get(), &baseContext);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE);
    GTEST_LOG_(INFO) << "GetBaseContext_WrongType_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, GetBaseContext_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBaseContext_Success_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    AbilityRuntime_ContextHandle baseContext = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_GetBaseContext(ctx.get(), &baseContext);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_NE(baseContext, nullptr);
    GTEST_LOG_(INFO) << "GetBaseContext_Success_001 end";
}

// ==================== StartSelfUIAbility ====================

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfUIAbility_NullContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbility_NullContext_001 start";
    AbilityBase_Want want;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(nullptr, &want);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "StartSelfUIAbility_NullContext_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfUIAbility_WrongType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbility_WrongType_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    AbilityBase_Want want;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(ctx.get(), &want);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE);
    GTEST_LOG_(INFO) << "StartSelfUIAbility_WrongType_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfUIAbility_ExpiredContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbility_ExpiredContext_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    // context is default-constructed weak_ptr, lock() returns nullptr
    AbilityBase_Want want;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(ctx.get(), &want);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST);
    GTEST_LOG_(INFO) << "StartSelfUIAbility_ExpiredContext_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfUIAbility_InvalidWant_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbility_InvalidWant_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    g_checkWantResult = ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    AbilityBase_Want want;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(ctx.get(), &want);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "StartSelfUIAbility_InvalidWant_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfUIAbility_TransformFail_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbility_TransformFail_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    OHOS::AAFwk::CWantManager::g_transformResult = 1; // non-zero = failure
    AbilityBase_Want want;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(ctx.get(), &want);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "StartSelfUIAbility_TransformFail_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfUIAbility_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbility_Success_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    AbilityBase_Want want;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(ctx.get(), &want);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    GTEST_LOG_(INFO) << "StartSelfUIAbility_Success_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfUIAbility_StartError_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfUIAbility_StartError_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    OHOS::AbilityRuntime::ModularObjectExtensionContext::g_startSelfResult = -1;
    AbilityBase_Want want;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(ctx.get(), &want);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
    GTEST_LOG_(INFO) << "StartSelfUIAbility_StartError_001 end";
}

// ==================== StartSelfUIAbilityWithStartOptions ====================

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfWithOpts_NullOptions_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfWithOpts_NullOptions_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    AbilityBase_Want want;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbilityWithStartOptions(ctx.get(), &want, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "StartSelfWithOpts_NullOptions_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfWithOpts_NullContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfWithOpts_NullContext_001 start";
    AbilityBase_Want want;
    AbilityRuntime_StartOptions options;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbilityWithStartOptions(nullptr, &want, &options);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "StartSelfWithOpts_NullContext_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfWithOpts_WrongType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfWithOpts_WrongType_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    AbilityBase_Want want;
    AbilityRuntime_StartOptions options;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbilityWithStartOptions(ctx.get(), &want, &options);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE);
    GTEST_LOG_(INFO) << "StartSelfWithOpts_WrongType_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfWithOpts_ExpiredContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfWithOpts_ExpiredContext_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    AbilityBase_Want want;
    AbilityRuntime_StartOptions options;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbilityWithStartOptions(ctx.get(), &want, &options);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST);
    GTEST_LOG_(INFO) << "StartSelfWithOpts_ExpiredContext_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfWithOpts_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfWithOpts_Success_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    AbilityBase_Want want;
    AbilityRuntime_StartOptions options;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbilityWithStartOptions(ctx.get(), &want, &options);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    GTEST_LOG_(INFO) << "StartSelfWithOpts_Success_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, StartSelfWithOpts_TransformFail_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSelfWithOpts_TransformFail_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    OHOS::AAFwk::CWantManager::g_transformResult = 1;
    AbilityBase_Want want;
    AbilityRuntime_StartOptions options;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbilityWithStartOptions(ctx.get(), &want, &options);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "StartSelfWithOpts_TransformFail_001 end";
}

// ==================== TerminateSelf ====================

HWTEST_F(ModularObjectExtensionContextCapiTest, TerminateSelf_NullContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelf_NullContext_001 start";
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_TerminateSelf(nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    GTEST_LOG_(INFO) << "TerminateSelf_NullContext_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, TerminateSelf_WrongType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelf_WrongType_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_TerminateSelf(ctx.get());
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE);
    GTEST_LOG_(INFO) << "TerminateSelf_WrongType_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, TerminateSelf_ExpiredContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelf_ExpiredContext_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_TerminateSelf(ctx.get());
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST);
    GTEST_LOG_(INFO) << "TerminateSelf_ExpiredContext_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, TerminateSelf_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelf_Success_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_TerminateSelf(ctx.get());
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    GTEST_LOG_(INFO) << "TerminateSelf_Success_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, TerminateSelf_Error_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelf_Error_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    OHOS::AbilityRuntime::ModularObjectExtensionContext::g_terminateResult = -1;
    auto ret = OH_AbilityRuntime_ModObjExtensionContext_TerminateSelf(ctx.get());
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
    GTEST_LOG_(INFO) << "TerminateSelf_Error_001 end";
}

// ==================== CreateIPCRemoteStub ====================

HWTEST_F(ModularObjectExtensionContextCapiTest, CreateIPCRemoteStub_NullDescriptor_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_NullDescriptor_001 start";
    auto stub = OH_AbilityRuntime_ModObjExtensionContext_CreateIPCRemoteStub(
        nullptr, nullptr, MockRemoteRequest, MockRemoteDestroy, nullptr);
    EXPECT_EQ(stub, nullptr);
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_NullDescriptor_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, CreateIPCRemoteStub_NullRequestCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_NullRequestCallback_001 start";
    auto stub = OH_AbilityRuntime_ModObjExtensionContext_CreateIPCRemoteStub(
        nullptr, "descriptor", nullptr, MockRemoteDestroy, nullptr);
    EXPECT_EQ(stub, nullptr);
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_NullRequestCallback_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, CreateIPCRemoteStub_WrongType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_WrongType_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    auto stub = OH_AbilityRuntime_ModObjExtensionContext_CreateIPCRemoteStub(
        ctx.get(), "descriptor", MockRemoteRequest, MockRemoteDestroy, nullptr);
    EXPECT_EQ(stub, nullptr);
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_WrongType_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, CreateIPCRemoteStub_ExpiredContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_ExpiredContext_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto stub = OH_AbilityRuntime_ModObjExtensionContext_CreateIPCRemoteStub(
        ctx.get(), "descriptor", MockRemoteRequest, MockRemoteDestroy, nullptr);
    EXPECT_EQ(stub, nullptr);
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_ExpiredContext_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, CreateIPCRemoteStub_CreateFail_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_CreateFail_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    g_remoteStubCreateSuccess = false;
    auto stub = OH_AbilityRuntime_ModObjExtensionContext_CreateIPCRemoteStub(
        ctx.get(), "descriptor", MockRemoteRequest, MockRemoteDestroy, nullptr);
    EXPECT_EQ(stub, nullptr);
    EXPECT_STREQ(g_lastCreateDescriptor, "descriptor");
    EXPECT_NE(g_lastRequestCallback, nullptr);
    EXPECT_NE(g_lastDestroyCallback, nullptr);
    EXPECT_NE(g_lastRemoteStubUserData, nullptr);
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_CreateFail_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, CreateIPCRemoteStub_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_Success_001 start";
    int userData = 42;
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    auto stub = OH_AbilityRuntime_ModObjExtensionContext_CreateIPCRemoteStub(
        ctx.get(), "descriptor", MockRemoteRequest, MockRemoteDestroy, &userData);
    EXPECT_EQ(stub, &g_mockRemoteStub);
    EXPECT_STREQ(g_lastCreateDescriptor, "descriptor");
    EXPECT_NE(g_lastRequestCallback, nullptr);
    EXPECT_NE(g_lastDestroyCallback, nullptr);
    EXPECT_NE(g_lastRemoteStubUserData, nullptr);
    GTEST_LOG_(INFO) << "CreateIPCRemoteStub_Success_001 end";
}

// ==================== DestroyIPCRemoteStub ====================

HWTEST_F(ModularObjectExtensionContextCapiTest, DestroyIPCRemoteStub_NullStub_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DestroyIPCRemoteStub_NullStub_001 start";
    OH_AbilityRuntime_ModObjExtensionContext_DestroyIPCRemoteStub(nullptr, nullptr);
    EXPECT_EQ(g_remoteStubDestroyCount, 1);
    EXPECT_EQ(g_lastDestroyedStub, nullptr);
    GTEST_LOG_(INFO) << "DestroyIPCRemoteStub_NullStub_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, DestroyIPCRemoteStub_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DestroyIPCRemoteStub_Success_001 start";
    OHIPCRemoteStub stub {};
    OH_AbilityRuntime_ModObjExtensionContext_DestroyIPCRemoteStub(nullptr, &stub);
    EXPECT_EQ(g_remoteStubDestroyCount, 1);
    EXPECT_EQ(g_lastDestroyedStub, &stub);
    GTEST_LOG_(INFO) << "DestroyIPCRemoteStub_Success_001 end";
}

HWTEST_F(ModularObjectExtensionContextCapiTest, DestroyIPCRemoteStub_ContextAndStubNotNull_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DestroyIPCRemoteStub_ContextAndStubNotNull_001 start";
    auto ctx = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    ctx->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    auto cppCtx = std::make_shared<OHOS::AbilityRuntime::ModularObjectExtensionContext>();
    ctx->context = cppCtx;
    OHIPCRemoteStub stub {};

    OH_AbilityRuntime_ModObjExtensionContext_DestroyIPCRemoteStub(ctx.get(), &stub);

    EXPECT_EQ(g_remoteStubDestroyCount, 1);
    EXPECT_EQ(g_lastDestroyedStub, &stub);
    GTEST_LOG_(INFO) << "DestroyIPCRemoteStub_ContextAndStubNotNull_001 end";
}
