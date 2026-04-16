/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_LAUNCH_DATA_H
#define OHOS_ABILITY_RUNTIME_APP_LAUNCH_DATA_H

#include <string>

#include "iremote_object.h"
#include "parcel.h"

#include "application_info.h"
#include "process_info.h"
#include "profile.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
struct UserTestRecord : public Parcelable {
    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    bool isFinished;
    int32_t userId;
    UserTestRecord() : observer(nullptr), isFinished(false), userId(-1)
    {}

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static UserTestRecord *Unmarshalling(Parcel &parcel);
};

struct StartupTaskData : public Parcelable {
    std::string action;
    std::string insightIntentName;
    std::string uri;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static StartupTaskData *Unmarshalling(Parcel &parcel);
};

struct AppUpdateInfo : public Parcelable {
    int32_t appRecordId = -1;
    std::string appRunningUniqueId;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static AppUpdateInfo *Unmarshalling(Parcel &parcel);
};

class AppLaunchData : public Parcelable {
public:
    /**
     * @brief setting information for the application.
     *
     * @param ApplicationInfo&, the current application info.
     */
    void SetApplicationInfo(const ApplicationInfo &info);

    /**
     * @brief Setting information for the profile.
     *
     * @param Profile&, the current profile.
     */
    void SetProfile(const Profile &profile);

    /**
     * @brief Setting information for the process.
     *
     * @param Profile&, the current process info.
     */
    void SetProcessInfo(const ProcessInfo &info);

    /**
     * @brief Setting id for app record.
     *
     * @param int32_t, the current app record id.
     */
    void SetRecordId(const int32_t recordId);

    /**
     * @brief Setting id for User.
     *
     * @param int32_t, the current app User.
     */
    void SetUId(const int32_t uId);

    /**
     * @brief set user test info.
     *
     * @param UserTestRecord, user test info.
     */
    void SetUserTestInfo(const std::shared_ptr<UserTestRecord> &record);

    void SetAppIndex(const int32_t appIndex);

    /**
     * @brief Obtains the info of the application.
     *
     * @return Returns the current application info.
     */
    inline const ApplicationInfo &GetApplicationInfo() const
    {
        return applicationInfo_;
    }

    /**
     * @brief Obtains the profile.
     *
     * @return Returns the current profile.
     */
    inline const Profile &GetProfile() const
    {
        return profile_;
    }

    /**
     * @brief Obtains the info of the process.
     *
     * @return Returns the current process info.
     */
    inline const ProcessInfo &GetProcessInfo() const
    {
        return processInfo_;
    }

    /**
     * @brief Obtains the id of the app record.
     *
     * @return Returns the current appRecord id.
     */
    inline int32_t GetRecordId() const
    {
        return recordId_;
    }

    /**
     * @brief Obtains the id of the User.
     *
     * @return Returns the current User id.
     */
    inline int32_t GetUId() const
    {
        return uId_;
    }

    /**
     * @brief get user test info.
     *
     * @return Returns user test info.
     */
    inline std::shared_ptr<UserTestRecord> GetUserTestInfo() const
    {
        return userTestRecord_;
    }

    inline int32_t GetAppIndex() const
    {
        return appIndex_;
    }

    inline void SetDebugApp(bool debugApp)
    {
        debugApp_ = debugApp;
    }

    inline bool GetDebugApp() const
    {
        return debugApp_;
    }

    inline void SetPerfCmd(const std::string &perfCmd)
    {
        perfCmd_ = perfCmd;
    }

    inline std::string GetPerfCmd() const
    {
        return perfCmd_;
    }

    inline void SetMultiThread(const bool multiThread)
    {
        isMultiThread_ = multiThread;
    }

    inline bool GetMultiThread() const
    {
        return isMultiThread_;
    }

    inline void SetErrorInfoEnhance(const bool errorInfoEnhance)
    {
        isErrorInfoEnhance_ = errorInfoEnhance;
    }

    inline bool GetErrorInfoEnhance() const
    {
        return isErrorInfoEnhance_;
    }

    inline void SetJITEnabled(const bool jitEnabled)
    {
        jitEnabled_ = jitEnabled;
    }

    inline bool IsJITEnabled() const
    {
        return jitEnabled_;
    }

    inline std::string GetAppRunningUniqueId() const
    {
        return appRunningUniqueId_;
    }

    inline void SetAppRunningUniqueId(const std::string &appRunningUniqueId)
    {
        appRunningUniqueId_ = appRunningUniqueId;
    }

    inline std::string GetPreloadModuleName() const
    {
        return preloadModuleName_;
    }

    inline void SetPreloadModuleName(const std::string& moduleName)
    {
        preloadModuleName_ = moduleName;
    }

    inline std::string GetInstanceKey() const
    {
        return instanceKey_;
    }

    inline void SetInstanceKey(const std::string& instanceKey)
    {
        instanceKey_ = instanceKey;
    }

    void SetStartupTaskData(std::shared_ptr<StartupTaskData> startupTaskData)
    {
        startupTaskData_ = startupTaskData;
    }

    std::shared_ptr<StartupTaskData> GetStartupTaskData() const
    {
        return startupTaskData_;
    }

    void SetImageProcessType(int32_t imageProcessType)
    {
        imageProcessType_ = imageProcessType;
    }

    int32_t GetImageProcessType() const
    {
        return imageProcessType_;
    }

    /**
     * @brief read this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     * @return Returns true if read successed; returns false otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * @brief Marshals this Sequenceable object into a Parcel.
     *
     * @param outParcel Indicates the Parcel object to which the Sequenceable object will be marshaled.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief to solve cyclomatic of Marshalling
     *
     * @param outParcel Indicates the Parcel object to which the Sequenceable object will be marshaled.
     */
    bool MarshallingExtend(Parcel &parcel) const;

    /**
     * @brief Unmarshals this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     */
    static AppLaunchData *Unmarshalling(Parcel &parcel);
    
    bool ReadStartupTaskDataFromParcel(Parcel &parcel);

    /**
     * @brief Setting is aa start with native.
     *
     * @param isNativeStart, is aa start with native.
     */
    void SetNativeStart(bool isNativeStart);
    /**
     * @brief Obtains is native start.
     *
     * @return Returns is native start.
     */
    bool isNativeStart() const;

    /**
     * @brief Setting is need preload module.
     *
     * @param isNeedPreloadModule, is need preload module.
     */
    void SetIsNeedPreloadModule(bool isNeedPreloadModule);
    /**
     * @brief Obtains  is need preload module..
     *
     * @return Returns  is need preload module..
     */
    bool IsNeedPreloadModule() const;

    /**
     * @brief Setting preload mode.
     *
     * @param preloadMode PreloadMode enum
     */
    void SetAppPreloadMode(PreloadMode preloadMode);

    /**
     * @brief Obtains preload mode.
     *
     * @return Returns preloadMode enum
     */
    PreloadMode GetAppPreloadMode() const;

    /**
     * @brief Setting if allow nweb preload.
     *
     * @param isAllowedNWebPreload, if allow nweb preload.
     */
    void SetNWebPreload(const bool isAllowedNWebPreload);
    /**
     * @brief Get if allow nweb preload.
     *
     * @return Returns if allow nweb preload.
     */
    bool IsAllowedNWebPreload() const;

    /**
     * @brief Setting debug mode.
     *
     * @param ideDebugMode, false:old debug, true:local debug.
     */
    void SetDebugFromLocal(bool isDebugFromLocal);
    /**
     * @brief Get debug mode.
     *
     * @return Returns debug mode.
     */
    bool GetDebugFromLocal() const;

private:
    bool debugApp_ = false;
    bool jitEnabled_ = false;
    bool isNativeStart_ = false;
    bool isMultiThread_ = false;
    bool isErrorInfoEnhance_ = false;
    bool isNeedPreloadModule_ = false;
    PreloadMode appPreloadMode_ = PreloadMode::PRELOAD_NONE;
    bool isAllowedNWebPreload_ = false;
    int32_t recordId_ = 0;
    int32_t uId_ = 0;
    int32_t appIndex_ = 0;
    int32_t imageProcessType_ = 0;
    std::shared_ptr<UserTestRecord> userTestRecord_ = nullptr;
    ApplicationInfo applicationInfo_;
    Profile profile_;
    ProcessInfo processInfo_;
    std::string perfCmd_;
    std::string appRunningUniqueId_;
    std::string instanceKey_;
    std::string preloadModuleName_;
    bool isDebugFromLocal_;
    std::shared_ptr<StartupTaskData> startupTaskData_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_INTERFACES_INNERKITS_APPEXECFWK_CORE_APPMGR_INCLUDE_APP_LAUNCH_DATA_H
