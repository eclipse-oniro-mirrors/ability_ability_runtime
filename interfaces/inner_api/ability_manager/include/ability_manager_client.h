/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H

#include <mutex>

#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "ability_scheduler_interface.h"
#include "auto_startup_info.h"
#include "caller_info.h"
#include "iremote_object.h"
#include "mission_info.h"
#include "system_memory_attr.h"
#include "ui_extension_window_command.h"
#include "want.h"
#include "intent_exemption_info.h"
#include "ihidden_start_observer.h"

namespace OHOS {
namespace AAFwk {
class Snapshot;
class ISnapshotHandler;
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
/**
 * @class AbilityManagerClient
 * AbilityManagerClient is used to access ability manager services.
 */
class AbilityManagerClient {
public:
    virtual ~AbilityManagerClient();
    /**
     * Get the singleton AbilityManagerClient instance.
     * The client is a process-wide singleton; the IPC proxy to the ability manager service is
     * created lazily on first use and is NOT thread-safe against service death. If AMS dies,
     * the death recipient resets the proxy and the next call reconnects automatically.
     *
     * @return The singleton client instance, never nullptr.
     */
    static std::shared_ptr<AbilityManagerClient> GetInstance();

    /**
     * Remove the death recipient registered on the ability manager service proxy and release the
     * cached proxy. Internal use: normally invoked by the destructor; external callers rarely need it.
     */
    void RemoveDeathRecipient();

    /**
     * Start a UIAbility of the caller's own application (NDK child-process scenario), without a
     * caller token. Supported only on products where IsStartOptionsWithAnimation() is true
     * (typically 2-in-1 devices). The want MUST set bundleName/abilityName explicitly (implicit
     * start returns ERR_NOT_ALLOW_IMPLICIT_START), the target MUST be a PAGE (UIAbility) type,
     * and the target bundle MUST belong to the caller process's bundle list. The caller process
     * must be in foreground, otherwise NOT_TOP_ABILITY is returned. The app clone index /
     * instance key parameters in the want are overwritten from the caller process's app mode.
     *
     * @param want The want of the ability to start; must contain explicit element (bundle + ability).
     * @return ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT if the product does not support it;
     *         ERR_NOT_ALLOW_IMPLICIT_START if bundleName empty; ERR_START_OTHER_APP_FAILED if the
     *         target belongs to another app; TARGET_BUNDLE_NOT_EXIST if bundle/ability missing;
     *         RESOLVE_CALL_ABILITY_TYPE_ERR if target is not UIAbility; CHECK_PERMISSION_FAILED if
     *         the target token fails VerifyStartSelfUIAbility; NOT_TOP_ABILITY if caller background;
     *         INNER_ERR if the caller pid is not a registered app child process or start is forbidden.
     */
    ErrCode StartSelfUIAbility(const Want &want);

    /**
     * Start a UIAbility of the caller's own application through ApplicationContext, reusing the
     * delayed-process-exit window: the ability is started in the current process which is about
     * to exit (delayed exit). Only supported when the product enables delayed process exit
     * (IsSupportDelayedProcessExit). The want MUST NOT target a remote device, a plugin, or use
     * implicit start — each is rejected with a dedicated START_UI_ABILITIES_NOT_SUPPORT_* error.
     *
     * @param want The want of the ability to start; explicit element required.
     * @return ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT when delayed process exit is off;
     *         START_UI_ABILITIES_NOT_SUPPORT_OPERATE_REMOTE / _START_PLUGIN / _IMPLICIT_START for
     *         the respective unsupported want forms; INNER_ERR when start is forbidden.
     */
    ErrCode StartSelfUIAbilityByAppContext(const Want &want);

    /**
     * Start a UIAbility of the caller's own application with StartOptions (NDK scenario).
     * Same restrictions as StartSelfUIAbility(want): explicit element only, own bundle only,
     * PAGE type only, foreground caller only, product-form gated. If options.processOptions is
     * provided the server marks it isStartFromNDK = true.
     *
     * @param want The want of the ability to start; explicit element required.
     * @param options StartOptions for the launch; window/process options supported on this path.
     * @return ERR_OK on success; error codes identical to StartSelfUIAbility(want).
     */
    ErrCode StartSelfUIAbilityWithStartOptions(const Want &want, const StartOptions &options);

    /**
     * Start a UIAbility of the caller's own application and arrange for the target process ID to
     * be reported asynchronously. Supported only on 2-in-1 style products (same gate as
     * StartSelfUIAbilityWithStartOptions). The callbackId identifies a callback previously
     * registered by the caller (NDK layer); the server stores it in the process options
     * (loadAbilityCallbackId) together with the calling pid, and the pid is delivered via the
     * load-ability callback when the process is spawned.
     *
     * @param want The want of the ability to start; explicit element required.
     * @param options StartOptions; a processOptions is created if not provided.
     * @param callbackId The id of the callback registered to receive the target process id.
     * @return ERR_OK on success, others on failure (same set as StartSelfUIAbilityWithStartOptions).
     */
    ErrCode StartSelfUIAbilityWithPidResult(const Want &want, StartOptions &options, uint64_t callbackId);

    /**
     * Start a UIAbility of the caller's own application, with an explicit caller ability token.
     * Foreground verification is performed against the process that owns callerToken (if it maps
     * to a valid pid), otherwise against the calling pid. Other restrictions identical to
     * StartSelfUIAbility(want).
     *
     * @param want The want of the ability to start; explicit element required.
     * @param callerToken Token of an ability in the caller's application; may be nullptr.
     * @return ERR_OK on success; NOT_TOP_ABILITY when the referenced process is background;
     *         other codes as StartSelfUIAbility(want).
     */
    ErrCode StartSelfUIAbilityWithToken(const Want &want, sptr<IRemoteObject> callerToken);

    /**
     * Start a UIAbility of the caller's own application with both StartOptions and a caller
     * ability token. Combines the semantics of StartSelfUIAbilityWithStartOptions (product gate,
     * explicit element, own bundle, PAGE type) and StartSelfUIAbilityWithToken (foreground check
     * by callerToken's process). options.processOptions is marked isStartFromNDK when present.
     *
     * @param want The want of the ability to start; explicit element required.
     * @param options StartOptions for the launch.
     * @param callerToken Token of an ability in the caller's application; may be nullptr.
     * @return ERR_OK on success, others on failure (union of the two variants above).
     */
    ErrCode StartSelfUIAbilityWithStartOptionsAndToken(const Want &want,
        const StartOptions &options, sptr<IRemoteObject> callerToken);

    /**
     * Attach the ability thread to the ability manager service. Internal use only: called by the
     * ability runtime (_ability) after the ability process finishes loading, to register the
     * IAbilityScheduler IPC channel for subsequent lifecycle dispatch. The caller process must
     * own the ability record referenced by token (self-call check). Do NOT call from SAs.
     *
     * @param scheduler The scheduler interface handle of the loaded ability. Must not be nullptr.
     * @param token The token of the ability being attached.
     * @return ERR_OK on success; ERR_INVALID_VALUE for null scheduler/token or unknown record;
     *         CHECK_PERMISSION_FAILED when the caller does not own the record.
     */
    ErrCode AttachAbilityThread(sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token);

    /**
     * Notify AMS that an ability finished a lifecycle transition (e.g. INACTIVE/FOREGROUND/
     * BACKGROUND/ACTIVE). Internal use only: invoked by the ability runtime after each lifecycle
     * callback returns. saveData carries the UI state saved on background for later recovery.
     * The caller process must own the ability record.
     *
     * @param token The token of the ability whose transition completed.
     * @param state The lifecycle state that was reached (AbilityState value).
     * @param saveData UI state data saved by the ability (used with app recovery).
     * @return ERR_OK on success; ERR_INVALID_VALUE if the token is unknown; CHECK_PERMISSION_FAILED
     *         when the caller does not own the record.
     */
    ErrCode AbilityTransitionDone(sptr<IRemoteObject> token, int state, const PacMap &saveData);

    /**
     * Notify AMS that an ability finished a transition and report the resulting window
     * configuration. Internal use only: invoked by the ability runtime for window-config
     * related transitions.
     *
     * @param token The token of the ability whose transition completed.
     * @param windowConfig The window configuration after the transition.
     * @return ERR_OK on success; ERR_INVALID_VALUE if the token is unknown or invalid.
     */
    ErrCode AbilityWindowConfigTransitionDone(sptr<IRemoteObject> token, const WindowConfig &windowConfig);

    /**
     * Notify AMS that a ServiceExtension finished handling onConnect. Internal use only: the
     * service side runtime calls this after onConnect returns, forwarding the callee stub
     * (remoteObject) that will be handed to the caller via IAbilityConnection::OnAbilityConnectDone.
     *
     * @param token The token of the service extension that connected.
     * @param remoteObject The callee-side IPC stub returned by onConnect.
     * @return ERR_OK on success; ERR_INVALID_VALUE if the token is unknown or record not found.
     */
    ErrCode ScheduleConnectAbilityDone(sptr<IRemoteObject> token, sptr<IRemoteObject> remoteObject);

    /**
     * Notify AMS that a ServiceExtension finished handling onDisconnect. Internal use only.
     *
     * @param token The token of the service extension that disconnected.
     * @return ERR_OK on success; ERR_INVALID_VALUE if the token is unknown.
     */
    ErrCode ScheduleDisconnectAbilityDone(sptr<IRemoteObject> token);

    /**
     * Notify AMS that a ServiceExtension finished handling onCommand. Internal use only.
     *
     * @param token The token of the service extension that was commanded.
     * @return ERR_OK on success; ERR_INVALID_VALUE if the token is unknown.
     */
    ErrCode ScheduleCommandAbilityDone(sptr<IRemoteObject> token);

    /**
     * Notify AMS that an ability finished handling a combined window + ability command issued by
     * SCB (window show/hide paired with ability foreground/background). Internal use only:
     * reserved for the SCB-driven UIAbility lifecycle path.
     *
     * @param token The token of the target ability.
     * @param sessionInfo The session info of the target window session.
     * @param winCmd The window command (e.g. SHOW/W_HIDE) that completed.
     * @param abilityCmd The ability command (e.g. FOREGROUND/BACKGROUND) that completed.
     * @return ERR_OK on success; ERR_INVALID_VALUE if the token is unknown.
     */
    ErrCode ScheduleCommandAbilityWindowDone(
        sptr<IRemoteObject> token,
        sptr<SessionInfo> sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd);

    /**
     * Get the ElementName of the current foreground (focus) ability. System-api gate: only
     * callers allowed to use system APIs or test processes get a real result; others receive an
     * empty ElementName. When SceneBoard is enabled the result comes from the focus session,
     * otherwise from the mission stack.
     *
     * @param isNeedLocalDeviceId If true, fill an empty deviceId field with the local device id.
     * @return The ElementName of the focus ability, or an empty ElementName on failure/no focus.
     */
    AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId = true);

    /**
     * Get the ElementName of the ability identified by token. No permission check — returns an
     * empty ElementName when the token is null or unknown (requires graphics support compiled in).
     *
     * @param token The ability token to query.
     * @param isNeedLocalDeviceId If true, fill an empty deviceId field with the local device id.
     * @return The ElementName of the ability, or an empty ElementName if not found.
     */
    AppExecFwk::ElementName GetElementNameByToken(sptr<IRemoteObject> token, bool isNeedLocalDeviceId = true);

    /**
     * Start an ability without a caller token. Intended for SA / shell callers (aa tool) that
     * have no ability record; application processes should use the callerToken overloads.
     * Notes and restrictions (server enforced):
     * - specifiedFullTokenId is only honored when the caller uid is DMS; otherwise it is IGNORED
     *   (reset to 0) — do not rely on it from other callers.
     * - Debug-related want params (flag "backgroundDebug", window rect params ohos.param.window.*
     *   etc.) require developer mode (ERR_NOT_DEVELOPER_MODE) or the local-debug permission
     *   (CHECK_PERMISSION_FAILED); screen must be unlocked (ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE).
     * - Window rect params in the want are rejected on products without animated start options
     *   (ERR_NOT_SUPPORTED_PRODUCT_TYPE).
     * - Starting with the FLAG_ABILITY_CONTINUATION flag returns ERR_INVALID_CONTINUATION_FLAG.
     * - Cross-user start (userId differs from caller's user) or the "start with account" param
     *   requires a system-app caller.
     * - Insight-intent / skill reserved params in the want are stripped by the server.
     *
     * @param want The want describing the target ability (implicit start allowed on this path).
     * @param requestCode Request code propagated to the target; DEFAULT_INVAL_VALUE means none.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param specifiedFullTokenId Full token id to attribute the start to; only effective for DMS.
     * @return ERR_OK on success; CHECK_PERMISSION_FAILED, ERR_NOT_DEVELOPER_MODE,
     *         ERR_NOT_SUPPORTED_PRODUCT_TYPE, ERR_INVALID_CONTINUATION_FLAG, ERR_CROSS_USER and
     *         resolve/start errors (TARGET_BUNDLE_NOT_EXIST, RESOLVE_ABILITY_ERR, ...) on failure.
     */
    ErrCode StartAbility(const Want &want, int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE, uint64_t specifiedFullTokenId = 0);

    /**
     * Start an ability and register an observer that is notified when the target ability reaches
     * the foreground ("start and wait", used by the aa tool's start -w). Shell callers only:
     * non-shell callers are rejected with ERR_PERMISSION_DENIED. The start itself uses the
     * token-less StartAbility path, so all its restrictions apply.
     *
     * @param want The want of the ability to start.
     * @param observer Observer receiving the foreground notification for the started ability.
     * @return ERR_OK on success (start accepted; wait via observer); ERR_PERMISSION_DENIED for
     *         non-shell callers; observer-registration or start errors otherwise.
     */
    ErrCode StartAbilityWithWait(Want &want, sptr<IAbilityStartWithWaitObserver> observer);

    /**
     * Start an ability with an explicit caller ability token (application-process entry point).
     * The caller token supplies caller identity (bundle, pid, uid, appIndex) for visibility and
     * background-start checks. specifiedFullTokenId is only honored for DMS callers (otherwise
     * ignored). Continuation flag is rejected. Insight-intent reserved params are stripped unless
     * the openlink flow set its keep-flag.
     *
     * @param want The want describing the target ability.
     * @param callerToken Token of the calling ability; may be nullptr for SA callers.
     * @param requestCode Request code propagated to the target; DEFAULT_INVAL_VALUE means none.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param specifiedFullTokenId Full token id for attribution; only effective for DMS.
     * @return ERR_OK on success; error set as the token-less overload plus ERR_INVALID_CALLER /
     *         CHECK_PERMISSION_FAILED when the caller token fails verification.
     */
    ErrCode StartAbility(
        const Want &want,
        sptr<IRemoteObject> callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE,
        uint64_t specifiedFullTokenId = 0);

    /**
     * Start an ability triggered by an InsightIntent execution. The callerToken must resolve to
     * an ability record owned by the calling process, the intentId must be known to the intent
     * manager, and — when the want carries a bundleName — it must equal both the bundle registered
     * for the intent and the caller's bundle. Effectively this can only start the caller's own
     * components; cross-bundle usage returns ERR_INSIGHT_INTENT_START_INVALID_COMPONENT.
     *
     * @param want The want of the ability to start (bundleName, when set, must match the intent's).
     * @param callerToken Token of the caller ability; must be valid and owned by the caller.
     * @param intentId Insight intent id previously registered by ExecuteIntent.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success; ERR_INVALID_VALUE for unknown token/intent; CHECK_PERMISSION_FAILED
     *         when not self-called; ERR_INSIGHT_INTENT_START_INVALID_COMPONENT on bundle mismatch.
     */
    ErrCode StartAbilityByInsightIntent(
        const Want &want,
        sptr<IRemoteObject> callerToken,
        uint64_t intentId,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start a UIAbility on behalf of an OpenHarmony-Ecological (OE) extension host. Internal use:
     * the caller is validated by OEExtensionUtils::ValidateCaller (uid/hostPid/host bundle rules);
     * only UIAbility targets are allowed (isUIAbilityOnly). specifiedFlag carries the
     * "specified"-launch-mode key for singleton-instance routing.
     *
     * @param want The want of the ability to start (UIAbility only).
     * @param callerToken Token of the OE extension host ability.
     * @param hostPid Process id of the extension host.
     * @param specifiedFlag Instance key used when the target declares "specified" launch mode.
     * @return ERR_OK on success; validation errors from ValidateCaller; standard start errors.
     */
    ErrCode StartAbilityByOEExt(
        const Want &want,
        sptr<IRemoteObject> callerToken,
        int32_t hostPid,
        const std::string &specifiedFlag);

    /**
     * Start an ability with an AbilityStartSetting carrying native-ABI / debug settings for the
     * target process (e.g. soPath for crash-dump scenarios). System use: the settings are applied
     * when the target process is loaded; standard permission/visibility checks of StartAbility
     * apply.
     *
     * @param want The want of the ability to start.
     * @param abilityStartSetting Process-load settings (native debug path, etc.).
     * @param callerToken Token of the calling ability; may be nullptr for SA callers.
     * @param requestCode Request code propagated to the target; DEFAULT_INVAL_VALUE means none.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success, others on failure (same family as StartAbility).
     */
    ErrCode StartAbility(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        sptr<IRemoteObject> callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start an ability with StartOptions (window mode, display id, animation/process options...).
     * Window-related options (windowMode/displayId/window rect/animation) only take effect on
     * products with animated start-options support; processOptions influence where the ability
     * runs (current process / child process). Standard StartAbility permission/visibility checks
     * apply; continuation flag rejected; InsightIntent params stripped.
     *
     * @param want The want of the ability to start.
     * @param startOptions Options for window display and process placement.
     * @param callerToken Token of the calling ability; may be nullptr for SA callers.
     * @param requestCode Request code propagated to the target; DEFAULT_INVAL_VALUE means none.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success, others on failure (same family as StartAbility).
     */
    ErrCode StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        sptr<IRemoteObject> callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start an ability attributing the call to the ORIGINAL caller identified by
     * asCallerSourceToken (used when a middleman app re-starts on behalf of its caller, e.g.
     * cross-device continuation). System-app callers only. The caller info (uid/pid/tokenId) in
     * the want is rewritten from asCallerSourceToken. Continuation flag is rejected.
     *
     * @param want The want of the ability to start.
     * @param callerToken Token of the current (middleman) caller ability.
     * @param asCallerSourceToken Token of the original caller whose identity is inherited.
     * @param requestCode Request code propagated to the target; DEFAULT_INVAL_VALUE means none.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP for non-system callers;
     *         ERR_INVALID_CONTINUATION_FLAG when the continuation flag is set; start errors.
     */
    ErrCode StartAbilityAsCaller(
            const Want &want,
            sptr<IRemoteObject> callerToken,
            sptr<IRemoteObject> asCallerSourceToken,
            int requestCode = DEFAULT_INVAL_VALUE,
            int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start an ability with StartOptions, attributing the call to the ORIGINAL caller identified
     * by asCallerSourceToken. System-app callers only; caller identity rewritten from the source
     * token; continuation flag rejected. Window/process options as in StartAbility(want, options).
     *
     * @param want The want of the ability to start.
     * @param startOptions Options for window display and process placement.
     * @param callerToken Token of the current (middleman) caller ability.
     * @param asCallerSourceToken Token of the original caller whose identity is inherited.
     * @param requestCode Request code propagated to the target; DEFAULT_INVAL_VALUE means none.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP / ERR_INVALID_CONTINUATION_FLAG / start errors.
     */
    ErrCode StartAbilityAsCaller(
            const Want &want,
            const StartOptions &startOptions,
            sptr<IRemoteObject> callerToken,
            sptr<IRemoteObject> asCallerSourceToken,
            int requestCode = DEFAULT_INVAL_VALUE,
            int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start an ability for result (startAbilityForResult semantics) attributing the call to the
     * original caller, so the RESULT is delivered back to the original caller's pending request.
     * System-app callers only; continuation flag rejected.
     *
     * @param want The want of the ability to start for result.
     * @param callerToken Token of the current caller ability.
     * @param requestCode Request code under which the result is reported.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode StartAbilityForResultAsCaller(
        const Want &want,
        sptr<IRemoteObject> callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start an ability for result with StartOptions, attributing the call to the original caller.
     * System-app callers only; continuation flag rejected; window options as in StartAbility.
     *
     * @param want The want of the ability to start for result.
     * @param startOptions Options for window display and process placement.
     * @param callerToken Token of the current caller ability.
     * @param requestCode Request code under which the result is reported.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode StartAbilityForResultAsCaller(
        const Want &want,
        const StartOptions &startOptions,
        sptr<IRemoteObject> callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start a secondary UIAbility bound to an existing primary window, producing a split-window
     * pair (linkage scene). Capability-gated: requires the product parameter
     * persist.ace.support_linkage_scene (SUPPORT_LINKE_SCENE) and SceneBoard enabled, otherwise
     * ERR_CAPABILITY_NOT_SUPPORT. System-app callers only; callerToken must be a valid ability
     * token (ERR_INVALID_CALLER otherwise).
     *
     * @param primaryWindowId The window id of the primary half of the split pair.
     * @param secondaryWant The want of the secondary ability to start.
     * @param callerToken Token of the calling ability; must be valid.
     * @return ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT on unsupported products;
     *         ERR_NOT_SYSTEM_APP for non-system callers; ERR_INVALID_CALLER for bad token.
     */
    ErrCode StartUIAbilitiesInSplitWindowMode(
        int32_t primaryWindowId,
        const AAFwk::Want &secondaryWant,
        sptr<IRemoteObject> callerToken);

    /**
     * Start 1 to 4 UIAbilities in one atomic request; the abilities are laid out by SCB in one
     * gesture (multi-open). Capability-gated (IsSupportStartAbilities + SceneBoard). wantList
     * size MUST be within [1, 4] — START_UI_ABILITIES_WANT_LIST_SIZE_ERROR otherwise.
     * callerToken must be a valid ability token (ERR_INVALID_CALLER otherwise).
     *
     * @param wantList A list of wants (1–4 entries) describing the abilities to start.
     * @param requestKey The unique key identifying this StartUIAbilities request (echoed by SCB).
     * @param callerToken Token of the calling ability; must be valid.
     * @return ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT when unsupported;
     *         START_UI_ABILITIES_WANT_LIST_SIZE_ERROR for wrong list size; ERR_INVALID_CALLER;
     *         per-want resolve errors abort the whole request.
     */
    ErrCode StartUIAbilities(const std::vector<AAFwk::Want> &wantList,
        const std::string &requestKey, sptr<IRemoteObject> callerToken);

    /**
     * Record the exit reason for all running processes of the given user. Internal use:
     * foundation-process callers only (uid check), intended for account/system scenarios such as
     * user logout. The exit reason must pass IsExitReasonValid (killId/exitMsg filled).
     *
     * @param userId The user whose running apps are recorded.
     * @param exitReason The compatible exit reason (killId + exitMsg).
     * @return ERR_OK on success (no process found is treated as success); CHECK_PERMISSION_FAILED
     *         for non-foundation callers; ERR_INVALID_VALUE for an invalid reason.
     */
    ErrCode RecordAppWithReasonByUserId(int32_t userId, const ExitReasonCompability &exitReason);

    /**
     * Start an ability from a UIExtensionContentSession (the start originates inside a UIExtension
     * component). The server verifies that sessionInfo->callerToken equals the token of the
     * CURRENT focus ability — starting from a non-focused UIExtension host returns NOT_TOP_ABILITY.
     * callerToken and sessionInfo must be non-null. Standard StartAbility checks apply afterwards.
     *
     * @param want The want of the ability to start.
     * @param callerToken Token of the UIExtension host ability.
     * @param sessionInfo The session info of the UIExtensionContentSession issuing the start.
     * @param requestCode Request code propagated to the target; DEFAULT_INVAL_VALUE means none.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success; ERR_INVALID_VALUE for null inputs / no focus token;
     *         NOT_TOP_ABILITY when the session caller is not the focus ability; start errors.
     */
    ErrCode StartAbilityByUIContentSession(
        const Want &want,
        sptr<IRemoteObject> callerToken,
        sptr<AAFwk::SessionInfo> sessionInfo,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start an ability from a UIExtensionContentSession with StartOptions. Same focus-token rule
     * as the options-less overload (sessionInfo->callerToken must equal the current focus ability
     * token, else NOT_TOP_ABILITY); window/process options as in StartAbility(want, options).
     *
     * @param want The want of the ability to start.
     * @param startOptions Options for window display and process placement.
     * @param callerToken Token of the UIExtension host ability.
     * @param sessionInfo The session info of the UIExtensionContentSession issuing the start.
     * @param requestCode Request code propagated to the target; DEFAULT_INVAL_VALUE means none.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success; ERR_INVALID_VALUE / NOT_TOP_ABILITY / start errors.
     */
    ErrCode StartAbilityByUIContentSession(
        const Want &want,
        const StartOptions &startOptions,
        sptr<IRemoteObject> callerToken,
        sptr<AAFwk::SessionInfo> sessionInfo,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start a UIAbility-only request with an explicit caller access-token id (foundation process
     * only — e.g. free-install / collaborator flows). Non-foundation callers get
     * CHECK_PERMISSION_FAILED. The target is restricted to UIAbility (isUIAbilityOnly) and the
     * FLAG_ABILITY_CONTINUATION is rejected.
     *
     * @param want The want of the ability to start (UIAbility target only).
     * @param callerToken Token of the caller ability; may be nullptr.
     * @param specifyTokenId Access token id attributed as the caller (permission checks are done
     *        against this token id).
     * @return ERR_OK on success; CHECK_PERMISSION_FAILED for non-foundation callers;
     *         ERR_INVALID_CONTINUATION_FLAG when the continuation flag is set; start errors.
     */
    ErrCode StartAbilityOnlyUIAbility(
        const Want &want,
        sptr<IRemoteObject> callerToken,
        uint32_t specifyTokenId);

    /**
     * Start a UIAbility and receive an asynchronous result callback. IMPORTANT: the callback is
     * only honored for SA callers (IsSACall); for non-SA callers the callback is silently dropped
     * (set to nullptr) — applications must not rely on it. On synchronous failure the callback
     * OnRequestStartAbilityResult(false) is invoked before returning the error code.
     *
     * @param want The want of the ability to start (UIAbility target only).
     * @param callerToken Token of the calling ability; may be nullptr.
     * @param callback Result callback (SA callers only; ignored otherwise).
     * @return ERR_OK on success, others on failure; callback also reports the outcome.
     */
    ErrCode StartUIAbilityWithCallback(
        const Want &want,
        sptr<IRemoteObject> callerToken,
        sptr<IRequestStartAbilityCallback> callback);

    /**
     * Start an ExtensionAbility (service-like, background). Except for UI_SERVICE / APP_SERVICE
     * types, the caller must be a SYSTEM APP. UI_SERVICE additionally requires the system
     * parameter const.abilityms.enable_uiservice (default false → ERR_CAPABILITY_NOT_SUPPORT).
     * When extensionType is set (not UNSPECIFIED) the resolved target's extension type must
     * equal it, else ERR_WRONG_INTERFACE_CALL. Extension rate limiting applies for non-SA
     * callers. Static-config permission (permissions in module.json5) and per-type call
     * permissions are verified server-side.
     *
     * @param want The want of the extension to start.
     * @param callerToken Token of the calling ability; may be nullptr for SA callers.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param extensionType Restrict the target to this extension type; UNSPECIFIED accepts all.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP for non-system callers; CHECK_PERMISSION_FAILED;
     *         ERR_WRONG_INTERFACE_CALL on type mismatch; ERR_CAPABILITY_NOT_SUPPORT for disabled
     *         UI_SERVICE; ERR_CROSS_USER when multi-user concurrency rejects the call.
     */
    ErrCode StartExtensionAbility(
        const Want &want,
        sptr<IRemoteObject> callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED);

    /**
     * Request a modal UIExtension on top of the CURRENT focus ability. System-app callers only.
     * The server requires that the focus window belongs to a PAGE-type ability whose bundleName
     * equals the "bundleName" string param in the want — the request is actually delivered to the
     * focus ability's CreateModalUIExtension; with no focus/mismatch the call fails. Not
     * supported for content URIs — the want must describe a UIExtension component.
     *
     * @param want The want of the modal UIExtension; must carry the caller "bundleName" param.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP for non-system callers; ERR_INVALID_VALUE when
     *         there is no focus ability or the bundle mismatch; CHECK_PERMISSION_FAILED on
     *         visibility/permission failure.
     */
    ErrCode RequestModalUIExtension(const Want &want);

    /**
     * Request a modal UIExtension in a cross-account scenario: the target runs under accountId.
     * System-app callers only; cross-account permission (VerifyAccountPermission) is verified —
     * CHECK_PERMISSION_FAILED otherwise. The focus-ability/bundleName rule of
     * RequestModalUIExtension applies as well.
     *
     * @param want The want of the modal UIExtension; must carry the caller "bundleName" param.
     * @param accountId The account (user) id under which the extension should run.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED on account permission
     *         failure; ERR_INVALID_VALUE when no focus ability / bundle mismatch.
     */
    ErrCode RequestModalUIExtensionWithAccount(const Want &want, int32_t accountId);

    /**
     * Preload (warm up) a UIExtensionAbility process without showing any window. System-app
     * callers only, and the caller must hold ohos.permission.PRELOAD_UI_EXTENSION_ABILITY —
     * otherwise ERR_PERMISSION_DENIED. hostBundleName is an in/out parameter: pass the caller
     * bundle name; the server may overwrite it while resolving clone indices. The target must be
     * visible (exported) or the call fails. Preloaded extensions are matched by later
     * StartUIExtensionAbility calls with the same element.
     *
     * @param want The want of the UIExtensionAbility to preload (explicit element recommended).
     * @param hostBundleName In/out: the caller (host) application bundle name.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param hostPid Host process id; DEFAULT_INVAL_VALUE means not specified.
     * @param requestCode Request code recorded with the preload; DEFAULT_INVAL_VALUE means none.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; ERR_PERMISSION_DENIED without
     *         ohos.permission.PRELOAD_UI_EXTENSION_ABILITY; visibility/resolve errors otherwise.
     */
    ErrCode PreloadUIExtensionAbility(const Want &want, std::string &hostBundleName,
        int32_t userId = DEFAULT_INVAL_VALUE, int32_t hostPid = DEFAULT_INVAL_VALUE,
        int32_t requestCode = DEFAULT_INVAL_VALUE);

    /**
     * Change the visibility (show/hide) of a UIAbility's window without destroying the ability —
     * used by NDK multi-ability / multi-window scenarios. Capability-gated: requires SceneBoard
     * AND the product process-options capability (IsStartOptionsWithProcessOptions); otherwise
     * ERR_CAPABILITY_NOT_SUPPORT.
     *
     * @param token The token of the target UIAbility.
     * @param isShow true to show the window, false to hide it.
     * @return ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT when unsupported;
     *         ERR_INVALID_VALUE when the manager cannot be resolved.
     */
    ErrCode ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow);

    /**
     * Change a UIAbility's visibility at SCB's request (window hide from the window manager).
     * SceneBoard callers ONLY — other callers get ERR_WRONG_INTERFACE_CALL.
     *
     * @param sessionInfo The session info of the target UIAbility; must not be nullptr.
     * @param isShow true to show, false to hide.
     * @return ERR_OK on success; ERR_WRONG_INTERFACE_CALL when not called by SCB;
     *         ERR_INVALID_VALUE for a null sessionInfo or unresolved manager.
     */
    ErrCode ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow);

    /**
     * Start a UIExtensionAbility from a UIExtension component context (session based). The
     * sessionInfo->callerToken must resolve to an ability record owned by the calling process
     * (ERR_INVALID_CALLER otherwise); DLP access check and account permission are verified.
     * When the want carries the screen-mode embedded key, only embedded pull-up of an ATOMIC
     * SERVICE is allowed (ERR_INVALID_CALLER / ERR_INVALID_VALUE otherwise). Want param
     * "uiExtensionType" must map to a valid extension type when present.
     *
     * @param extensionSessionInfo Session info whose want describes the extension to start.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK on success; ERR_INVALID_VALUE for null info/bad type; ERR_INVALID_CALLER for
     *         invalid caller token; CHECK_PERMISSION_FAILED on DLP/account/visibility failure.
     */
    ErrCode StartUIExtensionAbility(
        sptr<SessionInfo> extensionSessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Start a UIAbility from the SceneBoard session path (window-session driven start).
     * SceneBoard callers ONLY — other callers get ERR_WRONG_INTERFACE_CALL. Supports the
     * free-install flow via the want "sessionId" param (pre-install task resume). params carries
     * SCB-side start parameters (pageConfig, sceneFlag, isRestart...); isColdStart reports
     * whether the start resulted in a cold start. params.isRestart requires the
     * restart-app-with-window product capability.
     *
     * @param sessionInfo Session info of the window session requesting the start; must be valid.
     * @param params SCB start parameters (page config, scene flags, restart flag).
     * @param isColdStart Output: true when the target process was newly created.
     * @return ERR_OK on success; ERR_WRONG_INTERFACE_CALL when not SCB; ERR_INVALID_VALUE for null
     *         session; ERR_CAPABILITY_NOT_SUPPORT when restart-with-window is unsupported;
     *         ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK when blocked by first-boot screen lock.
     */
    ErrCode StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo, AbilityRuntime::StartParamsBySCB &params,
        bool &isColdStart);

    /**
     * Stop an ExtensionAbility. System-app callers only (except when extensionType is
     * APP_SERVICE). If extensionType is set, the resolved target's type must match, else
     * ERR_WRONG_INTERFACE_CALL. The caller token, when provided, must verify (SA DMS callers
     * with the specific SA access permission are exempt). The caller must match the extension's
     * visibility rules; stopping extensions of other apps generally requires system privileges.
     * Cross-user concurrency is enforced (ERR_CROSS_USER).
     *
     * @param want The want identifying the extension to stop.
     * @param callerToken Token of the calling ability; may be nullptr for SA callers.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param extensionType Restrict the target to this extension type; UNSPECIFIED accepts all.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; ERR_INVALID_CALLER; ERR_WRONG_INTERFACE_CALL
     *         on type mismatch; ERR_CROSS_USER; resolve/permission errors.
     */
    ErrCode StopExtensionAbility(
        const Want& want,
        sptr<IRemoteObject> callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED);

    /**
     * Terminate the ability identified by token and deliver a result to the caller that started
     * it for result. Internal use: invoked by the ability runtime on ability termination
     * (UIAbility terminateSelf). The LAUNCHER ability cannot be terminated (silently returns
     * ERR_OK without effect). resultCode/resultWant are propagated to the caller's
     * onAbilityResult.
     *
     * @param token The token of the ability to terminate (must be the caller's own).
     * @param resultCode Result code returned to the starter.
     * @param resultWant Result data returned to the starter; may be nullptr.
     * @return ERR_OK on success (also for launcher, as no-op); ERR_INVALID_VALUE when the token
     *         is unknown.
     */
    ErrCode TerminateAbility(sptr<IRemoteObject> token, int resultCode, const Want *resultWant);

    /**
     * Re-launch ("self start") the UIAbility identified by token — brings its own instance to
     * front per its launch mode without a Want. Requires the native-UIAbility product capability
     * (IsSupportNativeUIAbility), otherwise ERR_CAPABILITY_NOT_SUPPORT. Only the ability's own
     * process may call it (JudgeSelfCalled), and only UIAbility records are supported.
     *
     * @param token The token of the UIAbility to start again.
     * @return ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT when unsupported; CHECK_PERMISSION_FAILED
     *         when not self-called; ERR_INVALID_VALUE for unknown token / non-UIAbility record.
     */
    ErrCode StartSelf(sptr<IRemoteObject> token);

    /**
     * Return the result to the CALLER ability and finish (terminateSelfWithResult for FA/
     * delegated scenarios). The callerRequestCode (int64) identifies the caller's outstanding
     * request. The ability record must exist; the caller must have started this ability for
     * result, otherwise the result is dropped with an error.
     *
     * @param token The token of the ability returning the result.
     * @param resultCode Result code delivered to the caller.
     * @param resultWant Result data delivered to the caller; may be nullptr.
     * @param callerRequestCode The request code under which the caller awaits the result.
     * @return ERR_OK on success; ERR_INVALID_VALUE for unknown token; error when no caller
     *         request matches.
     */
    ErrCode BackToCallerAbilityWithResult(const sptr<IRemoteObject> &token, int resultCode,
        const Want *resultWant, int64_t callerRequestCode);

    /**
     * Terminate a UIServiceExtensionAbility. SceneBoard callers ONLY (ERR_WRONG_INTERFACE_CALL
     * otherwise); the token must be valid and reference an EXTENSION of type UI_SERVICE,
     * otherwise ERR_INVALID_VALUE.
     *
     * @param token The token of the UIServiceExtensionAbility to terminate.
     * @return ERR_OK on success; ERR_WRONG_INTERFACE_CALL when not SCB; ERR_INVALID_VALUE for
     *         invalid token or wrong type.
     */
    ErrCode TerminateUIServiceExtensionAbility(sptr<IRemoteObject> token);

    /**
     * Terminate a UIExtensionAbility from its component session (close the UIExtension window
     * without killing the process). Allowed for the extension itself or its host caller
     * (self-call rules); the target must be a UIExtension type (ERR_WRONG_INTERFACE_CALL for
     * other extension kinds) and must be visible.
     *
     * @param extensionSessionInfo Session info identifying the UIExtensionAbility; must not be null.
     * @param resultCode Result code; DEFAULT_INVAL_VALUE means none.
     * @param resultWant Result data; nullptr means none.
     * @return ERR_OK on success; ERR_INVALID_VALUE for null info/unknown record;
     *         CHECK_PERMISSION_FAILED when neither self nor caller owns it;
     *         ERR_WRONG_INTERFACE_CALL for non-UIExtension targets.
     */
    ErrCode TerminateUIExtensionAbility(sptr<SessionInfo> extensionSessionInfo,
        int resultCode = DEFAULT_INVAL_VALUE, const Want *resultWant = nullptr);

    /**
     * Close a UIExtensionAbility at the SceneBoard's request. SceneBoard callers ONLY — others
     * get ERR_PERMISSION_DENIED. The token must be valid and reference a UIExtension type
     * ability; the session is detached from display before termination.
     *
     * @param token The ability token of the UIExtensionAbility to close.
     * @return ERR_OK on success; ERR_PERMISSION_DENIED when not SCB; ERR_INVALID_VALUE for
     *         invalid token / non-UIExtension / missing session.
     */
    ErrCode CloseUIExtensionAbilityBySCB(const sptr<IRemoteObject> token);

    /**
     * Close a UIAbility at the SceneBoard's request (session terminated from the window side).
     * SceneBoard callers ONLY (ERR_WRONG_INTERFACE_CALL otherwise). When isUserRequestedExit is
     * true an exit reason "User Request" (REASON_USER_REQUEST) is recorded for the ability.
     * sceneFlag carries the SCB scene code of the close (reported in ability events).
     *
     * @param sessionInfo Session info of the UIAbility to close; must be valid.
     * @param isUserRequestedExit true when the close originates from an explicit user action.
     * @param sceneFlag SCB scene flag describing the close reason.
     * @return ERR_OK on success; ERR_WRONG_INTERFACE_CALL when not SCB; ERR_INVALID_VALUE for
     *         null session / unknown record; ERR_WOULD_BLOCK under ability-controller test mode.
     */
    ErrCode CloseUIAbilityBySCB(sptr<SessionInfo> sessionInfo,
        bool isUserRequestedExit = false, uint32_t sceneFlag = 0);

    /**
     * Deliver a startAbilityForResult result to a local ability on behalf of a remote (cross-
     * device) flow. Distributed-scheduler process ONLY — other callers get ERR_INVALID_VALUE.
     * The mission id of the target must be carried in resultWant under the DMS mission-id key
     * (removed after read); the ability referenced by that mission receives
     * onAbilityResult(requestCode, resultCode, resultWant).
     *
     * @param requestCode The request code of the original start-for-result.
     * @param resultCode The result code to deliver.
     * @param resultWant The result want; must contain the DMS mission id param.
     * @return ERR_OK on success; ERR_INVALID_VALUE when not DMS, mission id missing, or the
     *         mission/ability cannot be found.
     */
    ErrCode SendResultToAbility(int requestCode, int resultCode, Want& resultWant);

    /**
     * Move the ability identified by token to background (its window loses focus but the ability
     * stays alive). Self-call only (the caller process must own the record). Blocked with
     * ERR_WOULD_BLOCK when an ability-controller (wukong/test) holds the ability foreground.
     *
     * @param token The token of the ability to move to background.
     * @return ERR_OK on success; ERR_INVALID_VALUE for unknown token; CHECK_PERMISSION_FAILED when
     *         not self-called; ERR_WOULD_BLOCK under ability controller.
     */
    ErrCode MoveAbilityToBackground(sptr<IRemoteObject> token);

    /**
     * Move a UIAbility to background — application self-invoked variant (UIAbility
     * moveAbilityToBackground). Product capability gated (EnableMoveUIAbilityToBackgroundApi),
     * otherwise ERR_OPERATION_NOT_SUPPORTED_ON_CURRENT_DEVICE. Only the ability's own application
     * may call it; the ability MUST currently be foreground and MUST be a PAGE (UIAbility) type.
     *
     * @param token The token of the UIAbility to move to background.
     * @return ERR_OK on success; ERR_OPERATION_NOT_SUPPORTED_ON_CURRENT_DEVICE when unsupported;
     *         CHECK_PERMISSION_FAILED when not app-self called; ERR_ABILITY_NOT_FOREGROUND when
     *         the ability is not foreground; RESOLVE_CALL_ABILITY_TYPE_ERR for non-UIAbility.
     */
    ErrCode MoveUIAbilityToBackground(const sptr<IRemoteObject> token);

    /**
     * Close the ability identified by token, delivering a result to its starter. On SceneBoard
     * devices the request is forwarded to the scene-session manager (TerminateSessionNew) and
     * the close is async; otherwise it goes to AMS directly. resultWant is dereferenced on the
     * SCB path — must not be nullptr there.
     *
     * @param token The token of the ability to close.
     * @param resultCode Result code returned to the starter; DEFAULT_INVAL_VALUE means none.
     * @param resultWant Result data returned to the starter; nullptr means none (non-SCB path).
     * @return ERR_OK on success, others on failure (session errors mapped to mission error codes
     *         on the SCB path).
     */
    ErrCode CloseAbility(sptr<IRemoteObject> token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr);

    /**
     * Minimize the ability identified by token (window minimized to mission). Self-call only.
     * PAGE (UIAbility) abilities only — other types get ERR_WRONG_INTERFACE_CALL. Blocked with
     * ERR_WOULD_BLOCK under ability-controller test mode. fromUser marks whether the minimize
     * originates from a user gesture (affects exit-reason statistics).
     *
     * @param token The token of the ability to minimize.
     * @param fromUser true when caused by an explicit user action.
     * @return ERR_OK on success; ERR_INVALID_VALUE unknown token; CHECK_PERMISSION_FAILED not
     *         self-called; ERR_WRONG_INTERFACE_CALL for non-PAGE; ERR_WOULD_BLOCK under controller.
     */
    ErrCode MinimizeAbility(sptr<IRemoteObject> token, bool fromUser = false);

    /**
     * Minimize (background the window of) a UIExtensionAbility from its component session.
     * The sessionInfo->callerToken must resolve to a record owned by the caller (self-call).
     * Only UIExtension-kind targets are accepted (ERR_WRONG_INTERFACE_CALL otherwise) and the
     * target must be visible.
     *
     * @param extensionSessionInfo Session info identifying the UIExtensionAbility; non-null.
     * @param fromUser true when caused by an explicit user action.
     * @return ERR_OK on success (window backgrounded); ERR_INVALID_VALUE for null info/record;
     *         CHECK_PERMISSION_FAILED when not self-called; ERR_WRONG_INTERFACE_CALL for
     *         non-UIExtension targets.
     */
    ErrCode MinimizeUIExtensionAbility(sptr<SessionInfo> extensionSessionInfo, bool fromUser = false);

    /**
     * Minimize a UIAbility at the SceneBoard's request. SceneBoard callers ONLY
     * (ERR_WRONG_INTERFACE_CALL otherwise). backgroundReason carries the system reason code for
     * moving to background (e.g. 3 for screen-off); sceneFlag carries the SCB scene code. Both
     * are recorded in lifecycle events / exit statistics.
     *
     * @param sessionInfo Session info of the UIAbility to minimize; must be valid.
     * @param fromUser true when caused by an explicit user action.
     * @param sceneFlag SCB scene flag describing the minimize source.
     * @param backgroundReason System reason for backgrounding (e.g. 3: screen off).
     * @return ERR_OK on success; ERR_WRONG_INTERFACE_CALL when not SCB; ERR_INVALID_VALUE for
     *         null session / unknown record; ERR_WOULD_BLOCK under ability controller.
     */
    ErrCode MinimizeUIAbilityBySCB(sptr<SessionInfo> sessionInfo, bool fromUser = false, uint32_t sceneFlag = 0,
        int32_t backgroundReason = 0);

    /**
     * Connect to a ServiceExtensionAbility. SA-oriented overload without caller token.
     * Connection results are reported asynchronously through connect (OnAbilityConnectDone /
     * OnAbilityDisconnectDone). Interceptors (block-all/extension control...) and DLP checks run
     * server-side; rate limiting applies to non-SA callers. For SERVICE-type targets, a cross-
     * user connect requires a system-app caller.
     *
     * @param want The want describing the target service extension.
     * @param connect Callback receiving connect/disconnect notifications; must be non-null.
     * @param userId Target user id.
     * @return ERR_OK when the connect request is accepted (completion via callback);
     *         ERR_INVALID_VALUE for null connect; CHECK_PERMISSION_FAILED / interceptor errors.
     */
    ErrCode ConnectAbility(const Want &want, sptr<IAbilityConnection> connect, int32_t userId);

    /**
     * Connect to a ServiceExtensionAbility with a load timeout multiplier. loadTimeout
     * multiplies the ability load timeout (valid range 1–30; 0 = default timeout); it does not
     * take effect on ASAN builds. Same permission/interceptor rules as ConnectAbility.
     *
     * @param want The want describing the target service extension.
     * @param connect Callback receiving connect/disconnect notifications; must be non-null.
     * @param userId Target user id.
     * @param loadTimeout Load-stage timeout multiplier, range 1–30; 0 means default; ignored on asan.
     * @return ERR_OK when accepted; ERR_INVALID_VALUE / CHECK_PERMISSION_FAILED / interceptor
     *         errors as ConnectAbility.
     */
    ErrCode ConnectAbility(const Want &want, sptr<IAbilityConnection> connect, int32_t userId,
        int32_t loadTimeout);

    /**
     * Connect to a ServiceExtensionAbility with an explicit caller ability token
     * (application-process entry). specifiedFullTokenId is only honored for DMS callers
     * (otherwise silently reset to 0). Caller identity (uid/pid/bundle) is taken from
     * callerToken for permission and visibility checks.
     *
     * @param want The want describing the target service extension.
     * @param connect Callback receiving connect/disconnect notifications; must be non-null.
     * @param callerToken Token of the calling ability; may be nullptr for SA callers.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param specifiedFullTokenId Full token id for attribution; only effective for DMS.
     * @return ERR_OK when accepted; ERR_INVALID_VALUE / CHECK_PERMISSION_FAILED / interceptor
     *         errors.
     */
    ErrCode ConnectAbility(
        const Want &want,
        sptr<IAbilityConnection> connect,
        sptr<IRemoteObject> callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        uint64_t specifiedFullTokenId = 0);

    /**
     * Connect to an extension ability while providing indirect caller information (the real
     * initiator behind the SA). SA callers ONLY when indirectCallerInfo is provided — non-SA
     * callers passing it get CHECK_PERMISSION_FAILED. extensionType, when set, restricts the
     * target kind.
     *
     * @param want The want describing the target extension.
     * @param connect Callback receiving connect/disconnect notifications; must be non-null.
     * @param callerToken Token of the calling ability; may be nullptr.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param extensionType Restrict the target to this extension type; UNSPECIFIED accepts all.
     * @param indirectCallerInfo Real caller info (SA-only feature); nullptr to omit.
     * @return ERR_OK when accepted; CHECK_PERMISSION_FAILED when a non-SA caller passes
     *         indirectCallerInfo; other codes as ConnectAbility.
     */
    ErrCode ConnectAbilityWithIndirectCallerInfo(
        const Want &want,
        sptr<IAbilityConnection> connect,
        sptr<IRemoteObject> callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED,
        std::shared_ptr<IndirectCallerInfo> indirectCallerInfo = nullptr);
    
    /**
     * Connect to an extension ability restricting the target to the given extension type
     * (default SERVICE — designed for APP_SERVICE-style connections). The resolved target type
     * must equal extensionType, otherwise the request fails. Same connection rules as
     * ConnectAbility.
     *
     * @param want The want describing the target extension.
     * @param connect Callback receiving connect/disconnect notifications; must be non-null.
     * @param callerToken Token of the calling ability; may be nullptr.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param extensionType Required target extension type; defaults to SERVICE.
     * @return ERR_OK when accepted; type-mismatch and permission errors as ConnectAbility.
     */
    ErrCode ConnectAbilityWithExtensionType(
        const Want &want,
        sptr<IAbilityConnection> connect,
        sptr<IRemoteObject> callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::SERVICE);

    /**
     * Connect to a UIServiceExtensionAbility (UI service delivered over a session-ish channel).
     * The target is restricted to ExtensionAbilityType::UI_SERVICE; the reserved
     * UISERVICEHOSTPROXY_KEY want param must NOT be set on non-UI_SERVICE connects (it is only
     * meaningful here). Results arrive via connect callbacks.
     *
     * @param want The want describing the target UIServiceExtension.
     * @param connect Callback receiving connect/disconnect notifications; must be non-null.
     * @param callerToken Token of the calling ability; may be nullptr.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK when accepted; ERR_WRONG_INTERFACE_CALL when the host-proxy key is misused;
     *         permission/interceptor errors as ConnectAbility.
     */
    ErrCode ConnectUIServiceExtesnionAbility(
        const Want &want,
        sptr<IAbilityConnection> connect,
        sptr<IRemoteObject> callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Connect to a DataShareExtensionAbility (SA-side data access). Target restricted to
     * ExtensionAbilityType::DATASHARE. The want usually carries the data-share URI; results
     * arrive via connect callbacks.
     *
     * @param want The want (typically with a datashare:// URI) describing the target extension.
     * @param connect Callback receiving connect/disconnect notifications; must be non-null.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @return ERR_OK when accepted; ABILITY_SERVICE_NOT_CONNECTED when AMS is unreachable;
     *         permission/interceptor errors as ConnectAbility.
     */
    ErrCode ConnectDataShareExtensionAbility(const Want &want, sptr<IAbilityConnection> connect,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * Connect to an extension ability of any type (UNSPECIFIED) with an optional load timeout
     * multiplier. loadTimeout range 1–30 (0 = default), not effective on ASAN builds.
     *
     * @param want The want describing the target extension.
     * @param connect Callback receiving connect/disconnect notifications; must be non-null.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param loadTimeout Load-stage timeout multiplier, range 1–30; 0 means default; ignored on asan.
     * @return ERR_OK when accepted; ABILITY_SERVICE_NOT_CONNECTED when AMS is unreachable;
     *         permission/interceptor errors as ConnectAbility.
     */
    ErrCode ConnectExtensionAbility(const Want &want, sptr<IAbilityConnection> connect,
        int32_t userId = DEFAULT_INVAL_VALUE, int32_t loadTimeout = 0);

    /**
     * Connect to a UIExtensionAbility from a host component (session based). sessionInfo
     * identifies the host UIExtension component; connectInfo (optional) carries per-connection
     * data (e.g. from spreads of the component). Results arrive via connect callbacks; the
     * target type must be a UIExtension kind.
     *
     * @param want The want describing the target UIExtensionAbility.
     * @param connect Callback receiving connect/disconnect notifications; must be non-null.
     * @param sessionInfo The session info of the host UIExtension component.
     * @param userId Target user id; DEFAULT_INVAL_VALUE means the caller's current user.
     * @param connectInfo Optional connection info for the UIExtension channel; nullptr to omit.
     * @return ERR_OK when accepted; ABILITY_SERVICE_NOT_CONNECTED when AMS is unreachable;
     *         ERR_INVALID_VALUE / permission errors as ConnectAbility.
     */
    ErrCode ConnectUIExtensionAbility(const Want &want, sptr<IAbilityConnection> connect,
        sptr<SessionInfo> sessionInfo, int32_t userId = DEFAULT_INVAL_VALUE,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr);

    /**
     * Disconnect a previously established extension connection. The connect object must be the
     * same instance used in ConnectAbility; OnAbilityDisconnectDone is triggered when the
     * connection is torn down. Disconnecting an unknown connection returns an error without
     * side effects.
     *
     * @param connect The connection callback object used to connect; must be non-null.
     * @return ERR_OK on success; ERR_INVALID_VALUE for null/unknown connection.
     */
    ErrCode DisconnectAbility(sptr<IAbilityConnection> connect);

    /**
     * Acquire a DataAbility scheduler synchronously by URI, loading the DataAbility process if
     * needed. The URI scheme MUST be "dataability" and the path must be non-empty, otherwise
     * nullptr is returned. Caller-side visibility and static-config permission checks apply
     * (SA callers bypass the caller checks). tryBind semantics: when true, if the acquired
     * DataAbility process dies, AMS kills THIS client as well (to avoid dangling data access);
     * pass false to survive provider death. NOT for content:// URIs — use the DataShare stack
     * for those.
     *
     * @param uri The data ability uri; scheme must be "dataability" with a non-empty path.
     * @param tryBind true: the client is killed when the data ability dies; false: do nothing.
     * @param callerToken Specifies the caller ability token.
     * @return The data ability scheduler proxy, or nullptr on failure (invalid uri, permission
     *         denied, provider not found).
     */
    sptr<IAbilityScheduler> AcquireDataAbility(const Uri &uri, bool tryBind, sptr<IRemoteObject> callerToken);

    /**
     * Release a DataAbility scheduler acquired via AcquireDataAbility, dropping this client's
     * reference. When the last reference is released the provider may be stopped. Both the
     * scheduler and callerToken must be non-null.
     *
     * @param dataAbilityScheduler The scheduler obtained from AcquireDataAbility.
     * @param callerToken The caller ability token used at acquire time.
     * @return ERR_OK on success; ERR_INVALID_VALUE for null scheduler/token or unknown scheduler.
     */
    ErrCode ReleaseDataAbility(sptr<IAbilityScheduler> dataAbilityScheduler, sptr<IRemoteObject> callerToken);

    /**
     * Dump ability stack state (user id, mission stacks, mission records, ability info) for
     * debugging (aa dump / hidumper). The call is one-way on the client: the client returns
     * ERR_OK once the proxy is available; the result arrives in state. Diagnostic use only — no
     * permission contract is offered to normal apps; intended for hidumper/aa tooling.
     *
     * @param args Dump arguments string (e.g. "-a").
     * @param state Output vector receiving the dump lines.
     * @return ERR_OK once dispatched; ABILITY_SERVICE_NOT_CONNECTED when the AMS proxy is
     *         unavailable. The server-side dump result is not reflected here — check state.
     */
    ErrCode DumpState(const std::string &args, std::vector<std::string> &state);

    /**
     * Dump extended system state (mission list, pending wants, processes, screen) with filters.
     * As with DumpState, the client returns ERR_OK once the proxy is available; the payload
     * arrives in state.
     *
     * @param args Dump arguments string.
     * @param state Output vector receiving the dump lines.
     * @param isClient Whether to dump from the client side.
     * @param isUserID Whether the UserID filter applies.
     * @param UserID User id filter when isUserID is true.
     * @return ERR_OK once dispatched; ABILITY_SERVICE_NOT_CONNECTED when the AMS proxy is
     *         unavailable.
     */
    ErrCode DumpSysState(
        const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID);
    /**
     * Explicitly connect to the ability manager system ability (SA id 180) and register a death
     * recipient. Normally NOT needed — every public method auto-connects lazily. Useful to
     * pre-warm the proxy or check service availability.
     *
     * @return ERR_OK on success; GET_ABILITY_SERVICE_FAILED when the registry or service is
     *         unavailable.
     */
    ErrCode Connect();

    /**
     * Destroy a ServiceAbility by want (service self-stop or SA stop). SA and shell callers may
     * omit the token; other callers must provide a token resolving to one of their own ability
     * records (ERR_INVALID_VALUE otherwise). The resolved target must be SERVICE or EXTENSION
     * type (TARGET_ABILITY_NOT_SERVICE otherwise) and visible to the caller. Cross-user
     * concurrency is enforced.
     *
     * @param want The want identifying the service ability to stop.
     * @param token The caller ability token; nullptr allowed for SA/shell callers.
     * @return ERR_OK on success; ERR_INVALID_VALUE for unknown token/ability; CHECK_PERMISSION_FAILED;
     *         TARGET_ABILITY_NOT_SERVICE for wrong type; ERR_CROSS_USER on multi-user rejection.
     */
    ErrCode StopServiceAbility(const Want &want, sptr<IRemoteObject> token = nullptr);

    /**
     * Kill all processes of the application identified by bundleName. Permission: the caller
     * must be shell OR hold ohos.permission.KILL_APP_PROCESSES (or pass the app-side
     * kill-process verification) — otherwise ERR_PERMISSION_DENIED. Keep-alive (persistent)
     * applications are NOT killed when memory is sufficient (KILL_PROCESS_KEEP_ALIVE).
     * Collaborator-managed bundles are routed to their collaborator first. clearPageStack also
     * clears the mission stack records; appIndex selects a clone instance (0 = base app).
     *
     * @param bundleName Bundle name of the application to kill.
     * @param clearPageStack true to also clear mission/page-stack records of the app.
     * @param appIndex Clone application index; 0 means the base application.
     * @param reason Kill reason string recorded in exit statistics.
     * @return ERR_OK on success; ERR_PERMISSION_DENIED without permission;
     *         GET_BUNDLE_INFO_FAILED when the bundle cannot be resolved;
     *         KILL_PROCESS_KEEP_ALIVE for persistent apps; KILL_PROCESS_FAILED on failure.
     */
    ErrCode KillProcess(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0,
        const std::string& reason = "Abilityms::KillProcess");

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Force a lifecycle timeout for the named ability for testing (simulates ANR/timeout paths).
     * Only compiled in when ABILITY_COMMAND_FOR_TEST is defined (test/aa-debug builds).
     *
     * @param abilityName The name of the ability to force timeout.
     * @param state The lifecycle state string to force ("initial"/"foreground"/"background"...).
     * @return ERR_OK on success, others on failure.
     */
    ErrCode ForceTimeoutForTest(const std::string &abilityName, const std::string &state);
    #endif

    /**
     * Continue a mission to another device (mission center cross-device continuation). System-app
     * callers only, and the caller must hold ohos.permission.MANAGE_MISSIONS. srcDeviceId,
     * dstDeviceId and callback must be non-empty/non-null (ERR_INVALID_VALUE otherwise). The
     * request is forwarded to the distributed scheduler; the callback receives the continuation
     * outcome.
     *
     * @param srcDeviceId Origin device id (non-empty).
     * @param dstDeviceId Target device id (non-empty).
     * @param missionId The mission to continue.
     * @param callback Remote callback notified with the result (non-null).
     * @param wantParams Extended parameters forwarded to the remote side.
     * @return ERR_OK on success; ERR_INVALID_VALUE for empty ids/null callback; ERR_NOT_SYSTEM_APP;
     *         CHECK_PERMISSION_FAILED without MANAGE_MISSIONS; DMS errors.
     */
    ErrCode ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId, int32_t missionId,
        sptr<IRemoteObject> callback, AAFwk::WantParams &wantParams);

    /**
     * Continue a mission to another device using a structured ContinueMissionInfo (bundle/ability
     * level continuation). System-app callers only + ohos.permission.MANAGE_MISSIONS.
     * srcDeviceId/dstDeviceId inside the info and the callback must be valid.
     *
     * @param continueMissionInfo Structured continuation info (src/dst device, bundle, params).
     * @param callback Remote callback notified with the result (non-null).
     * @return ERR_OK on success; ERR_INVALID_VALUE for invalid info/callback; ERR_NOT_SYSTEM_APP;
     *         CHECK_PERMISSION_FAILED; DMS errors.
     */
    ErrCode ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo, const sptr<IRemoteObject> &callback);

    /**
     * Start a continuation session for the local ability (application-side entry when the user
     * picks continuation). The want must reference a remote target (deviceId + element present),
     * otherwise ERR_INVALID_VALUE; abilityToken must be non-null. status is the continuation
     * state agreed with the remote side.
     *
     * @param want The want of the remote target to continue to (must carry deviceId/element).
     * @param abilityToken Token of the local ability being continued.
     * @param status Continuation status flag.
     * @return ERR_OK on success; ERR_INVALID_VALUE for a local-only want or null token; DMS errors.
     */
    ErrCode StartContinuation(const Want &want, sptr<IRemoteObject> abilityToken, int32_t status);

    /**
     * Notify DMS that a continuation completed locally. Void IPC; no return value. Internal use
     * by the continuation framework.
     *
     * @param deviceId The source device which started the continuation.
     * @param sessionId The id representing this continuation session.
     * @param isSuccess The continuation result.
     */
    void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess);

    /**
     * Trigger continuation of a local mission at the DMS's request. Distributed-scheduler
     * process ONLY — other callers get ERR_INVALID_VALUE. versionCode is the remote target's
     * application version used for compatibility decisions; userId >= 0 selects the user's
     * manager (default -1 = current).
     *
     * @param deviceId The target device id.
     * @param missionId The local mission to continue.
     * @param versionCode Version code of the remote target ability.
     * @param userId Owner user id; -1 means the current user.
     * @return ERR_OK on success; ERR_INVALID_VALUE when not called by DMS or the mission is unknown.
     */
    ErrCode ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode,
        int32_t userId = -1);

    /**
     * Report the continuation result of a mission back to the application side. Internal use by
     * the continuation framework; the result is delivered to the ability that initiated
     * continuation.
     *
     * @param missionId The mission whose continuation finished.
     * @param result The continuation result code.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode NotifyContinuationResult(int32_t missionId, int32_t result);

    /**
     * Lock a mission so it is excluded from "clean all missions". System-app callers only, and
     * the caller must hold ohos.permission.MANAGE_MISSIONS. On SceneBoard devices the request is
     * routed to the scene session manager (LockSession) with SCB errors mapped to mission error
     * codes.
     *
     * @param missionId The id of the mission to lock.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED without
     *         MANAGE_MISSIONS; ERR_NO_INIT when the mission manager is unavailable.
     */
    ErrCode LockMissionForCleanup(int32_t missionId);

    /**
     * Unlock a previously locked mission so it can be cleaned again. System-app callers only +
     * ohos.permission.MANAGE_MISSIONS. SCB path uses UnlockSession.
     *
     * @param missionId The id of the mission to unlock.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED; ERR_NO_INIT.
     */
    ErrCode UnlockMissionForCleanup(int32_t missionId);

    /**
     * Set the locked state of an ability record (session) directly. SceneBoard callers ONLY —
     * calls from other processes are silently ignored (void return). Internal use.
     *
     * @param sessionId The persistent session/record id of the target ability.
     * @param lockedState true to lock, false to unlock.
     */
    void SetLockedState(int32_t sessionId, bool lockedState);

    /**
     * Register a mission (snapshot/created/deleted/switched...) listener. System-app callers
     * only + ohos.permission.MANAGE_MISSIONS. On SceneBoard devices the listener is registered
     * with the scene session manager instead of AMS.
     *
     * @param listener The mission listener to register; must be non-null.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED; ERR_NO_INIT.
     */
    ErrCode RegisterMissionListener(sptr<IMissionListener> listener);

    /**
     * Unregister a mission listener. System-app callers only + ohos.permission.MANAGE_MISSIONS.
     *
     * @param listener The mission listener to remove; must be non-null.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED; ERR_NO_INIT.
     */
    ErrCode UnRegisterMissionListener(sptr<IMissionListener> listener);

    /**
     * Register a remote mission listener for a remote device (distributed mission sync).
     * Distributed use; the listener receives remote mission change notifications.
     *
     * @param deviceId The remote device id to listen on.
     * @param listener The remote mission listener; must be non-null.
     * @return ERR_OK on success, others on failure (e.g. DMS errors).
     */
    ErrCode RegisterMissionListener(const std::string &deviceId, sptr<IRemoteMissionListener> listener);

    /**
     * Register an "on"-type remote event listener for the given event type (distributed
     * notification bridge for mission/continuation events).
     *
     * @param type The event type string to listen for.
     * @param listener The remote on-listener; must be non-null.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode RegisterOnListener(const std::string &type, sptr<IRemoteOnListener> listener);

    /**
     * Register an "off"-type remote event listener for the given event type (counterpart of
     * RegisterOnListener for unsubscribe-style events).
     *
     * @param type The event type string to listen for.
     * @param listener The remote off-listener; must be non-null.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode RegisterOffListener(const std::string &type, sptr<IRemoteOnListener> listener);

    /**
     * Unregister a remote mission listener previously registered for a device.
     *
     * @param deviceId The remote device id.
     * @param listener The remote mission listener to remove.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode UnRegisterMissionListener(const std::string &deviceId, sptr<IRemoteMissionListener> listener);

    /**
     * Query mission infos (local or remote). System-app callers only +
     * ohos.permission.MANAGE_MISSIONS. When deviceId refers to a remote device the query is
     * forwarded to DMS; on SceneBoard devices the list comes from the scene session manager
     * (GetSessionInfos). numMax caps the number of returned missions.
     *
     * @param deviceId Local (empty/local id) or remote device id.
     * @param numMax Maximum number of missions to return.
     * @param missionInfos Output vector of mission infos.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED without
     *         MANAGE_MISSIONS; ERR_NO_INIT; DMS errors for remote queries.
     */
    ErrCode GetMissionInfos(const std::string &deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos);

    /**
     * Query a single mission info by id. System-app callers only +
     * ohos.permission.MANAGE_MISSIONS. Remote deviceId queries go through DMS.
     *
     * @param deviceId Local or remote device id.
     * @param missionId Id of the target mission.
     * @param missionInfo Output mission info of the target mission.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED; ERR_NO_INIT;
     *         INVALID_PARAMETERS_ERR / MISSION_NOT_FOUND variants; DMS errors.
     */
    ErrCode GetMissionInfo(const std::string &deviceId, int32_t missionId, MissionInfo &missionInfo);

    /**
     * Query a single mission info together with the display info of its window. SceneBoard
     * devices only: the non-SceneBoard path is NOT implemented and returns
     * ERR_CAPABILITY_NOT_SUPPORT. System-app + ohos.permission.MANAGE_MISSIONS still apply on
     * the SCB path.
     *
     * @param deviceId Local or remote device id.
     * @param missionId Id of the target mission.
     * @param missionInfo Output mission info.
     * @param displayInfo Output display info of the mission's window.
     * @return ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT on non-SCB devices;
     *         CHECK_PERMISSION_FAILED without MANAGE_MISSIONS; SCB-mapped errors.
     */
    ErrCode GetMissionInfo(const std::string& deviceId, int32_t missionId, MissionInfo &missionInfo,
        DisplayInfo &displayInfo);

    /**
     * Get the snapshot (screenshot pixel map) of a mission. System-app callers only +
     * ohos.permission.MANAGE_MISSIONS. Remote deviceId queries go through DMS; on SceneBoard
     * devices the snapshot comes from the scene session manager.
     *
     * @param deviceId Local or remote device id.
     * @param missionId Id of the target mission.
     * @param snapshot Output snapshot of the target mission.
     * @param isLowResolution true to fetch a low-resolution snapshot (smaller payload).
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED; INNER_ERR when the
     *         snapshot cannot be captured; DMS errors for remote.
     */
    ErrCode GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution = false);

    /**
     * Clean (destroy) a single mission. On SceneBoard devices the request is routed to the
     * scene session manager (ClearSession) with SCB errors mapped to mission error codes.
     *
     * @param missionId Id of the target mission to clean.
     * @return ERR_OK on success; INVALID_PARAMETERS_ERR / MISSION_NOT_FOUND variants;
     *         CHECK_PERMISSION_FAILED (mapped from SCB permission error); ERR_NO_INIT.
     */
    ErrCode CleanMission(int32_t missionId);

    /**
     * Clean all missions in the system (except locked/protected ones). SCB path uses
     * ClearAllSessions. Affects all applications — use with care.
     *
     * @return ERR_OK on success; CHECK_PERMISSION_FAILED; ERR_NO_INIT; SCB-mapped errors.
     */
    ErrCode CleanAllMissions();

    /**
     * Bring a mission to the front (switch to it). System-app callers only +
     * ohos.permission.MANAGE_MISSIONS. Blocked with ERR_WOULD_BLOCK when an ability controller
     * (wukong) intercepts the start.
     *
     * @param missionId Id of the target mission.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED without
     *         MANAGE_MISSIONS; ERR_WOULD_BLOCK under ability controller; ERR_NO_INIT.
     */
    ErrCode MoveMissionToFront(int32_t missionId);

    /**
     * Bring a mission to the front with StartOptions (target display / window mode for the
     * switch). System-app callers only + ohos.permission.MANAGE_MISSIONS.
     *
     * @param missionId Id of the target mission.
     * @param startOptions Special options (display id, window mode) applied on the switch.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED; ERR_WOULD_BLOCK;
     *         ERR_NO_INIT; ERR_INVALID_VALUE.
     */
    ErrCode MoveMissionToFront(int32_t missionId, const StartOptions &startOptions);

    /**
     * Move multiple missions to foreground in one operation (multi-window / split scenarios).
     * System-app callers only + ohos.permission.MANAGE_MISSIONS. topMissionId selects which
     * mission ends on top; -1 keeps the relative z-order of missionIds. On SCB devices the
     * window layering is done via MoveSessionsToForeground, then the top mission is moved
     * through AMS; an empty missionIds vector is rejected (ERR_INVALID_VALUE).
     *
     * @param missionIds Ids of the target missions (non-empty).
     * @param topMissionId Mission to place on top; -1 keeps the given order.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED;
     *         ERR_INVALID_VALUE for an empty list; ERR_NO_INIT.
     */
    ErrCode MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId);

    /**
     * Move multiple missions to background in one operation. System-app callers only +
     * ohos.permission.MANAGE_MISSIONS. On SCB devices the operation is fully handled by the
     * scene session manager (MoveSessionsToBackground).
     *
     * @param missionIds Ids of the target missions.
     * @param result Output: ids of missions actually moved, sorted by zOrder.
     * @return ERR_OK on success; ERR_NOT_SYSTEM_APP; CHECK_PERMISSION_FAILED; ERR_NO_INIT /
     *         ERR_INVALID_VALUE.
     */
    ErrCode MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result);

    /**
     * Get the mission (session) id of the ability identified by token. Self-call or DMS callers
     * only; a non-positive result (unknown token) is reported as MISSION_NOT_FOUND.
     *
     * @param token The ability token to query.
     * @param missionId Output mission id (> 0 on success).
     * @return ERR_OK on success; MISSION_NOT_FOUND when no mission matches the token
     *         (including a null/invalid token); ABILITY_SERVICE_NOT_CONNECTED when the AMS
     *         proxy is unavailable.
     */
    ErrCode GetMissionIdByToken(sptr<IRemoteObject> token, int32_t &missionId);

    /**
     * Start an ability "by call": connect to its callee (call) stub; if the target is not
     * running it is started first (its window is NOT brought to front). This overload passes no
     * caller token (SA-oriented). isSilent suppresses the interceptor-failure dialog;
     * isVisible controls whether the after-check failure dialog shows. Cross-account requires
     * account permission; app-clone index in the want is validated. The connection result
     * (callee stub) arrives via connect->OnAbilityConnectDone.
     *
     * @param want The want of the target ability (callee side).
     * @param connect Callback receiving the callee stub / disconnect events; must be non-null.
     * @param isSilent true: no dialog shown when intercepted at the interceptor stage.
     * @param isVisible true: show the failure dialog of after-check stage on rejection.
     * @return ERR_OK when accepted (callee arrives via callback); ERR_INVALID_VALUE for null
     *         connect; CHECK_PERMISSION_FAILED; ERR_CROSS_USER; ERR_APP_CLONE_INDEX_INVALID;
     *         interceptor errors.
     */
    ErrCode StartAbilityByCall(const Want &want, sptr<IAbilityConnection> connect, bool isSilent = false,
        bool isVisible = false);

    /**
     * Start an ability by call with caller token and account. callToken identifies the caller
     * for self-call/permission checks (must be the caller's own record when provided).
     * accountId selects the target user (0 is treated as unspecified). promotePriority raises
     * the target process scheduling priority but is honored ONLY for SA callers — silently
     * ignored otherwise. isSilent/isVisible as in the SA overload.
     *
     * @param want The want of the target ability (callee side).
     * @param connect Callback receiving the callee stub; must be non-null.
     * @param callToken Token of the calling ability; may be nullptr for SA callers.
     * @param accountId Target account/user id; DEFAULT_INVAL_VALUE (and 0) means current user.
     * @param isSilent true: no dialog on interceptor rejection.
     * @param promotePriority true: promote target process priority (SA callers only).
     * @param isVisible true: show the after-check failure dialog.
     * @return ERR_OK when accepted; error family as the SA overload plus CHECK_PERMISSION_FAILED
     *         when callToken belongs to another app.
     */
    ErrCode StartAbilityByCall(const Want &want, sptr<IAbilityConnection> connect,
        sptr<IRemoteObject> callToken, int32_t accountId = DEFAULT_INVAL_VALUE, bool isSilent = false,
        bool promotePriority = false, bool isVisible = false);

    /**
     * StartAbilityByCall variant that additionally returns a human-readable failure reason in
     * errMsg (e.g. "verify account permission failed", "app index is error"). Same semantics,
     * permission rules and promotePriority SA-only restriction as StartAbilityByCall.
     *
     * @param want The want of the target ability (callee side).
     * @param connect Callback receiving the callee stub; must be non-null.
     * @param callToken Token of the calling ability; may be nullptr for SA callers.
     * @param accountId Target account/user id.
     * @param errMsg Output: failure reason string (set on error paths).
     * @param isSilent true: no dialog on interceptor rejection.
     * @param promotePriority true: promote target process priority (SA callers only).
     * @param isVisible true: show the after-check failure dialog.
     * @return ERR_OK when accepted; same error family as StartAbilityByCall; errMsg carries the
     *         reason on failure.
     */
    int32_t StartAbilityByCallWithErrMsg(const Want &want, sptr<IAbilityConnection> connect,
        sptr<IRemoteObject> callToken, int32_t accountId, std::string &errMsg, bool isSilent = false,
        bool promotePriority = false, bool isVisible = false);

    /**
     * Prelaunch (pre-warm) a UIAbility ahead of user interaction. Render-Surface-Service
     * process ONLY (RSS_UID check) — all other callers get RESOLVE_CALL_NO_PERMISSIONS.
     * SceneBoard must be enabled. The want must be explicit (no implicit start). Target
     * restrictions: stage-model PAGE abilities only, launch mode must NOT be SPECIFIED, and
     * isolated-process abilities are not supported (RESOLVE_CALL_ABILITY_TYPE_ERR /
     * RESOLVE_CALL_ABILITY_VERSION_ERR). Except on clone-for-account products, a specified
     * app clone index is forcibly reset to 0 (base instance). frameNum identifies the frame at
     * which the start window should be removed.
     *
     * @param want The want of the ability to prelaunch (explicit element required).
     * @param frameNum Frame number used to remove the start window; 0 means not specified.
     * @return ERR_OK on success; RESOLVE_CALL_NO_PERMISSIONS for non-RSS callers;
     *         ERR_INVALID_VALUE for implicit want / SCB disabled; RESOLVE_CALL_ABILITY_TYPE_ERR /
     *         RESOLVE_CALL_ABILITY_VERSION_ERR for unsupported targets; interceptor errors.
     */
    ErrCode StartAbilityForPrelaunch(const Want &want, const int32_t frameNum = 0);

    /**
     * Start a sandbox-clone instance of an application (CLI-tool driven sandbox scenario).
     * CLI-tool tokens ONLY (AccessTokenKit::IsCliToolToken) — other callers get
     * ERR_PERMISSION_DENIED. The want MUST carry a valid sandbox clone index param
     * (ohos.sandbox.cloneIndex); app-clone-index and DLP-index params in the want are stripped
     * by the server. params carries the invoking CLI tool identity (callerBundleName / uid /
     * tokenId) used for attribution.
     *
     * @param want The want of the ability to start; must contain a valid sandbox clone index.
     * @param params Caller information of the CLI tool (bundle name, uid, token id).
     * @return ERR_OK on success; ERR_PERMISSION_DENIED for non-CLI-tool callers;
     *         ERR_INVALID_VALUE when the clone index param is missing;
     *         ERR_SANDBOX_CLONE_INDEX_INVALID for an out-of-range index; start errors otherwise.
     */
    ErrCode StartSandboxCloneAbility(const Want &want, const SandboxCloneParams &params);

    /**
     * Register the callee stub produced by the ability's onCallRequest. Internal use only:
     * invoked by the ability runtime when the application handles a call request, making the
     * stub available to the caller waiting in StartAbilityByCall. Self-call enforced (other
     * processes are ignored). Void IPC — no return value.
     *
     * @param token The token of the ability that produced the callee.
     * @param callStub The callee IPC stub returned by the ability.
     */
    void CallRequestDone(sptr<IRemoteObject> token, sptr<IRemoteObject> callStub);

    /**
     * Reverse lookup: get the ability token that registered the given callee stub (mapping
     * created by CallRequestDone). Internal use for the call framework (e.g. routing release/
     * disconnect). token is set to nullptr when the stub is unknown. Void IPC.
     *
     * @param callStub The callee stub previously registered via CallRequestDone.
     * @param token Output: the owning ability token, or nullptr if not found.
     */
    void GetAbilityTokenByCalleeObj(sptr<IRemoteObject> callStub, sptr<IRemoteObject> &token);

    /**
     * Release a call established by StartAbilityByCall: disconnect the connection and, when the
     * callee has no remaining callers, stop the callee ability. element identifies the callee
     * (remote device id in the element routes the release to DMS). connect must be non-null.
     *
     * @param connect The connection object used in StartAbilityByCall; must be non-null.
     * @param element The ElementName of the callee ability.
     * @return ERR_OK on success; ERR_INVALID_VALUE for null connect/unknown connection;
     *         DMS errors for remote release.
     */
    ErrCode ReleaseCall(sptr<IAbilityConnection> connect, const AppExecFwk::ElementName &element);

    /**
     * @brief Get running information of all abilities of the calling user.
     * System API. Called by system apps (e.g. ability info panels) to enumerate
     * running ability records. Without ohos.permission.RUNNING_STATE_PERMISSION
     * only caller's own records are returned (server checks VerifyRunningInfoPerm).
     * @param info Output, running info list of abilities.
     * @return Returns ERR_OK on success; ERR_INVALID_VALUE if internal manager is null.
     */
    ErrCode GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info);

    /**
     * @brief Get running information of extension abilities for all foreground users.
     * System API, system app only (CHECK_CALLER_IS_SYSTEM_APP). Results are collected
     * across all foreground users; @param upperLimit caps the returned count per user.
     * Without RUNNING_STATE_PERMISSION only caller's own extensions are visible.
     * @param upperLimit Max number of records desired.
     * @param info Output, extension running info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info);

    /**
     * @brief Get running information of all application processes.
     * Delegates to AppManagerService; visibility of other apps' info follows
     * RUNNING_STATE_PERMISSION rules of appMgr.
     * @param info Output, running process info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info);

    /**
     * @brief Start synchronizing missions from a remote device (distributed).
     * System app only; requires mission permission
     * (ohos.permission.MANAGE_MISSION_STACK or RECENT_TASK).
     * @param devId Remote device id.
     * @param fixConflict True to resolve sync conflicts automatically.
     * @param tag Caller defined tag passed to DMS.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED otherwise.
     */
    ErrCode StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag);

    /**
     * @brief Stop synchronizing missions from a remote device.
     * System app only; requires mission permission
     * (ohos.permission.MANAGE_MISSION_STACK or RECENT_TASK).
     * @param devId Remote device id.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED otherwise.
     */
    ErrCode StopSyncRemoteMissions(const std::string &devId);

    /**
     * @brief Switch foreground user on the given display and start the new user.
     * Internal use: called by SCB/account subsystem during user switching.
     * callback is mandatory; the callback always receives OnUserCmdDone with the
     * final result. Starting user 1 or the current foreground user is a no-op
     * returning ERR_OK. When low-memory "block all app start" mode is on,
     * returns ERR_ALL_APP_START_BLOCKED.
     * @param accountId Target user id.
     * @param displayId Logical display id the user switches to.
     * @param callback Result callback, must not be nullptr.
     * @param isAppRecovery True when starting for app recovery (default false);
     *        skips restart-frequency limiting.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartUser(int accountId, uint64_t displayId, sptr<IUserCallback> callback, bool isAppRecovery = false);

    /**
     * @brief Stop a background user: kill its processes, clear mission data and
     * auto-startup state, record exit reason REASON_USER_STOP.
     * Internal use. Stopping the current foreground user is a no-op returning
     * ERR_OK via callback. callback is mandatory.
     * @param accountId User id to stop.
     * @param callback Result callback, must not be nullptr.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StopUser(int accountId, sptr<IUserCallback> callback);

    /**
     * @brief Log out a user: record REASON_USER_LOGOUT exit reason, clear user data
     * and kill all its processes. Unlike StopUser it works for the foreground user
     * as well. Internal use by account/SCB.
     * @param accountId User id to log out.
     * @param callback Optional result callback; if nullptr the request is rejected
     *        with INVALID_PARAMETERS_ERR.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode LogoutUser(int32_t accountId, sptr<IUserCallback> callback = nullptr);

    /**
     * @brief Register the mission snapshot handler (mission snapshot provider).
     * Internal use: only the samgr-registered window/service (SAMGR_UID) may call;
     * other callers get CHECK_PERMISSION_FAILED. Only effective on non-SCB devices.
     * @param handler The snapshot handler.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterSnapshotHandler(sptr<ISnapshotHandler> handler);

    /**
     * @brief Ask AMS whether an ability can be terminated now (onPrepareToTerminate).
     * Internal use by application framework before self-termination; caller must be
     * the ability itself (JudgeSelfCalled). If termination is deferred, a timeout
     * task triggers callback->DoPrepareTerminate() automatically.
     * @param token Ability token, must not be nullptr.
     * @param callback Callback invoked when prepare-terminate finishes.
     * @return Returns ERR_OK on success; ERR_INVALID_VALUE if callback null.
     */
    ErrCode PrepareTerminateAbility(sptr<IRemoteObject> token, sptr<IPrepareTerminateCallback> callback);

    /**
     * @brief Register the status bar delegate. SCB only (IsCallerSceneBoard);
     * other callers get ERR_WRONG_INTERFACE_CALL.
     * @param delegate Status bar delegate.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate);

    /**
     * @brief Kill processes after their prepareTerminate callbacks complete.
     * SCB only, or callers holding ohos.permission.KILL_APP_PROCESSES.
     * Asynchronous: results are delivered via KillProcessWithPrepareTerminateDone.
     * @param pids Process ids to kill.
     * @param clear True to clear mission records without prepare-terminate flow.
     * @return Returns ERR_OK on success; ERR_WRONG_INTERFACE_CALL when unauthorized.
     */
    ErrCode KillProcessWithPrepareTerminate(const std::vector<int32_t> &pids, bool clear = false);

    /**
     * @brief Kill one process and record the exit reason. System API.
     * Requires ohos.permission.KILL_APP_PROCESSES (shell allowed when product
     * enables persist.ams.support_kill_with_reason). Foreground/focused process is
     * not killed unless reason.shouldKillForeground is set (returns
     * ERR_KILL_APP_WHILE_FOREGROUND); a starting process returns
     * ERR_KILL_APP_WHILE_STARTING. Under resource-schedule enablement, RSS may
     * reject the kill (ERR_PERMISSION_DENIED).
     * @param pid Process id.
     * @param reason Kill reason, see ExitReason; must carry a valid reason id.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode KillProcessWithReason(int32_t pid, const ExitReason &reason);

    /**
     * @brief Register a system callback for auto-startup state changes.
     * System API: caller must pass system-API check and hold
     * ohos.permission.MANAGE_APP_BOOT; auto-startup must be enabled on the product
     * (otherwise ERR_NOT_SUPPORTED_PRODUCT_TYPE). Duplicate registration is ignored.
     * @param callback Remote callback object.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterAutoStartupSystemCallback(sptr<IRemoteObject> callback);

    /**
     * @brief Unregister a previously registered auto-startup system callback.
     * Same permission requirements as RegisterAutoStartupSystemCallback.
     * @param callback Remote callback object to remove.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnregisterAutoStartupSystemCallback(sptr<IRemoteObject> callback);

    /**
     * @brief Enable auto-startup for an application (user level setting).
     * System API, requires system-API check + ohos.permission.MANAGE_APP_BOOT.
     * info.isHiddenStart is not accepted here (EDM only, returns ERR_INVALID_VALUE).
     * Cross-user setting requires INTERACT_ACROSS_LOCAL_ACCOUNTS permission.
     * @param info bundle/module/ability name identifying the target component.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetApplicationAutoStartup(const AutoStartupInfo &info);

    /**
     * @brief Disable auto-startup of an application (user level setting).
     * Same permission/limitation rules as SetApplicationAutoStartup.
     * @param info bundle/module/ability name identifying the target component.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CancelApplicationAutoStartup(const AutoStartupInfo &info);

    /**
     * @brief Query auto-startup info of all applications for every foreground user.
     * System API with the same MANAGE_APP_BOOT permission requirement.
     * @param infoList Output, auto startup info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList);

    /**
     * @brief Query whether auto-startup is enabled for the calling application
     * itself (identified by calling token id). No special permission needed.
     * @param isAutoStartEnabled Output, true if enabled.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetAutoStartupStatusForSelf(bool &isAutoStartEnabled);

    /**
     * @brief Manually launch all enabled auto-startup apps of a user.
     * Internal system API: requires
     * ohos.permission.MANAGE_APP_BOOT_INTERNAL (system service use only, e.g. EDM).
     * Asynchronous; returns immediately after triggering.
     * @param userId Target user id.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED without permission.
     */
    ErrCode ManualStartAutoStartupApps(int32_t userId);

    /**
     * @brief Query the cached caller token id for an ANCO session.
     * Internal use: only the broker process (BROKER_UID) may call.
     * @param userId User id owning the ability records.
     * @param asCallerForAncoSessionId Anco session id of the cached entry.
     * @param callerTokenId Output, cached caller token id.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED for other callers.
     */
    ErrCode QueryCallerTokenIdForAnco(int32_t userId, const std::string &asCallerForAncoSessionId,
        uint32_t &callerTokenId);

    /**
     * @brief Launch a game with customized (pre-launch) flow, verified by game SA.
     * Only game SA uid, or shell in developer mode for a debug app, is allowed;
     * native-ability targets are rejected. The target must not already be
     * preloaded (ERR_PRELOAD_APP_RECORD_ALREADY_EXIST otherwise).
     * @param bundleName Game bundle name.
     * @param userId User id.
     * @param appIndex App clone index; only appIndex = 0 is supported.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED for others.
     */
    ErrCode LaunchGameCustomized(const std::string &bundleName, int32_t userId, int32_t appIndex = 0);
    
    /**
     * @brief Report the pre-launch completion time (ms) of a game.
     * Internal: only game SA (GAME_SA_UID) may call.
     * @param userId User id.
     * @param completeTime Complete time in milliseconds, must be >= 0.
     * @return Returns ERR_OK on success; ERR_INVALID_VALUE for negative completeTime;
     *         CHECK_PERMISSION_FAILED for non-game-SA callers; ABILITY_SERVICE_NOT_CONNECTED
     *         when the AMS proxy is unavailable.
     */
    ErrCode SetGamePreLaunchCompleteTime(int32_t userId, int64_t completeTime);

    /**
     * @brief Query whether a UIAbility's onPrepareToTerminate allows termination.
     * SCB only (IsCallerSceneBoard); sessionInfo with valid sessionToken required.
     * @param sessionInfo Session info of the ability to terminate.
     * @param isPrepareTerminate Output, true if the ability agrees to terminate.
     * @return Returns ERR_OK on success; ERR_WRONG_INTERFACE_CALL if not SCB.
     */
    ErrCode PrepareTerminateAbilityBySCB(sptr<SessionInfo> sessionInfo, bool &isPrepareTerminate);

    /**
     * @brief Set mission continuation state (active/inactive) of the caller's own
     * ability. Only the ability's own application (calling accessTokenId must
     * equal the record's accessTokenId) may set; also notifies DMS for distributed
     * continuation.
     * @param token Ability token.
     * @param state Continue state, see AAFwk::ContinueState.
     * @param sessionToken On SceneBoard-enabled products, forwarded to the scene session
     *                     manager to address the target session; ignored on the non-SCB path.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetMissionContinueState(sptr<IRemoteObject> token, const AAFwk::ContinueState &state,
        sptr<IRemoteObject> sessionToken);

#ifdef SUPPORT_SCREEN
    /**
     * @brief Set the mission label shown in recent missions. Only available when
     * SUPPORT_SCREEN is compiled in. Self only: the calling accessTokenId must
     * equal the ability's accessTokenId.
     * @param abilityToken Ability token.
     * @param label Label displayed in recent missions.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetMissionLabel(sptr<IRemoteObject> abilityToken, const std::string &label);

    /**
     * @brief Set the mission icon shown in recent missions.
     * System app only (CHECK_CALLER_IS_SYSTEM_APP) and self only (calling
     * accessTokenId must equal the ability's accessTokenId).
     * @param abilityToken Ability token.
     * @param icon Icon pixel map.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetMissionIcon(sptr<IRemoteObject> abilityToken,
        std::shared_ptr<OHOS::Media::PixelMap> icon);

    /**
     * @brief Register the window manager service handler. Internal use:
     * SAMGR_UID only, and only supported on non-SCB (Rosen window mode) devices.
     * @param handler WMS handler.
     * @param animationEnabled Whether start animation is enabled, default true.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED otherwise.
     */
    ErrCode RegisterWindowManagerServiceHandler(sptr<IWindowManagerServiceHandler> handler,
        bool animationEnabled = true);

    /**
     * @brief Notify AMS that the first frame of an ability has been drawn.
     * Internal use: only foundation (FOUNDATION_UID) process may call; non-foundation
     * calls are silently ignored. Non-SCB mode only.
     * @param abilityToken Ability token.
     */
    void CompleteFirstFrameDrawing(sptr<IRemoteObject> abilityToken);

    /**
     * @brief Notify AMS that the first frame of a session has been drawn.
     * SCB only (IsCallerSceneBoard); other calls are silently ignored.
     * @param sessionId Session id of the UIAbility.
     */
    void CompleteFirstFrameDrawing(int32_t sessionId);

    /**
     * @brief Update the snapshot of a mission. Internal use: only foundation
     * process (IS_FOUNDATION_CALL) may call; others are silently ignored.
     * Non-SCB mode only.
     * @param token The target ability token.
     * @param pixelMap The new snapshot.
     */
    void UpdateMissionSnapShot(sptr<IRemoteObject> token,
        std::shared_ptr<OHOS::Media::PixelMap> pixelMap);

    /**
     * @brief Get info of a modal dialog session by its id. System app only.
     * @param dialogSessionId Dialog session id.
     * @param info Output dialog session info.
     * @return Returns ERR_OK on success; INNER_ERR if session not found.
     */
    ErrCode GetDialogSessionInfo(const std::string &dialogSessionId, sptr<DialogSessionInfo> &info);

    /**
     * @brief Send the user's decision of a system dialog back to AMS.
     * System app only; used by the dialog UIExtension to deliver allow/deny.
     * @param want Result want (may contain selected component).
     * @param dialogSessionId Dialog session id.
     * @param isAllow True if user allowed.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllow);
#endif

    /**
     * @brief Start a user test (XTS/test-runner) process for a bundle.
     * The want must carry the bundle name in parameter "-b"; app clone is not
     * supported (appIndex must be 0, otherwise ERR_NOT_SUPPORT_APP_CLONE);
     * requesting debug of a release app returns ERR_NOT_IN_APP_PROVISION_MODE.
     * Optional "-u" parameter must reference a foreground user.
     * @param want Test want containing "-b" bundleName, optional "-u" userId.
     * @param observer Test observer callback, must not be nullptr.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartUserTest(const Want &want, sptr<IRemoteObject> observer);

    /**
     * @brief Finish a user test and deliver results via the registered observer.
     * bundleName must be non-empty; forwarded to AppManagerService.
     * @param msg User test message.
     * @param resultCode User test result code.
     * @param bundleName User test bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName);

    /**
     * @brief Get the token of the current top ability. System ability only:
     * the caller must be an SA (IsSACall), otherwise CHECK_PERMISSION_FAILED.
     * Client side: when SCB is enabled the client routes to scene board first.
     * @param token Output, top ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetTopAbility(sptr<IRemoteObject> &token);

    /**
     * @brief Check whether a UIExtension (or window extension) belongs to the
     * currently focused top ability. Requires SA permission (GetTopAbility path);
     * displayId selects which display's top ability is checked (default 0).
     * @param uiExtensionTokenId Token id of the UIExtension.
     * @param isFocused Output, true if focused.
     * @param displayId Logical display id, default 0.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused, uint64_t displayId = 0);

    /**
     * @brief Ability delegator (test framework) moves the ability to foreground.
     * Caller pid must equal the ability's pid (self process). Under SCB only the
     * controller-start check is done; ability controller may block with
     * ERR_WOULD_BLOCK.
     * @param token Ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DelegatorDoAbilityForeground(sptr<IRemoteObject> token);

    /**
     * @brief Ability delegator moves the ability to background (minimize).
     * Caller pid must equal the ability's pid (self process). Under SCB this is a
     * no-op returning ERR_OK.
     * @param token Ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DelegatorDoAbilityBackground(sptr<IRemoteObject> token);

    /**
     * @brief Report ability foreground transition (internal, called by ability
     * runtime scheduler). Self process only (JudgeSelfCalled); only PAGE type
     * abilities are accepted (ERR_WRONG_INTERFACE_CALL otherwise); ability
     * controller may block (ERR_WOULD_BLOCK).
     * @param token Ability token.
     * @param flag Scene flag, e.g. lock/unlock screen related flags.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DoAbilityForeground(sptr<IRemoteObject> token, uint32_t flag);

    /**
     * @brief Report ability background transition, i.e. minimize the ability
     * (internal, called by ability runtime scheduler). flag is recorded as
     * sceneFlag in lifecycle state info.
     * @param token Ability token.
     * @param flag Scene flag passed to the lifecycle callback.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DoAbilityBackground(sptr<IRemoteObject> token, uint32_t flag);

    /**
     * @brief Set the global ability controller used by stability tests to
     * allow/deny ability start/background. Requires controller permission
     * (ohos.permission.CONTROL_ABILITY_STACK / shell). Internal test framework use.
     * @param abilityController The ability controller.
     * @param imAStabilityTest True when running a stability test.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED otherwise.
     */
    virtual int SetAbilityController(sptr<AppExecFwk::IAbilityController> abilityController,
        bool imAStabilityTest);

    /**
     * @brief Free-install and start an ability requested from a remote device.
     * DMS only: calling uid must be DMS_UID, others get CHECK_PERMISSION_FAILED.
     * @param want Ability want.
     * @param callback Free install result callback.
     * @param userId User ID.
     * @param requestCode Ability request code, default DEFAULT_INVAL_VALUE.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode FreeInstallAbilityFromRemote(const Want &want, sptr<IRemoteObject> callback, int32_t userId,
        int requestCode = DEFAULT_INVAL_VALUE);

    /**
     * @brief Called by the target ability when its asynchronous dump finished,
     * delivering dump content back to the requester. Self process only
     * (JudgeSelfCalled).
     * @param infos The dump info lines.
     * @param callerToken The ability token that performed the dump.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DumpAbilityInfoDone(std::vector<std::string> &infos, sptr<IRemoteObject> callerToken);

    /**
     * @brief Enable app recovery for the ability (called by app framework when the
     * app enables recovery). Self only (calling accessTokenId must match);
     * rejected when a clear-mission flag is pending; registers recover info
     * (scene board mode stores by session id).
     * @param token Ability identify.
     */
    void EnableRecoverAbility(sptr<IRemoteObject> token);

    /**
     * @brief Report that the ability has saved its recovery state (session data).
     * Self only (calling accessTokenId must match).
     * @param token Ability identify.
     */
    void SubmitSaveRecoveryInfo(sptr<IRemoteObject> token);

    /**
     * @brief Trigger in-app recovery: restart the current ability in the same
     * process. Self only; only PAGE abilities; at most once per minute
     * (subsequent calls kill the app); optional want must target the same bundle
     * and an explicit ability. Ability must be foreground.
     * @param token Ability identify.
     * @param reason See AppExecFwk::StateReason.
     * @param want Optional target want; bundle must match, default nullptr.
     */
    void ScheduleRecoverAbility(sptr<IRemoteObject> token, int32_t reason, const Want *want = nullptr);

    /**
     * @brief Clear all saved recovery/exit-reason data of the calling application
     * (identified by calling uid/token). Internal use by app framework.
     */
    void ScheduleClearRecoveryPageStack();

    /**
     * @brief Register an observer to receive free-install results of asynchronous
     * StartFreeInstall requests initiated by the caller.
     * @param callToken The caller ability token.
     * @param observer Free install observer.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AddFreeInstallObserver(const sptr<IRemoteObject> callToken,
        const sptr<AbilityRuntime::IFreeInstallObserver> observer);

    /**
     * @brief Check whether mission ids are still valid for the calling user.
     * Note: at most 20 results are returned (searchCount = 20); extra ids beyond
     * the first 20 are not checked.
     * @param missionIds Query mission id list (only first 20 checked).
     * @param results Output, up to 20 validity results.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results);

    /**
     * @brief Verify whether the application owning the uid has been granted a
     * permission. FA model only: stage-based caller bundles are rejected
     * (CHECK_PERMISSION_FAILED). Internal use for FA VerifyPermission.
     * @param permission Permission name, must not be empty.
     * @param pid Process id (reserved; verification is by uid).
     * @param uid Target process uid.
     * @return Returns ERR_OK if granted; CHECK_PERMISSION_FAILED otherwise.
     */
    ErrCode VerifyPermission(const std::string &permission, int pid, int uid);

    /**
     * @brief Request shared data (mission sharing) from the ability of a mission.
     * System app only. Asynchronous: the target ability replies via
     * ShareDataDone, and the callback registered here is invoked with the data.
     * An internal uniqueId is generated to match request and response.
     * @param missionId The missionId of the target ability.
     * @param shareData The IAcquireShareDataCallback object.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AcquireShareData(int32_t missionId, sptr<IAcquireShareDataCallback> shareData);

    /**
     * @brief Called by the target ability to return shared data for a previous
     * AcquireShareData request. Self only (JudgeSelfCalled); uniqueId must match
     * the request delivered to the ability.
     * @param token Ability token of the sharing ability.
     * @param resultCode The result of sharing data.
     * @param uniqueId The uniqueId from the request object.
     * @param wantParam The shared data params.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ShareDataDone(
        sptr<IRemoteObject> token, int32_t resultCode, int32_t uniqueId, WantParams &wantParam);

    /**
     * @brief Send a modal service-extension dialog request (RequestDialogService
     * API). The target component must be a ServiceExtension of dialog type;
     * continuation flag in want is rejected (ERR_INVALID_CONTINUATION_FLAG);
     * DLP-handled wants rejected (ERR_WRONG_INTERFACE_CALL). Static permission
     * config of the target extension is enforced.
     * @param want Target component want.
     * @param callerToken Caller ability token, must be valid.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RequestDialogService(
        const Want &want,
        sptr<IRemoteObject> callerToken);

    /**
     * @brief Force stop an application by pid and record exit reason (killId
     * forced to REASON_AA_FORCE_STOP). SA or shell only; kills the whole
     * application (all pids of the bundle/appIndex).
     * @param pid Process id.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED for others.
     */
    ErrCode ForceExitApp(const int32_t pid, const ExitReason &exitReason);

    /**
     * @brief Record exit reason of the caller's own application (application
     * process reporting its own exit). No extra permission.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RecordAppExitReason(const ExitReason &exitReason);

    /**
     * @brief Record the exit reason of a process that is about to be killed.
     * SA or shell only (ERR_PERMISSION_DENIED otherwise).
     * @param pid The process id.
     * @param exitReason The reason of process exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason);

    /**
     * @brief Record process exit reason with explicit uid. Reserved for hiview
     * (HIVIEW_UID) and memory manager (MEMMGR_UID) only; other callers get
     * ERR_NO_PERMISSION_CALLER.
     * @param pid The process id.
     * @param uid The process uid.
     * @param exitReason The reason of process exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RecordProcessExitReason(int32_t pid, int32_t uid, const ExitReason &exitReason);

    /**
     * @brief Kill an app by pid with a validated exit reason. Requires
     * ohos.permission.KILL_APP_PROCESSES (or SA-kill support / shell). Invalid
     * killId is rejected with ERR_INVALID_VALUE (and recorded). Foreground apps
     * are protected unless shouldKillForeground set
     * (ERR_KILL_APP_WHILE_FOREGROUND); starting apps return
     * ERR_KILL_APP_WHILE_STARTING.
     * @param pid The process id.
     * @param exitReason The reason of kill app, must contain valid reason/killId.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode KillAppWithReason(int32_t pid, const ExitReasonCompability &exitReason);

    /**
     * @brief Kill all processes of a bundle (by userId/appIndex) with a recorded
     * reason. Same KILL_APP_PROCESSES permission and reason validation as
     * KillAppWithReason. Keep-alive (resident) bundles are NOT killed when memory
     * is sufficient (KILL_PROCESS_KEEP_ALIVE).
     * @param bundleName Bundle name of the app.
     * @param appIndex App clone index.
     * @param userId User ID.
     * @param exitReason The reason, must be valid.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode KillBundleWithReason(
        const std::string &bundleName, int32_t userId, int32_t appIndex, const ExitReasonCompability &exitReason);

    /**
     * @brief Record exit reason for an app without killing it. Allowed for
     * KILL_APP_PROCESSES holders, shell, SA-kill support, or the app itself
     * (calling uid == uid parameter); reason validity enforced.
     * @param pid The process id.
     * @param uid The process uid.
     * @param exitReason The reason of app exit, must be valid.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RecordAppWithReason(int32_t pid, int32_t uid, const ExitReasonCompability &exitReason);

    /**
     * @brief Set the root scene session token of SCB. SCB only
     * (IsCallerSceneBoard); other calls are silently ignored.
     * @param rootSceneSession Root scene session of SCB.
     */
    void SetRootSceneSession(sptr<IRemoteObject> rootSceneSession);

    /**
     * @brief SCB-driven UIAbility startup entry: resolve an ability for a session
     * and report whether it is a cold start. SCB only (IsCallerSceneBoard).
     * @param sessionInfo The session info of the ability to be called.
     * @param params Start parameters, filled in by AMS.
     * @param isColdStart Output, true when the session is cold started.
     */
    void CallUIAbilityBySCB(sptr<SessionInfo> sessionInfo, AbilityRuntime::StartParamsBySCB &params, bool &isColdStart);

    /**
     * @brief Start a specified-mode (SPECIFIED launch mode) ability for SCB.
     * SCB only. If the target user is still locked at first boot,
     * ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK is returned.
     * @param want Want information.
     * @param params Parameters for specified ability start (caller of
     *        AcceptWantFromService).
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED if not SCB.
     */
    int32_t StartSpecifiedAbilityBySCB(const Want &want, const StartSpecifiedAbilityParams &params);

    /**
     * @brief Notify the sandbox app of the result of a "save as" operation.
     * System app only; with DLP enabled, the caller must be the DLP manager.
     * Note: current server-side implementation validates and returns ERR_OK
     * without further dispatch.
     * @param want Result of saving file, contains the file uri on success.
     * @param resultCode Indicates the action's result.
     * @param requestCode Pass the requestCode to match request.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode NotifySaveAsResult(const Want &want, int resultCode, int requestCode);

    /**
     * @brief Register the scene session manager service proxy into window manager.
     * SCB only; others get ERR_WRONG_INTERFACE_CALL.
     * @param sessionManagerService The session manager service remote object.
     * @return Returns ERR_OK on success; SET_SMS_FAILED on failure.
     */
    ErrCode SetSessionManagerService(sptr<IRemoteObject> sessionManagerService);

    /**
     * @brief Report that the caller ability has finished drawing (drawn
     * completed), used for startup tracing. Reports a DRAWN_COMPLETED event;
     * token must map to an existing ability record.
     * @param token Caller ability token.
     * @return Returns ERR_OK on success; INNER_ERR for invalid token.
     */
    ErrCode ReportDrawnCompleted(sptr<IRemoteObject> token);

    /**
     * @brief Register an ability manager collaborator (application collocation).
     * Reserved: SA call AND broker uid (collaborator broker configured uid) only.
     * type must be CollaboratorType RESERVE_TYPE(0) or OTHERS_TYPE(1).
     * @param type Collaborator type, 0 or 1.
     * @param impl Collaborator implementation.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED otherwise.
     */
    ErrCode RegisterIAbilityManagerCollaborator(
        int32_t type, sptr<IAbilityManagerCollaborator> impl);

    /**
     * @brief Unregister an ability manager collaborator. Same broker-only
     * restriction and type validation as registration.
     * @param type Collaborator type, 0 or 1.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnregisterIAbilityManagerCollaborator(int32_t type);

    /**
     * @brief Get the registered reserve-type ability manager collaborator.
     * Foundation process only; returns nullptr for other callers or when no
     * collaborator registered.
     * @return Returns collaborator pointer on success, nullptr otherwise.
     */
    sptr<IAbilityManagerCollaborator> GetAbilityManagerCollaborator();

    /**
     * @brief Register the session handler used to notify SCB of session events.
     * SCB only.
     * @param object The ISessionHandler remote object.
     * @return Returns ERR_OK on success; ERR_WRONG_INTERFACE_CALL if not SCB.
     */
    ErrCode RegisterSessionHandler(sptr<IRemoteObject> object);

    /**
     * @brief Register a listener for app debug attach/detach events. SA only.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED for non-SA.
     */
    ErrCode RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener);

    /**
     * @brief Unregister the app debug listener. SA only.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener);

    /**
     * @brief Attach debug to an application (await debug). SA or shell only.
     * isDebugFromLocal=true additionally requires local-debug permission; when
     * false, device must be in developer mode (ERR_NOT_DEVELOPER_MODE).
     * Target must be a debug-provisioned app (ERR_NOT_IN_APP_PROVISION_MODE
     * for release apps).
     * @param bundleName The application bundle name.
     * @param isDebugFromLocal True when triggered by local debugging (default false).
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal = false);

    /**
     * @brief Detach debug from an application. Same caller/permission rules as
     * AttachAppDebug.
     * @param bundleName The application bundle name.
     * @param isDebugFromLocal True when triggered by local debugging (default false).
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DetachAppDebug(const std::string &bundleName, bool isDebugFromLocal = false);

    /**
     * @brief Check if the registered ability controller allows starting the
     * ability in want. Broker-only: only collaborator broker uid gets a real
     * answer; other callers always get true (check skipped).
     * @param want The want of ability to start.
     * @return Return true to allow ability to start, or false to reject.
     */
    bool IsAbilityControllerStart(const Want &want);

    /**
     * @brief Open a file by uri through the registered reserve-type collaborator
     * (application collocation file access). Requires a collaborator registered;
     * returns -1 otherwise. Internal use.
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or
     *        Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @return int The file descriptor on success, or -1 on failure.
     */
    int32_t OpenFile(const Uri& uri, uint32_t flag);

    /**
     * @brief Execute an insight intent. Cross-user execution requires account
     * permission and multi-user concurrency. When param.deviceId is set
     * (distributed intent), the caller needs
     * ohos.permission.EXECUTE_DISTRIBUTED_INTENT and flood-attack limiting applies
     * (too many requests in a window return INNER_ERR). executeMode
     * UI_EXTENSION_ABILITY is not supported (ERR_INVALID_OPERATION).
     * @param key The key of the intent executing client (callback mapping).
     * @param callerToken Caller ability token.
     * @param param The intent execute param (bundle/module/intent name, execute
     *        mode, target user).
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ExecuteIntent(uint64_t key, sptr<IRemoteObject> callerToken,
        const InsightIntentExecuteParam &param);

    /**
     * @brief Execute an intent forwarded from a remote device. DMS only
     * (DMS_UID); want must be a valid insight intent want (contains intent
     * params); target user taken from want param INTENT_USER_ID.
     * @param want The want containing intent execution information.
     * @param srcDeviceId The source device id.
     * @param requestCode The intent request code, used as callback key.
     * @param specifiedFullTokenId Caller token id on the source device, default 0.
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED for non-DMS.
     */
    ErrCode ExecuteIntentForDistributed(const Want &want, const std::string &srcDeviceId,
        uint64_t requestCode, uint64_t specifiedFullTokenId = 0);

    /**
     * @brief Query entity info of a target app via insight intent (background
     * call to its entity provider). Cross-user requires account permission and
     * multi-user concurrency. Result is returned asynchronously via the intent
     * client callback registered with the key.
     * @param key The key of the intent executing client.
     * @param callerToken Caller ability token.
     * @param param The intent query param (bundle/module/intent/class name,
     *        queryType, target user).
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode QueryEntityInfo(uint64_t key, sptr<IRemoteObject> callerToken,
        const InsightIntentQueryParam &param);
     
    /**
     * @brief Execute an insight intent synchronously and wait for the result.
     * Client-side wrapper over ExecuteIntent using a promise; blocks up to
     * timeoutMs (default 30000 ms) and returns INSIGHT_INTENT_EXECUTE_TIMEOUT
     * on timeout (the underlying execution continues).
     * @param param The intent execute param.
     * @param result Output, intent execute result filled by the target app.
     * @param timeoutMs Timeout in milliseconds, default 30000.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ExecuteIntentWithResult(const InsightIntentExecuteParam &param, InsightIntentExecuteResult &result,
        int32_t timeoutMs = 30000);

    /**
     * @brief Report insight intent execution result from the target ability.
     * Self only (JudgeSelfCalled); the reporting ability's bundle must match the
     * bundle stored when the intent was dispatched; URIs in result may be granted
     * to the original caller.
     * @param token Ability's token.
     * @param intentId Insight intent id.
     * @param result Insight intent execute result.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ExecuteInsightIntentDone(sptr<IRemoteObject> token, uint64_t intentId,
        const InsightIntentExecuteResult &result);

    /**
     * @brief Get all foreground UIAbilities (of foreground users). System app
     * only and requires running-info permission
     * (ohos.permission.RUNNING_STATE_PERMISSION), otherwise CHECK_PERMISSION_FAILED.
     * @param list Output, foreground UIAbility state list.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list);

    /**
     * @brief Restore session info after SCB restart: SCB resends the surviving
     * session list and AMS returns the session ids that still have records.
     * SCB only.
     * @param sessionInfos The session info list reported by SCB.
     * @param userId User id (reserved; manager selected by calling uid).
     * @param sessionIds Output, session ids still alive in AMS.
     * @return Returns ERR_OK on success; ERR_WRONG_INTERFACE_CALL if not SCB.
     */
    int32_t UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, int32_t userId,
        std::vector<int32_t> &sessionIds);

    /**
     * @brief Restart the calling application itself (app self-restart).
     * The want must resolve to a PAGE ability of the caller's own bundle/clone
     * (ERR_RESTART_APP_INCORRECT_ABILITY otherwise); caller process must be
     * foreground/focusing (NOT_TOP_ABILITY otherwise); restart frequency is
     * limited (ERR_RESTART_APP_FREQUENT). App clone/multi-instance restart is
     * not supported here.
     * @param want Restart want, must be UIAbility of self bundle.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RestartApp(const AAFwk::Want &want);

    /**
     * @brief Get the root host UIAbility info of a UIExtension window tree.
     * SA or SCB only (ERR_PERMISSION_DENIED otherwise). Falls back to user 0
     * manager when not found in the given user.
     * @param token The UIAbility/UIExtension token.
     * @param hostInfo Output, root host element name and session name.
     * @param userId User id, DEFAULT_INVAL_VALUE means caller's user.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token, UIExtensionHostInfo &hostInfo,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * @brief Get session info of a UIExtension. SA or SCB only.
     * @param token The ability token.
     * @param uiExtensionSessionInfo Output session info.
     * @param userId User id, DEFAULT_INVAL_VALUE means caller's user.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetUIExtensionSessionInfo(const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * @brief Pop-up launch of a full-screen atomic service (jump screen mode).
     * Note: the options parameter is ignored by the server (screen mode forced to
     * JUMP_SCREEN_MODE). Undergoing free-install when the service is not
     * installed. Caller must be foreground for background-start restrictions.
     * @param want The want of the atomic service (bundle or link).
     * @param options Reserved, not used.
     * @param callerToken Caller ability token.
     * @param requestCode Ability request code, default DEFAULT_INVAL_VALUE.
     * @param userId The user ID, default DEFAULT_INVAL_VALUE (caller's user).
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t OpenAtomicService(Want& want, const StartOptions &options, sptr<IRemoteObject> callerToken,
        int32_t requestCode = DEFAULT_INVAL_VALUE, int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * @brief Query whether embedded open of an atomic service is allowed for the
     * caller. Only HAP-token callers on products supporting embedded UIAbility
     * launch; the caller UIAbility must be foreground and own the token; query
     * goes through free-install + ERMS ecological rule.
     * @param callerToken The caller UIAbility token.
     * @param appId The appId of the target atomic service application.
     * @return Returns true to allow, or false to reject.
     */
    bool IsEmbeddedOpenAllowed(sptr<IRemoteObject> callerToken, const std::string &appId);

    /**
     * @brief Request display of the assert fault dialog for a debug app crash.
     * Product must enable assert dialog (PRODUCT_ASSERT_FAULT_DIALOG_ENABLED)
     * and be in developer mode; caller must be a debug app; shows a modal
     * UIExtension and waits for user operation via callback.
     * @param callback Listen for user operation callbacks.
     * @param wantParams Assert dialog display information (ASSERT_FAULT_DETAIL).
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RequestAssertFaultDialog(const sptr<IRemoteObject> &callback, const AAFwk::WantParams &wantParams);

    /**
     * @brief Notify AMS of the user's operation on the assert fault dialog.
     * System app only; requires
     * ohos.permission.NOTIFY_DEBUG_ASSERT_RESULT; product assert dialog must
     * be enabled.
     * @param assertFaultSessionId The request id of the assert fault dialog.
     * @param userStatus Operation status of the user.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyDebugAssertResult(uint64_t assertFaultSessionId, AAFwk::UserStatus userStatus);

    /**
     * @brief Enable/disable the resident (keep-alive) processes configured by the
     * calling application. System app only; the caller may only toggle its own
     * configured resident processes.
     * @param bundleName The bundle name of the resident process.
     * @param enable Set resident process enable status.
     * @return Returns ERR_OK on success; ERR_NOT_SYSTEM_APP for non-system apps.
     */
    int32_t SetResidentProcessEnabled(const std::string &bundleName, bool enable);

    /**
     * @brief Start the ability associated with a shortcut (system launcher use).
     * System app only + requires ohos.permission.START_SHORTCUT.
     * @param want The want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED otherwise.
     */
    int32_t StartShortcut(const Want &want, const StartOptions &startOptions);

    /**
     * @brief Get ability (session) state by persistent id. DMS process only
     * (distributed mission sync); requires SCB enabled, otherwise INNER_ERR.
     * @param persistentId The persistentId (= session id under SCB) of the target.
     * @param state Output, true when foreground/active.
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED for non-DMS.
     */
    int32_t GetAbilityStateByPersistentId(int32_t persistentId, bool &state);

    /**
     * @brief An ExtensionAbility returns a start-for-result result to its caller
     * (transfer to ability result chain). Self only; caller record must be an
     * EXTENSION type ability.
     * @param callerToken The extension ability token that returns the result.
     * @param resultCode The result code.
     * @param want The result want.
     * @return Returns ERR_OK on success; ERR_INVALID_VALUE otherwise.
     */
    int32_t TransferAbilityResultForExtension(const sptr<IRemoteObject> &callerToken, int32_t resultCode,
        const Want &want);

    /**
     * @brief Notify AMS that RSS froze processes; AMS suspends related extension
     * connections. RSS process only (checked by process name); others silently
     * ignored.
     * @param pidList The pid list of the frozen processes.
     * @param uid The uid of the frozen processes.
     */
    void NotifyFrozenProcessByRSS(const std::vector<int32_t> &pidList, int32_t uid);

    /**
     * @brief Open the atomic service window before free install completes
     * (pre-start mission). System app + specific atomic-service pre-start
     * permission (AG) required; a matching free-install task must exist
     * (ERR_FREE_INSTALL_TASK_NOT_EXIST otherwise); if the task already finished,
     * its result code is returned directly.
     * @param bundleName Bundle name of the atomic service.
     * @param moduleName Module name (used for task matching).
     * @param abilityName Ability name of the atomic service.
     * @param startTime Start time key of the free install task.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t PreStartMission(const std::string& bundleName, const std::string& moduleName,
        const std::string& abilityName, const std::string& startTime);

    /**
     * @brief Clean (terminate) a UIAbility session requested by user or system.
     * SCB only. isUserRequestedExit=true records exit reason
     * REASON_USER_REQUEST; sceneFlag carries the termination scene to lifecycle.
     * @param sessionInfo The session info of the ability to clean.
     * @param isUserRequestedExit True when the user requested the exit (default false).
     * @param sceneFlag Termination scene flag, default 0.
     * @return Returns ERR_OK on success; ERR_WRONG_INTERFACE_CALL if not SCB;
     *         ERR_INVALID_VALUE for null sessionInfo; ABILITY_SERVICE_NOT_CONNECTED when
     *         the AMS proxy is unavailable.
     */
    ErrCode CleanUIAbilityBySCB(sptr<SessionInfo> sessionInfo,
        bool isUserRequestedExit = false, uint32_t sceneFlag = 0);

    /**
     * @brief Open a deep link / app link of an ability or atomic service.
     * Routing: explicit normal apps start directly; atomic service links go
     * through free install (background start returns NOT_TOP_ABILITY);
     * app-linking-only links that cannot resolve return RESOLVE_ABILITY_ERR.
     * hideFailureTipDialog suppresses the failure tip dialog.
     * @param want Ability want (uri/link).
     * @param callerToken Caller ability token.
     * @param userId User ID.
     * @param requestCode Ability request code.
     * @param hideFailureTipDialog Hide the failure tip dialog, default false.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t OpenLink(const Want &want, sptr<IRemoteObject> callerToken, int32_t userId, int requestCode,
        bool hideFailureTipDialog = false);

    /**
     * @brief Terminate a mission (clear mission) by id. System app only +
     * ohos.permission.KILL_APP_PROCESSES required. Note: legacy non-SCB mission
     * manager path; on SCB devices mission ids map to sessions.
     * @param missionId The mission id of the UIAbility to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode TerminateMission(int32_t missionId);

    /**
     * @brief Set the global flag to block all app starting (low-memory emergency
     * mode). Requires ohos.permission.BLOCK_ALL_APP_START and a product
     * supporting start-animation options; when the flag is on, StartUser /
     * ability starts fail with ERR_ALL_APP_START_BLOCKED.
     * @param flag True to block all app start.
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED otherwise.
     */
    ErrCode BlockAllAppStart(bool flag);

    /**
     * @brief Update the associate (whitelist) config list used for extension
     * associate-control. RSS process only (RESOURCE_SCHEDULE_UID). flag
     * UPDATE_CONFIG_FLAG_COVER replaces the whole list; UPDATE_CONFIG_FLAG_APPEND
     * appends entries.
     * @param configs The config map (bundle -> associated bundles).
     * @param exportConfigs The export config list.
     * @param flag UPDATE_CONFIG_FLAG_COVER or UPDATE_CONFIG_FLAG_APPEND.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED for non-RSS.
     */
    ErrCode UpdateAssociateConfigList(const std::map<std::string, std::list<std::string>>& configs,
        const std::list<std::string>& exportConfigs, int32_t flag);

    /**
     * @brief Get all insight-intent exemption (schedule exemption) info entries,
     * each with the exempt uid and fixed exemption duration. Internal reporting
     * use; no additional permission check beyond system access.
     * @param info Output, exemption info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetAllIntentExemptionInfo(std::vector<AppExecFwk::IntentExemptionInfo>& info);

    /**
     * @brief Register an observer to receive the ecological-rule (ERMS) query
     * result of an embedded atomic service startup. Caller token must be the
     * ability itself (JudgeSelfCalled).
     * @param callerToken The caller ability token.
     * @param observer The ERMS query result observer.
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED for invalid caller.
     */
    ErrCode AddQueryERMSObserver(sptr<IRemoteObject> callerToken,
        sptr<AbilityRuntime::IQueryERMSObserver> observer);

    /**
     * @brief Query the ERMS startup rule (open/embedded allowed) for an atomic
     * service. Requires product embedded-launch support, HAP caller token,
     * foreground caller ability. When ERMS returns ecological-control status, a
     * selection dialog may be triggered and ERR_ECOLOGICAL_CONTROL_STATUS is
     * returned (result delivered via the registered observer).
     * @param callerToken The caller ability token.
     * @param appId The appId of the target atomic service.
     * @param startTime The start time key of the query (for dialog matching).
     * @param rule Output, the ERMS rule.
     * @return Returns ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT if product
     *         does not support embedded launch.
     */
    ErrCode QueryAtomicServiceStartupRule(sptr<IRemoteObject> callerToken,
        const std::string &appId, const std::string &startTime, AtomicServiceStartupRule &rule);

    /**
     * @brief Restart the calling atomic service itself via free install. Caller
     * must be an atomic service (ERR_CALLER_NOT_ATOMIC_SERVICE otherwise), be
     * foreground (NOT_TOP_ABILITY otherwise) and not exceed restart frequency
     * limit (ERR_RESTART_APP_FREQUENT).
     * @param callerToken The caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
     ErrCode RestartSelfAtomicService(sptr<IRemoteObject> callerToken);

    /**
     * @brief Called by SCB (or the ability) when PrepareTerminateAbility flow is
     * done. Self only; if isTerminate is false the stored callback's
     * DoPrepareTerminate is invoked and the pending timeout task is cancelled.
     * @param token The ability token to terminate.
     * @param isTerminate True if the ability will actually terminate.
     */
    void PrepareTerminateAbilityDone(sptr<IRemoteObject> token, bool isTerminate);

    /**
     * @brief Called by an application module when its prepareTermination callback
     * finishes, unblocking the pending KillProcessWithPrepareTerminate request.
     * Internal use (SCB kill flow).
     * @param moduleName The module name of the application.
     * @param prepareTermination The result of the module's prepareTermination.
     * @param isExist Whether the prepareTerminate function is implemented.
     */
    void KillProcessWithPrepareTerminateDone(const std::string &moduleName, int32_t prepareTermination, bool isExist);

    /**
     * @brief Force kill all processes of the given accessTokenId after a
     * permission update, record exit reason REASON_PERMISSION_UPDATE and notify
     * SCB. Requires ohos.permission.KILL_APP_PROCESSES.
     * @param accessTokenId The access token id of the target app.
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED otherwise.
     */
    ErrCode KillProcessForPermissionUpdate(uint32_t accessTokenId);

    /**
     * @brief Register an observer for hidden-start (start ability to hidden
     * window) events. Requires
     * ohos.permission.START_UIABILITY_TO_HIDDEN_WINDOW (or observer permission
     * set).
     * @param observer The hidden start observer.
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED otherwise.
     */
    ErrCode RegisterHiddenStartObserver(const sptr<IHiddenStartObserver> &observer);

    /**
     * @brief Unregister the hidden start observer. Same permission requirement
     * as registration.
     * @param observer The observer to remove.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnregisterHiddenStartObserver(const sptr<IHiddenStartObserver> &observer);

    /**
     * @brief Query how many preloaded UIExtension records exist for a
     * bundle/ability. System app only +
     * ohos.permission.PRELOAD_UI_EXTENSION_ABILITY required.
     * @param element The UIExtension ElementName (bundle/ability name).
     * @param moduleName The module name (used with element for matching).
     * @param hostPid The caller host pid to match (-1 for any).
     * @param recordNum Output, count of preload records.
     * @param userId User id, default DEFAULT_INVAL_VALUE (caller's user).
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED otherwise.
     */
    ErrCode QueryPreLoadUIExtensionRecord(const AppExecFwk::ElementName &element,
                                          const std::string &moduleName,
                                          const int32_t hostPid,
                                          int32_t &recordNum,
                                          int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * @brief Revoke the delegator permission of a UIAbility token (undo
     * delegation). SCB-enabled products only; caller must be a HAP token.
     * @param token Ability token whose delegation is revoked.
     * @return Returns ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT on non-SCB;
     *         ERR_NO_PERMISSION_CALLER for non-HAP callers.
     */
    ErrCode RevokeDelegator(sptr<IRemoteObject> token);

    /**
     * @brief Get all insight intent infos of the target user.
     * Requires system-app/permission check (CheckGetInsightIntenInfoPermission);
     * cross-user query requires account permission + multi-user concurrency.
     * flag: GET_FULL_INSIGHT_INTENT returns full intent list; GET_SUMMARY_
     * INSIGHT_INTENT returns generic info; GET_ENTITY_INFO (bit-or) additionally
     * returns entity info.
     * @param flag The get type, combination of GetInsightIntentFlag bits.
     * @param infos Output, intent info list.
     * @param userId User id, default DEFAULT_INVAL_VALUE (caller's user).
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetAllInsightIntentInfo(
        AbilityRuntime::GetInsightIntentFlag flag,
        std::vector<InsightIntentInfoForQuery> &infos,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * @brief Get insight intent infos of one bundle. Requires system app or the
     * caller itself being the queried bundle (permission check with bundleName);
     * cross-user rules apply as in GetAllInsightIntentInfo.
     * @param flag The get type (see GetAllInsightIntentInfo).
     * @param bundleName The bundle to query.
     * @param infos Output, intent info list.
     * @param userId User id, default DEFAULT_INVAL_VALUE.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetInsightIntentInfoByBundleName(
        AbilityRuntime::GetInsightIntentFlag flag,
        const std::string &bundleName,
        std::vector<InsightIntentInfoForQuery> &infos,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * @brief Get one intent info by bundle/module/intent name. Same permission
     * rules as GetInsightIntentInfoByBundleName; config-file intents take
     * priority over extracted intents.
     * @param flag The get type (see GetAllInsightIntentInfo).
     * @param bundleName The bundle to query.
     * @param moduleName The module to query.
     * @param intentName The intent name to query.
     * @param info Output, matched intent info (empty when not found).
     * @param userId User id, default DEFAULT_INVAL_VALUE.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetInsightIntentInfoByIntentName(
        AbilityRuntime::GetInsightIntentFlag flag,
        const std::string &bundleName,
        const std::string &moduleName,
        const std::string &intentName,
        InsightIntentInfoForQuery &info,
        int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * @brief Update the kiosk application list (allowed apps in kiosk mode).
     * Internal use by the kiosk management subsystem (EDM); the server relies on
     * the caller being the configured kiosk manager.
     * @param appList Bundle name list allowed in kiosk mode.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UpdateKioskApplicationList(const std::vector<std::string> &appList);

    /**
     * @brief Enter kiosk mode. The caller must be the ability owning the token
     * (self only).
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED if not self.
     */
    ErrCode EnterKioskMode(sptr<IRemoteObject> callerToken);

    /**
     * @brief Exit kiosk mode. Self only (same as EnterKioskMode).
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ExitKioskMode(sptr<IRemoteObject> callerToken);

    /**
     * @brief Get current kiosk mode status.
     * @param kioskStatus Output, current kiosk status.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetKioskStatus(AAFwk::KioskStatus &kioskStatus);

    /**
     * @brief Register a start-interceptor SA (ability start rule interception).
     * Internal: only specific partner uids (PENG_LAI_UID / TASKMGR_UID) allowed;
     * others get CHECK_PERMISSION_FAILED.
     * @param interceptor The SA interceptor.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterSAInterceptor(sptr<AbilityRuntime::ISAInterceptor> interceptor);

    /**
     * @brief Unregister a start-interceptor SA. TASKMGR_UID only.
     * @param interceptor The interceptor remote object to remove.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED otherwise.
     */
    ErrCode UnregisterSAInterceptor(sptr<IRemoteObject> interceptor);

    /**
     * @brief Suspend (freeze) a connected service-extension session.
     * SA only; the connection must exist in AMS.
     * @param connect The IAbilityConnection used when connecting.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED for non-SA;
     *         CONNECT_MAMAGER_NOT_FIND_BY_USERID if connection not found.
     */
    ErrCode SuspendExtensionAbility(sptr<IAbilityConnection> connect);

    /**
     * @brief Resume a suspended service-extension session. SA only.
     * @param connect The IAbilityConnection used when connecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ResumeExtensionAbility(sptr<IAbilityConnection> connect);

    /**
     * @brief Set the scenarios in which onNewWant is skipped for the caller
     * UIAbility. Self only; non-UIAbility records are accepted but ignored.
     * @param callerToken Caller ability token.
     * @param scenarios Bit flags of skip scenarios.
     * @return Returns ERR_OK on success; ERR_INVALID_CONTEXT/ERR_INVALID_CALLER
     *         for invalid token.
     */
    ErrCode SetOnNewWantSkipScenarios(sptr<IRemoteObject> callerToken, int32_t scenarios);

    /**
     * @brief SCB notifies AMS that a startup request was intercepted/failed on
     * the SCB side, so AMS can clean the pending request. SCB only.
     * @param requestId The pending start request id.
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED if not SCB.
     */
    ErrCode NotifyStartupExceptionBySCB(int32_t requestId);

    /**
     * @brief Preload an application to a configured phase (process / ability
     * stage / window stage). Requires preload permission
     * (VerifyPreloadApplicationPermission) and product enablement
     * (IsPreloadApplicationEnabled). Reserved: only appIndex = 0 is supported
     * (any other value returns ERR_CAPABILITY_NOT_SUPPORT). Native abilities are
     * not supported; repeated preload of a started app returns
     * ERR_PRELOAD_APP_RECORD_ALREADY_EXIST; window-stage preload requires
     * STANDARD launch mode.
     * @param bundleName Name of the application.
     * @param userId User id.
     * @param appIndex Reserved, only 0 is supported.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode PreloadApplication(const std::string &bundleName, int32_t userId, int32_t appIndex);

    /**
     * @brief Start another UIAbility of the caller's own bundle in the current
     * process (in-process UIAbility). Product support required
     * (IsStartUIAbilityInCurrentProcess); explicit bundle+ability names required
     * (no implicit start); app clone / multi-instance targets rejected; caller
     * process must be foreground. specifiedFlag is only used when the target
     * launch mode is SPECIFIED; hasOptions tells whether startOptions is valid.
     * @param want Ability want, must be explicit and same bundle.
     * @param specifiedFlag Specified-mode instance key flag.
     * @param startOptions Start options used when hasOptions is true.
     * @param hasOptions Whether the startOptions parameter is valid.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT when the
     *         product does not support it.
     */
    ErrCode StartSelfUIAbilityInCurrentProcess(const Want &want, const std::string &specifiedFlag,
        const AAFwk::StartOptions &startOptions, bool hasOptions, sptr<IRemoteObject> callerToken);

    /**
     * @brief Cancel game pre-launch and kill the prelaunched process. SCB must
     * be enabled (ERR_CAPABILITY_NOT_SUPPORT otherwise); the target ability is
     * identified by callerToken.
     * @param callerToken The caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode NotifyCancelGamePreLaunch(const sptr<IRemoteObject> callerToken);

    /**
     * @brief Notify that game pre-launch is complete and clear the prelaunch
     * flag. SCB must be enabled.
     * @param callerToken The caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode NotifyCompleteGamePreLaunch(const sptr<IRemoteObject> callerToken);

    /**
     * @brief Start a self-bundle UIAbility in a new child process attached to
     * the parent. Product support (IsSupportNativeUIAbility) required; explicit
     * same-bundle want only; app clone / multi-instance rejected; caller ability
     * must be foreground. specifiedFlag applies to SPECIFIED launch mode.
     * @param want Ability want, explicit same bundle.
     * @param specifiedFlag Specified-mode instance key flag.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success; ERR_CAPABILITY_NOT_SUPPORT when
     *         unsupported.
     */
    ErrCode StartSelfUIAbilityInChildProcess(
        const Want &want, const std::string &specifiedFlag, sptr<IRemoteObject> callerToken);

    /**
     * @brief Check whether the calling app is within the restart-frequency limit
     * window (restart too frequently). Client-only check against the same
     * restart history the server uses.
     * @return Returns true if restart is currently limited.
     */
    bool IsRestartAppLimit();

    /**
     * @brief Clear (unload) one preloaded UIExtension by its record id.
     * System app + ohos.permission.PRELOAD_UI_EXTENSION_ABILITY required.
     * @param extensionAbilityId The preload record id returned by preload.
     * @param userId User id, default DEFAULT_INVAL_VALUE (caller's user).
     * @return Returns ERR_OK on success; ERR_PERMISSION_DENIED otherwise.
     */
    ErrCode ClearPreloadedUIExtensionAbility(int32_t extensionAbilityId, int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * @brief Clear all preloaded UIExtension records of a user. System app +
     * ohos.permission.PRELOAD_UI_EXTENSION_ABILITY required.
     * @param userId User id, default DEFAULT_INVAL_VALUE (caller's user).
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ClearPreloadedUIExtensionAbilities(int32_t userId = DEFAULT_INVAL_VALUE);

    /**
     * @brief Register the caller as a preload UIExtension host client so cached
     * UIExtensions can be reused. System app +
     * ohos.permission.PRELOAD_UI_EXTENSION_ABILITY required; callerToken must be
     * non-null.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken);

    /**
     * @brief Unregister a preload UIExtension host client by pid.
     * System app + ohos.permission.PRELOAD_UI_EXTENSION_ABILITY required.
     * @param callerPid Pid of the host client, DEFAULT_INVAL_VALUE for caller
     *        itself.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnRegisterPreloadUIExtensionHostClient(int32_t callerPid = DEFAULT_INVAL_VALUE);

    /**
     * @brief Query modular-object extension infos of the calling application
     * itself (by caller uid -> bundleName + appIndex). Product support for
     * modular object extension required (ERR_CAPABILITY_NOT_SUPPORT otherwise).
     * @param extensionInfos Output, queried extension infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode QuerySelfModularObjectExtensionInfos(std::vector<ModularObjectExtensionInfo> &extensionInfos);

    /**
     * @brief Get bundles launched before the first unlock of a user (locked
     * boot phase). Foundation process only.
     * @param userId The User Id.
     * @param userLockedBundleList Output, bundle list launched before unlock.
     * @return Returns ERR_OK on success; CHECK_PERMISSION_FAILED for non-foundation.
     */
    ErrCode GetUserLockedBundleList(int32_t userId, std::unordered_set<std::string> &userLockedBundleList);

    /**
     * @brief Set the app recovery/restart flag for the caller's own ability
     * (see AppExecFwk::RestartFlag). Self only; when restart-on-crash flags are
     * set a death recipient is installed so AMS can restart the app after a
     * CPP crash (rate limited to once per minute).
     * @param token Ability token, must belong to caller.
     * @param flag Restart flag combination, see AppExecFwk::RestartFlag.
     * @return Returns ERR_OK on success; ERR_INVALID_VALUE for invalid token.
     */
    int32_t SetAppRecoveryFlag(const sptr<IRemoteObject>& token, int flag);

    /**
     * @brief Execute an in-app skill (skill entry defined in the target bundle).
     * The skill config is queried from bundle framework; caller permission is
     * verified against the skill's configured allow rules (CheckSkillPermission).
     * Result is delivered asynchronously via callback with a requestCode.
     * @param bundleName Target bundle name.
     * @param moduleName Target module name.
     * @param skillName Target skill name.
     * @param arkTSPath Optional skill script path override, default "".
     * @param funcName Optional function name override, default "".
     * @param skillArgs Optional skill arguments, default nullptr.
     * @param callback Optional result callback, default nullptr.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ExecuteInAppSkill(const std::string &bundleName, const std::string &moduleName,
        const std::string &skillName, const std::string &arkTSPath = "",
        const std::string &funcName = "",
        const std::shared_ptr<AAFwk::WantParams> &skillArgs = nullptr,
        const sptr<ISkillExecuteCallback> &callback = nullptr);

    /**
     * @brief Execute an in-app skill on behalf of an explicit caller token id
     * (used by broker/framework scenarios). The request.callerTokenId must
     * resolve to a valid HAP token; permission is checked with that identity.
     * @param request Skill execute request (bundle/module/skill, script path,
     *        function name, args, callerTokenId).
     * @param callback Result callback.
     * @return Returns ERR_OK on success; ERR_INVALID_VALUE for bad token.
     */
    ErrCode ExecuteInAppSkillWithTokenId(const AppExecFwk::SkillExecuteRequest &request,
        const sptr<ISkillExecuteCallback> &callback);

    /**
     * @brief Called by the target ability when a skill execution finishes;
     * delivers the result to the waiting callback registered by the requester.
     * Self only; requestCode must match the execute record.
     * @param token The executing ability's token.
     * @param requestCode The skill execute request code.
     * @param resultCode Result code of the execution.
     * @param result The skill execute result payload.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ExecuteSkillDone(sptr<IRemoteObject> token, const std::string &requestCode,
        int32_t resultCode, const AppExecFwk::SkillExecuteResult &result);

    /**
     * @brief Notify that the skill function of the target ability has actually
     * been invoked (launch completed), unblocking the pending callback.
     * Self only.
     * @param token The target ability token.
     * @param requestCode The skill execute request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode NotifySkillFunctionInvoked(sptr<IRemoteObject> token, const std::string &requestCode);

    /**
     * @brief Query the type of a configured skill (e.g. UI ability vs service
     * extension). No special permission; queried for the caller's user.
     * @param bundleName Target bundle name.
     * @param moduleName Target module name.
     * @param skillName Target skill name.
     * @param skillType Output, skill type (AppExecFwk::SkillType).
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode QuerySkillType(const std::string &bundleName, const std::string &moduleName,
        const std::string &skillName, int32_t &skillType);

private:
    AbilityManagerClient();
    DISALLOW_COPY_AND_MOVE(AbilityManagerClient);

    class AbilityMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        AbilityMgrDeathRecipient() = default;
        ~AbilityMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        DISALLOW_COPY_AND_MOVE(AbilityMgrDeathRecipient);
    };

    sptr<IAbilityManager> GetAbilityManager();
    void ResetProxy(wptr<IRemoteObject> remote);
    void HandleDlpApp(Want &want);

    static std::once_flag singletonFlag_;
    static std::shared_ptr<AbilityManagerClient> instance_;
    sptr<IAbilityManager> proxy_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    std::recursive_mutex mutex_;
    std::mutex topAbilityMutex_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H
