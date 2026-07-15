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

#include "mock_my_flag.h"

namespace OHOS {
namespace AAFwk {
int MyFlag::flag_ = 0;
bool MyFlag::permissionFileAccessManager_ = false;
bool MyFlag::permissionAgentFileAccess_ = false;
bool MyFlag::permissionAgentFileAccessDeclared_ = false;
bool MyFlag::permissionWriteImageVideo_ = false;
bool MyFlag::permissionReadImageVideo_ = false;
bool MyFlag::permissionAllMedia_ = false;
bool MyFlag::permissionWriteAudio_ = false;
bool MyFlag::permissionReadAudio_ = false;
bool MyFlag::permissionProxyAuthorization_ = false;
bool MyFlag::permissionAll_ = false;
bool MyFlag::permissionPrivileged_ = false;
bool MyFlag::permissionReadWriteDownload_ = false;
bool MyFlag::permissionReadWriteDesktop_ = false;
bool MyFlag::permissionReadWriteDocuments_ = false;
bool MyFlag::IsSystemAppCall_ = false;
bool MyFlag::permissionFileAccessPersist_ = false;
bool MyFlag::permissionGrantUriPermissionAsCaller_ = false;
bool MyFlag::permissionSandboxAccessManager_ = false;
bool MyFlag::isSAOrSystemAppCall_ = false;
bool MyFlag::isSystemAppCall_ = false;
bool MyFlag::isPrivilegedSACall_ = false;
bool MyFlag::isUriTypeValid_ = false;
bool MyFlag::isDocsCloudUri_ = false;
int32_t MyFlag::getTokenIdByBundleNameStatus_ = 0;
int32_t MyFlag::processUdmfKeyRet_ = 0;
std::vector<std::string> MyFlag::udmfUtilsUris_ = {};
bool MyFlag::upmsUtilsCheckIsSystemAppByBundleNameRet_ = true;
bool MyFlag::upmsUtilsCheckIsSystemAppByTokenIdRet_ = false;
bool MyFlag::upmsUtilsGetDirByBundleNameAndAppIndexRet_ = true;
std::string MyFlag::upmsUtilsAlterBundleName_ = "";
bool MyFlag::upmsUtilsGetAlterBundleNameByTokenIdRet_ = true;
std::string MyFlag::upmsUtilsBundleName_ = "";
bool MyFlag::upmsUtilsGetBundleNameByTokenIdRet_ = false;
std::string MyFlag::upmsUtilsAppId_ = "";
int32_t MyFlag::upmsUtilsGetAppIdByBundleNameRet_ = 0;
bool MyFlag::upmsUtilsIsFoundationCallRet_ = false;
uint32_t MyFlag::upmsUtilsTokenId_ = 0;
bool MyFlag::fudUtilsGenerateFUDAppInfoRet_ = true;
int32_t MyFlag::fudAppInfoUserId_ = -1;
std::string MyFlag::bundleName_ = "";
std::vector<FUDAppInfoMockResult> MyFlag::generateFUDAppInfoResults_ = {};
TokenInfoMap MyFlag::tokenInfos = {};
bool MyFlag::isUdmfOrPasteboardCallRet_ = false;
bool MyFlag::isDFSCallRet_ = false;
bool MyFlag::isSandboxAppRet_ = false;
bool MyFlag::isSACall_ = false;
} // namespace AAFwk
} // namespace OHOS