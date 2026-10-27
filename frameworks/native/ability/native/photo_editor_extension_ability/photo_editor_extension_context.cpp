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

#include "photo_editor_extension_context.h"
#include <cstdlib>
#include <fstream>
#include <cerrno>
#include <cstring>
#include "media_errors.h"
#include "hilog_tag_wrapper.h"
#include "file_uri.h"

namespace OHOS {
namespace AbilityRuntime {

const size_t PhotoEditorExtensionContext::CONTEXT_TYPE_ID(std::hash<const char *>{}("PhotoEditorExtensionContext"));
constexpr const char *PANEL_TRANSFER_FILE_PATH = "transferFile";
const uint64_t MAX_IMAGE_SIZE = 50 * 1024 * 1024;

PhotoEditorErrorCode PhotoEditorExtensionContext::SaveEditedContent(const std::string &uri, AAFwk::Want &newWant)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "Save content editing with uri begin, uri: %{public}s", uri.c_str());

    auto pos = uri.find_last_of(".");
    auto sep = uri.find_last_of('/');
    if (pos == std::string::npos || (sep != std::string::npos && pos < sep) || pos + 1 >= uri.size()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uri has no valid extension");
        return PhotoEditorErrorCode::ERROR_CODE_PARAM_ERROR;
    }

    const std::string panelUri = want_->GetStringParam(PANEL_TRANSFER_FILE_PATH);
    TAG_LOGD(AAFwkTag::UI_EXT, "PanelUri: %{public}s", panelUri.c_str());

    PhotoEditorErrorCode errCode = CopyImageToPanel(uri, panelUri);
    if (errCode == PhotoEditorErrorCode::ERROR_OK) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Save content succeed");
        newWant.SetType("image/" + uri.substr(pos + 1));
        newWant.SetUri(panelUri);
    }
    return errCode;
}

PhotoEditorErrorCode PhotoEditorExtensionContext::SaveEditedContent(const std::shared_ptr<Media::PixelMap> &image,
                                                                    const Media::PackOption &packOption,
                                                                    AAFwk::Want &newWant)
{
    const std::string panelUri = want_->GetStringParam(PANEL_TRANSFER_FILE_PATH);
    AppFileService::ModuleFileUri::FileUri fileUri(panelUri);
    std::string panelPhysicalPath = fileUri.GetRealPath();
    TAG_LOGD(AAFwkTag::UI_EXT, "PanelPhysicalPath: %{public}s", panelPhysicalPath.c_str());

    std::ofstream panelFile;
    panelFile.open(panelPhysicalPath, std::ios::binary);
    if (!panelFile.is_open()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not open panel file, reason: %{public}s", strerror(errno));
        panelFile.close();
        return PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR;
    }

    Media::ImagePacker imagePacker;
    uint32_t err = imagePacker.StartPacking(panelFile, packOption);
    if (err != Media::SUCCESS) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Fail to StartPacking %{public}d", err);
        panelFile.close();
        return PhotoEditorErrorCode::ERROR_CODE_INTERNAL_ERROR;
    }

    err = imagePacker.AddImage(*image);
    if (err != Media::SUCCESS) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Fail to AddImage %{public}d", err);
        panelFile.close();
        return PhotoEditorErrorCode::ERROR_CODE_IMAGE_INPUT_ERROR;
    }

    int64_t packedSize = 0;
    if (imagePacker.FinalizePacking(packedSize) != Media::SUCCESS) {
        TAG_LOGE(AAFwkTag::UI_EXT, "FinalizePacking failed");
        panelFile.close();
        return PhotoEditorErrorCode::ERROR_CODE_IMAGE_INPUT_ERROR;
    }

    if (packedSize > static_cast<int64_t>(MAX_IMAGE_SIZE)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "image is bigger than 50M");
        panelFile.close();
        return PhotoEditorErrorCode::ERROR_CODE_IMAGE_TOO_BIG_ERROR;
    }

    panelFile.close();
    newWant.SetUri(panelUri);
    newWant.SetType(packOption.format);
    TAG_LOGD(AAFwkTag::UI_EXT, "Save content succeed");
    return PhotoEditorErrorCode::ERROR_OK;
}

void PhotoEditorExtensionContext::SetWant(const std::shared_ptr<AAFwk::Want> &want)
{
    want_ = want;
    TAG_LOGD(AAFwkTag::UI_EXT, "Set want done");
}

PhotoEditorErrorCode PhotoEditorExtensionContext::CopyImageToPanel(const std::string &imageUri,
                                                                   const std::string &panelUri)
{
    AppFileService::ModuleFileUri::FileUri fileUri(panelUri);
    std::string panelPhysicalPath = fileUri.GetRealPath();
    TAG_LOGD(AAFwkTag::UI_EXT, "ImageUri: %{public}s, panelPhysicalPath: %{public}s", imageUri.c_str(),
             panelPhysicalPath.c_str());

    char imagePath[PATH_MAX] = {0};
    if (realpath(imageUri.c_str(), imagePath) == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "realpath error: %{public}d.", errno);
        return PhotoEditorErrorCode::ERROR_CODE_IMAGE_INPUT_ERROR;
    }

    std::ifstream sourceFile;
    sourceFile.open(imagePath, std::ios::binary);
    if (!sourceFile.is_open()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not open source file");
        sourceFile.close();
        return PhotoEditorErrorCode::ERROR_CODE_IMAGE_INPUT_ERROR;
    }

    sourceFile.seekg(0, sourceFile.end);
    std::streampos imageSize = sourceFile.tellg();
    if (static_cast<uint64_t>(imageSize) > MAX_IMAGE_SIZE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "image is bigger than 50M");
        sourceFile.close();
        return PhotoEditorErrorCode::ERROR_CODE_IMAGE_TOO_BIG_ERROR;
    }
    sourceFile.seekg(0, sourceFile.beg);

    std::ofstream panelFile;
    panelFile.open(panelPhysicalPath, std::ios::binary);
    if (!panelFile.is_open()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not open panel file");
        sourceFile.close();
        return PhotoEditorErrorCode::ERROR_CODE_IMAGE_INPUT_ERROR;
    }

    char buffer[4096];
    while (sourceFile.read(buffer, sizeof(buffer))) {
        panelFile.write(buffer, sizeof(buffer));
    }
    panelFile.write(buffer, sourceFile.gcount());
    sourceFile.close();
    panelFile.close();

    TAG_LOGD(AAFwkTag::UI_EXT, "Copy succeed");
    return PhotoEditorErrorCode::ERROR_OK;
}

} // namespace AbilityRuntime
} // namespace OHOS