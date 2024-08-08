/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CONTEXT_IMPL_H
#define OHOS_ABILITY_RUNTIME_CONTEXT_IMPL_H

#include "context.h"

#include "configuration.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
struct RunningProcessInfo;
class BundleMgrHelper;
}
namespace AAFwk {
class Want;
}
namespace AbilityRuntime {
class ContextImpl : public Context {
public:
    ContextImpl() = default;
    virtual ~ContextImpl() = default;

    /**
     * @brief Obtains the bundle name of the current ability.
     *
     * @return Returns the bundle name of the current ability.
     */
    std::string GetBundleName() const override;

    /**
     * @brief Obtains the path of the package containing the current ability. The returned path contains the resources,
     *  source code, and configuration files of a module.
     *
     * @return Returns the path of the package file.
     */
    std::string GetBundleCodeDir() override;

    /**
     * @brief Obtains the application-specific cache directory on the device's internal storage. The system
     * automatically deletes files from the cache directory if disk space is required elsewhere on the device.
     * Older files are always deleted first.
     *
     * @return Returns the application-specific cache directory.
     */
    std::string GetCacheDir() override;

    /**
     * @brief Checks whether the configuration of this ability is changing.
     *
     * @return Returns true if the configuration of this ability is changing and false otherwise.
     */
    bool IsUpdatingConfigurations() override;

    /**
     * @brief Informs the system of the time required for drawing this Page ability.
     *
     * @return Returns the notification is successful or fail
     */
    bool PrintDrawnCompleted() override;

    /**
     * @brief Obtains the temporary directory.
     *
     * @return Returns the application temporary directory.
     */
    std::string GetTempDir() override;

    std::string GetResourceDir() override;

    /**
     * @brief Get all temporary directories.
     *
     * @param tempPaths Return all temporary directories of the application.
     */
    virtual void GetAllTempDir(std::vector<std::string> &tempPaths);

    /**
     * @brief Obtains the directory for storing files for the application on the device's internal storage.
     *
     * @return Returns the application file directory.
     */
    std::string GetFilesDir() override;

    /**
     * @brief Obtains the local database path.
     * If the local database path does not exist, the system creates one and returns the created path.
     *
     * @return Returns the local database file.
     */
    std::string GetDatabaseDir() override;

    /**
     * @brief Obtains the local system database path.
     * If the local group database path does not exist, the system creates one and returns the created path.
     *
     * @return Returns the local group database file.
     */
    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override;

    /**
     * @brief Obtains the path storing the storage file of the application.
     *
     * @return Returns the local storage file.
     */
    std::string GetPreferencesDir() override;

    /**
     * @brief Obtains the path storing the system storage file of the application.
     *
     * @return Returns the local system storage file.
     */
    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override;

    /**
     * @brief Obtains the path storing the group file of the application by the groupId.
     *
     * @return Returns the local group file.
     */
    std::string GetGroupDir(std::string groupId) override;

    /**
     * @brief Obtains the path distributed file of the application
     *
     * @return Returns the distributed file.
     */
    std::string GetDistributedFilesDir() override;

    std::string GetCloudFileDir() override;

    /**
     * @brief Switch file area
     *
     * @param mode file area.
     */
    void SwitchArea(int mode) override;

    /**
     * @brief Set color mode
     *
     * @param colorMode color mode.
     */
    void SetColorMode(int colorMode);

    /**
     * @brief Set language
     *
     * @param language language.
     */
    void SetLanguage(std::string language);

    /**
     * @brief Set font
     *
     * @param Font font.
     */
    void SetFont(std::string font);

    void SetMcc(std::string mcc);

    void SetMnc(std::string mnc);

    /**
     * @brief clear the application data by app self
     */
    void ClearUpApplicationData();

    /**
     * @brief Creates a Context object for a hap with the given module name.
     *
     * @param moduleName Indicates the module name of the hap.
     *
     * @return Returns a Context object created for the specified hap and app.
     */
    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;

    /**
     * @brief Creates a Context object for a hap with the given hap name and app name.
     *
     * @param bundleName Indicates the app name of the application.
     *
     * @param moduleName Indicates the module name of the hap.
     *
     * @return Returns a Context object created for the specified hap and app.
     */
    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName) override;

    /**
     * @brief Get file area
     *
     * @return file area.
     */
    int GetArea() override;

    /**
     * @brief set the ResourceManager.
     *
     * @param the ResourceManager has been initialized.
     */
    void SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager);

    /**
    * @brief Obtains a resource manager.
    *
    * @return Returns a ResourceManager object.
    */
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;

    /**
     * @brief Creates a Context object for an application with the given bundle name.
     *
     * @param bundleName Indicates the bundle name of the application.
     *
     * @return Returns a Context object created for the specified application.
     */
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;

    /**
     * @brief Creates a ResourceManager object for a hap with the given hap name and app name.
     *
     * @param bundleName Indicates the app name of the application.
     *
     * @param moduleName Indicates the module name of the hap.
     *
     * @return Returns a ResourceManager object created for the specified hap and app.
     */
    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override;

    int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName, const std::string &moduleName,
        std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override;

    /**
    * @brief Obtains an IBundleMgr instance.
    * You can use this instance to obtain information about the application bundle.
    *
    * @return Returns an IBundleMgr instance.
    */
    ErrCode GetBundleManager();

    /**
     * @brief Set ApplicationInfo
     *
     * @param info ApplicationInfo instance.
     */
    void SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info);

    /**
     * @brief Obtains information about the current application. The returned application information includes basic
     * information such as the application name and application permissions.
     *
     * @return Returns the ApplicationInfo for the current application.
     */
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;

    /**
     * @brief Set ApplicationInfo
     *
     * @param info ApplicationInfo instance.
     */
    void SetParentContext(const std::shared_ptr<Context> &context);

    /**
     * @brief Obtains the path of the package containing the current ability. The returned path contains the resources,
     *  source code, and configuration files of a module.
     *
     * @return Returns the path of the package file.
     */
    std::string GetBundleCodePath() const override;

    /**
     * @brief Obtains the HapModuleInfo object of the application.
     *
     * @return Returns the HapModuleInfo object of the application.
     */
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;

    /**
     * @brief Set HapModuleInfo
     *
     * @param hapModuleInfo HapModuleInfo instance.
     */
    void InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);

    /**
     * @brief Set HapModuleInfo
     *
     * @param hapModuleInfo HapModuleInfo instance.
     */
    void InitHapModuleInfo(const AppExecFwk::HapModuleInfo &hapModuleInfo);

    /**
     * @brief Set the token witch the app launched.
     *
     * @param token The token which the is launched by app.
     */
    void SetToken(const sptr<IRemoteObject> &token) override;

    /**
     * @brief Get the token witch the app launched.
     *
     * @return token The token which the is launched by app.
     */
    sptr<IRemoteObject> GetToken() override;

    /**
     * @brief Get the token witch the app launched.
     *
     * @return token The token which the is launched by app.
     */
    void SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config);

    /**
     * @brief Kill process itself
     *
     * @return error code
     */
    void KillProcessBySelf(const bool clearpagestack = false);

    /**
     * @brief Get running informationfor cuirrent process
     *
     * @return error code
     */
    int32_t GetProcessRunningInformation(AppExecFwk::RunningProcessInfo &info);

    /**
     * @brief Restart app
     *
     * @return error code
     */
    int32_t RestartApp(const AAFwk::Want& want);

    /**
     * @brief Get the token witch the app launched.
     *
     * @return token The token which the is launched by app.
     */
    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;

    /**
     * @brief Obtains the application base directory on the device's internal storage.
     *
     * @return Returns the application base directory.
     */
    std::string GetBaseDir() const override;

    /**
     * @brief Obtains the Device Type.
     *
     * @return Returns the Device Type.
     */
    Global::Resource::DeviceType GetDeviceType() const override;

    int32_t SetSupportedProcessCacheSelf(bool isSupport);

    void PrintTokenInfo() const;

    void AppHasDarkRes(bool &darkRes);

    static const int EL_DEFAULT = 1;

protected:
    sptr<IRemoteObject> token_;

private:
    static const int64_t CONTEXT_CREATE_BY_SYSTEM_APP;
    static const std::string CONTEXT_DATA_APP;
    static const std::string CONTEXT_BUNDLE;
    static const std::string CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE;
    static const std::string CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE;
    static const std::string CONTEXT_DISTRIBUTEDFILES;
    static const std::string CONTEXT_CLOUDFILE;
    static const std::string CONTEXT_FILE_SEPARATOR;
    static const std::string CONTEXT_DATA;
    static const std::string CONTEXT_DATA_STORAGE;
    static const std::string CONTEXT_BASE;
    static const std::string CONTEXT_PRIVATE;
    static const std::string CONTEXT_CACHE;
    static const std::string CONTEXT_PREFERENCES;
    static const std::string CONTEXT_GROUP;
    static const std::string CONTEXT_DATABASE;
    static const std::string CONTEXT_TEMP;
    static const std::string CONTEXT_FILES;
    static const std::string CONTEXT_HAPS;
    static const std::string CONTEXT_ELS[];
    static const std::string CONTEXT_RESOURCE_END;
    int flags_ = 0x00000000;

    void InitResourceManager(const AppExecFwk::BundleInfo &bundleInfo, const std::shared_ptr<ContextImpl> &appContext,
                             bool currentBundle = false, const std::string &moduleName = "");
    bool IsCreateBySystemApp() const;
    int GetCurrentAccountId() const;
    void SetFlags(int64_t flags);
    int GetCurrentActiveAccountId() const;
    void CreateDirIfNotExist(const std::string& dirPath, const mode_t& mode) const;

    int GetOverlayModuleInfos(const std::string &bundleName, const std::string &moduleName,
        std::vector<AppExecFwk::OverlayModuleInfo> &overlayModuleInfos);

    void OnOverlayChanged(const EventFwk::CommonEventData &data,
        const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager, const std::string &bundleName,
        const std::string &moduleName, const std::string &loadPath);

    std::vector<std::string> GetAddOverlayPaths(
        const std::vector<AppExecFwk::OverlayModuleInfo> &overlayModuleInfos);

    std::vector<std::string> GetRemoveOverlayPaths(
        const std::vector<AppExecFwk::OverlayModuleInfo> &overlayModuleInfos);

    void ChangeToLocalPath(const std::string &bundleName,
        const std::string &sourcDir, std::string &localPath);

    void CreateDirIfNotExistWithCheck(const std::string& dirPath, const mode_t& mode, bool checkExist = true);
    int32_t GetDatabaseDirWithCheck(bool checkExist, std::string &databaseDir);
    int32_t GetGroupDatabaseDirWithCheck(const std::string &groupId, bool checkExist, std::string &databaseDir);
    int32_t GetPreferencesDirWithCheck(bool checkExist, std::string &preferencesDir);
    int32_t GetGroupPreferencesDirWithCheck(const std::string &groupId, bool checkExist, std::string &preferencesDir);
    int32_t GetGroupDirWithCheck(const std::string &groupId, bool checkExist, std::string &groupDir);
    std::shared_ptr<Global::Resource::ResourceManager> InitOthersResourceManagerInner(
        const AppExecFwk::BundleInfo &bundleInfo, bool currentBundle, const std::string& moduleName);
    std::shared_ptr<Global::Resource::ResourceManager> InitResourceManagerInner(
        const AppExecFwk::BundleInfo &bundleInfo, bool currentBundle, const std::string& moduleName);
    void GetOverlayPath(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
        const std::string &bundleName, const std::string &moduleName, std::string &loadPath, bool currentBundle);
    void AddPatchResource(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
        const std::string &loadPath, const std::string &hqfPath, bool isDebug);
    void SubscribeToOverlayEvents(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
        const std::string &name, const std::string &hapModuleName, std::string &loadPath,
        std::vector<AppExecFwk::OverlayModuleInfo> overlayModuleInfos);
    void UpdateResConfig(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager);
    int32_t GetBundleInfo(const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo, bool &currentBundle);
    void GetBundleInfo(const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo, const int &accountId);
    ErrCode GetOverlayMgrProxy();

    static Global::Resource::DeviceType deviceType_;
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo_ = nullptr;
    std::shared_ptr<Context> parentContext_ = nullptr;
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager_ = nullptr;
    std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo_ = nullptr;
    std::shared_ptr<AppExecFwk::Configuration> config_ = nullptr;
    std::string currArea_ = "el2";
    std::vector<AppExecFwk::OverlayModuleInfo> overlayModuleInfos_;
    std::set<std::string> checkedDirSet_;
    std::mutex checkedDirSetLock_;

    std::mutex bundleManagerMutex_;
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bundleMgr_;
    std::mutex overlayMgrProxyMutex_;
    sptr<AppExecFwk::IOverlayManager> overlayMgrProxy_ = nullptr;

    // True: need to get a new fms remote object,
    // False: no need to get a new fms remote object.
    volatile bool resetFlag_ = false;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONTEXT_IMPL_H
