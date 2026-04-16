/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_URI_UTILS_H
#define OHOS_ABILITY_RUNTIME_URI_UTILS_H

#include <string>
#include <vector>

#include "ability_record.h"
#include "check_result.h"
#include "nocopyable.h"
#include "uri.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
struct GrantUriPermissionInfo {
    std::string targetBundleName;
    std::string targetAbilityName = "";
    uint32_t callerTokenId = 0;
    uint32_t flag = 0;
    int32_t appIndex = 0;
    int32_t userId = -1;
    int32_t targetUid = -1;
    int32_t callerUid = -1;
    int32_t collaboratorType = 0;
    bool isSandboxApp = false;
    bool isNotifyCollaborator = false;
};

class UriUtils {
public:
    static UriUtils &GetInstance();

    /**
     * @brief Get list of URIs with granted permissions
     * @param uriVec Input vector of URI strings
     * @param checkResults Vector indicating permission check results
     * @param callerTokenId Token ID of the caller
     * @param targetBundleName Bundle name of the target application
     * @param want The Want object containing URI information
     * @return Vector of Uri objects with granted permissions
     */
    std::vector<Uri> GetPermissionedUriList(const std::vector<std::string> &uriVec,
        const std::vector<CheckResult> &checkResults, uint32_t callerTokenId,
        const std::string &targetBundleName, Want &want);

    /**
     * @brief Extract URI list from Want object
     * @param want The Want object containing URI information
     * @param uriVec Output vector to store extracted URIs
     * @return true if URIs were successfully extracted, false otherwise
     */
    bool GetUriListFromWant(Want &want, std::vector<std::string> &uriVec);

#ifdef SUPPORT_UPMS
    /**
     * @brief Check if URI permission flag is set in Want
     * @param flag The flag for grant uri permission
     * @return true if URI permission flag is set, false otherwise
     */
    bool IsGrantUriPermissionFlag(uint32_t flag);
#endif // SUPPORT_UPMS

    /**
     * @brief Check if extension type is service extension
     * @param extensionAbilityType The extension ability type to check
     * @return true if the type is service extension, false otherwise
     */
    bool IsServiceExtensionType(AppExecFwk::ExtensionAbilityType extensionAbilityType);

#ifdef SUPPORT_UPMS
    /**
     * @brief Grant URI permission
     * @param want The Want object containing URI information
     * @param callerTokenId Token ID of the caller
     * @param targetBundleName Bundle name of the target application
     * @param appIndex Application index
     */
    bool GrantDmsUriPermission(Want &want, uint32_t callerTokenId, std::string targetBundleName, int32_t appIndex);

    /**
     * @brief Grant URI permission for service extension ability
     * @param abilityRequest The ability request containing URI information
     * @return true if permission was successfully granted, false otherwise
     */
    bool GrantUriPermissionForServiceExtension(const AbilityRequest &abilityRequest);

    /**
     * @brief Grant URI permission for UI or service extension ability
     * @param abilityRequest The ability request containing URI information
     * @return true if permission was successfully granted, false otherwise
     */
    bool GrantUriPermissionForUIOrServiceExtension(const AbilityRequest &abilityRequest);

    /**
     * @brief Grant URI permission with detailed parameters
     * @param want The Want object containing URI information
     * @param grantInfo The param for grant uri permission
     */
    bool GrantUriPermission(Want &want, const GrantUriPermissionInfo &grantInfo);

    /**
     * @brief Check URI permission for the caller
     * @param callerTokenId Token ID of the caller
     * @param want The Want object containing URI information
     */
    void CheckUriPermission(uint32_t callerTokenId, Want &want);

    /**
     * @brief Grant URI permission for a list of URIs
     * @param uriVec Vector of URI strings
     * @param flag Permission flag
     * @param targetBundleName Bundle name of the target application
     * @param appIndex Application index
     * @param initiatorTokenId Token ID of the initiator
     */
    void GrantUriPermission(const std::vector<std::string> &uriVec, int32_t flag,
        const std::string &targetBundleName, int32_t appIndex, uint32_t initiatorTokenId);
#endif // SUPPORT_UPMS
private:
    UriUtils();
    ~UriUtils();

#ifdef SUPPORT_UPMS
    /**
     * @brief Grant URI permission for shell process
     * @param strUriVec Vector of URI strings
     * @param flag Permission flag
     * @param targetPkg Package name of the target
     * @param appIndex Application index
     * @return true if permission was successfully granted, false otherwise
     */
    bool GrantShellUriPermission(const std::vector<std::string> &strUriVec, uint32_t flag,
        const std::string &targetPkg, int32_t appIndex);
    
    /**
     * @brief Internal implementation of URI permission granting
     * @param uriVec Vector of URI strings
     * @param grantInfo The param for grant uri permission
     * @param want Want object containing URI information
     * @param isBrokerCall Whether the call is from broker
     * @return true if permission was successfully granted, false otherwise
     * @note When isBrokerCall is true, content URIs are automatically marked as authorized
     */
    bool GrantUriPermissionInner(const std::vector<std::string> &uriVec, const GrantUriPermissionInfo &grantInfo,
        Want &want, bool isBrokerCall);
#endif // SUPPORT_UPMS

    /**
     * @brief Check if the call is from DMS (Distributed Mission Service)
     * @param fromTokenId Token ID of the caller
     * @return true if the call is from DMS, false otherwise
     */
    bool IsDmsCall(uint32_t fromTokenId);

    /**
     * @brief Check if the application is a sandbox application
     * @param tokenId Token ID of the application
     * @return true if the application is a sandbox app, false otherwise
     */
    bool IsSandboxApp(uint32_t tokenId);

    /**
     * @brief Get URI list from Want for DMS
     * @param want The Want object containing URI information
     * @return Vector of Uri objects
     */
    std::vector<Uri> GetUriListFromWantDms(Want &want);

    /**
     * @brief Publish file open event
     * @param want The Want object containing file information
     * @param grantInfo The param for grant uri permission
     */
    void PublishFileOpenEvent(const Want &want, const GrantUriPermissionInfo &grantInfo);

    /**
     * @brief Check if bundle is in Anco app identifier
     * @param bundleName Bundle name to check
     * @return true if the bundle is in Anco app identifier, false otherwise
     */
    bool IsInAncoAppIdentifier(const std::string &bundleName);

    /**
     * @brief Check if identifier matches bundle name in Anco context
     * @param identifier Identifier to check
     * @param bundleName Bundle name to match
     * @return true if identifier matches bundle name, false otherwise
     */
    bool CheckIsInAncoAppIdentifier(const std::string &identifier, const std::string &bundleName);

    /**
     * @brief Process UDMF key in Want object
     * @param want The Want object to process
     */
    void ProcessUDMFKey(Want &want);

    /**
     * @brief Process URI in Want object
     * @param checkResult Result of permission check
     * @param apiVersion API version of the caller
     * @param want The Want object to process
     * @param permissionedUris Output vector for processed URIs
     * @return true if processing was successful, false otherwise
     */
    bool ProcessWantUri(bool checkResult, int32_t apiVersion, Want &want, std::vector<Uri> &permissionedUris);

    /**
     * @brief Get caller name and API version
     * @param tokenId Token ID of the caller
     * @param callerName Output parameter for caller name
     * @param apiVersion Output parameter for API version
     * @return true if information was successfully retrieved, false otherwise
     */
    bool GetCallerNameAndApiVersion(uint32_t tokenId, std::string &callerName, int32_t &apiVersion);

    /**
     * @brief Send URI permission grant event
     * @param callerBundleName Bundle name of the caller
     * @param targetBundleName Bundle name of the target
     * @param oriUri Original URI string
     * @param apiVersion API version of the caller
     * @param eventType Type of the event
     * @return true if event was successfully sent, false otherwise
     */
    bool SendGrantUriPermissionEvent(const std::string &callerBundleName, const std::string &targetBundleName,
        const std::string &oriUri, int32_t apiVersion, const std::string &eventType);

    /**
     * @brief Notify collaborator grant uri permission started.
     * @param isNotifyCollaborator Need notify collaborator.
     * @param uris The uri list to grant permission.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param userId The user id of target application.
     * @return result of notify collaborator.
     */
    bool NotifyGrantUriPermissionStart(bool isNotifyCollaborator, const std::vector<std::string> &uris,
        uint32_t flag, int32_t userId);

    /**
     * @brief Notify collaborator grant uri Permission finished.
     * @param isNotifyCollaborator Need notify collaborator.
     * @param uris The uri list to grant permission.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param userId The user id of target application.
     * @param checkResults The result of check uri permission.
     * @return result of notify collaborator.
     */
    bool NotifyGrantUriPermissionEnd(bool isNotifyCollaborator, const std::vector<std::string> &uris,
        uint32_t flag, int32_t userId, const std::vector<bool> &checkResults);

    /**
     * @brief Mark content URIs as authorized for broker call
     * @param uriVec Vector of URI strings
     * @param checkResults Check results to update
     * @param isBrokerCall Whether the call is from broker
     */
    void MarkContentUriAuthorizedForBroker(const std::vector<std::string> &uriVec,
        std::vector<CheckResult> &checkResults, bool isBrokerCall);

    DISALLOW_COPY_AND_MOVE(UriUtils);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_URI_UTILS_H