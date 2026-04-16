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

#include "modular_object_extension_manager.h"

#include <cstring>
#include "ability_manager/include/modular_object_extension_info.h"
#include "hilog_tag_wrapper.h"


struct OH_AbilityRuntime_AllModularObjectExtensionInfos {
    std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> allMoeInfos;
};

using namespace testing::ext;
namespace OHOS {
namespace AAFwk {

class AbilityRuntimeModularObjectExtensionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityRuntimeModularObjectExtensionManagerTest::SetUpTestCase()
{}

void AbilityRuntimeModularObjectExtensionManagerTest::TearDownTestCase()
{}

void AbilityRuntimeModularObjectExtensionManagerTest::SetUp()
{}

void AbilityRuntimeModularObjectExtensionManagerTest::TearDown()
{}

/**
 * @tc.name: OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode_001
 * @tc.desc: OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeModularObjectExtensionManagerTest,
    OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode_001, TestSize.Level1)
{
    std::unique_ptr<OH_AbilityRuntime_AllModularObjectExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_AllModularObjectExtensionInfos>();
    std::vector<ModularObjectExtensionInfo> dataList;

    ModularObjectExtensionInfo info;
    info.launchMode = MoeLaunchMode::IN_PROCESS;
    dataList.push_back(info);
    infos->allMoeInfos = dataList;
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos = infos.release();
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo = nullptr;
    auto ret = OH_AbilityRuntime_GetModObjExtensionInfoByIndex(allExtensionInfos, 0, &extensionInfo);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(nullptr, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(extensionInfo, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_LaunchMode launchMode;
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(extensionInfo, &launchMode);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(launchMode, OH_AbilityRuntime_LaunchMode::OH_ABILITY_RUNTIME_LAUNCH_MODE_IN_PROCESS);
    ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(&allExtensionInfos);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

/**
 * @tc.name: OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode_001
 * @tc.desc: OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeModularObjectExtensionManagerTest,
    OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode_001, TestSize.Level1)
{
    std::unique_ptr<OH_AbilityRuntime_AllModularObjectExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_AllModularObjectExtensionInfos>();
    std::vector<ModularObjectExtensionInfo> dataList;

    ModularObjectExtensionInfo info;
    info.processMode = MoeProcessMode::BUNDLE;
    dataList.push_back(info);
    infos->allMoeInfos = dataList;
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos = infos.release();
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo = nullptr;
    auto ret = OH_AbilityRuntime_GetModObjExtensionInfoByIndex(allExtensionInfos, 0, &extensionInfo);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(nullptr, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(extensionInfo, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_ProcessMode processMode;
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(extensionInfo, &processMode);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(processMode, OH_AbilityRuntime_ProcessMode::OH_ABILITY_RUNTIME_PROCESS_MODE_BUNDLE);
    ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(&allExtensionInfos);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

/**
 * @tc.name: OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode_001
 * @tc.desc: OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeModularObjectExtensionManagerTest,
    OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode_001, TestSize.Level1)
{
    std::unique_ptr<OH_AbilityRuntime_AllModularObjectExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_AllModularObjectExtensionInfos>();
    std::vector<ModularObjectExtensionInfo> dataList;

    ModularObjectExtensionInfo info;
    info.threadMode = MoeThreadMode::TYPE;
    dataList.push_back(info);
    infos->allMoeInfos = dataList;
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos = infos.release();
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo = nullptr;
    auto ret = OH_AbilityRuntime_GetModObjExtensionInfoByIndex(allExtensionInfos, 0, &extensionInfo);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(nullptr, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(extensionInfo, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_ThreadMode threadMode;
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(extensionInfo, &threadMode);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(threadMode, OH_AbilityRuntime_ThreadMode::OH_ABILITY_RUNTIME_THREAD_MODE_TYPE);
    ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(&allExtensionInfos);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

/**
 * @tc.name: OH_AbilityRuntime_GetModularObjectExtensionInfoElementName_001
 * @tc.desc: OH_AbilityRuntime_GetModularObjectExtensionInfoElementName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeModularObjectExtensionManagerTest,
    OH_AbilityRuntime_GetModularObjectExtensionInfoElementName_001, TestSize.Level1)
{
    std::unique_ptr<OH_AbilityRuntime_AllModularObjectExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_AllModularObjectExtensionInfos>();
    std::vector<ModularObjectExtensionInfo> dataList;

    ModularObjectExtensionInfo info;
    info.bundleName = "bundleName";
    info.moduleName = "moduleName";
    info.abilityName = "abilityName";
    dataList.push_back(info);
    infos->allMoeInfos = dataList;
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos = infos.release();
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo = nullptr;
    auto ret = OH_AbilityRuntime_GetModObjExtensionInfoByIndex(allExtensionInfos, 0, &extensionInfo);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(nullptr, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(extensionInfo, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    AbilityBase_Element element;
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(extensionInfo, &element);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(std::string(element.bundleName), "bundleName");
    EXPECT_EQ(std::string(element.moduleName), "moduleName");
    EXPECT_EQ(std::string(element.abilityName), "abilityName");
    ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(&allExtensionInfos);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

/**
 * @tc.name: OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState_001
 * @tc.desc: OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeModularObjectExtensionManagerTest,
    OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState_001, TestSize.Level1)
{
    std::unique_ptr<OH_AbilityRuntime_AllModularObjectExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_AllModularObjectExtensionInfos>();
    std::vector<ModularObjectExtensionInfo> dataList;

    ModularObjectExtensionInfo info;
    info.isDisabled = true;
    dataList.push_back(info);
    infos->allMoeInfos = dataList;
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos = infos.release();
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo = nullptr;
    auto ret = OH_AbilityRuntime_GetModObjExtensionInfoByIndex(allExtensionInfos, 0, &extensionInfo);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(nullptr, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(extensionInfo, nullptr);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    bool isDisabled = false;
    ret = OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(extensionInfo, &isDisabled);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_TRUE(isDisabled);
    ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(&allExtensionInfos);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

/**
 * @tc.name: OH_AbilityRuntime_GetCountFromAllModObjExtensionInfos_001
 * @tc.desc: OH_AbilityRuntime_GetCountFromAllModObjExtensionInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeModularObjectExtensionManagerTest,
    OH_AbilityRuntime_GetCountFromAllModObjExtensionInfos_001, TestSize.Level1)
{
    std::unique_ptr<OH_AbilityRuntime_AllModularObjectExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_AllModularObjectExtensionInfos>();
    std::vector<ModularObjectExtensionInfo> dataList;

    ModularObjectExtensionInfo info1;
    dataList.push_back(info1);
    ModularObjectExtensionInfo info2;
    dataList.push_back(info2);
    infos->allMoeInfos = dataList;
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos = infos.release();

    size_t count = 0;
    auto ret = OH_AbilityRuntime_GetCountFromAllModObjExtensionInfos(allExtensionInfos, &count);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(count, 2);
    ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(&allExtensionInfos);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

/**
 * @tc.name: OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos_001
 * @tc.desc: OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityRuntimeModularObjectExtensionManagerTest,
    OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos(nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos = nullptr;
    ret = OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos(&allExtensionInfos);
    EXPECT_NE(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(&allExtensionInfos);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);

    std::unique_ptr<OH_AbilityRuntime_AllModularObjectExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_AllModularObjectExtensionInfos>();
    std::vector<ModularObjectExtensionInfo> dataList;
    ModularObjectExtensionInfo info;
    dataList.push_back(info);
    infos->allMoeInfos = dataList;
    allExtensionInfos = infos.release();
    ret = OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos(&allExtensionInfos);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED);

    ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(&allExtensionInfos);
    ASSERT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS
