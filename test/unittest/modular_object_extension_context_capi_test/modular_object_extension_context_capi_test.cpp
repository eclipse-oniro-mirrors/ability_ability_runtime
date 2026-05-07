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
    }
    void TearDown() override {}
};

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
