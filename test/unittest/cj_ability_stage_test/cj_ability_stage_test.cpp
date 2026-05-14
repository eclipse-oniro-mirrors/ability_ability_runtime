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

#include <gtest/gtest.h>
#include <securec.h>

#include "cj_ability_stage.h"
#include "cj_ability_stage_context.h"
#include "cj_runtime.h"
#include "context_impl.h"
#include "ffi_remote_data.h"
#include "hilog_wrapper.h"
#include "runtime.h"
#include "cj_ability_stage_object.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;

namespace OHOS {
namespace AbilityRuntime {
class CjAbilityStageTest : public testing::Test {
public:
    CjAbilityStageTest()
    {}
    ~CjAbilityStageTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
    std::unique_ptr<AbilityRuntime::CJAbilityStage> cjAbilityStage_ = nullptr;
    std::unique_ptr<AbilityRuntime::CJAbilityStage> initCjAbilityStage_ = nullptr;
};

void (*g_registerFunc)(CJAbilityStageFuncs* result) = [](CJAbilityStageFuncs *result) {
        result->LoadAbilityStage = [](const char *moduleName) -> int64_t { return moduleName[0] == '0' ? 0 : 1; };
        result->ReleaseAbilityStage = [](int64_t handle) {};
        result->AbilityStageOnCreate = [](int64_t handle) {};
        result->AbilityStageOnAcceptWant = [](int64_t handle, OHOS::AAFwk::Want *want) -> char* {
            std::string str = "Hello, world!";
            char* cstr = new char[str.length() + 1];
            if (memcpy_s(cstr, str.length() + 1, str.c_str(), str.size()) != EOK) {
                delete[] cstr;
                return nullptr;
            }
            return cstr;
            };
        result->AbilityStageOnConfigurationUpdated = [](int64_t id, CJConfiguration configuration) {};
        result->AbilityStageOnMemoryLevel = [](int64_t id, int32_t level) {};
    };
void CjAbilityStageTest::SetUpTestCase(void)
{}

void CjAbilityStageTest::TearDownTestCase(void)
{}

void CjAbilityStageTest::SetUp(void)
{
    RegisterCJAbilityStageFuncs(g_registerFunc);
    std::string moduleName = "0";
    auto proxy = CJAbilityStageObject::LoadModule(moduleName);
    cjAbilityStage_ = std::make_unique<AbilityRuntime::CJAbilityStage>(proxy);
    auto cjProxy = CJAbilityStageObject::LoadModule("1");
    initCjAbilityStage_ = std::make_unique<AbilityRuntime::CJAbilityStage>(cjProxy);
}

void CjAbilityStageTest::TearDown(void)
{}

/**
 * @tc.name: CjAbilityStageTestCreate_001
 * @tc.desc: CjAbilityStageTest test for Create.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestCreate_001, TestSize.Level2)
{
    AppExecFwk::HapModuleInfo hapModuleInfo;
    std::unique_ptr<Runtime> nullRuntime = nullptr;
    auto cjStage = CJAbilityStage::Create(nullRuntime, hapModuleInfo);
    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);
    auto cjAbilityStage = CJAbilityStage::Create(runtime, hapModuleInfo);
    EXPECT_TRUE(cjAbilityStage == nullptr);
}

/**
 * @tc.name: CjAbilityStageTestOnCreate_001
 * @tc.desc: CjAbilityStageTest test for OnCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestOnCreate_001, TestSize.Level2)
{
    AAFwk::Want want;
    cjAbilityStage_->OnCreate(want);
    EXPECT_NE(cjAbilityStage_, nullptr);
    initCjAbilityStage_->OnCreate(want);
    EXPECT_NE(initCjAbilityStage_, nullptr);
}

/**
 * @tc.name: CjAbilityStageTestOnAcceptWant_001
 * @tc.desc: CjAbilityStageTest test for OnAcceptWant.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestOnAcceptWant_001, TestSize.Level2)
{
    AAFwk::Want want;
    auto info = cjAbilityStage_->OnAcceptWant(want);
    EXPECT_TRUE(info == "");
    auto ret = initCjAbilityStage_->OnAcceptWant(want);
    EXPECT_TRUE(ret != "");
}

/**
 * @tc.name: CjAbilityStageTestOnMemoryLevel_001
 * @tc.desc: CjAbilityStageTest test for OnMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestOnMemoryLevel_001, TestSize.Level2)
{
    int level = 1;
    cjAbilityStage_->OnMemoryLevel(level);
    EXPECT_NE(cjAbilityStage_, nullptr);
    initCjAbilityStage_->OnMemoryLevel(level);
    EXPECT_NE(initCjAbilityStage_, nullptr);
}

/**
 * @tc.name: CjAbilityStageTest_FFICJGetConfigurationV2_001
 * @tc.desc: Test FFICJGetConfigurationV2 with invalid id, covers null-check branch.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTest_FFICJGetConfigurationV2_001, TestSize.Level1)
{
    auto result = FFICJGetConfigurationV2(0);
    EXPECT_EQ(result.language, nullptr);

    result = FFICJGetConfigurationV2(-1);
    EXPECT_EQ(result.language, nullptr);
}

/**
 * @tc.name: CjAbilityStageTest_FFICJGetConfigurationV2_002
 * @tc.desc: Test FFICJGetConfigurationV2 with valid context but null configuration,
 *           covers CJAbilityStageContext::GetConfigurationV2 null-configuration branch.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTest_FFICJGetConfigurationV2_002, TestSize.Level1)
{
    auto contextImpl = std::make_shared<ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto cjStageContext = FFIData::Create<CJAbilityStageContext>(contextImpl);
    EXPECT_NE(cjStageContext, nullptr);

    auto result = FFICJGetConfigurationV2(cjStageContext->GetID());
    EXPECT_EQ(result.language, nullptr);
}

/**
 * @tc.name: CjAbilityStageTest_FFICJGetConfigurationV2_003
 * @tc.desc: Test FFICJGetConfigurationV2 with valid context and configuration,
 *           covers success path through FillConfigV1Fields conversion.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTest_FFICJGetConfigurationV2_003, TestSize.Level1)
{
    auto contextImpl = std::make_shared<ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);

    auto configuration = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(configuration, nullptr);
    configuration->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh_CN");
    contextImpl->SetConfiguration(configuration);

    auto cjStageContext = FFIData::Create<CJAbilityStageContext>(contextImpl);
    EXPECT_NE(cjStageContext, nullptr);

    auto result = FFICJGetConfigurationV2(cjStageContext->GetID());
    EXPECT_NE(result.language, nullptr);
    EXPECT_EQ(result.colorMode, -1);
    EXPECT_EQ(result.direction, -1);
}

/**
 * @tc.name: CjAbilityStageTest_CallConvertConfigV2_001
 * @tc.desc: Test CallConvertConfigV2 with configuration, covers FillConfigV1Fields conversion.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTest_CallConvertConfigV2_001, TestSize.Level1)
{
    auto configuration = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(configuration, nullptr);
    configuration->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");

    auto result = CallConvertConfigV2(configuration);
    EXPECT_NE(result.language, nullptr);
    EXPECT_EQ(result.colorMode, -1);
    EXPECT_EQ(result.direction, -1);
}

/**
 * @tc.name: CjAbilityStageTest_GetConfigurationV2_NullContext_001
 * @tc.desc: Test CJAbilityStageContext::GetConfigurationV2 with null context,
 *           covers null-context branch.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTest_GetConfigurationV2_NullContext_001, TestSize.Level1)
{
    std::weak_ptr<Context> weakContext;
    auto cjStageContext = FFIData::Create<CJAbilityStageContext>(std::move(weakContext));
    EXPECT_NE(cjStageContext, nullptr);

    auto result = cjStageContext->GetConfigurationV2();
    EXPECT_EQ(result.language, nullptr);
}

}  // namespace AbilityRuntime
}  // namespace OHOS
