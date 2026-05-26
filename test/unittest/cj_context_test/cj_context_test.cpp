/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "cj_context.h"
#include "context.h"
#include "context_impl.h"
#include "cj_utils_ffi.h"
#include "cj_macro.h"
#include "cj_application_context.h"
#include "cj_extension_context.h"
#include "extension_context.h"
#include "cj_common_ffi.h"

using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FfiContext;

extern "C" {
    void FfiContextSwitchArea(int64_t id, int32_t mode);
    int32_t FfiContextGetArea(int64_t id, int32_t type);
    CJ_EXPORT int32_t FFICJExtCtxGetConfigV2(int64_t id, void* paramConfig);
}

class FfiContextSwitchAreaTest : public ::testing::Test {};

/**
 * @tc.name  : FfiContextSwitchArea_ShouldLogError_WhenContextIsNull
 * @tc.number: FfiContextSwitchAreaTest_001
 */
HWTEST_F(FfiContextSwitchAreaTest, ATC_FfiContextSwitchArea_ShouldLogError_WhenContextIsNull, TestSize.Level0) {
    int64_t id = 123;
    int32_t mode = 4;
    FfiContextSwitchArea(id, mode);
}

/**
 * @tc.name  : FfiContextSwitchArea_ShouldCallSwitchArea_WhenContextIsNotNull
 * @tc.number: FfiContextSwitchAreaTest_002
 */
HWTEST_F(FfiContextSwitchAreaTest, ATC_FfiContextSwitchArea_ShouldCallSwitchArea_WhenContextIsNotNull,
    TestSize.Level0) {
    int32_t mode = 4;
    std::shared_ptr<ContextImpl> mockContext = std::make_shared<ContextImpl>();
    EXPECT_NE(mockContext, nullptr);
    auto cjContext = FFIData::Create<CJContext>(mockContext);
    EXPECT_NE(cjContext, nullptr);
    FfiContextSwitchArea(cjContext->GetID(), mode);
    auto modeGet = FfiContextGetArea(cjContext->GetID(), 0);
    EXPECT_EQ(modeGet, mode);
}

class FfiExtCtxGetConfigV2Test : public ::testing::Test {};

/**
 * @tc.name: FfiExtCtxGetConfigV2_ErrorBranches_001
 * @tc.desc: Test FFICJExtCtxGetConfigV2 with nullptr paramConfig, invalid id, and null ExtensionContext.
 * @tc.type: FUNC
 */
HWTEST_F(FfiExtCtxGetConfigV2Test, FfiExtCtxGetConfigV2_ErrorBranches_001, TestSize.Level1)
{
    int32_t result = FFICJExtCtxGetConfigV2(0, nullptr);
    EXPECT_EQ(result, ERR_INVALID_INSTANCE_CODE);

    CConfigurationV2 config = {};
    result = FFICJExtCtxGetConfigV2(-1, &config);
    EXPECT_EQ(result, ERR_INVALID_INSTANCE_CODE);

    std::shared_ptr<ExtensionContext> nullExtContext;
    auto abilityInfo = std::make_shared<OHOS::AppExecFwk::AbilityInfo>();
    auto cjExtContext = FFIData::Create<CJExtensionContext>(nullExtContext, abilityInfo);
    EXPECT_NE(cjExtContext, nullptr);

    CConfigurationV2 config2 = {};
    result = FFICJExtCtxGetConfigV2(cjExtContext->GetID(), &config2);
    EXPECT_EQ(result, ERR_INVALID_INSTANCE_CODE);
}

/**
 * @tc.name: FfiExtCtxGetConfigV2_SuccessBranch_001
 * @tc.desc: Test FFICJExtCtxGetConfigV2 with valid ExtensionContext and Configuration.
 * @tc.type: FUNC
 */
HWTEST_F(FfiExtCtxGetConfigV2Test, FfiExtCtxGetConfigV2_SuccessBranch_001, TestSize.Level1)
{
    auto extContext = std::make_shared<ExtensionContext>();
    EXPECT_NE(extContext, nullptr);

    auto configuration = std::make_shared<OHOS::AppExecFwk::Configuration>();
    configuration->AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh_CN");
    extContext->SetConfiguration(configuration);

    auto abilityInfo = std::make_shared<OHOS::AppExecFwk::AbilityInfo>();
    auto cjExtContext = FFIData::Create<CJExtensionContext>(extContext, abilityInfo);
    EXPECT_NE(cjExtContext, nullptr);

    CConfigurationV2 config = {};
    int32_t result = FFICJExtCtxGetConfigV2(cjExtContext->GetID(), &config);
    EXPECT_EQ(result, SUCCESS_CODE);
    EXPECT_TRUE(config.language != nullptr);

    FreeCConfigurationV2(&config);
}