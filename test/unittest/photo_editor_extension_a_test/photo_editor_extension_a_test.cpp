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
{}

void PhotoEditorExtensionContextTest::TearDownTestCase(void)
{}

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
    std::string uri = TMP_IMAGE_NO_EXT;
    AAFwk::Want newWant;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    PhotoInfos->SetWant(want);
    want->SetParam(PANEL_TRANSFER_FILE_PATHS, uri);
    PhotoEditorErrorCode res = PhotoInfos->SaveEditedContent(uri, newWant);
    EXPECT_EQ(res, PhotoEditorErrorCode::ERROR_CODE_IMAGE_INPUT_ERROR);
    GTEST_LOG_(INFO) << "SaveEditedContent_0300 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
