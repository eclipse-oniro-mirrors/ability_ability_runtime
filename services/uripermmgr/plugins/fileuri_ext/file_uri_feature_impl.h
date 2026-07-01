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

#ifndef OHOS_AAFWK_FILE_URI_FEATURE_IMPL_H
#define OHOS_AAFWK_FILE_URI_FEATURE_IMPL_H

#include <string>

#include "feature/ifile_uri_feature.h"
#include "file_uri.h"

namespace OHOS {
namespace AAFwk {

// Plugin implementation of IFileUriFeature, compiled into libupms_fileuri_ext.z.so.
// Ports the former FilePermissionManager::GetPathPolicyInfoFromUri file_uri usage.
class FileUriFeatureImpl : public IFileUriFeature {
public:
    FileUriFeatureImpl() = default;
    ~FileUriFeatureImpl() override = default;

    std::string GetRealPathBySA(const std::string &uriString, const std::string &bundleName) override;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_FILE_URI_FEATURE_IMPL_H
