/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ecological_rule/ability_ecological_rule_mgr_service_param.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace EcologicalRuleMgrService {

AbilityExperienceRule *AbilityExperienceRule::Unmarshalling(Parcel &in)
{
    auto *rule = new (std::nothrow) AbilityExperienceRule();
    if (rule == nullptr) {
        return nullptr;
    }

    if (!in.ReadInt32(rule->resultCode)) {
        TAG_LOGW(AAFwkTag::ECOLOGICAL_RULE, "read resultCode failed");
        delete rule;
        return nullptr;
    }

    if (!in.ReadString(rule->sceneCode)) {
        TAG_LOGW(AAFwkTag::ECOLOGICAL_RULE, "read sceneCode failed");
        delete rule;
        return nullptr;
    }

    rule->replaceWant = in.ReadParcelable<Want>();

    if (!in.ReadBool(rule->isBackSkuExempt)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read isBackSkuExempt failed");
        rule->isBackSkuExempt = true;
    }

    if (!in.ReadInt32(rule->embedResultCode)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read embedResultCode failed");
        rule->embedResultCode = 1;
    }

    return rule;
}

bool AbilityExperienceRule::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write resultCode failed");
        return false;
    }

    if (!parcel.WriteString(sceneCode)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write sceneCode failed");
        return false;
    }
    if (!parcel.WriteParcelable(replaceWant)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write replaceWant failed");
        return false;
    }

    if (!parcel.WriteBool(isBackSkuExempt)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write isBackSkuExempt failed");
        return false;
    }

    if (!parcel.WriteInt32(embedResultCode)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write embedResultCode failed");
        return false;
    }

    return true;
}

bool AbilityCallerInfo::ReadFromParcel(Parcel &parcel)
{
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "read from parcel");
    return true;
}

AbilityCallerInfo *AbilityCallerInfo::Unmarshalling(Parcel &in)
{
    auto *info = new (std::nothrow) AbilityCallerInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "info null");
        return nullptr;
    }

    info->packageName = in.ReadString();
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "read packageName result: %{public}s", info->packageName.c_str());

    if (!in.ReadInt32(info->uid)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read uid failed");
        delete info;
        return nullptr;
    }

    if (!in.ReadInt32(info->pid)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read pid failed");
        delete info;
        return nullptr;
    }

    if (!in.ReadInt32(info->callerAppType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read callerAppType failed");
        delete info;
        return nullptr;
    }

    if (!in.ReadInt32(info->targetAppType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read targetAppType failed");
        delete info;
        return nullptr;
    }

    if (!in.ReadInt32(info->callerModelType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read callerModelType failed");
        delete info;
        return nullptr;
    }

    info->targetAppDistType = in.ReadString();
    info->targetLinkFeature = in.ReadString();

    if (!in.ReadInt32(info->targetLinkType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read targetLinkType failed");
        delete info;
        return nullptr;
    }

    info->callerAbilityType = static_cast<AppExecFwk::AbilityType>(in.ReadInt32());

    info->embedded = in.ReadInt32();
    info->callerAppProvisionType = in.ReadString();
    info->targetAppProvisionType = in.ReadString();
    info->callerExtensionAbilityType = static_cast<AppExecFwk::ExtensionAbilityType>(in.ReadInt32());
    info->targetAbilityType = static_cast<AppExecFwk::AbilityType>(in.ReadInt32());
    info->targetExtensionAbilityType = static_cast<AppExecFwk::ExtensionAbilityType>(in.ReadInt32());
    return info;
}

bool AbilityCallerInfo::Marshalling(Parcel &parcel) const
{
    if (!DoMarshallingOne(parcel)) {
        return false;
    }

    if (!DoMarshallingTwo(parcel)) {
        return false;
    }

    return true;
}

bool AbilityCallerInfo::DoMarshallingOne(Parcel &parcel) const
{
    if (!parcel.WriteString(packageName)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write packageName failed");
        return false;
    }

    if (!parcel.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write uid failed");
        return false;
    }

    if (!parcel.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write pid failed");
        return false;
    }

    if (!parcel.WriteInt32(callerAppType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerAppType failed");
        return false;
    }

    if (!parcel.WriteInt32(targetAppType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetAppType failed");
        return false;
    }

    if (!parcel.WriteInt32(callerModelType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerModelType failed");
        return false;
    }

    if (!parcel.WriteString(targetAppDistType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetAppDistType failed");
        return false;
    }

    if (!parcel.WriteString(targetLinkFeature)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetLinkFeature failed");
        return false;
    }

    if (!parcel.WriteInt32(targetLinkType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetLinkType failed");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(callerAbilityType))) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerAbilityType failed");
        return false;
    }

    return true;
}

bool AbilityCallerInfo::DoMarshallingTwo(Parcel &parcel) const
{
    if (!parcel.WriteInt32(embedded)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write embedded failed");
        return false;
    }

    if (!parcel.WriteString(callerAppProvisionType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerAppProvisionType failed");
        return false;
    }

    if (!parcel.WriteString(targetAppProvisionType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetAppProvisionType failed");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(callerExtensionAbilityType))) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerExtensionAbilityType failed");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(targetAbilityType))) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetAbilityType failed");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(targetExtensionAbilityType))) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetExtensionAbilityType failed");
        return false;
    }

    if (!parcel.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write userId failed");
        return false;
    }

    if (!parcel.WriteInt32(targetApplicationReservedFlag)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetApplicationReservedFlag failed");
        return false;
    }

    if (!parcel.WriteBool(isAsCaller)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write isAsCaller failed");
        return false;
    }

    if (!parcel.WriteString(targetBundleName)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetBundleName failed");
        return false;
    }

    return true;
}

std::string AbilityCallerInfo::ToString() const
{
    std::string str = "CallerInfo{packageName:" + packageName + ",uid:" + std::to_string(uid) +
        ",pid:" + std::to_string(pid) + ",callerAppType:" + std::to_string(callerAppType) +
        ",targetAppType:" + std::to_string(targetAppType) + ",callerModelType:" + std::to_string(callerModelType) +
        ",targetAppDistType:" + targetAppDistType + ",targetLinkFeature:" + targetLinkFeature + ",targetLinkType:" +
        std::to_string(targetLinkType) + ",callerAbilityType:" +
        std::to_string(static_cast<int32_t>(callerAbilityType)) + ",callerExtensionAbilityType:" +
        std::to_string(static_cast<int32_t>(callerExtensionAbilityType)) + ",embedded:" +
        std::to_string(embedded) + ",callerAppProvisionType:" + callerAppProvisionType + ",targetAppProvisionType:" +
        targetAppProvisionType + ",targetAbilityType:" +
        std::to_string(static_cast<int32_t>(targetAbilityType)) + ",targetExtensionAbilityType:" +
        std::to_string(static_cast<int32_t>(targetExtensionAbilityType)) + ",isAsCaller:" +
        std::to_string(static_cast<int32_t>(isAsCaller)) + "}";
    return str;
}
} // namespace EcologicalRuleMgrService
} // namespace OHOS
