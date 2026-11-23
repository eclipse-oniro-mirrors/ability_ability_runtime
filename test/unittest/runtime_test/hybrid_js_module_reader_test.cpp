/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define protected public
#include "hybrid_js_module_reader.h"
#include "extractor.h"
#undef private
#undef protected


using namespace testing;
using namespace testing::ext;
using HybridJsModuleReader = OHOS::AbilityRuntime::HybridJsModuleReader;
using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AAFwk {
class HybridJsModuleReaderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void HybridJsModuleReaderTest::SetUpTestCase()
{}

void HybridJsModuleReaderTest::TearDownTestCase()
{}

void HybridJsModuleReaderTest::SetUp()
{}

void HybridJsModuleReaderTest::TearDown()
{}

/**
 * @tc.name: HybridJsModuleReaderTest_0100
 * @tc.desc: HybridJsModuleReaderTest Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(HybridJsModuleReaderTest, HybridJsModuleReaderTest_0100, TestSize.Level0)
{
    HybridJsModuleReader HybridJsModuleReader("HybridJsModuleReader", "");
    uint8_t *buff = nullptr;
    size_t buffSize = 0;
    std::string errorMsg = "";
    auto result = HybridJsModuleReader("", &buff, &buffSize, errorMsg);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: HybridJsModuleReaderTest_0200
 * @tc.desc: HybridJsModuleReaderTest Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(HybridJsModuleReaderTest, HybridJsModuleReaderTest_0200, TestSize.Level0)
{
    HybridJsModuleReader HybridJsModuleReader("HybridJsModuleReader", "");
    uint8_t *buff = nullptr;
    size_t buffSize = 0;
    std::string errorMsg = "";
    auto result = HybridJsModuleReader("bundleName/moduleName", &buff, &buffSize, errorMsg);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetPresetAppHapPathTest_0100
 * @tc.desc: GetPresetAppHapPath Test
 * @tc.type: FUNC
 */
HWTEST_F(HybridJsModuleReaderTest, GetPresetAppHapPathTest_0100, TestSize.Level0)
{
    HybridJsModuleReader HybridJsModuleReader("HybridJsModuleReader", "");
    bool needFindPluginHsp = true;
    std::string hapPath = HybridJsModuleReader.GetPresetAppHapPath("bundleName/unittest_hybrid_module", "",
        needFindPluginHsp);
    EXPECT_EQ(hapPath, "bundleName/unittest_hybrid_module");
}

/**
 * @tc.name: GetPresetAppHapPathTest_0200
 * @tc.desc: GetPresetAppHapPath Test
 * @tc.type: FUNC
 */
HWTEST_F(HybridJsModuleReaderTest, GetPresetAppHapPathTest_0200, TestSize.Level0)
{
    HybridJsModuleReader HybridJsModuleReader("HybridJsModuleReader", "/data/storage/el1/test.hsp");
    bool needFindPluginHsp = true;
    std::string hapPath = HybridJsModuleReader.GetPresetAppHapPath("/data/storage/el1/unittest_hybrid_module.hsp",
        "", needFindPluginHsp);
    EXPECT_EQ(hapPath, "/data/storage/el1/unittest_hybrid_module.hsp");
}
}  // namespace AAFwk
}  // namespace OHOS
