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

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "madvise_utils.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AbilityRuntime {
class MadviseUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MadviseUtilsTest::SetUpTestCase(void)
{}

void MadviseUtilsTest::TearDownTestCase(void)
{}

void MadviseUtilsTest::SetUp()
{}

void MadviseUtilsTest::TearDown()
{}

/**
 * @tc.number: MadviseSingleLibrary_0100
 * @tc.desc: Test MadviseSingleLibrary works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseSingleLibrary_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseSingleLibrary_0100 start.");

    const char* libName = "";
    bool result = MadviseUtil::MadviseSingleLibrary(libName);
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "MadviseSingleLibrary_0100 end.");
}

/**
 * @tc.number: MadviseSingleLibrary_0200
 * @tc.desc: Test MadviseSingleLibrary works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseSingleLibrary_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseSingleLibrary_0200 start.");

    const char* libName = "testtest.so";
    bool result = MadviseUtil::MadviseSingleLibrary(libName);
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "MadviseSingleLibrary_0200 end.");
}

/**
 * @tc.number: MadviseGeneralFiles_0100
 * @tc.desc: Test MadviseGeneralFiles works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseGeneralFiles_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseGeneralFiles_0100 start.");

    std::vector<std::string> filenames ;
    int32_t result = MadviseUtil::MadviseGeneralFiles(filenames);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "MadviseGeneralFiles_0100 end.");
}

/**
 * @tc.number: MadviseGeneralFiles_0200
 * @tc.desc: Test MadviseGeneralFiles works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseGeneralFiles_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseGeneralFiles_0200 start.");

    std::vector<std::string> filenames = { "testtest.hsp" };
    int32_t result = MadviseUtil::MadviseGeneralFiles(filenames);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "MadviseGeneralFiles_0200 end.");
}

/**
 * @tc.number: MadviseWithConfigFile_0100
 * @tc.desc: Test MadviseWithConfigFile works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseWithConfigFile_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseWithConfigFile_0100 start.");

    const char* bundleName = "";
    int32_t result = MadviseUtil::MadviseWithConfigFile(bundleName);
    EXPECT_EQ(result, -1);

    TAG_LOGI(AAFwkTag::TEST, "MadviseWithConfigFile_0100 end.");
}

/**
 * @tc.number: IsValidEvictFileName_0100
 * @tc.desc: Test IsValidEvictFileName with .so extension returns true
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, IsValidEvictFileName_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0100 start.");

    EXPECT_EQ(MadviseUtil::IsValidEvictFileName("libtest.so"), true);

    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0100 end.");
}

/**
 * @tc.number: IsValidEvictFileName_0200
 * @tc.desc: Test IsValidEvictFileName with .hap extension returns true
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, IsValidEvictFileName_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0200 start.");

    EXPECT_EQ(MadviseUtil::IsValidEvictFileName("entry.hap"), true);

    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0200 end.");
}

/**
 * @tc.number: IsValidEvictFileName_0300
 * @tc.desc: Test IsValidEvictFileName with .hsp extension returns true
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, IsValidEvictFileName_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0300 start.");

    EXPECT_EQ(MadviseUtil::IsValidEvictFileName("feature.hsp"), true);

    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0300 end.");
}

/**
 * @tc.number: IsValidEvictFileName_0400
 * @tc.desc: Test IsValidEvictFileName with empty string returns false
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, IsValidEvictFileName_0400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0400 start.");

    EXPECT_EQ(MadviseUtil::IsValidEvictFileName(""), false);

    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0400 end.");
}

/**
 * @tc.number: IsValidEvictFileName_0500
 * @tc.desc: Test IsValidEvictFileName with unsupported extension returns false
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, IsValidEvictFileName_0500, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0500 start.");

    EXPECT_EQ(MadviseUtil::IsValidEvictFileName("config.json"), false);
    EXPECT_EQ(MadviseUtil::IsValidEvictFileName("library"), false);
    EXPECT_EQ(MadviseUtil::IsValidEvictFileName("archive.bin"), false);

    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0500 end.");
}

/**
 * @tc.number: IsValidEvictFileName_0600
 * @tc.desc: Test IsValidEvictFileName with extension in the middle returns false
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, IsValidEvictFileName_0600, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0600 start.");

    // EndsWith requires the suffix to be a true suffix; a bare ".so" is rejected
    // because the size check is strictly greater-than.
    EXPECT_EQ(MadviseUtil::IsValidEvictFileName(".so"), false);
    EXPECT_EQ(MadviseUtil::IsValidEvictFileName(".hap"), false);
    EXPECT_EQ(MadviseUtil::IsValidEvictFileName(".hsp"), false);
    EXPECT_EQ(MadviseUtil::IsValidEvictFileName("lib.so.bak"), false);

    TAG_LOGI(AAFwkTag::TEST, "IsValidEvictFileName_0600 end.");
}

/**
 * @tc.number: EvictFilePages_0100
 * @tc.desc: Test EvictFilePages with empty file list returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, EvictFilePages_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0100 start.");

    std::vector<std::string> fileNames;
    int32_t result = MadviseUtil::EvictFilePages(fileNames);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0100 end.");
}

/**
 * @tc.number: EvictFilePages_0200
 * @tc.desc: Test EvictFilePages skips empty entries and returns 0
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, EvictFilePages_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0200 start.");

    std::vector<std::string> fileNames = { "", "", "" };
    int32_t result = MadviseUtil::EvictFilePages(fileNames);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0200 end.");
}

/**
 * @tc.number: EvictFilePages_0300
 * @tc.desc: Test EvictFilePages routes .so names to MadviseSingleLibrary
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, EvictFilePages_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0300 start.");

    // Non-matching library name: MadviseSingleLibrary returns false, so the
    // overall success count is 0.
    std::vector<std::string> fileNames = { "libnotexist.so" };
    int32_t result = MadviseUtil::EvictFilePages(fileNames);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0300 end.");
}

/**
 * @tc.number: EvictFilePages_0400
 * @tc.desc: Test EvictFilePages batches non-.so names through MadviseGeneralFiles
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, EvictFilePages_0400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0400 start.");

    // Non-existent hap: no VMA regions resolved, MadviseGeneralFiles returns 0.
    std::vector<std::string> fileNames = { "notexist.hap", "notexist.hsp" };
    int32_t result = MadviseUtil::EvictFilePages(fileNames);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0400 end.");
}

/**
 * @tc.number: EvictFilePages_0500
 * @tc.desc: Test EvictFilePages with mixed so/hap/empty entries
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, EvictFilePages_0500, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0500 start.");

    std::vector<std::string> fileNames = { "", "libnotexist.so", "notexist.hap", "notexist.hsp" };
    int32_t result = MadviseUtil::EvictFilePages(fileNames);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "EvictFilePages_0500 end.");
}

/**
 * @tc.number: EvictModuleFilePages_0100
 * @tc.desc: Test EvictModuleFilePages with empty module list returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, EvictModuleFilePages_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "EvictModuleFilePages_0100 start.");

    std::vector<std::string> moduleNames;
    ErrCode result = MadviseUtil::EvictModuleFilePages(moduleNames);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "EvictModuleFilePages_0100 end.");
}

/**
 * @tc.number: EvictModuleFilePages_0200
 * @tc.desc: Test EvictModuleFilePages returns ERR_EVICT_CONFIG_PARSE when caller hap cannot be resolved
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, EvictModuleFilePages_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "EvictModuleFilePages_0200 start.");

    // In the unit-test environment there is no live BundleMgrHelper backing,
    // so GetCallerHapModules fails and the function short-circuits with
    // ERR_EVICT_CONFIG_PARSE before reaching module lookup or file eviction.
    std::vector<std::string> moduleNames = { "entry" };
    ErrCode result = MadviseUtil::EvictModuleFilePages(moduleNames);
    EXPECT_EQ(result, AAFwk::ERR_EVICT_CONFIG_PARSE);

    TAG_LOGI(AAFwkTag::TEST, "EvictModuleFilePages_0200 end.");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
