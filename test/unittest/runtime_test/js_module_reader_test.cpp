/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "js_module_reader.h"
#include "extractor.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using JsModuleReader = OHOS::AbilityRuntime::JsModuleReader;
using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AAFwk {
class JsModuleReaderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsModuleReaderTest::SetUpTestCase()
{}

void JsModuleReaderTest::TearDownTestCase()
{}

void JsModuleReaderTest::SetUp()
{}

void JsModuleReaderTest::TearDown()
{}

/**
 * @tc.name: JsModuleReaderTest_0100
 * @tc.desc: JsModuleReaderTest Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(JsModuleReaderTest, JsModuleReaderTest_0100, TestSize.Level2)
{
    JsModuleReader jsModuleReader("JsModuleReader", "");
    uint8_t *buff = nullptr;
    size_t buffSize = 0;
    std::string errorMsg = "";
    auto result = jsModuleReader("", &buff, &buffSize, errorMsg);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: JsModuleReaderTest_0200
 * @tc.desc: JsModuleReaderTest Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(JsModuleReaderTest, JsModuleReaderTest_0200, TestSize.Level2)
{
    JsModuleReader jsModuleReader("JsModuleReader", "");
    uint8_t *buff = nullptr;
    size_t buffSize = 0;
    std::string errorMsg = "";
    auto result = jsModuleReader("bundleName/moduleName", &buff, &buffSize, errorMsg);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetPresetAppHapPathTest_0100
 * @tc.desc: GetPresetAppHapPath Test
 * @tc.type: FUNC
 */
HWTEST_F(JsModuleReaderTest, GetPresetAppHapPathTest_0100, TestSize.Level2)
{
    JsModuleReader jsModuleReader("JsModuleReader", "");
    bool needFindPluginHsp = true;
    std::string errorMsg = "";
    std::string hapPath = jsModuleReader.GetPresetAppHapPath("", "", needFindPluginHsp, errorMsg);
    EXPECT_TRUE(hapPath.empty());
}

/**
 * @tc.name: GetPresetAppHapPathTest_0200
 * @tc.desc: GetPresetAppHapPath Test
 * @tc.type: FUNC
 */
HWTEST_F(JsModuleReaderTest, GetPresetAppHapPathTest_0200, TestSize.Level2)
{
    JsModuleReader jsModuleReader("JsModuleReader", "/data/storage/el1/test.hsp");
    bool needFindPluginHsp = true;
    std::string errorMsg = "";
    std::string hapPath = jsModuleReader.GetPresetAppHapPath("", "", needFindPluginHsp, errorMsg);
    EXPECT_TRUE(hapPath.empty());
}

/**
 * @tc.name: GetFormAppHspPathTest_0100
 * @tc.desc: GetFormAppHspPath Test
 * @tc.type: FUNC
 */
HWTEST_F(JsModuleReaderTest, GetFormAppHspPathTest_0100, TestSize.Level2)
{
    JsModuleReader jsModuleReader("JsModuleReader", "");
    bool needFindPluginHsp = false;
    std::string errorMsg = "";
    auto realHapPath = jsModuleReader.GetFormAppHspPath("inputPath", needFindPluginHsp, errorMsg);
    EXPECT_EQ(realHapPath, "/data/bundles/JsModuleReader/inputPath.hsp");
}

/**
 * @tc.name: GetPresetAppHapPath_0100
 * @tc.desc: GetPresetAppHapPath Test
 * @tc.type: FUNC
 */
HWTEST_F(JsModuleReaderTest, GetPresetAppHapPath_0100, TestSize.Level2)
{
    JsModuleReader jsModuleReader("JsModuleReader", "");
    std::string inputPath = "inputPath/inputPath2";
    std::string bundleName = "bundleName";
    bool needFindPluginHsp = true;
    std::string errorMsg = "";
    auto realHapPath = jsModuleReader.GetPresetAppHapPath(inputPath, bundleName, needFindPluginHsp, errorMsg);
    EXPECT_EQ(realHapPath, "inputPath/inputPath2");
}
}  // namespace AAFwk
}  // namespace OHOS
