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
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "window_manager_service_handler_stub.h"
#include "windowmanagerservicehandlerstub_fuzzer.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class WindowManagerServiceHandlerStubFuzz : public WindowManagerServiceHandlerStub {
public:
    void NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo, sptr<AbilityTransitionInfo> toInfo,
        bool &animaEnabled) override {};
    int32_t GetFocusWindow(sptr<IRemoteObject> &abilityToken) override
    {
        return 0;
    }
    void StartingWindow(sptr<AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap,
        uint32_t bgColor) override {};
    void StartingWindow(sptr<AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap) override {};
    void CancelStartingWindow(sptr<IRemoteObject> abilityToken) override {};
    void NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info) override {};
    int32_t MoveMissionsToForeground(const std::vector<int32_t> &missionIds, int32_t topMissionId) override
    {
        return 0;
    }
    int32_t MoveMissionsToBackground(const std::vector<int32_t> &missionIds,
        std::vector<int32_t> &result) override
    {
        return 0;
    }
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 5) {
        case 0: {
            actualCode =
                static_cast<uint32_t>(IWindowManagerServiceHandler::WMSCmd::ON_NOTIFY_WINDOW_TRANSITION);
            AbilityTransitionInfo fromInfo;
            parcel.WriteParcelable(&fromInfo);
            AbilityTransitionInfo toInfo;
            parcel.WriteParcelable(&toInfo);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 1: {
            actualCode =
                static_cast<uint32_t>(IWindowManagerServiceHandler::WMSCmd::ON_GET_FOCUS_ABILITY);
            break;
        }
        case 2: {
            actualCode =
                static_cast<uint32_t>(IWindowManagerServiceHandler::WMSCmd::ON_CANCEL_STARTING_WINDOW);
            bool flag = fdp.ConsumeBool();
            parcel.WriteBool(flag);
            if (flag) {
                parcel.WriteRemoteObject(nullptr);
            }
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(
                IWindowManagerServiceHandler::WMSCmd::ON_NOTIFY_ANIMATION_ABILITY_DIED);
            AbilityTransitionInfo info;
            parcel.WriteParcelable(&info);
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(
                IWindowManagerServiceHandler::WMSCmd::ON_MOVE_MISSINONS_TO_FOREGROUND);
            parcel.WriteInt32Vector(FuzzUtil::BuildMaliciousInt32Vector(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(WindowManagerServiceHandlerStubFuzz, FuzzUtil::Tokens::WMS_HANDLER)
} // namespace OHOS
