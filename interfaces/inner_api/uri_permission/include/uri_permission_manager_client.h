/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H

#include <functional>
#include <sstream>

#include "check_result.h"
#include "iuri_permission_manager.h"
#include "uri.h"
#include "uri_permission_raw_data.h"
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
#include "policy_info.h"
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER

namespace OHOS {
namespace AAFwk {
using ProxyClearProxyCallback = std::function<void()>;
constexpr int32_t DEFAULT_HIDE_SENSITIVE_TYPE = 4;
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
using namespace AccessControl::SandboxManager;
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
class UriPermissionManagerClient {
public:
    /**
     * @brief Get the singleton instance of UriPermissionManagerClient.
     *
     * The client is a process-wide singleton. All URI permission operations are dispatched
     * through this instance, which lazily connects to the UriPermissionManager service
     * (SA id 183, running in the foundation process) on first use.
     *
     * @return The singleton reference of UriPermissionManagerClient.
     * @note Thread safe. The underlying proxy is automatically reconnected if the
     *       remote service dies.
     */
    static UriPermissionManagerClient& GetInstance();

    /**
     * @brief Default destructor. Releases the internal service proxy.
     *
     * No explicit cleanup is required by callers. The IPC proxy to the
     * UriPermissionManager service is released automatically when the singleton
     * is destroyed at process exit.
     */
    ~UriPermissionManagerClient() = default;

    /**
     * @brief Grant the read/write permission of a single file URI to a target application.
     *
     * Only for system applications or SA callers; the caller must own the URI (its permission
     * on the URI is verified before granting). Typical scenario: a system app/SA shares a file
     * with another application identified by bundle name.
     *
     * @param uri The file URI to grant. Only "file://" scheme URIs are supported
     *            (file://media/..., file://docs/..., file://<bundleName>/...,
     *            distributed docs URI with ?networkid=...). Content URI is not supported.
     * @param flag Must be Want::FLAG_AUTH_READ_URI_PERMISSION or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION (or both). Granting write also
     *             implies read. Other flag values return ERR_CODE_INVALID_URI_FLAG.
     * @param targetBundleName Bundle name of the application that receives the permission.
     * @param appIndex Index of the target application instance, 0 for the default instance,
     *                 non-zero for the clone (dual-instance) application index.
     * @param initiatorTokenId Token ID of the real initiator. Only effective when the IPC
     *                         caller is a privileged SA (holding
     *                         PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED); otherwise the
     *                         server uses the IPC calling token ID instead.
     * @return Returns ERR_OK on success; returns ERR_NOT_SYSTEM_APP if the caller is not a
     *         system app/SA, ERR_CODE_INVALID_URI_TYPE for unsupported URI,
     *         CHECK_PERMISSION_FAILED if the caller has no permission on the URI,
     *         INNER_ERR on IPC/service errors.
     * @note Temporary authorization: the grant is revoked automatically when the target
     *       application exits (ClearPermissionTokenByMap). Sandbox applications cannot call.
     */
    int GrantUriPermission(const Uri &uri, uint32_t flag, const std::string targetBundleName, int32_t appIndex = 0,
        uint32_t initiatorTokenId = 0);

    /**
     * @brief Grant the read/write permission of a batch of file URIs to a target application.
     *
     * Only for system applications or SA callers. The caller's permission on every URI is
     * verified before granting; if none of the URIs passes the check the call fails.
     * Typical scenario: batch file sharing (e.g. share sheet, file picker multi-select).
     *
     * @param uriVec The file URI list, size must be in range (0, 200000]. Only "file://"
     *               scheme URIs are supported; content URIs are only honored when the
     *               caller is UDMF/pasteboard, otherwise they are invalid.
     * @param flag Must be Want::FLAG_AUTH_READ_URI_PERMISSION or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION (or both). Granting write also
     *             implies read.
     * @param targetBundleName Bundle name of the application that receives the permission.
     * @param appIndex Index of the target application instance, 0 for the default instance,
     *                 non-zero for the clone application index.
     * @param initiatorTokenId Token ID of the real initiator. Only effective when the IPC
     *                         caller is a privileged SA; otherwise the server uses the IPC
     *                         calling token ID.
     * @return Returns ERR_OK if at least one URI is granted successfully; returns
     *         ERR_URI_LIST_OUT_OF_RANGE for empty/oversized list, ERR_NOT_SYSTEM_APP if the
     *         caller is not a system app/SA, ERR_CODE_INVALID_URI_TYPE if all URIs are
     *         invalid, CHECK_PERMISSION_FAILED if the caller has no permission on any URI,
     *         INNER_ERR on IPC/service errors.
     * @note Partial success is possible: the return value is ERR_OK as long as one URI is
     *       granted. Temporary authorization, auto-revoked on target application exit.
     */
    int GrantUriPermission(const std::vector<Uri> &uriVec, uint32_t flag, const std::string targetBundleName,
        int32_t appIndex = 0, uint32_t initiatorTokenId = 0);

    /**
     * @brief Privileged batch grant of URI permission, skipping the caller's own permission
     * check on the URIs. Only for SA/system-app callers holding the
     * PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED permission (system permission, not
     * requestable by normal applications).
     *
     * Typical scenario: a privileged SA grants file access on behalf of an application
     * (e.g. file picker proxy grant). Use CheckUriAuthorization first when the sharing
     * application's permission still needs to be verified.
     *
     * @param uriVec The file URI list, size must be in range (0, 200000]. Supported:
     *               file://media/... (media, requires media library enabled),
     *               file://docs/... and file://<bundleName>/... (sandbox policy),
     *               distributed docs URI, anco:// content URI, and the target app's own
     *               sandbox URI (granted implicitly).
     * @param flag Must be Want::FLAG_AUTH_READ_URI_PERMISSION or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION (or both); may additionally OR
     *             Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION for a persistable sandbox
     *             policy grant. Granting write also implies read.
     * @param targetBundleName Bundle name of the application that receives the permission.
     * @param appIndex Index of the target application instance, 0 for the default instance,
     *                 non-zero for the clone application index.
     * @param initiatorTokenId Token ID of the real initiator (the application on whose
     *                         behalf the grant is made). Only honored when the IPC caller
     *                         is the foundation process; otherwise the IPC calling token
     *                         ID is used.
     * @param hideSensitiveType Hide-sensitive level applied to media URI grants.
     *                          Only effective for foundation callers; ignored (reset to
     *                          the default value 4) for other callers.
     * @return Returns ERR_OK if at least one URI is granted successfully; returns
     *         CHECK_PERMISSION_FAILED without the privileged permission,
     *         ERR_URI_LIST_OUT_OF_RANGE for empty/oversized list,
     *         ERR_CODE_INVALID_URI_FLAG for an invalid flag,
     *         ERR_CODE_INVALID_URI_TYPE if all URIs are invalid, INNER_ERR on errors.
     * @note Temporary authorization unless the persistable flag is set; auto-revoked when
     *       the target application exits. Sandbox applications cannot call.
     */
    int32_t GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
        const std::string &targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0,
        int32_t hideSensitiveType = DEFAULT_HIDE_SENSITIVE_TYPE);
    
    /**
     * @brief Privileged batch grant with an explicit policy type per URI. Only for
     * foundation-process callers (calling uid must be the foundation uid); other callers
     * get CHECK_PERMISSION_FAILED.
     *
     * @param uriVec The file URI list, size must be in range (0, 200000] and equal to
     *               permissionTypes.size(). URI type restrictions are the same as
     *               GrantUriPermissionPrivileged.
     * @param flag Must be Want::FLAG_AUTH_READ_URI_PERMISSION or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION (or both); may additionally OR
     *             Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION. Granting write also
     *             implies read.
     * @param targetBundleName Bundle name of the application that receives the permission.
     * @param appIndex Index of the target application instance, 0 for the default instance.
     * @param initiatorTokenId Token ID of the real initiator. Must be non-zero, otherwise
     *                         ERR_UPMS_INVALID_CALLER_TOKENID is returned.
     * @param hideSensitiveType Hide-sensitive level applied to media URI grants.
     * @param permissionTypes Per-URI sandbox policy type (PolicyType) applied to the
     *                        docs/sandbox policy grant; must have the same size as uriVec.
     * @return Returns ERR_OK if at least one URI is granted successfully; returns
     *         CHECK_PERMISSION_FAILED for non-foundation callers,
     *         ERR_URI_LIST_OUT_OF_RANGE for size mismatch or out-of-range list,
     *         ERR_CODE_INVALID_URI_FLAG for an invalid flag,
     *         ERR_UPMS_INVALID_CALLER_TOKENID if initiatorTokenId is 0,
     *         ERR_CODE_INVALID_URI_TYPE if all URIs are invalid.
     */
    int32_t GrantUriPermissionWithType(const std::vector<Uri> &uriVec, uint32_t flag,
        const std::string &targetBundleName, int32_t appIndex, uint32_t initiatorTokenId,
        int32_t hideSensitiveType, const std::vector<int32_t> &permissionTypes);

    /**
     * @brief Grant URI permission identified by token IDs. Only for SA callers holding the
     * PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED permission.
     *
     * Unlike the bundle-name based overloads, the target and the original caller are
     * identified directly by token ID; the caller's permission on the URI is still verified
     * against oriCallerTokenId before granting.
     *
     * @param uriVec The file URI string list, size must be in range (0, 200000]. URI type
     *               restrictions are the same as GrantUriPermissionPrivileged.
     * @param flag Must be Want::FLAG_AUTH_READ_URI_PERMISSION or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION (or both). Granting write also
     *             implies read.
     * @param targetTokenId Token ID of the application that receives the permission.
     *                      Must be non-zero and belong to an existing application.
     * @param oriCallerTokenId Token ID of the original caller (the application that owns
     *                         or shares the URI). Must be non-zero. Only meaningful when
     *                         the IPC caller holds
     *                         PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED.
     * @return Returns ERR_OK if at least one URI is granted successfully; returns
     *         CHECK_PERMISSION_FAILED without the privileged permission,
     *         ERR_UPMS_INVALID_CALLER_TOKENID / ERR_UPMS_INVALID_TARGET_TOKENID for
     *         invalid (zero, non-existent or cross-user) token IDs,
     *         ERR_URI_LIST_OUT_OF_RANGE for empty/oversized list,
     *         ERR_CODE_INVALID_URI_TYPE if all URIs are invalid, INNER_ERR on errors.
     * @note Cross-user grant is only allowed when either side is a native SA or runs in
     *       user 0. Temporary authorization, auto-revoked on target application exit.
     */
    int32_t GrantUriPermission(const std::vector<std::string>& uriVec, uint32_t flag,
        uint32_t targetTokenId, uint32_t oriCallerTokenId = 0);

    /**
     * @brief Revoke all URI permissions (both granted-by and granted-to records, including
     * content URIs) related to a token ID. Only for foundation-process callers; other
     * callers get CHECK_PERMISSION_FAILED.
     *
     * Typical scenario: called by appmgr/foundation when an application is terminated, to
     * clean up its temporary URI authorization records.
     *
     * @param tokenId Token ID of the application whose URI permissions are revoked.
     * @return Returns ERR_OK on success (including the case where no record exists);
     *         returns CHECK_PERMISSION_FAILED for non-foundation callers, INNER_ERR on
     *         IPC/service errors.
     * @note This clears the UPMS in-memory cache and unset sandbox policies; it does not
     *       fail if there is no record.
     */
    int RevokeAllUriPermissions(const uint32_t tokenId);

    /**
     * @brief Manually revoke the URI permission previously granted to a target application.
     *
     * Only the grantor (the caller that made the grant), the URI owner, or the target
     * itself can revoke successfully; other callers get a silent ERR_OK without effect.
     * Typical scenario: a system application (e.g. file manager) revokes a share it made.
     *
     * @param uri The file URI whose permission is revoked. Only file:// and anco://
     *            schemes are supported; invalid scheme returns ERR_CODE_INVALID_URI_TYPE.
     *            Distributed docs URIs, media URIs and docs/sandbox URIs are routed to
     *            their respective revoke paths.
     * @param bundleName Bundle name of the application whose permission is revoked.
     * @param appIndex Index of the target application instance, 0 for the default instance.
     * @return Returns ERR_OK on success or when no matching grant record exists;
     *         returns CHECK_PERMISSION_FAILED if the caller is neither a system app nor
     *         the pasteboard broker, ERR_CODE_INVALID_URI_TYPE for an unsupported URI,
     *         or a bundle-manager error if bundleName cannot be resolved.
     * @note Permission required: caller must be a system application (or the pasteboard
     *       broker uid).
     */
    int RevokeUriPermissionManually(const Uri &uri, const std::string bundleName, int32_t appIndex = 0);

    /**
     * @brief Verify whether a token ID holds the read/write permission on a URI, including
     * temporary and persistable permission records. Only for the distributed file system
     * (DFS) service caller (calling uid must be the DFS uid); other callers always get
     * false.
     *
     * @param uri The file URI to verify. Must be file:// scheme; media URIs are not
     *            supported (returns false). Distributed docs URIs are verified against
     *            the temporary grant map (including sub-directory URIs); other file URIs
     *            are verified against sandbox policies.
     * @param flag Must be Want::FLAG_AUTH_READ_URI_PERMISSION and/or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION; other bits are ignored.
     * @param tokenId Token ID of the application being verified.
     * @return Returns true if the token ID has the permission; returns false if not, if
     *         the caller is not DFS, or on any IPC/service error.
     */
    bool VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId);

    /**
     * @brief Batch check whether a token ID has the permission to share each URI.
     * Only for privileged SA callers (holding
     * PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED or being the collaboration-framework
     * uid); other callers get an all-false result.
     *
     * Typical scenario: an SA verifies that the sharing application owns the URIs before
     * calling GrantUriPermission/GrantUriPermissionPrivileged on its behalf.
     *
     * @param uriVec The file URI string list, size must be in range (0, 200000]. File
     *               (media/docs/sandbox/distributed docs) and anco content URIs are
     *               supported; "content://" scheme URIs are not supported.
     * @param flag Must be Want::FLAG_AUTH_READ_URI_PERMISSION and/or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION; an invalid flag yields an
     *             all-false result without error.
     * @param tokenId Token ID of the application whose sharing permission is checked.
     * @return Returns a bool vector of the same size and order as uriVec; true means the
     *         token ID may share the corresponding URI. On any error (empty/oversized
     *         list, service unavailable) an all-false vector of uriVec.size() is returned.
     */
    std::vector<bool> CheckUriAuthorization(const std::vector<std::string> &uriVec, uint32_t flag, uint32_t tokenId);

    /**
     * @brief Batch check whether a token ID has the permission to share each URI, with a
     * detailed per-URI result. Only for foundation-process callers; other callers get a
     * default-constructed (invalid) result vector.
     *
     * @param uriVec The file URI string list, size must be in range (0, 200000]. File
     *               (media/docs/sandbox/distributed docs) and anco content URIs are
     *               supported; "content://" scheme URIs are not supported.
     * @param flag Must contain Want::FLAG_AUTH_READ_URI_PERMISSION and/or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION; an invalid flag returns
     *             ERR_CODE_INVALID_URI_FLAG and a default result vector.
     * @param tokenId Token ID of the application being checked; must be non-zero,
     *                otherwise ERR_UPMS_INVALID_CALLER_TOKENID is returned.
     * @return Returns a CheckResult vector of the same size and order as uriVec, each
     *         element describing the check outcome and reason for the corresponding URI.
     *         On error a default-constructed vector of uriVec.size() is returned.
     */
    std::vector<CheckResult> CheckUriAuthorizationWithType(const std::vector<std::string> &uriVec,
        uint32_t flag, uint32_t tokenId);

    /**
     * @brief Clear the temporary URI permission records of a token ID when the application
     * exits. Only for foundation-process callers; other callers get
     * ERR_PERMISSION_DENIED.
     *
     * Typical scenario: appmgr notifies UPMS (through foundation) that an application
     * process died, so its temporary file-URI grants, sandbox policies and content-URI
     * grants are revoked.
     *
     * @param tokenId Token ID of the exited application.
     * @return Returns ERR_OK on success (including the case where no record exists);
     *         returns ERR_UPMS_SERVICE_NOT_START if the URI permission service has not
     *         been started, ERR_PERMISSION_DENIED for non-foundation callers, INNER_ERR
     *         on IPC/service errors.
     * @note Must be called after SetUriPermServiceStarted(); earlier calls fail fast
     *       with ERR_UPMS_SERVICE_NOT_START.
     */
    int32_t ClearPermissionTokenByMap(uint32_t tokenId);

    /**
     * @brief Resolve a UDMF uniform data key to its file URI list and grant the URIs to a
     * target token ID. The IPC caller itself is treated as the URI sharer and must hold
     * the permission on every URI in the record.
     *
     * Requires the UDMF feature (ABILITY_RUNTIME_UDMF_ENABLE); otherwise returns
     * ERR_CAPABILITY_NOT_SUPPORT.
     *
     * @param key The UDMF uniform data key (e.g. "odmf://..."), whose record must contain
     *            a file URI list resolvable by UDMF.
     * @param flag Must be Want::FLAG_AUTH_READ_URI_PERMISSION and/or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetTokenId Token ID of the grantee. Must be non-zero, different from the
     *                      caller, and in the same user as the caller (cross-user grant
     *                      is rejected).
     * @return Returns ERR_OK on success; returns ERR_NOT_SYSTEM_APP if the caller is not
     *         a system app, ERR_CODE_INVALID_URI_FLAG for an invalid flag,
     *         ERR_UPMS_INVALID_TARGET_TOKENID if target equals caller or is
     *         invalid/cross-user, ERR_UPMS_GET_FILE_URIS_BY_KEY_FAILED if the key cannot
     *         be resolved, ERR_UPMS_NO_PERMISSION_GRANT_URI if the caller lacks
     *         permission on any URI, ERR_CAPABILITY_NOT_SUPPORT when UDMF is disabled.
     * @note Permission/caller restriction: system application callers only; sandbox
     *       (clone) apps are rejected.
     */
    int32_t GrantUriPermissionByKey(const std::string &key, uint32_t flag, uint32_t targetTokenId);

    /**
     * @brief Resolve a UDMF uniform data key and grant the URIs to a target token ID on
     * behalf of a specified caller (e.g. the pasteboard service granting on behalf of the
     * application that copied the data).
     *
     * Requires the UDMF feature; otherwise returns ERR_CAPABILITY_NOT_SUPPORT.
     *
     * @param key The UDMF uniform data key whose record contains the file URI list.
     * @param flag Must be Want::FLAG_AUTH_READ_URI_PERMISSION and/or
     *             Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param callerTokenId Token ID of the on-behalf sharer whose permission on the URIs
     *                      is verified. Must be non-zero, different from targetTokenId,
     *                      and in the same user as targetTokenId.
     * @param targetTokenId Token ID of the grantee, same restrictions as callerTokenId.
     * @return Returns ERR_OK on success; error codes are the same as
     *         GrantUriPermissionByKey, plus CHECK_PERMISSION_FAILED if the IPC caller is
     *         a system app without PERMISSION_GRANT_URI_PERMISSION_AS_CALLER.
     * @note Permission required: the IPC caller must be a system application holding
     *       PERMISSION_GRANT_URI_PERMISSION_AS_CALLER (system permission); sandbox apps
     *       are rejected.
     */
    int32_t GrantUriPermissionByKeyAsCaller(const std::string &key, uint32_t flag, uint32_t callerTokenId,
        uint32_t targetTokenId);

    /**
     * @brief Mark that the UriPermissionManager service has been started.
     *
     * Once set, ClearPermissionTokenByMap is allowed to send IPC to the service. Internal
     * lifecycle helper: only the component that observes the service startup (load
     * callback) should call this.
     */
    void SetUriPermServiceStarted();

    /**
     * @brief Check whether the UriPermissionManager service has been marked as started.
     *
     * @return Returns true if SetUriPermServiceStarted has been called; false otherwise.
     */
    bool IsUriPermServiceStarted();

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    /**
     * @brief Activate persistent file access policies for the calling application, mapping
     * persisted URI policies into active (start-accessing) state. Only available when the
     * sandbox manager feature (ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER) is enabled.
     *
     * @param policy The policy list to activate, size must be in range (0, 200000]. Each
     *               PolicyInfo contains a file path and an access mode.
     * @param result Output parameter, per-policy result codes returned by the sandbox
     *               manager, same size and order as policy.
     * @return Returns ERR_OK if the activation request is dispatched successfully;
     *         returns 1 (sandbox manager permission denied) if the caller does not hold
     *         PERMISSION_FILE_ACCESS_PERSIST, ERR_URI_LIST_OUT_OF_RANGE for an
     *         empty/oversized policy list, INNER_ERR on IPC/service errors.
     * @note Permission required: the caller must hold PERMISSION_FILE_ACCESS_PERSIST.
     *       ERR_OK only means the request was processed; inspect the per-policy result
     *       vector for individual outcomes.
     */
    int32_t Active(const std::vector<PolicyInfo> &policy, std::vector<uint32_t> &result);
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER

    /**
     * @brief Callback invoked when the UriPermissionManager system ability is loaded
     * successfully. Stores the remote object as the service proxy.
     *
     * Intended to be invoked from a SystemAbilityLoadCallback; do not call directly.
     *
     * @param remoteObject The remote object of the loaded UriPermissionManager service.
     */
    void OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);

    /**
     * @brief Callback invoked when loading the UriPermissionManager system ability fails.
     * Resets the cached service proxy to nullptr so that a later call retries the load.
     *
     * Intended to be invoked from a SystemAbilityLoadCallback; do not call directly.
     */
    void OnLoadSystemAbilityFail();
private:
    UriPermissionManagerClient() = default;
    sptr<IUriPermissionManager> ConnectUriPermService();
    void ClearProxy();
    bool LoadUriPermService();
    void SetUriPermMgr(const sptr<IRemoteObject> &remoteObject);
    sptr<IUriPermissionManager> GetUriPermMgr();
    DISALLOW_COPY_AND_MOVE(UriPermissionManagerClient);
    bool RawDataToBoolVec(const UriPermissionRawData& rawData, std::vector<bool>& boolVec);
    void StringVecToRawData(const std::vector<std::string>& stringVec, UriPermissionRawData& rawData);
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    void PolicyInfoToRawData(const std::vector<PolicyInfo> &policy, UriPermissionRawData &policyRawData);
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    class UpmsDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit UpmsDeathRecipient(const ProxyClearProxyCallback& proxy) : proxy_(proxy) {}
        ~UpmsDeathRecipient() = default;
        virtual void OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote) override;

    private:
        ProxyClearProxyCallback proxy_;
    };

private:
    std::mutex mutex_;
    sptr<IUriPermissionManager> uriPermMgr_ = nullptr;
    std::atomic_bool isUriPermServiceStarted_ = false;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
