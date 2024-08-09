/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "app_launch_data.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
void AppLaunchData::SetApplicationInfo(const ApplicationInfo &info)
{
    applicationInfo_ = info;
}

void AppLaunchData::SetProfile(const Profile &profile)
{
    profile_ = profile;
}

void AppLaunchData::SetProcessInfo(const ProcessInfo &info)
{
    processInfo_ = info;
}

void AppLaunchData::SetRecordId(const int32_t recordId)
{
    recordId_ = recordId;
}

void AppLaunchData::SetUId(const int32_t uId)
{
    uId_ = uId;
}

void AppLaunchData::SetAppIndex(const int32_t appIndex)
{
    appIndex_ = appIndex;
}

void AppLaunchData::SetUserTestInfo(const std::shared_ptr<UserTestRecord> &record)
{
    userTestRecord_ = record;
}

bool AppLaunchData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&applicationInfo_) ||
        !parcel.WriteParcelable(&profile_) || !parcel.WriteParcelable(&processInfo_)) {
        return false;
    }
    if (!parcel.WriteInt32(recordId_) ||
        !parcel.WriteInt32(uId_) || !parcel.WriteInt32(appIndex_)) {
        return false;
    }

    bool valid = userTestRecord_ ? true : false;
    if (!parcel.WriteBool(valid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write the flag which indicate whether userTestRecord_ is null");
        return false;
    }
    if (valid) {
        if (!parcel.WriteParcelable(userTestRecord_.get())) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write userTestRecord_");
            return false;
        }
    }

    if (!parcel.WriteBool(debugApp_)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write debug flag.");
        return false;
    }

    if (!parcel.WriteString(perfCmd_)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write perf cmd.");
        return false;
    }

    if (!parcel.WriteBool(jitEnabled_) || !parcel.WriteBool(isNativeStart_)) {
        return false;
    }

    if (!parcel.WriteString(appRunningUniqueId_)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Marshalling, Failed to write app running unique id.");
        return false;
    }

    if (!parcel.WriteBool(isMultiThread_)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write is multi thread flag.");
        return false;
    }

    if (!parcel.WriteBool(isErrorInfoEnhance_)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write is error info enhance flag.");
        return false;
    }

    return true;
}

bool AppLaunchData::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<ApplicationInfo> applicationInfoRead(parcel.ReadParcelable<ApplicationInfo>());
    if (!applicationInfoRead) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed, applicationInfoRead is nullptr");
        return false;
    }
    applicationInfo_ = *applicationInfoRead;

    std::unique_ptr<Profile> profileRead(parcel.ReadParcelable<Profile>());
    if (!profileRead) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed, profileRead is nullptr");
        return false;
    }
    profile_ = *profileRead;

    std::unique_ptr<ProcessInfo> processInfoRead(parcel.ReadParcelable<ProcessInfo>());
    if (!processInfoRead) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed, processInfoRead is nullptr");
        return false;
    }
    processInfo_ = *processInfoRead;

    recordId_ = parcel.ReadInt32();
    uId_ = parcel.ReadInt32();
    appIndex_ = parcel.ReadInt32();

    bool valid = parcel.ReadBool();
    if (valid) {
        userTestRecord_ = std::shared_ptr<UserTestRecord>(parcel.ReadParcelable<UserTestRecord>());
        if (!userTestRecord_) {
            TAG_LOGE(AAFwkTag::APPMGR, "failed, userTestRecord is nullptr");
            return false;
        }
    }

    debugApp_ = parcel.ReadBool();
    perfCmd_ = parcel.ReadString();
    jitEnabled_ = parcel.ReadBool();
    isNativeStart_ = parcel.ReadBool();
    appRunningUniqueId_ = parcel.ReadString();
    isMultiThread_ = parcel.ReadBool();
    isErrorInfoEnhance_ = parcel.ReadBool();
    return true;
}

AppLaunchData *AppLaunchData::Unmarshalling(Parcel &parcel)
{
    AppLaunchData *appLaunchData = new AppLaunchData();
    if (appLaunchData && !appLaunchData->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "failed, because ReadFromParcel failed");
        delete appLaunchData;
        appLaunchData = nullptr;
    }
    return appLaunchData;
}

bool UserTestRecord::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write want");
        return false;
    }

    auto valid = observer ? true : false;
    if (!parcel.WriteBool(valid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write the flag which indicate whether observer is null");
        return false;
    }

    if (valid) {
        if (!(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(observer)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write observer");
            return false;
        }
    }

    if (!parcel.WriteBool(isFinished)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write isFinished");
        return false;
    }

    if (!parcel.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write userId");
        return false;
    }
    return true;
}

UserTestRecord *UserTestRecord::Unmarshalling(Parcel &parcel)
{
    UserTestRecord *userTestRecord = new (std::nothrow) UserTestRecord();
    if (userTestRecord && !userTestRecord->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "failed, because ReadFromParcel failed");
        delete userTestRecord;
        userTestRecord = nullptr;
    }
    return userTestRecord;
}

bool UserTestRecord::ReadFromParcel(Parcel &parcel)
{
    AAFwk::Want *wantPtr = parcel.ReadParcelable<AAFwk::Want>();
    if (wantPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "wantPtr is nullptr");
        return ERR_INVALID_VALUE;
    }
    want = *wantPtr;
    delete wantPtr;

    auto valid = parcel.ReadBool();
    if (valid) {
        observer = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
        if (!observer) {
            TAG_LOGE(AAFwkTag::APPMGR, "observer is nullptr");
            return false;
        }
    }

    isFinished = parcel.ReadBool();
    userId = parcel.ReadInt32();
    return true;
}

void AppLaunchData::SetNativeStart(bool isNativeStart)
{
    isNativeStart_ = isNativeStart;
}

bool AppLaunchData::isNativeStart() const
{
    return isNativeStart_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
