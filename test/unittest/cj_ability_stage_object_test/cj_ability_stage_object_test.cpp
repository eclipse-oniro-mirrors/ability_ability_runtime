/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cj_ability_stage_object.h"
#include "cj_runtime.h"
#include "hilog_wrapper.h"
#include "runtime.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class CjAbilityStageObjectTest : public testing::Test {
public:
    CjAbilityStageObjectTest()
    {}
    ~CjAbilityStageObjectTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void CjAbilityStageObjectTest::SetUpTestCase(void)
{}

void CjAbilityStageObjectTest::TearDownTestCase(void)
{}

void CjAbilityStageObjectTest::SetUp(void)
{}

void CjAbilityStageObjectTest::TearDown(void)
{}

HWTEST_F(CjAbilityStageObjectTest, CjAbilityNoInit_001, TestSize.Level0)
{
    std::shared_ptr<CJAbilityStageObject> cjAbilityStageObject = CJAbilityStageObject::LoadModule("test");
    cjAbilityStageObject->OnCreate();

    AAFwk::Want want;
    cjAbilityStageObject->OnAcceptWant(want);

    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    cjAbilityStageObject->OnConfigurationUpdated(configuration);

    int32_t level = 1;
    cjAbilityStageObject->OnMemoryLevel(level);
}

HWTEST_F(CjAbilityStageObjectTest, CjAbilityNoInit_002, TestSize.Level0)
{
    RegisterCJAbilityStageFuncs(nullptr);
    auto registerFunc = [](CJAbilityStageFuncs *funcs) {
        funcs->LoadAbilityStage = [](const char *moduleName) -> int64_t { return moduleName[0] == '0' ? 0 : 1; };
        funcs->ReleaseAbilityStage = [](int64_t handle) {};
        funcs->AbilityStageOnCreate = [](int64_t handle) {};
        funcs->AbilityStageOnAcceptWant = [](int64_t handle, OHOS::AAFwk::Want *want) -> char* {
            std::string str = "Hello, world!";
            char *cstr = new char[str.length() + 1];
            memcpy_s(cstr, str.length() + 1, str.c_str(), str.size());
            return cstr;
        };
        funcs->AbilityStageOnConfigurationUpdated = [](int64_t id, CJConfiguration configuration) {};
        funcs->AbilityStageOnMemoryLevel = [](int64_t id, int32_t level) {};
    };
    RegisterCJAbilityStageFuncs(registerFunc);
    RegisterCJAbilityStageFuncs(registerFunc);
    std::shared_ptr<CJAbilityStageObject> cjAbilityStageObject = CJAbilityStageObject::LoadModule("1");
    cjAbilityStageObject->OnCreate();
    std::shared_ptr<CJAbilityStageObject> cjAbilityStageObjectAnother = CJAbilityStageObject::LoadModule("0");

    AAFwk::Want want;
    cjAbilityStageObject->OnAcceptWant(want);

    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    cjAbilityStageObject->OnConfigurationUpdated(configuration);

    int32_t level = 1;
    cjAbilityStageObject->OnMemoryLevel(level);
}

}  // namespace AbilityRuntime
}  // namespace OHOS
