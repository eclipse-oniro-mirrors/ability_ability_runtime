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

#include "cj_ability_context_object.h"
#include "cj_ability_context_broker.h"
#include "cj_ability_context.h"
#include "ability_context_impl.h"
#include "cj_utils_ffi.h"
#include "ffi_remote_data.h"
#include "ability_business_error.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;

namespace OHOS {
namespace AbilityRuntime {
class CjAbilityContextProxyTest : public testing::Test {
};

int g_result = 0;

HWTEST_F(CjAbilityContextProxyTest, RegisterCJAbilityCallbacks_0100, TestSize.Level1)
{
    RegisterCJAbilityCallbacks(nullptr);
    void (*registerFunc)(CJAbilityCallbacks*) = [](CJAbilityCallbacks* cjAbilityCallbacks)
    {
        if (cjAbilityCallbacks != nullptr) g_result += 1;
        cjAbilityCallbacks->invokeAbilityResultCallback = [](int64_t id, int32_t error,
                                                              CJAbilityResult* cjAbilityResult) {};
        cjAbilityCallbacks->invokePermissionRequestResultCallback =
            [](int64_t id, int32_t error, CJPermissionRequestResult* cjPermissionRequestResult) {};
        cjAbilityCallbacks->invokeDialogRequestResultCallback = [](int64_t id, int32_t error,
                                                                    CJDialogRequestResult* cjDialogRequestResult) {};
    };
    RegisterCJAbilityCallbacks(registerFunc);
    RegisterCJAbilityCallbacks(registerFunc);
    EXPECT_EQ(g_result, 1);
}

class FFIAbilityContextPropConfigurationV2Test : public testing::Test {
};

/**
 * @tc.name  : FFIAbilityContextPropConfigurationV2_InvalidId_0100
 * @tc.desc  : Test FFIAbilityContextPropConfigurationV2 with invalid id, errCode should be ERR_INVALID_INSTANCE_CODE.
 * @tc.type  : FUNC
 */
HWTEST_F(FFIAbilityContextPropConfigurationV2Test, FFIAbilityContextPropConfigurationV2_InvalidId_0100, TestSize.Level1)
{
    int32_t errCode = 0;
    auto result = FFIAbilityContextPropConfigurationV2(0, &errCode);
    EXPECT_EQ(errCode, ERR_INVALID_INSTANCE_CODE);

    errCode = 0;
    result = FFIAbilityContextPropConfigurationV2(-1, &errCode);
    EXPECT_EQ(errCode, ERR_INVALID_INSTANCE_CODE);
}

/**
 * @tc.name  : FFIAbilityContextPropConfigurationV2_NullConfiguration_0100
 * @tc.desc  : Test FFIAbilityContextPropConfigurationV2 with valid context but null configuration,
 *             errCode should be ERROR_CODE_INVALID_CONTEXT.
 * @tc.type  : FUNC
 */
HWTEST_F(FFIAbilityContextPropConfigurationV2Test, FFIAbilityContextPropConfigurationV2_NullConfiguration_0100,
    TestSize.Level1)
{
    auto abilityContext = std::make_shared<AbilityContextImpl>();
    EXPECT_NE(abilityContext, nullptr);
    auto cjAbilityContext = FFIData::Create<CJAbilityContext>(abilityContext);
    EXPECT_NE(cjAbilityContext, nullptr);

    int32_t errCode = 0;
    auto result = FFIAbilityContextPropConfigurationV2(cjAbilityContext->GetID(), &errCode);
    EXPECT_EQ(errCode, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    (void)result;
}

/**
 * @tc.name  : FFIAbilityContextPropConfigurationV2_Success_0100
 * @tc.desc  : Test FFIAbilityContextPropConfigurationV2 with valid context and configuration,
 *             errCode should be SUCCESS_CODE and CConfigurationV2 fields should reflect
 *             the converted values from FillConfigV1Fields (colorMode=-1, direction=-1 for empty config).
 * @tc.type  : FUNC
 */
HWTEST_F(FFIAbilityContextPropConfigurationV2Test, FFIAbilityContextPropConfigurationV2_Success_0100, TestSize.Level1)
{
    auto abilityContext = std::make_shared<AbilityContextImpl>();
    EXPECT_NE(abilityContext, nullptr);

    auto configuration = std::make_shared<OHOS::AppExecFwk::Configuration>();
    EXPECT_NE(configuration, nullptr);
    abilityContext->SetConfiguration(configuration);

    auto cjAbilityContext = FFIData::Create<CJAbilityContext>(abilityContext);
    EXPECT_NE(cjAbilityContext, nullptr);

    int32_t errCode = 0;
    auto result = FFIAbilityContextPropConfigurationV2(cjAbilityContext->GetID(), &errCode);
    EXPECT_EQ(errCode, SUCCESS_CODE);
    EXPECT_EQ(result.language, nullptr);
    EXPECT_EQ(result.colorMode, -1);
    EXPECT_EQ(result.direction, -1);
}
}
}