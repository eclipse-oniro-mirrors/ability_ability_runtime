/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_FA_ABILITY_THREAD_H
#define OHOS_ABILITY_RUNTIME_FA_ABILITY_THREAD_H

#include "ability.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "ability_thread.h"
#include "context.h"
#include "extension_impl.h"
#include "ipc_singleton.h"
#include "ohos_application.h"
#include "pac_map.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityImpl;
class AbilityHandler;
class AbilityLocalRecord;
} // namespace AppExecFwk
namespace AbilityRuntime {
using LifeCycleStateInfo = OHOS::AAFwk::LifeCycleStateInfo;
class FAAbilityThread : public AppExecFwk::AbilityThread {
public:
    /**
     * @brief Default constructor used to create a FAAbilityThread instance.
     */
    FAAbilityThread();
    ~FAAbilityThread() override;

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord current running ability record
     * @param mainRunner The runner which main_thread holds.
     * @param appContext the AbilityRuntime context
     */
    void Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner,
        const std::shared_ptr<Context> &appContext) override;

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord current running ability record
     * @param appContext the AbilityRuntime context
     */
    void Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<Context> &appContext) override;

    /**
     * @brief Provide operating system AbilityTransaction information to the observer
     * @param want Indicates the structure containing Transaction information about the ability.
     * @param targetState Indicates the lifecycle state.
     * @param sessionInfo Indicates the session info.
     */
    bool ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState,
        sptr<AAFwk::SessionInfo> sessionInfo = nullptr) override;

    /**
     * @brief Provide operating system ShareData information to the observer
     * @param requestCode Indicates the Ability request code.
     */
    void ScheduleShareData(const int32_t &requestCode) override;

    /**
     * @brief Provide operating system ConnectAbility information to the observer
     * @param want Indicates the structure containing connect information about the ability.
     */
    void ScheduleConnectAbility(const Want &want) override;

    /**
     * @brief Provide operating system DisconnectAbility information to the observer
     * @param want Indicates the structure containing connect information about the ability.
     */
    void ScheduleDisconnectAbility(const Want &want) override;

    /**
     * @brief Provide operating system CommandAbility information to the observer
     * @param want The Want object to command to.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by
     * 1 every time the ability is started. For example, if the ability has been started for six times, the value of
     * startId is 6.
     */
    void ScheduleCommandAbility(const Want &want, bool restart, int startId) override;

    /**
     * @brief Schedule Command AbilityWindow
     * @param want The Want object to command to.
     * @param sessionInfo Indicates the session info.
     * @param winCmd Indicates the WindowCommand of winCmd
     */
    void ScheduleCommandAbilityWindow(
        const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd) override;

    /**
     * @brief Provide operating system PrepareTerminateAbility information to the observer
     */
    bool SchedulePrepareTerminateAbility() override;

    /**
     * @brief Provide operating system SaveabilityState information to the observer
     */
    void ScheduleSaveAbilityState() override;

    /**
     * @brief Provide operating system RestoreAbilityState information to the observer
     * @param state Indicates resotre ability state used to dispatchRestoreAbilityState.
     */
    void ScheduleRestoreAbilityState(const AppExecFwk::PacMap &state) override;

    /**
     * @brief ScheduleUpdateConfiguration, scheduling update configuration.
     * @param config Indicates the updated configuration information
     */
    void ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config) override;

    /**
     * @brief Send the result code and data to be returned by this Page ability to the caller.
     * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int32_t, int32_t, Want)
     * method to receive the result set in the current method. This method can be called only after the ability has
     * been initialized.
     * @param requestCode Indicates the request code for send.
     * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result
     * code to identify an error.
     * @param want Indicates the data returned after the ability is destroyed. You can define the data returned. This
     * parameter can be null.
     */
    void SendResult(int requestCode, int resultCode, const Want &resultData) override;

    /**
     * @brief Obtains the MIME types of files supported.
     * @param uri Indicates the path of the files to obtain.
     * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
     * @return Returns the matched MIME types. If there is no match, null is returned.
     */
    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override;

    /**
     * @brief Opens a file in a specified remote path.
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
     * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
     * or "rwt" for read and write access that truncates any existing file.
     * @return Returns the file descriptor.
     */
    int OpenFile(const Uri &uri, const std::string &mode) override;

    /**
     * @brief This is like openFile, open a file that need to be able to return sub-sections of files often assets
     * inside of their .hap.
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
     * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
     * data, or "rwt" for read and write access that truncates any existing file.
     * @return Returns the RawFileDescriptor object containing file descriptor.
     */
    int OpenRawFile(const Uri &uri, const std::string &mode) override;

    /**
     * @brief Inserts a single data record into the database.
     * @param uri Indicates the path of the data to operate.
     * @param value Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
     * @return Returns the index of the inserted data record.
     */
    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override;

    /**
     * @brief Calls the method of the Data ability.
     * @param method Indicates the method to call
     * @param arg Indicates the parameter of the String type.
     * @param pacMap Defines a PacMap object for storing a series of values.
     * @return Returns the call result.
     */
    std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap) override;

    /**
     * @brief Updates data records in the database.
     * @param uri Indicates the path of data to update.
     * @param value Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     * @return Returns the number of data records updated.
     */
    int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override;

    /**
     * @brief Deletes one or more data records from the database.
     * @param uri Indicates the path of the data to operate.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     * @return Returns the number of data records deleted.
     */
    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override;

    /**
     * @brief Deletes one or more data records from the database.
     * @param uri Indicates the path of data to query.
     * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     * @return Returns the query result.
     */
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates) override;

    /**
     * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
     * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
     * @param uri Indicates the URI of the data.
     * @return Returns the MIME type that matches the data specified by uri.
     */
    std::string GetType(const Uri &uri) override;

    /**
     * @brief Reloads data in the database.
     * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
     * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call. This
     * parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be transferred across
     * processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for the custom object.
     * @return Returns true if the data is successfully reloaded; returns false otherwise.
     */
    bool Reload(const Uri &uri, const AppExecFwk::PacMap &extras) override;

    /**
     * @brief Inserts multiple data records into the database.
     * @param uri Indicates the path of the data to operate.
     * @param values Indicates the data records to insert.
     * @return Returns the number of data records inserted.
     */
    int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override;

    /**
     * @brief continue ability to target device.
     * @param deviceId target deviceId
     * @param versionCode Target bundle version.
     */
    void ContinueAbility(const std::string &deviceId, uint32_t versionCode) override;

    /**
     * @brief notify this ability continuation result.
     * @param result: Continuation result
     */
    void NotifyContinuationResult(int32_t result) override;

    /**
     * @brief notify this ability current memory level.
     * @param level Current memory level
     */
    void NotifyMemoryLevel(int32_t level) override;

    /**
     * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
     * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
     * context has changed. If you implement URI normalization for a Data ability, you must also implement
     * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to
     * any method that is called on the Data ability must require normalization verification and denormalization. The
     * default implementation of this method returns null, indicating that this Data ability does not support URI
     * normalization.
     * @param uri Indicates the Uri object to normalize.
     * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
     */
    Uri NormalizeUri(const Uri &uri) override;

    /**
     * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
     * The default implementation of this method returns the original URI passed to it.
     * @param uri uri Indicates the Uri object to denormalize.
     * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed
     * to this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found
     * in the current environment.
     */
    Uri DenormalizeUri(const Uri &uri) override;

    /**
     * @brief Registers an observer to DataObsMgr specified by the given Uri.
     * @param uri Indicates the path of the data to operate.
     * @param dataObserver Indicates the IDataAbilityObserver object.
     */
    bool ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override;

    /**
     * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
     * @param uri Indicates the path of the data to operate.
     * @param dataObserver Indicates the IDataAbilityObserver object.
     */
    bool ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override;

    /**
     * @brief Notifies the registered observers of a change to the data resource specified by Uri.
     * @param uri Indicates the path of the data to operate.
     */
    bool ScheduleNotifyChange(const Uri &uri) override;

    /**
     * @brief Dump ability runner info.
     * @param params the params need to be Dumped
     * @param info ability runner info.
     */
    void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info) override;

    /**
     * @brief Call Request
     */
    void CallRequest() override;

    void ExecuteSkill(const Want &want) override;

    /**
     * @brief Performs batch operations on the database
     * @param operations Indicates a list of database operations on the database.
     * @return Returns the result of each operation, in array.
     */
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations) override;

    /**
     * @brief create modal UIExtension.
     * @param want Create modal UIExtension with want object.
     */
    int CreateModalUIExtension(const Want &want) override;

    /**
     * @brief Update sessionToken.
     * @param sessionToken The token of session.
     */
    void UpdateSessionToken(sptr<IRemoteObject> sessionToken) override;

private:
    /**
     * @brief Dump Ability Runner info.
     * @param params the params need to be Dumped
     * @param info ability runner info
     */
    void DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info);

    /**
     * @brief Dump other Ability Runner info.
     * @param info ability runner info
     */
    void DumpOtherInfo(std::vector<std::string> &info);

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord current running ability record
     * @param mainRunner The runner which main_thread holds.
     */
    void AttachExtension(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner);

    /**
     * @brief Init extension Ability flag.
     * @param abilityRecord current running ability record
     */
    void InitExtensionFlag(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord);

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord current running ability record
     */
    void AttachExtension(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord);

    /**
     * @brief To continue attaching The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord current running ability record
     * @param stageContext the AbilityRuntime context
     */
    void AttachInner(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<Context> &stageContext);

    /**
     * @brief Create the abilityname.
     * @param abilityRecord current running ability record
     * @param application Indicates the application.
     * @return Returns the abilityname.
     */
    std::string CreateAbilityName(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application);

    /**
     * @brief Create the extension abilityname.
     * @param application Indicates the application.
     * @param abilityInfo Indicates the abilityInfo.
     * @param abilityName Indicates the parameter about abilityName.
     */
    void CreateExtensionAbilityName(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo, std::string &abilityName);

    /**
     * @brief Create the extension abilityname which support graphics.
     * @param abilityInfo Indicates the abilityInfo.
     * @param abilityName Indicates the parameter about abilityName.
     */
    void CreateExtensionAbilityNameSupportGraphics(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
        std::string &abilityName);

    /**
     * @brief Create and init contextDeal.
     * @param application Indicates the main process.
     * @param abilityRecord current running ability record
     * @param abilityObject Indicates the abilityObject.
     * @return Returns the contextDeal.
     */
    std::shared_ptr<AppExecFwk::ContextDeal> CreateAndInitContextDeal(
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AppExecFwk::AbilityContext> &abilityObject);

    /**
     * @brief Handle the life cycle of Ability.
     * @param want Indicates the structure containing lifecycle information about the ability.
     * @param lifeCycleStateInfo Indicates the lifeCycleStateInfo.
     * @param sessionInfo Indicates the sessionInfo.
     */
    void HandleAbilityTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo,
        sptr<AAFwk::SessionInfo> sessionInfo = nullptr);

    /**
     * @brief Handle the life cycle of Extension.
     * @param want Indicates the structure containing lifecycle information about the extension.
     * @param lifeCycleStateInfo Indicates the lifeCycleStateInfo.
     * @param sessionInfo Indicates the sessionInfo.
     */
    void HandleExtensionTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo,
        sptr<AAFwk::SessionInfo> sessionInfo = nullptr);

    /**
     * @brief Handle the current connection of Ability.
     * @param want Indicates the structure containing connection information about the ability.
     */
    void HandleConnectAbility(const Want &want);

    /**
     * @brief Handle the current disconnection of Ability.
     * @param want Indicates the structure containing connection information about the ability.
     */
    void HandleDisconnectAbility(const Want &want);

    /**
     * @brief Handle the current command of Ability.
     * @param want The Want object to command to.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by
     * 1 every time the ability is started. For example, if the ability has been started for six times, the value of
     * startId is 6.
     */
    void HandleCommandAbility(const Want &want, bool restart, int32_t startId);

    /**
     * @brief Handle the current connection of Extension.
     * @param want Indicates the structure containing connection information about the extension.
     */
    void HandleConnectExtension(const Want &want);

    /**
     * @brief Handle the current disconnection of Extension.
     * @param want Indicates the structure containing connection information about the extension.
     */
    void HandleDisconnectExtension(const Want &want);

    /**
     * @brief Handle the current command of Extension.
     * @param want The Want object to command to.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service extension has been started. The startId is incremented
     * by 1 every time the extension is started. For example, if the extension has been started for six times, the
     * value of startId is 6.
     */
    void HandleCommandExtension(const Want &want, bool restart, int32_t startId);

    /**
     * @brief Handle Command Extension Window
     * @param want The Want object to command to.
     * @param sessionInfo Indicates the sessionInfo
     * @param winCmd Indicates the winCmd
     */
    void HandleCommandExtensionWindow(
        const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd);

    /**
     * @brief Handle the restoreAbility state.
     * @param state Indicates save ability state used to dispatchRestoreAbilityState.
     */
    void HandleRestoreAbilityState(const AppExecFwk::PacMap &state);

    /**
     * @brief Handle the scheduling update configuration
     * @param config Indicates the updated configuration information
     */
    void HandleUpdateConfiguration(const AppExecFwk::Configuration &config);

    /**
     * @brief Handle the scheduling update configuration of extension.
     * @param config Indicates the updated configuration information
     */
    void HandleExtensionUpdateConfiguration(const AppExecFwk::Configuration &config);

    /**
     * @brief Handle the scheduling prepare terminate ability.
     */
    void HandlePrepareTerminateAbility();

    /**
     * @brief Provide operating system ShareData information to the observer
     * @param requestCode Indicates the Ability request code.
     */
    void HandleShareData(const int32_t &requestCode);

    /**
     * @brief Registers an observer to DataObsMgr specified by the given Uri.
     * @param uri Indicates the path of the data to operate.
     * @param dataObserver Indicates the IDataAbilityObserver object.
     * @return The operation succeeded or failed. Procedure
     */
    bool HandleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);

    /**
     * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
     * @param uri Indicates the path of the data to operate.
     * @param dataObserver Indicates the IDataAbilityObserver object.
     */
    bool HandleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);

    /**
     * @brief Notifies the registered observers of a change to the data resource specified by Uri.
     * @param uri Indicates the path of the data to operate.
     */
    bool HandleNotifyChange(const Uri &uri);

    /**
     * @brief Build Ability Context
     * @param abilityInfo Indicate the Ability information.
     * @param application Indicates the main process.
     * @param token the remote token
     * @param stageContext Indicates the stage of Context
     */
    std::shared_ptr<AbilityContext> BuildAbilityContext(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application, const sptr<IRemoteObject> &token,
        const std::shared_ptr<Context> &stageContext);

    void AddLifecycleEvent(uint32_t state, std::string &methodName) const;

    std::shared_ptr<AppExecFwk::AbilityImpl> abilityImpl_;
    std::shared_ptr<AppExecFwk::Ability> currentAbility_;
    std::shared_ptr<ExtensionImpl> extensionImpl_;
    std::shared_ptr<Extension> currentExtension_;
    bool isExtension_ = false;
    bool isUIAbility_ = false;
    bool isPrepareTerminate_ = false;
    std::atomic_bool isPrepareTerminateAbilityDone_ = false;
    std::mutex mutex_;
    std::condition_variable cv_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FA_ABILITY_THREAD_H
