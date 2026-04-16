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

#ifndef MOCK_SCENE_SESSION_MANAGER_LITE_H
#define MOCK_SCENE_SESSION_MANAGER_LITE_H

#include <gmock/gmock.h>
#include <iremote_broker.h>
#include "common/include/window_session_property.h"
#include "iability_manager_collaborator.h"
#include "interfaces/include/ws_common.h"
#include "interfaces/include/ws_common_inner.h"
#include "mission_info.h"
#include "mission_listener_interface.h"
#include "mission_snapshot.h"
#include "session_info.h"
#include "session_lifecycle_listener_interface.h"
#include "session_manager/include/zidl/scene_session_manager_lite_interface.h"
#include "zidl/window_manager_lite_interface.h"
namespace OHOS::Media {
class PixelMap;
} // namespace OHOS::Media

namespace OHOS::Rosen {
using ISessionListener = AAFwk::IMissionListener;
using SessionInfoBean = AAFwk::MissionInfo;
using SessionSnapshot = AAFwk::MissionSnapshot;
class MockSceneSessionManagerLite : public OHOS::Rosen::ISceneSessionManagerLite {
public:
    MOCK_METHOD(WSError, PendingSessionToForeground,
        (const sptr<IRemoteObject>& token, int32_t windowMode), (override));
    MOCK_METHOD(WSError, PendingSessionToBackgroundForDelegator,
        (const sptr<IRemoteObject>& token, bool shouldBackToCaller, int32_t reason), (override));
    MOCK_METHOD(WSError, MoveSessionsToForeground,
        (const std::vector<std::int32_t>& sessionIds, int32_t topSessionId), (override));
    MOCK_METHOD(WSError, MoveSessionsToBackground,
        (const std::vector<std::int32_t>& sessionIds, std::vector<std::int32_t>& result), (override));
    MOCK_METHOD(WSError, TerminateSessionNew,
        (const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker), (override));
    MOCK_METHOD(WSError, ClearSession, (int32_t persistentId), (override));
    MOCK_METHOD(WSError, ClearAllSessions, (), (override));
    MOCK_METHOD(WSError, SetSessionLabel, (const sptr<IRemoteObject>& token, const std::string& label), (override));
    MOCK_METHOD(WSError, SetSessionIcon,
        (const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon), (override));
    MOCK_METHOD(WSError, RegisterIAbilityManagerCollaborator,
        (int32_t type, const sptr<AAFwk::IAbilityManagerCollaborator>& impl), (override));
    MOCK_METHOD(WSError, UnregisterIAbilityManagerCollaborator, (int32_t type), (override));
    MOCK_METHOD(WSError, RegisterSessionListener,
        (const sptr<ISessionListener>& listener, bool isRecover), (override));
    MOCK_METHOD(WSError, UnRegisterSessionListener, (const sptr<ISessionListener>& listener), (override));
    MOCK_METHOD(WSError, GetSessionInfos,
        (const std::string& deviceId, int32_t numMax, std::vector<SessionInfoBean>& sessionInfos), (override));
    MOCK_METHOD(WSError, GetSessionInfo,
        (const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo), (override));
    MOCK_METHOD(WSError, GetSessionInfo,
        (const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo,
        AAFwk::DisplayInfo& displayInfo), (override));
    MOCK_METHOD(WSError, GetSessionInfoByContinueSessionId,
        (const std::string& continueSessionId, SessionInfoBean& sessionInfo), (override));
    MOCK_METHOD(WSError, SetSessionContinueState,
        (const sptr<IRemoteObject>& token, const ContinueState& continueState), (override));
    MOCK_METHOD(WSError, IsValidSessionIds,
        (const std::vector<int32_t>& sessionIds, std::vector<bool>& results), (override));
    MOCK_METHOD(WSError, GetFocusSessionToken, (sptr<IRemoteObject>& token, DisplayId displayId), (override));
    MOCK_METHOD(WSError, GetFocusSessionElement, (AppExecFwk::ElementName& element, DisplayId displayId), (override));
    MOCK_METHOD(WSError, GetSessionSnapshot,
        (const std::string& deviceId, int32_t persistentId,
         SessionSnapshot& snapshot, bool isLowResolution), (override));
    MOCK_METHOD(WSError, LockSession, (int32_t sessionId), (override));
    MOCK_METHOD(WSError, UnlockSession, (int32_t sessionId), (override));
    MOCK_METHOD(WSError, RaiseWindowToTop, (int32_t persistentId), (override));
    MOCK_METHOD(WMError, GetWindowStyleType, (WindowStyleType& windowStyleType), (override));
    MOCK_METHOD(WMError, ListWindowInfo,
        (const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos), (override));
    MOCK_METHOD(WSError, NotifyAppUseControlList,
        (ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList), (override));
    MOCK_METHOD(WSError, GetMainWindowStatesByPid,
        (int32_t pid, std::vector<MainWindowState>& windowStates), (override));
    MOCK_METHOD(WMError, MinimizeMainSession,
        (const std::string& bundleName, int32_t appIndex, int32_t userId), (override));
    MOCK_METHOD(WMError, LockSessionByAbilityInfo, (const AbilityInfoBase& abilityInfo, bool isLock), (override));
    MOCK_METHOD(WMError, HasFloatingWindowForeground,
        (const sptr<IRemoteObject>& abilityToken, bool& hasOrNot), (override));
    MOCK_METHOD(WMError, RegisterSessionLifecycleListenerByIds,
        (const sptr<ISessionLifecycleListener>& listener, const std::vector<int32_t>& persistentIdList), (override));
    MOCK_METHOD(WMError, RegisterSessionLifecycleListenerByBundles,
        (const sptr<ISessionLifecycleListener>& listener,
         const std::vector<std::string>& bundleNameList), (override));
    MOCK_METHOD(WMError, UnregisterSessionLifecycleListener,
        (const sptr<ISessionLifecycleListener>& listener), (override));
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, (), (override));
    MOCK_METHOD(WMError, RegisterWindowManagerAgent,
        (WindowManagerAgentType type, const sptr<IWindowManagerAgent>& agent), (override));
    MOCK_METHOD(WMError, UnregisterWindowManagerAgent,
        (WindowManagerAgentType type, const sptr<IWindowManagerAgent>& agent), (override));
    MOCK_METHOD(void, GetFocusWindowInfo, (FocusChangeInfo& focusInfo, DisplayId displayId), (override));
    MOCK_METHOD(WMError, CheckWindowId, (int32_t windowId, int32_t& pid), (override));
    MOCK_METHOD(WMError, CheckUIExtensionCreation,
        (int32_t windowId, uint32_t tokenId, const AppExecFwk::ElementName& element,
         AppExecFwk::ExtensionAbilityType extensionAbilityType, int32_t& pid), (override));
    MOCK_METHOD(WMError, GetMainWindowInfos, (int32_t topNum, std::vector<MainWindowInfo>& topNInfo), (override));
    MOCK_METHOD(WMError, GetCallingWindowInfo, (CallingWindowInfo& callingWindowInfo), (override));
    MOCK_METHOD(WMError, GetAllMainWindowInfos, (std::vector<MainWindowInfo>& infos), (override));
    MOCK_METHOD(WMError, ClearMainSessions,
        (const std::vector<int>& persistentIds, std::vector<int32_t>& clearFailedIds), (override));
    MOCK_METHOD(WMError, TerminateSessionByPersistentId, (int32_t persistentId), (override));
    MOCK_METHOD(WMError, CloseTargetFloatWindow, (const std::string& bundleName), (override));
    MOCK_METHOD(WMError, CloseTargetPiPWindow, (const std::string& bundleName), (override));
    MOCK_METHOD(WMError, GetCurrentPiPWindowInfo, (std::string& bundleName), (override));
    MOCK_METHOD(WMError, GetRootMainWindowId, (int32_t persistentId, int32_t& hostWindowId), (override));
    MOCK_METHOD(WMError, GetAccessibilityWindowInfo,
        (std::vector<sptr<AccessibilityWindowInfo>>& infos), (override));
    WSError GetRecentMainSessionInfoList(std::vector<RecentSessionInfo>& recentSessionInfoList) override {
        return WSError::WS_OK;
    }
    MOCK_METHOD(WMError, CreateNewInstanceKey, (const std::string& bundleName, std::string& instanceKey), (override));
    MOCK_METHOD(WMError, RemoveInstanceKey, (const std::string& bundleName, const std::string& instanceKey), (override));
    MOCK_METHOD(WSError, PendingSessionToBackground,
        (const sptr<IRemoteObject>& token, const BackgroundParams& params), (override));
    MOCK_METHOD(WMError, TransferSessionToTargetScreen, (const TransferSessionInfo& info), (override));
    MOCK_METHOD(WMError, GetRouterStackInfo, (int32_t persistentId,
        const sptr<ISessionRouterStackListener>& listener), (override));
    MOCK_METHOD(WSError, IsFocusWindowParent, (const sptr<IRemoteObject>& token, bool& isParent), (override));
    MOCK_METHOD(WMError, GetDisplayIdByWindowId,
        ((const std::vector<uint64_t> &windowIds), (std::unordered_map<uint64_t, DisplayId> & windowDisplayIdMap)),
        (override));
    MOCK_METHOD(WMError, GetParentMainWindowId, (int32_t windowIds, int32_t& mainWindowId), (override));
    MOCK_METHOD(WSError, SetSessionIconForThirdParty, (const sptr<IRemoteObject>& token,
        const std::shared_ptr<Media::PixelMap>& icon), (override));
    MOCK_METHOD(WMError, GetMainWindowInfoByToken, (const sptr<IRemoteObject>& abilityToken,
        MainWindowInfo& windowInfo), (override));
    MOCK_METHOD(void, GetAllGroupInfo, ((std::unordered_map<DisplayId, DisplayGroupId> &),
        (std::vector<sptr<FocusChangeInfo>> &)), (override));
};
} // namespace OHOS::Rosen
#endif // MOCK_SCENE_SESSION_MANAGER_LITE_H
