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
#include <cstdio>
#include <fstream>

#include "ability_handler.h"
#include "js_photo_editor_extension.h"
#define private public
#include "photo_editor_extension_context.h"
#undef private
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;
const std::string PANEL_TRANSFER_FILE_PATHS = "transferFile";
const std::string PANEL_URI_ONE_PATHS = "file://com.example.testdemo/data/storage/el2/base/haps/test.jpg";
const std::string PANEL_URI_TWO_PATHS = "file://com.hmos.notepad/data/storage/el2/distributedfiles/dir/1.txt";
const std::string TMP_DIR = "/data/local/tmp/photo_editor_test/";
const std::string TMP_IMAGE_WITH_EXT = TMP_DIR + "test_image.jpg";
const std::string TMP_IMAGE_NO_EXT = TMP_DIR + "test_image_no_ext";
const std::string TMP_PANEL_FILE = TMP_DIR + "panel_output.jpg";
class PhotoEditorExtensionContextTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PhotoEditorExtensionContextTest::SetUpTestCase(void)
{
    // create tmp dir and dummy files for real-fs test cases
    std::string mkdirCmd = "mkdir -p " + TMP_DIR;
    system(mkdirCmd.c_str());
    std::ofstream extFile(TMP_IMAGE_WITH_EXT, std::ios::binary);
    extFile << "dummy jpg content";
    extFile.close();
    std::ofstream noExtFile(TMP_IMAGE_NO_EXT, std::ios::binary);
    noExtFile << "dummy no-ext content";
    noExtFile.close();
    std::ofstream panelFile(TMP_PANEL_FILE, std::ios::binary);
    panelFile << "panel placeholder";
    panelFile.close();
}

void PhotoEditorExtensionContextTest::TearDownTestCase(void)
{
    std::string rmCmd = "rm -rf " + TMP_DIR;
    system(rmCmd.c_str());
}

void PhotoEditorExtensionContextTest::SetUp(void)
{}

void PhotoEditorExtensionContextTest::TearDown(void)
{}

/**
 * @tc.number: SaveEditedContent_0100
 * @tc.name: SaveEditedContent
 * @tc.desc: Call SaveEditedContent open file
 */

HWTEST_F(PhotoEditorExtensionContextTest, SaveEditedContent_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SaveEditedContent_0100 start";
    auto PhotoInfos = std::make_shared<PhotoEditorExtensionContext>();
    std::string uri = PANEL_URI_ONE_PATHS;
    AAFwk::Want newWant;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    PhotoInfos->SetWant(want);
    want->SetParam(PANEL_TRANSFER_FILE_PATHS, uri);
    PhotoEditorErrorCode res = PhotoInfos->SaveEditedContent(uri, newWant);
    EXPECT_EQ(res, PhotoEditorErrorCode::ERROR_CODE_IMAGE_INPUT_ERROR);
    GTEST_LOG_(INFO) << "SaveEditedContent_0100 end";
}

/**
 * @tc.number: SaveEditedContent_0200
 * @tc.name: SaveEditedContent
 * @tc.desc: Call SaveEditedContent open file
 */

HWTEST_F(PhotoEditorExtensionContextTest, SaveEditedContent_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SaveEditedContent_0200 start";
    auto PhotoInfos = std::make_shared<PhotoEditorExtensionContext>();
    const std::shared_ptr<OHOS::Media::PixelMap> image = nullptr;
    const Media::PackOption packOption;
    AAFwk::Want newWant;
    std::string uri = PANEL_URI_TWO_PATHS;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    PhotoInfos->SetWant(want);
    want->SetParam(PANEL_TRANSFER_FILE_PATHS, uri);
    PhotoEditorErrorCode res = PhotoInfos->SaveEditedContent(image, packOption, newWant);
    EXPECT_EQ(res, PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR);
    GTEST_LOG_(INFO) << "SaveEditedContent_0200 start";
}

/**
 * @tc.number: SetWant_0100
 * @tc.name: SetWant_0100
 * @tc.desc: Call SetWant for wan is not nullpetr;
 */

HWTEST_F(PhotoEditorExtensionContextTest, SetWant_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SetWant_0100 start";
    auto PhotoInfos = std::make_shared<PhotoEditorExtensionContext>();
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    PhotoInfos->SetWant(want);
    EXPECT_NE(want, nullptr);
    GTEST_LOG_(INFO) << "SetWant_0100 end";
}

/**
 * @tc.number: SaveEditedContent_0300
 * @tc.name: SaveEditedContent
 * @tc.desc: uri without extension and file not exist, CopyImageToPanel fails first
 */

HWTEST_F(PhotoEditorExtensionContextTest, SaveEditedContent_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SaveEditedContent_0300 start";
    auto PhotoInfos = std::make_shared<PhotoEditorExtensionContext>();
    std::string uri = "/data/local/tmp/photo_editor_nonexistent_noext";
    AAFwk::Want newWant;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    PhotoInfos->SetWant(want);
    want->SetParam(PANEL_TRANSFER_FILE_PATHS, uri);
    PhotoEditorErrorCode res = PhotoInfos->SaveEditedContent(uri, newWant);
    EXPECT_EQ(res, PhotoEditorErrorCode::ERROR_CODE_IMAGE_INPUT_ERROR);
    GTEST_LOG_(INFO) << "SaveEditedContent_0300 end";
}

/**
 * @tc.number: SaveEditedContent_0400
 * @tc.name: SaveEditedContent
 * @tc.desc: uri without extension but file exists, should return PARAM_ERROR after fix
 */

HWTEST_F(PhotoEditorExtensionContextTest, SaveEditedContent_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SaveEditedContent_0400 start";
    auto PhotoInfos = std::make_shared<PhotoEditorExtensionContext>();
    std::string uri = TMP_IMAGE_NO_EXT;
    AAFwk::Want newWant;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    PhotoInfos->SetWant(want);
    want->SetParam(PANEL_TRANSFER_FILE_PATHS, TMP_PANEL_FILE);
    PhotoEditorErrorCode res = PhotoInfos->SaveEditedContent(uri, newWant);
    // CopyImageToPanel succeeds, then find_last_of(".") returns npos → PARAM_ERROR
    EXPECT_EQ(res, PhotoEditorErrorCode::ERROR_CODE_PARAM_ERROR);
    GTEST_LOG_(INFO) << "SaveEditedContent_0400 end";
}

/**
 * @tc.number: SaveEditedContent_0500
 * @tc.name: SaveEditedContent
 * @tc.desc: uri with extension and file exists, should succeed and set correct type
 */

HWTEST_F(PhotoEditorExtensionContextTest, SaveEditedContent_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "SaveEditedContent_0500 start";
    auto PhotoInfos = std::make_shared<PhotoEditorExtensionContext>();
    std::string uri = TMP_IMAGE_WITH_EXT;
    AAFwk::Want newWant;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    PhotoInfos->SetWant(want);
    want->SetParam(PANEL_TRANSFER_FILE_PATHS, TMP_PANEL_FILE);
    PhotoEditorErrorCode res = PhotoInfos->SaveEditedContent(uri, newWant);
    EXPECT_EQ(res, PhotoEditorErrorCode::ERROR_OK);
    // verify Want type is correctly set to "image/jpg"
    EXPECT_EQ(newWant.GetType(), "image/jpg");
    GTEST_LOG_(INFO) << "SaveEditedContent_0500 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
