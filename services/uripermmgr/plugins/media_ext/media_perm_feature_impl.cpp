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

#include "media_perm_feature_impl.h"

#include <vector>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

Media::MediaPermissionHelper *MediaPermFeatureImpl::GetMediaPermissionHelper()
{
    TAG_LOGI(AAFwkLogTag::URIPERMMGR, "GetMediaPermissionHelper.");
    auto mediaPermissionHelper = Media::MediaPermissionHelper::GetMediaPermissionHelper();
    if (mediaPermissionHelper == nullptr) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "GetMediaPermissionHelper failed.");
        return mediaPermissionHelper;
    }
    mediaPermissionHelper->InitMediaPermissionHelper();
    return mediaPermissionHelper;
}

std::vector<bool> MediaPermFeatureImpl::CheckUriPermission(const std::vector<std::string> &uriVec,
                                                           uint32_t callerTokenId, uint32_t flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkLogTag::URIPERMMGR, "uris:%{public}zu, callerTokenId:%{public}u, flag:%{public}u", uriVec.size(),
             callerTokenId, flag);
    std::vector<bool> results = std::vector<bool>(uriVec.size(), false);
    flag &= (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION);
    auto mediaPermissionHelper = GetMediaPermissionHelper();
    if (mediaPermissionHelper == nullptr) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "GetMediaPermissionHelper failed.");
        return results;
    }
    std::vector<uint32_t> flags(uriVec.size(), flag);
    auto ret = IN_PROCESS_CALL(mediaPermissionHelper->CheckPhotoUriPermission(callerTokenId, uriVec, results, flags));
    TAG_LOGD(AAFwkLogTag::URIPERMMGR, "CheckPhotoUriPermission finished.");
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "Check photo uri permission failed, ret is %{public}d", ret);
        results = std::vector<bool>(uriVec.size(), false);
        return results;
    }
    if (results.size() != uriVec.size()) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "size of results is unexpected: %{public}zu", results.size());
        results = std::vector<bool>(uriVec.size(), false);
        return results;
    }
    return results;
}

int32_t MediaPermFeatureImpl::GrantUriPermission(const std::vector<std::string> &uris, uint32_t flag,
                                                 uint32_t callerTokenId, uint32_t targetTokenId,
                                                 int32_t hideSensitiveType)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    flag &= (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION);
    auto photoPermissionType = FlagToFileOpenMode(flag);
    TAG_LOGI(AAFwkLogTag::URIPERMMGR, "uris:%{public}zu, flag:%{public}u, perType:%{public}d, senType:%{public}d",
             uris.size(), flag, static_cast<int>(photoPermissionType), hideSensitiveType);
    auto mediaPermissionHelper = GetMediaPermissionHelper();
    if (mediaPermissionHelper == nullptr) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "GetMediaPermissionHelper failed.");
        return INNER_ERR;
    }
    auto mediahideSensitiveType = ConvertHideSensitiveType(hideSensitiveType);
    std::vector<Media::PhotoPermissionType> photoPermissionTypes(uris.size(), photoPermissionType);
    auto ret = IN_PROCESS_CALL(mediaPermissionHelper->GrantPhotoUriPermission(
        callerTokenId, targetTokenId, uris, photoPermissionTypes, mediahideSensitiveType));
    TAG_LOGD(AAFwkLogTag::URIPERMMGR, "GrantPhotoUriPermission finished.");
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "Grant photo uri permission failed, ret is %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

Media::PhotoPermissionType MediaPermFeatureImpl::FlagToFileOpenMode(uint32_t flag)
{
    if (flag == (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) {
        return Media::PhotoPermissionType::TEMPORARY_READWRITE_IMAGEVIDEO;
    }
    if (flag == Want::FLAG_AUTH_WRITE_URI_PERMISSION) {
        return Media::PhotoPermissionType::TEMPORARY_WRITE_IMAGEVIDEO;
    }
    return Media::PhotoPermissionType::TEMPORARY_READ_IMAGEVIDEO;
}

Media::HideSensitiveType MediaPermFeatureImpl::ConvertHideSensitiveType(int32_t hideSensitiveType)
{
    return static_cast<Media::HideSensitiveType>(hideSensitiveType);
}

int32_t MediaPermFeatureImpl::RevokeUriPermission(uint32_t callerTokenId, uint32_t targetTokenId,
                                                  const std::string &uri)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkLogTag::URIPERMMGR, "called, %{private}u, %{private}u, %{private}s", callerTokenId, targetTokenId,
             uri.c_str());
    std::vector<std::string> uris = {uri};
    auto mediaPermissionHelper = GetMediaPermissionHelper();
    if (mediaPermissionHelper == nullptr) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "GetMediaPermissionHelper failed.");
        return INNER_ERR;
    }
    auto ret = IN_PROCESS_CALL(mediaPermissionHelper->CancelPhotoUriPermission(callerTokenId, targetTokenId, uris));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkLogTag::URIPERMMGR, "Revoke media uri permission failed, ret:%{public}d", ret);
        return ret;
    }
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
