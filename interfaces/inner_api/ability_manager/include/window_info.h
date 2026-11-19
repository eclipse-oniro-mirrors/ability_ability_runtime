/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_WINDOW_INFO_H
#define OHOS_ABILITY_RUNTIME_WINDOW_INFO_H

#ifdef SUPPORT_GRAPHICS
#include <typeinfo>

#include "ability_info.h"
#include "iremote_object.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
namespace {
    constexpr int32_t WINDOW_MODE_MAX_SIZE = 4;
}

enum class TransitionReason : uint32_t {
    MINIMIZE = 0,
    CLOSE,
    ABILITY_TRANSITION,
    BACK_TRANSITION,
    CLOSE_BUTTON,
    BACKGROUND_TRANSITION,
};

enum BackgroundReason : int32_t {
    BACKGROUND_REASON_UNKNOWN = 0,
    BACKGROUND_REASON_SCREEN_OFF = 3, //WMS already has enumeration values for screen_off
};

struct AbilityTransitionInfo : public Parcelable {
    bool isShowWhenLocked_ = false;
    bool isRecent_ = false;
    uint32_t mode_ = 0;
    uint32_t maxWindowWidth_;
    uint32_t minWindowWidth_;
    uint32_t maxWindowHeight_;
    uint32_t minWindowHeight_;
    uint32_t apiCompatibleVersion_ = 0;
    int32_t missionId_;
    TransitionReason reason_ = TransitionReason::ABILITY_TRANSITION;
    uint64_t displayId_ = 0;
    double maxWindowRatio_;
    double minWindowRatio_;
    sptr<IRemoteObject> abilityToken_ = nullptr;
    std::string bundleName_;
    std::string abilityName_;
    std::vector<AppExecFwk::SupportWindowMode> windowModes_;
    
    AppExecFwk::DisplayOrientation orientation_ = AppExecFwk::DisplayOrientation::UNSPECIFIED;

    virtual bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteString(bundleName_)) {
            return false;
        }

        if (!parcel.WriteString(abilityName_)) {
            return false;
        }

        if (!parcel.WriteUint32(mode_)) {
            return false;
        }

        if (!WriteAbilityToken(parcel)) {
            return false;
        }

        if (!(parcel.WriteUint64(displayId_) && parcel.WriteBool(isShowWhenLocked_) && parcel.WriteBool(isRecent_))) {
            return false;
        }

        auto size = windowModes_.size();
        if (size > 0 && size <= WINDOW_MODE_MAX_SIZE) {
            if (!parcel.WriteUint32(static_cast<uint32_t>(size))) {
                return false;
            }
            for (decltype(size) i = 0; i < size; i++) {
                if (!parcel.WriteUint32(static_cast<uint32_t>(windowModes_[i]))) {
                    return false;
                }
            }
        } else {
            if (!parcel.WriteUint32(0)) {
                return false;
            }
        }

        if (!WriteWindowInfo(parcel)) {
            return false;
        }

        if (!parcel.WriteInt32(missionId_)) {
            return false;
        }

        if (!parcel.WriteUint32(static_cast<uint32_t>(reason_))) {
            return false;
        }

        if (!parcel.WriteUint32(static_cast<uint32_t>(orientation_))) {
            return false;
        }

        if (!parcel.WriteUint32(static_cast<uint32_t>(apiCompatibleVersion_))) {
            return false;
        }
        return true;
    }

    bool WriteAbilityToken(Parcel& parcel) const
    {
        if (!abilityToken_) {
            if (!parcel.WriteBool(false)) {
                return false;
            }
        } else {
            if (!parcel.WriteBool(true)) {
                return false;
            }
            if (!parcel.WriteRemoteObject(abilityToken_)) {
                return false;
            }
        }

        return true;
    }

    bool WriteWindowInfo(Parcel& parcel) const
    {
        return (parcel.WriteDouble(maxWindowRatio_) && parcel.WriteDouble(minWindowRatio_) &&
            parcel.WriteUint32(maxWindowWidth_) && parcel.WriteUint32(minWindowWidth_) &&
            parcel.WriteUint32(maxWindowHeight_) && parcel.WriteUint32(minWindowHeight_));
    }

    static AbilityTransitionInfo* Unmarshalling(Parcel& parcel)
    {
        AbilityTransitionInfo* info = new AbilityTransitionInfo();
        info->bundleName_ = parcel.ReadString();
        info->abilityName_ = parcel.ReadString();
        info->mode_ = parcel.ReadUint32();
        if (parcel.ReadBool()) {
            info->abilityToken_ = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
        }
        info->displayId_ = parcel.ReadUint64();
        info->isShowWhenLocked_ = parcel.ReadBool();
        info->isRecent_ = parcel.ReadBool();
        auto size = parcel.ReadUint32();
        if (size > 0 && size <= WINDOW_MODE_MAX_SIZE) {
            for (decltype(size) i = 0; i < size; i++) {
                info->windowModes_.push_back(static_cast<AppExecFwk::SupportWindowMode>(parcel.ReadUint32()));
            }
        }
        info->maxWindowRatio_ = parcel.ReadDouble();
        info->minWindowRatio_ = parcel.ReadDouble();
        info->maxWindowWidth_ = parcel.ReadUint32();
        info->minWindowWidth_ = parcel.ReadUint32();
        info->maxWindowHeight_ = parcel.ReadUint32();
        info->minWindowHeight_ = parcel.ReadUint32();
        info->missionId_ = parcel.ReadInt32();
        info->reason_ = static_cast<TransitionReason>(parcel.ReadUint32());
        info->orientation_ = static_cast<AppExecFwk::DisplayOrientation>(parcel.ReadUint32());
        info->apiCompatibleVersion_ = parcel.ReadUint32();
        return info;
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif
#endif // OHOS_ABILITY_RUNTIME_WINDOW_INFO_H
