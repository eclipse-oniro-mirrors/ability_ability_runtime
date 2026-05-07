/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "app_utils.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AppUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppUtilsTest::SetUpTestCase(void)
{}

void AppUtilsTest::TearDownTestCase(void)
{}

void AppUtilsTest::SetUp()
{}

void AppUtilsTest::TearDown()
{}

/**
 * @tc.number: AppUtilsTest_0100
 * @tc.desc: Test IsInheritWindowSplitScreenMode works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_0100 called.");
    bool isInheritWindowSplitScreenMode = AAFwk::AppUtils::GetInstance().IsInheritWindowSplitScreenMode();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isInheritWindowSplitScreenMode == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isInheritWindowSplitScreenMode == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isInheritWindowSplitScreenMode == false);
    }
}

/**
 * @tc.number: AppUtilsTest_0200
 * @tc.desc: Test IsSupportAncoApp works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_0200 called.");
    bool isSupportAncoApp = AAFwk::AppUtils::GetInstance().IsSupportAncoApp();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isSupportAncoApp == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isSupportAncoApp == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isSupportAncoApp == false);
    }
}

/**
 * @tc.number: AppUtilsTest_0300
 * @tc.desc: Test GetTimeoutUnitTimeRatio works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_0300 called.");
    int32_t timeoutUnitTimeRatio = AAFwk::AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(timeoutUnitTimeRatio == 1);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(timeoutUnitTimeRatio == 1);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(timeoutUnitTimeRatio == 10);
    }
}

/**
 * @tc.number: AppUtilsTest_0400
 * @tc.desc: Test IsSelectorDialogDefaultPossion works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_0400 called.");
    bool isSelectorDialogDefaultPossion = AAFwk::AppUtils::GetInstance().IsSelectorDialogDefaultPossion();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isSelectorDialogDefaultPossion == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isSelectorDialogDefaultPossion == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isSelectorDialogDefaultPossion == false);
    }
}

/**
 * @tc.number: AppUtilsTest_0500
 * @tc.desc: Test IsStartSpecifiedProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0500, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_0500 called.");
    bool isStartSpecifiedProcess = AAFwk::AppUtils::GetInstance().IsStartSpecifiedProcess();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isStartSpecifiedProcess == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isStartSpecifiedProcess == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isStartSpecifiedProcess == true);
    }
}

/**
 * @tc.number: AppUtilsTest_0600
 * @tc.desc: Test IsUseMultiRenderProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0600, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_0600 called.");
    bool isUseMultiRenderProcess = AAFwk::AppUtils::GetInstance().IsUseMultiRenderProcess();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isUseMultiRenderProcess == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isUseMultiRenderProcess == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isUseMultiRenderProcess == true);
    }
}

/**
 * @tc.number: AppUtilsTest_0700
 * @tc.desc: Test IsLimitMaximumOfRenderProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0700, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_0700 called.");
    bool isLimitMaximumOfRenderProcess = AAFwk::AppUtils::GetInstance().IsLimitMaximumOfRenderProcess();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isLimitMaximumOfRenderProcess == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isLimitMaximumOfRenderProcess == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isLimitMaximumOfRenderProcess == false);
    }
}

/**
 * @tc.number: AppUtilsTest_0800
 * @tc.desc: Test IsGrantPersistUriPermission works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0800, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_0800 called.");
    bool isGrantPersistUriPermission = AAFwk::AppUtils::GetInstance().IsGrantPersistUriPermission();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isGrantPersistUriPermission == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isGrantPersistUriPermission == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isGrantPersistUriPermission == true);
    }
}

/**
 * @tc.number: AppUtilsTest_0900
 * @tc.desc: Test IsStartOptionsWithAnimation works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_0900, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_0900 called.");
    bool isStartOptionsWithAnimation = AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isStartOptionsWithAnimation == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isStartOptionsWithAnimation == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isStartOptionsWithAnimation == true);
    }
}

/**
 * @tc.number: AppUtilsTest_1000
 * @tc.desc: Test IsMultiProcessModel works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1000, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1000 called.");
    bool isMultiProcessModel = AAFwk::AppUtils::GetInstance().IsMultiProcessModel();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isMultiProcessModel == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isMultiProcessModel == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isMultiProcessModel == true);
    }
}

/**
 * @tc.number: AppUtilsTest_1100
 * @tc.desc: Test IsStartOptionsWithProcessOptions works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1100 called.");
    bool isStartOptionsWithProcessOptions = AAFwk::AppUtils::GetInstance().IsStartOptionsWithProcessOptions();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isStartOptionsWithProcessOptions == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isStartOptionsWithProcessOptions == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isStartOptionsWithProcessOptions == true);
    }
}

/**
 * @tc.number: AppUtilsTest_1200
 * @tc.desc: Test EnableMoveUIAbilityToBackgroundApi works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1200 called.");
    bool enableMoveUIAbilityToBackgroundApi = AAFwk::AppUtils::GetInstance().EnableMoveUIAbilityToBackgroundApi();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(enableMoveUIAbilityToBackgroundApi == true);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(enableMoveUIAbilityToBackgroundApi == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(enableMoveUIAbilityToBackgroundApi == false);
    }
}

/**
 * @tc.number: AppUtilsTest_1300
 * @tc.desc: Test MaxChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1300 called.");
    auto maxChildProcess = AAFwk::AppUtils::GetInstance().MaxChildProcess();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "phone" || deviceType == "2in1") {
        EXPECT_TRUE(maxChildProcess != 0);
    }
}

/**
 * @tc.number: AppUtilsTest_1400
 * @tc.desc: Test IsAllowNativeChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1400 called.");
    auto allow = AAFwk::AppUtils::GetInstance().IsAllowNativeChildProcess("com.test.demo");
    EXPECT_FALSE(allow);
}

/**
 * @tc.number: AppUtilsTest_1500
 * @tc.desc: Test IsSupportMultiInstance works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1500, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1500 called.");
    bool isSupportMultiInstance = AAFwk::AppUtils::GetInstance().IsSupportMultiInstance();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isSupportMultiInstance == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isSupportMultiInstance == false);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isSupportMultiInstance == true);
    }
}

/**
 * @tc.number: AppUtilsTest_1600
 * @tc.desc: Test AllowChildProcessInMultiProcessFeatureApp works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1600, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1600 start.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    bool allow = appUtils.AllowChildProcessInMultiProcessFeatureApp();
    EXPECT_FALSE(allow);

    appUtils.allowChildProcessInMultiProcessFeatureApp_.value = true;
    allow = appUtils.AllowChildProcessInMultiProcessFeatureApp();
    EXPECT_TRUE(allow);
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1600 end.");
}

/**
 * @tc.number: AppUtilsTest_1700
 * @tc.desc: Test MaxMultiProcessFeatureChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1700, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1700 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();

    appUtils.maxMultiProcessFeatureChildProcess_.isLoaded = true;
    appUtils.maxMultiProcessFeatureChildProcess_.value = 512;
    auto maxProcess = appUtils.MaxMultiProcessFeatureChildProcess();
    EXPECT_TRUE(maxProcess == 512);
}

/**
 * @tc.number: AppUtilsTest_1800
 * @tc.desc: Test InResidentWhiteList works
 * @tc.desc: Test InResidentWhiteList works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1800, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1800 called.");
    auto allow = AAFwk::AppUtils::GetInstance().InResidentWhiteList("com.test.demo");
    EXPECT_FALSE(allow);
}

/**
 * @tc.number: AppUtilsTest_1900
 * @tc.desc: Test GetResidentWhiteList works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_1900, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_1900 called.");
    auto residentWhiteList = AAFwk::AppUtils::GetInstance().GetResidentWhiteList();
    bool isExist = false;
    for (const auto &item: residentWhiteList) {
        if (item == "com.test.demo") {
            isExist = true;
        }
    }
    EXPECT_FALSE(isExist);
}

/**
 * @tc.number: AppUtilsTest_2000
 * @tc.desc: Test IsSupportGrantUriPermission works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2000, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2000 called.");
    auto isSupport = AAFwk::AppUtils::GetInstance().IsSupportGrantUriPermission();
    EXPECT_TRUE(isSupport);
}

/**
 * @tc.number: AppUtilsTest_2100
 * @tc.desc: Test IsSupportGrantUriPermission works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isGrantTempUriPermission_.isLoaded = true;
    auto isSupport = appUtils.IsSupportGrantUriPermission();
    EXPECT_TRUE(isSupport);
}

/**
 * @tc.number: IsSupportStartAbilities_0100
 * @tc.desc: Test IsSupportStartAbilities works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportStartAbilities_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportStartAbilities_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportStartAbilities_.isLoaded = false;
    bool isSupportStartAbilities = appUtils.IsSupportStartAbilities();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isSupportStartAbilities == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isSupportStartAbilities == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isSupportStartAbilities == false);
    } else if (deviceType == "tablet") {
        EXPECT_TRUE(isSupportStartAbilities == true);
    }
}

/**
 * @tc.number: IsSupportStartAbilities_0200
 * @tc.desc: Test IsSupportStartAbilities works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportStartAbilities_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportStartAbilities_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportStartAbilities_.isLoaded = true;
    bool isSupportStartAbilities = appUtils.IsSupportStartAbilities();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "default") {
        EXPECT_TRUE(isSupportStartAbilities == false);
    } else if (deviceType == "phone") {
        EXPECT_TRUE(isSupportStartAbilities == true);
    } else if (deviceType == "2in1") {
        EXPECT_TRUE(isSupportStartAbilities == false);
    } else if (deviceType == "tablet") {
        EXPECT_TRUE(isSupportStartAbilities == true);
    }
}

/**
 * @tc.number: IsLauncher_0100
 * @tc.desc: Test IsLauncher works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsLauncher_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_0100 called.");
    std::string bundleName = "com.ohos.launcher";
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    bool result = appUtils.IsLauncher(bundleName);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: IsLauncher_0200
 * @tc.desc: Test IsLauncher works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsLauncher_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_0200 called.");
    std::string bundleName = "com.ohos.launcher.test";
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    bool result = appUtils.IsLauncher(bundleName);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsLauncher_0300
 * @tc.desc: Test IsLauncher works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsLauncher_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLauncher_0200 called.");
    std::string bundleName = "com.ohos.sceneboard";
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSceneBoard_ = true;
    bool result = appUtils.IsLauncher(bundleName);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: IsSupportNativeChildProcess_0100
 * @tc.desc: Test IsSupportNativeChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportNativeChildProcess_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportNativeChildProcess_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    bool result = appUtils.IsSupportNativeChildProcess();
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsSupportNativeChildProcess_0200
 * @tc.desc: Test IsSupportNativeChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportNativeChildProcess_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportNativeChildProcess_0200 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportNativeChildProcess_.isLoaded = true;
    appUtils.isSupportNativeChildProcess_.value = true;
    bool result = appUtils.IsSupportNativeChildProcess();
    EXPECT_TRUE(result);
}

/**
 * @tc.number: GetAncoAppIdentifiers_0100
 * @tc.desc: Test GetAncoAppIdentifiers works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, GetAncoAppIdentifiers_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAncoAppIdentifiers_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    std::string result = appUtils.GetAncoAppIdentifiers();
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: IsCacheAbilityEnabled_0100
 * @tc.desc: Test IsCacheAbilityEnabled works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsCacheAbilityEnabled_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCacheAbilityEnabled_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    bool result = appUtils.IsCacheAbilityEnabled();
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsSupportAppServiceExtension_0100
 * @tc.desc: Test IsSupportAppServiceExtension works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportAppServiceExtension_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportAppServiceExtension_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    bool result = appUtils.IsSupportAppServiceExtension();
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsSupportAppServiceExtension_0200
 * @tc.desc: Test IsSupportAppServiceExtension works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportAppServiceExtension_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportAppServiceExtension_0200 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportAppServiceExtension_.isLoaded = true;
    appUtils.isSupportAppServiceExtension_.value = true;
    bool result = appUtils.IsSupportAppServiceExtension();
    EXPECT_TRUE(result);
}

/**
 * @tc.number: MaxMultiProcessFeatureChildProcess_0100
 * @tc.desc: Test MaxMultiProcessFeatureChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, MaxMultiProcessFeatureChildProcess_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MaxMultiProcessFeatureChildProcess_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.maxMultiProcessFeatureChildProcess_.isLoaded = false;
    bool result = appUtils.MaxMultiProcessFeatureChildProcess();
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsSystemReasonMessage_0100
 * @tc.desc: Test IsSystemReasonMessage works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSystemReasonMessage_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSystemReasonMessage_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    std::string reasonMessage = "";
    bool result = appUtils.IsSystemReasonMessage(reasonMessage);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsGrantPersistUriPermission_0100
 * @tc.desc: Test IsGrantPersistUriPermission works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsGrantPersistUriPermission_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsGrantPersistUriPermission_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isGrantPersistUriPermission_.isLoaded = false;
    bool result = appUtils.IsGrantPersistUriPermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.number: GetMigrateClientBundleName_0100
 * @tc.desc: Test GetMigrateClientBundleName works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, GetMigrateClientBundleName_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetMigrateClientBundleName_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.migrateClientBundleName_.isLoaded = false;
    std::string result = appUtils.GetMigrateClientBundleName();
    EXPECT_EQ(result, "");
}

/**
 * @tc.number: IsUseMultiRenderProcess_0100
 * @tc.desc: Test IsUseMultiRenderProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsUseMultiRenderProcess_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUseMultiRenderProcess_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isUseMultiRenderProcess_.isLoaded = false;
    bool result = appUtils.IsUseMultiRenderProcess();
    EXPECT_TRUE(result);
}

/**
 * @tc.number: IsLaunchEmbededUIAbility_0100
 * @tc.desc: Test IsLaunchEmbededUIAbility works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsLaunchEmbededUIAbility_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLaunchEmbededUIAbility_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isLaunchEmbededUIAbility_.isLoaded = false;
    bool result = appUtils.IsLaunchEmbededUIAbility();
    EXPECT_FALSE(result);
}

/**
 * @tc.number: AppUtilsTest_2200
 * @tc.desc: Test IsLauncher works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2200 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSceneBoard_ = true;
    std::string abilityName = "com.ohos.sceneboard";
    auto isLauncher = appUtils.IsLauncher(abilityName);
    EXPECT_TRUE(isLauncher);
}

/**
 * @tc.number: AppUtilsTest_2300
 * @tc.desc: Test IsLauncher works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2300 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSceneBoard_ = false;
    std::string abilityName = "com.ohos.launcher";
    auto isLauncher = appUtils.IsLauncher(abilityName);
    EXPECT_TRUE(isLauncher);
}

/**
 * @tc.number: AppUtilsTest_2400
 * @tc.desc: Test IsLauncherAbility works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2400 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSceneBoard_ = true;
    std::string abilityName = "com.ohos.sceneboard.MainAbility";
    auto isLauncherAbility = appUtils.IsLauncherAbility(abilityName);
    EXPECT_TRUE(isLauncherAbility);
}

/**
 * @tc.number: AppUtilsTest_2500
 * @tc.desc: Test IsLauncherAbility works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2500, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2500 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSceneBoard_ = false;
    std::string abilityName = "com.ohos.launcher.MainAbility";
    auto isLauncherAbility = appUtils.IsLauncherAbility(abilityName);
    EXPECT_TRUE(isLauncherAbility);
}

/**
 * @tc.number: AppUtilsTest_2600
 * @tc.desc: Test IsLaunchEmbededUIAbility works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2600, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2600 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isLaunchEmbededUIAbility_.isLoaded = false;
    auto value = OHOS::system::GetBoolParameter("const.abilityms.launch_embeded_ui_ability", false);
    auto isLaunchEmbededUIAbility = appUtils.IsLaunchEmbededUIAbility();
    EXPECT_EQ(value, isLaunchEmbededUIAbility);
}

/**
 * @tc.number: AppUtilsTest_2700
 * @tc.desc: Test IsLaunchEmbededUIAbility works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2700, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2700 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isLaunchEmbededUIAbility_.isLoaded = true;
    appUtils.isLaunchEmbededUIAbility_.value = true;
    auto isLaunchEmbededUIAbility = appUtils.IsLaunchEmbededUIAbility();
    EXPECT_TRUE(isLaunchEmbededUIAbility);
}

/**
 * @tc.number: AppUtilsTest_2800
 * @tc.desc: Test IsSystemReasonMessage works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2800, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2800 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    std::string reasonMessage = "ReasonMessage_SystemShare";
    auto isSystemReasonMessage = appUtils.IsSystemReasonMessage(reasonMessage);
    EXPECT_TRUE(isSystemReasonMessage);
}

/**
 * @tc.number: AppUtilsTest_2900
 * @tc.desc: Test IsCacheAbilityEnabled works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_2900, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_2900 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    auto value = OHOS::system::GetBoolParameter("persist.sys.abilityms.cache_ability_enable", false);
    auto isCacheAbilityEnabled = appUtils.IsCacheAbilityEnabled();
    EXPECT_EQ(value, isCacheAbilityEnabled);
}

/**
 * @tc.number: AppUtilsTest_3000
 * @tc.desc: Test IsSupportAppServiceExtension works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3000, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3000 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportAppServiceExtension_.isLoaded = false;
    auto value = OHOS::system::GetBoolParameter("const.abilityms.support_app_service", false);
    auto isSupportAppServiceExtension = appUtils.IsSupportAppServiceExtension();
    EXPECT_EQ(value, isSupportAppServiceExtension);
}

/**
 * @tc.number: AppUtilsTest_3100
 * @tc.desc: Test IsSupportAppServiceExtension works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportAppServiceExtension_.isLoaded = true;
    appUtils.isSupportAppServiceExtension_.value = true;
    auto isSupportAppServiceExtension = appUtils.IsSupportAppServiceExtension();
    EXPECT_TRUE(isSupportAppServiceExtension);
}

/**
 * @tc.number: AppUtilsTest_3200
 * @tc.desc: Test GetAncoAppIdentifiers works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3200 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    std::string value = system::GetParameter("persist.hmos_fusion_mgr.anco_identifier", "");
    auto identifiers = appUtils.GetAncoAppIdentifiers();
    EXPECT_EQ(value, identifiers);
}

/**
 * @tc.number: AppUtilsTest_3300
 * @tc.desc: Test GetMigrateClientBundleName works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3300 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.migrateClientBundleName_.isLoaded = false;
    auto value = system::GetParameter("const.sys.abilityms.migrate_client_bundle_name", "");
    auto migrateClientBundleName = appUtils.GetMigrateClientBundleName();
    EXPECT_EQ(value, migrateClientBundleName);
}

/**
 * @tc.number: AppUtilsTest_3400
 * @tc.desc: Test GetMigrateClientBundleName works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3400 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.migrateClientBundleName_.isLoaded = true;
    appUtils.migrateClientBundleName_.value = "value";
    auto migrateClientBundleName = appUtils.GetMigrateClientBundleName();
    EXPECT_EQ("value", migrateClientBundleName);
}

/**
 * @tc.number: AppUtilsTest_3500
 * @tc.desc: Test GetMigrateClientBundleName works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3500, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3500 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.maxMultiProcessFeatureChildProcess_.isLoaded = false;
    auto value = system::GetIntParameter<int32_t>("const.sys.abilityms.max_multi_process_feature_child_process", 0);
    auto maxProcess = appUtils.MaxMultiProcessFeatureChildProcess();
    EXPECT_EQ(value, maxProcess);
}

/**
 * @tc.number: AppUtilsTest_3600
 * @tc.desc: Test IsAllowResidentInExtremeMemory works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3600, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3600 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.residentProcessInExtremeMemory_.value.emplace_back("bundleName", "abilityName");
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    auto ret = appUtils.IsAllowResidentInExtremeMemory(bundleName, abilityName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AppUtilsTest_3700
 * @tc.desc: Test IsBigMemoryUnrelatedKeepAliveProc works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3700, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3700 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.processProhibitedFromRestarting_.value.emplace_back("bundleName");
    std::string bundleName = "bundleName";
    auto ret = appUtils.IsBigMemoryUnrelatedKeepAliveProc(bundleName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AppUtilsTest_3800
 * @tc.desc: Test IsRequireBigMemoryProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3800, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3800 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.requireBigMemoryApp_.value.emplace_back("bundleName");
    std::string bundleName = "bundleName";
    auto ret = appUtils.IsRequireBigMemoryProcess(bundleName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AppUtilsTest_3900
 * @tc.desc: Test IsAllowStartAbilityWithoutCallerToken works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_3900, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_3900 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.startAbilityWithoutCallerToken_.value.emplace_back("bundleName", "abilityName");
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    auto ret = appUtils.IsAllowStartAbilityWithoutCallerToken(bundleName, abilityName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AppUtilsTest_4000
 * @tc.desc: Test IsSupportNativeChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_4000, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_4000 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportNativeChildProcess_.isLoaded = false;
    auto value = system::GetBoolParameter("persist.sys.abilityms.start_native_child_process", false);
    auto isSupportNativeChildProcess = appUtils.IsSupportNativeChildProcess();
    EXPECT_EQ(value, isSupportNativeChildProcess);
}

/**
 * @tc.number: AppUtilsTest_4100
 * @tc.desc: Test IsSupportNativeChildProcess works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, AppUtilsTest_4100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppUtilsTest_4100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportNativeChildProcess_.isLoaded = true;
    appUtils.isSupportNativeChildProcess_.value = true;
    auto isSupportNativeChildProcess = appUtils.IsSupportNativeChildProcess();
    EXPECT_TRUE(isSupportNativeChildProcess);
}

/**
 * @tc.number: InOnNewProcessEnableList_0100
 * @tc.desc: Test InOnNewProcessEnableList works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, InOnNewProcessEnableList_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "InOnNewProcessEnableList_0100 called.");
    std::string bundleName = "cn.wps.office.hap";
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    bool result = appUtils.InOnNewProcessEnableList(bundleName);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: InOnNewProcessEnableList_0200
 * @tc.desc: Test InOnNewProcessEnableList works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, InOnNewProcessEnableList_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "InOnNewProcessEnableList_0200 called.");
    std::string bundleName = "cn.wps.office.hap";
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.onNewProcessEnableList_.isLoaded = true;
    appUtils.onNewProcessEnableList_.value.emplace_back("cn.wps.office.hap");
    bool result = appUtils.InOnNewProcessEnableList(bundleName);
    EXPECT_TRUE(result);
}

/**
 * @tc.number: IsSupportRestartAppWithWindow_0100
 * @tc.desc: Test IsSupportRestartAppWithWindow works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportRestartAppWithWindow_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportRestartAppWithWindow_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportRestartAppWithWindow_.isLoaded = false;
    appUtils.IsSupportRestartAppWithWindow();
    EXPECT_TRUE(appUtils.isSupportRestartAppWithWindow_.isLoaded);
}

/**
 * @tc.number: IsSupportRestartAppWithWindow_0200
 * @tc.desc: Test IsSupportRestartAppWithWindow works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportRestartAppWithWindow_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportRestartAppWithWindow_0200 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportRestartAppWithWindow_.isLoaded = true;
    appUtils.isSupportRestartAppWithWindow_.value = true;
    EXPECT_TRUE(appUtils.IsSupportRestartAppWithWindow());
}

/**
 * @tc.number: IsSupportAllowDebugPermission_0100
 * @tc.desc: Test IsSupportAllowDebugPermission works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportAllowDebugPermission_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportAllowDebugPermission_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportAllowDebugPermission_.isLoaded = false;
    appUtils.IsSupportAllowDebugPermission();
    EXPECT_TRUE(appUtils.isSupportAllowDebugPermission_.isLoaded);
}

/**
 * @tc.number: IsSupportAllowDebugPermission_0200
 * @tc.desc: Test IsSupportAllowDebugPermission works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportAllowDebugPermission_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportAllowDebugPermission_0200 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportAllowDebugPermission_.isLoaded = true;
    appUtils.isSupportAllowDebugPermission_.value = true;
    EXPECT_TRUE(appUtils.IsSupportAllowDebugPermission());
}

/**
 * @tc.number: IsSupportNativeUIAbility_0100
 * @tc.desc: Test IsSupportNativeUIAbility works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportNativeUIAbility_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportNativeUIAbility_0100 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportNativeUIAbility_.isLoaded = false;
    appUtils.IsSupportNativeUIAbility();
    EXPECT_TRUE(appUtils.isSupportNativeUIAbility_.isLoaded);
}

/**
 * @tc.number: IsSupportNativeUIAbility_0200
 * @tc.desc: Test IsSupportNativeUIAbility works
 * @tc.type: FUNC
 */
HWTEST_F(AppUtilsTest, IsSupportNativeUIAbility_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSupportNativeUIAbility_0200 called.");
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportNativeUIAbility_.isLoaded = true;
    appUtils.isSupportNativeUIAbility_.value = true;
    EXPECT_TRUE(appUtils.IsSupportNativeUIAbility());
}
}  // namespace AbilityRuntime
}  // namespace OHOS
