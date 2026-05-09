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
#include "app_utils.h"
#include "native_ability_util.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class NativeModuleUtilTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

protected:
    AppExecFwk::AbilityInfo abilityInfo_;
    void AddMetadata(const std::string& name, const std::string& value);
};

void NativeModuleUtilTest::SetUpTestCase(void) {}
void NativeModuleUtilTest::TearDownTestCase(void) {}
void NativeModuleUtilTest::SetUp()
{
    abilityInfo_.metadata.clear();
}
void NativeModuleUtilTest::TearDown() {}

void NativeModuleUtilTest::AddMetadata(const std::string& name, const std::string& value)
{
    AppExecFwk::Metadata meta;
    meta.name = name;
    meta.value = value;
    abilityInfo_.metadata.push_back(meta);
}

// ==================== InitData Default / No Metadata ====================

/**
 * @tc.number: InitData_0100
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: No metadata at all, withNativeModule should be false
 */
HWTEST_F(NativeModuleUtilTest, InitData_0100, TestSize.Level1)
{
    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
    EXPECT_TRUE(data.nativeModuleSource.empty());
    EXPECT_TRUE(data.nativeModuleFunc.empty());
}

/**
 * @tc.number: InitData_0200
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule="false", should disable native module
 */
HWTEST_F(NativeModuleUtilTest, InitData_0200, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "false");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_0300
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule="0", should disable native module
 */
HWTEST_F(NativeModuleUtilTest, InitData_0300, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "0");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

// ==================== InitData withNativeModule Boolean Parsing ====================

/**
 * @tc.number: InitData_0400
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule="true" (lowercase), should enable native module
 */
HWTEST_F(NativeModuleUtilTest, InitData_0400, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
}

/**
 * @tc.number: InitData_0500
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule="1", ParseBool only accepts "true", should disable native module
 */
HWTEST_F(NativeModuleUtilTest, InitData_0500, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "1");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_0600
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule="TRUE" (uppercase), ParseBool only accepts lowercase "true", should disable
 */
HWTEST_F(NativeModuleUtilTest, InitData_0600, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "TRUE");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_0700
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule="True" (mixed case), ParseBool only accepts "true", should disable
 */
HWTEST_F(NativeModuleUtilTest, InitData_0700, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "True");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_0800
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule="FALSE" (uppercase), should disable native module
 */
HWTEST_F(NativeModuleUtilTest, InitData_0800, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "FALSE");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_0900
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule="abc" (invalid), should use default false
 */
HWTEST_F(NativeModuleUtilTest, InitData_0900, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "abc");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_1000
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule metadata with empty value, should use default false
 */
HWTEST_F(NativeModuleUtilTest, InitData_1000, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

// ==================== InitData Missing Source / Func ====================

/**
 * @tc.number: InitData_1100
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule=true but no source metadata, should disable native module
 */
HWTEST_F(NativeModuleUtilTest, InitData_1100, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");
    // Missing nativeModuleSource

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_1200
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule=true but source is empty, should disable native module
 */
HWTEST_F(NativeModuleUtilTest, InitData_1200, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleSource", "");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_1300
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule=true but no func metadata, should disable native module
 */
HWTEST_F(NativeModuleUtilTest, InitData_1300, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    // Missing nativeModuleFun

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_1400
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule=true but func is empty, should disable native module
 */
HWTEST_F(NativeModuleUtilTest, InitData_1400, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

// ==================== InitData StartupPhase Parsing ====================

/**
 * @tc.number: InitData_1500
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: startupPhase="pre_window", should parse to PRE_WINDOW
 */
HWTEST_F(NativeModuleUtilTest, InitData_1500, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "pre_window");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
}

/**
 * @tc.number: InitData_1600
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: startupPhase="pre_foreground", should parse to PRE_FOREGROUND
 */
HWTEST_F(NativeModuleUtilTest, InitData_1600, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "pre_foreground");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_FOREGROUND);
}

/**
 * @tc.number: InitData_1700
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: startupPhase="foreground", should parse to FOREGROUND
 */
HWTEST_F(NativeModuleUtilTest, InitData_1700, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "foreground");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::FOREGROUND);
}

/**
 * @tc.number: InitData_1800
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: startupPhase is empty, should default to PRE_WINDOW
 */
HWTEST_F(NativeModuleUtilTest, InitData_1800, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
}

/**
 * @tc.number: InitData_1900
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: startupPhase is invalid value, should default to PRE_WINDOW
 */
HWTEST_F(NativeModuleUtilTest, InitData_1900, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "invalid_phase");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
}

/**
 * @tc.number: InitData_2000
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: No startupPhase metadata at all, should default to PRE_WINDOW
 */
HWTEST_F(NativeModuleUtilTest, InitData_2000, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");
    // No startupPhase metadata

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
}

// ==================== InitData Source and Func Values ====================

/**
 * @tc.number: InitData_2100
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: Verify source and func are correctly parsed
 */
HWTEST_F(NativeModuleUtilTest, InitData_2100, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleSource", "libmyability.so");
    AddMetadata("ohos.ability.nativeModuleFun", "MyCustomMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.nativeModuleSource, "libmyability.so");
    EXPECT_EQ(data.nativeModuleFunc, "MyCustomMain");
}

/**
 * @tc.number: InitData_2200
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: Full metadata with all fields set, verify all fields
 */
HWTEST_F(NativeModuleUtilTest, InitData_2200, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "pre_foreground");
    AddMetadata("ohos.ability.nativeModuleSource", "libfull.so");
    AddMetadata("ohos.ability.nativeModuleFun", "FullMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_FOREGROUND);
    EXPECT_EQ(data.nativeModuleSource, "libfull.so");
    EXPECT_EQ(data.nativeModuleFunc, "FullMain");
}

// ==================== InitData with Unrelated Metadata ====================

/**
 * @tc.number: InitData_2300
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: Metadata has unrelated entries mixed with native module entries
 */
HWTEST_F(NativeModuleUtilTest, InitData_2300, TestSize.Level1)
{
    AddMetadata("ohos.ability.otherKey", "irrelevant");
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("some.unrelated.meta", "value");
    AddMetadata("ohos.ability.startupPhase", "foreground");
    AddMetadata("ohos.ability.nativeModuleSource", "libmixed.so");
    AddMetadata("ohos.ability.nativeModuleFun", "MixedMain");
    AddMetadata("another.key", "anotherValue");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::FOREGROUND);
    EXPECT_EQ(data.nativeModuleSource, "libmixed.so");
    EXPECT_EQ(data.nativeModuleFunc, "MixedMain");
}

/**
 * @tc.number: InitData_2400
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: Only unrelated metadata, no native module config, should be disabled
 */
HWTEST_F(NativeModuleUtilTest, InitData_2400, TestSize.Level1)
{
    AddMetadata("ohos.ability.otherKey", "value1");
    AddMetadata("some.unrelated.meta", "value2");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
    EXPECT_TRUE(data.nativeModuleSource.empty());
    EXPECT_TRUE(data.nativeModuleFunc.empty());
}

// ==================== InitData Pre-initialized Data Overwrite ====================

/**
 * @tc.number: InitData_2500
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: InitData should reset all fields even if data was pre-initialized
 */
HWTEST_F(NativeModuleUtilTest, InitData_2500, TestSize.Level1)
{
    // Pre-initialize with non-default values
    NativeAbilityMetaData data;
    data.withNativeModule = true;
    data.startupPhase = StartupPhase::FOREGROUND;
    data.nativeModuleSource = "old_source.so";
    data.nativeModuleFunc = "OldMain";

    // No metadata — InitData should reset everything
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
    EXPECT_TRUE(data.nativeModuleSource.empty());
    EXPECT_TRUE(data.nativeModuleFunc.empty());
}

/**
 * @tc.number: InitData_2600
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: Pre-initialized data should be fully overwritten when withNativeModule=true
 */
HWTEST_F(NativeModuleUtilTest, InitData_2600, TestSize.Level1)
{
    NativeAbilityMetaData data;
    data.withNativeModule = false;
    data.startupPhase = StartupPhase::PRE_WINDOW;
    data.nativeModuleSource = "old.so";
    data.nativeModuleFunc = "OldFunc";

    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "foreground");
    AddMetadata("ohos.ability.nativeModuleSource", "libnew.so");
    AddMetadata("ohos.ability.nativeModuleFun", "NewMain");

    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::FOREGROUND);
    EXPECT_EQ(data.nativeModuleSource, "libnew.so");
    EXPECT_EQ(data.nativeModuleFunc, "NewMain");
}

// ==================== InitData Edge Cases ====================

/**
 * @tc.number: InitData_2700
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: withNativeModule=true, source present but func missing (missing metadata entry)
 */
HWTEST_F(NativeModuleUtilTest, InitData_2700, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    // func completely missing

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
}

/**
 * @tc.number: InitData_2800
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: Duplicate withNativeModule metadata — first one should win (find_if returns first match)
 */
HWTEST_F(NativeModuleUtilTest, InitData_2800, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");
    // Add duplicate with false
    AddMetadata("ohos.ability.withNativeModule", "false");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    // First metadata entry "true" should be used
    EXPECT_TRUE(data.withNativeModule);
}

/**
 * @tc.number: InitData_2900
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: StartupPhase with "PRE_WINDOW" (uppercase), should default to PRE_WINDOW (not exact match)
 */
HWTEST_F(NativeModuleUtilTest, InitData_2900, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "PRE_WINDOW");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_TRUE(data.withNativeModule);
    // "PRE_WINDOW" is not "pre_window", so falls through to default PRE_WINDOW
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
}

// ==================== InitData IsSupportNativeUIAbility Gate ====================

/**
 * @tc.number: InitData_3000
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: IsSupportNativeUIAbility disabled, all metadata should be ignored, defaults returned
 */
HWTEST_F(NativeModuleUtilTest, InitData_3000, TestSize.Level1)
{
    AppUtils::SetSupportNativeUIAbility(false);

    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "foreground");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    NativeAbilityMetaData data;
    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
    EXPECT_TRUE(data.nativeModuleSource.empty());
    EXPECT_TRUE(data.nativeModuleFunc.empty());

    AppUtils::SetSupportNativeUIAbility(true);
}

/**
 * @tc.number: InitData_3100
 * @tc.name: NativeAbilityMetaData::InitData
 * @tc.desc: IsSupportNativeUIAbility disabled, pre-initialized data should be reset to defaults
 */
HWTEST_F(NativeModuleUtilTest, InitData_3100, TestSize.Level1)
{
    AppUtils::SetSupportNativeUIAbility(false);

    NativeAbilityMetaData data;
    data.withNativeModule = true;
    data.startupPhase = StartupPhase::FOREGROUND;
    data.nativeModuleSource = "old.so";
    data.nativeModuleFunc = "OldFunc";

    NativeAbilityMetaData::InitData(abilityInfo_, data);

    EXPECT_FALSE(data.withNativeModule);
    EXPECT_EQ(data.startupPhase, StartupPhase::PRE_WINDOW);
    EXPECT_TRUE(data.nativeModuleSource.empty());
    EXPECT_TRUE(data.nativeModuleFunc.empty());

    AppUtils::SetSupportNativeUIAbility(true);
}

// ==================== HideWindowOnStartup ====================

/**
 * @tc.number: HideWindowOnStartup_0100
 * @tc.name: NativeAbilityMetaData::HideWindowOnStartup
 * @tc.desc: No metadata at all, should return false
 */
HWTEST_F(NativeModuleUtilTest, HideWindowOnStartup_0100, TestSize.Level1)
{
    EXPECT_FALSE(NativeAbilityMetaData::HideWindowOnStartup(abilityInfo_));
}

/**
 * @tc.number: HideWindowOnStartup_0200
 * @tc.name: NativeAbilityMetaData::HideWindowOnStartup
 * @tc.desc: withNativeModule=false, should return false
 */
HWTEST_F(NativeModuleUtilTest, HideWindowOnStartup_0200, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "false");

    EXPECT_FALSE(NativeAbilityMetaData::HideWindowOnStartup(abilityInfo_));
}

/**
 * @tc.number: HideWindowOnStartup_0300
 * @tc.name: NativeAbilityMetaData::HideWindowOnStartup
 * @tc.desc: withNativeModule=true, startupPhase=pre_window, should return true
 */
HWTEST_F(NativeModuleUtilTest, HideWindowOnStartup_0300, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "pre_window");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    EXPECT_TRUE(NativeAbilityMetaData::HideWindowOnStartup(abilityInfo_));
}

/**
 * @tc.number: HideWindowOnStartup_0400
 * @tc.name: NativeAbilityMetaData::HideWindowOnStartup
 * @tc.desc: withNativeModule=true, startupPhase=pre_foreground, should return true
 */
HWTEST_F(NativeModuleUtilTest, HideWindowOnStartup_0400, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "pre_foreground");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    EXPECT_TRUE(NativeAbilityMetaData::HideWindowOnStartup(abilityInfo_));
}

/**
 * @tc.number: HideWindowOnStartup_0500
 * @tc.name: NativeAbilityMetaData::HideWindowOnStartup
 * @tc.desc: withNativeModule=true, startupPhase=foreground, should return false
 */
HWTEST_F(NativeModuleUtilTest, HideWindowOnStartup_0500, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.startupPhase", "foreground");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    EXPECT_FALSE(NativeAbilityMetaData::HideWindowOnStartup(abilityInfo_));
}

// ==================== IsWithNative ====================

/**
 * @tc.number: IsWithNative_0100
 * @tc.name: NativeAbilityMetaData::IsWithNative
 * @tc.desc: No metadata, should return false
 */
HWTEST_F(NativeModuleUtilTest, IsWithNative_0100, TestSize.Level1)
{
    EXPECT_FALSE(NativeAbilityMetaData::IsWithNative(abilityInfo_));
}

/**
 * @tc.number: IsWithNative_0200
 * @tc.name: NativeAbilityMetaData::IsWithNative
 * @tc.desc: withNativeModule="true" with valid source and func, should return true
 */
HWTEST_F(NativeModuleUtilTest, IsWithNative_0200, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleSource", "libtest.so");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    EXPECT_TRUE(NativeAbilityMetaData::IsWithNative(abilityInfo_));
}

/**
 * @tc.number: IsWithNative_0300
 * @tc.name: NativeAbilityMetaData::IsWithNative
 * @tc.desc: withNativeModule="true" but missing source, should return false
 */
HWTEST_F(NativeModuleUtilTest, IsWithNative_0300, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "true");
    AddMetadata("ohos.ability.nativeModuleFun", "OHMain");

    EXPECT_FALSE(NativeAbilityMetaData::IsWithNative(abilityInfo_));
}

/**
 * @tc.number: IsWithNative_0400
 * @tc.name: NativeAbilityMetaData::IsWithNative
 * @tc.desc: withNativeModule="false", should return false
 */
HWTEST_F(NativeModuleUtilTest, IsWithNative_0400, TestSize.Level1)
{
    AddMetadata("ohos.ability.withNativeModule", "false");

    EXPECT_FALSE(NativeAbilityMetaData::IsWithNative(abilityInfo_));
}
}  // namespace AAFwk
}  // namespace OHOS
