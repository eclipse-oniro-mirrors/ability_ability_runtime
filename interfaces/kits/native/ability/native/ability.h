/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_H
#define OHOS_ABILITY_RUNTIME_ABILITY_H

#include <functional>
#include <string>
#include <unistd.h>

#include "ability_context.h"
#include "ability_continuation_interface.h"
#include "ability_event_interface.h"
#include "ability_lifecycle_executor.h"
#include "ability_lifecycle_interface.h"
#include "ability_transaction_callback_info.h"
#include "configuration.h"
#include "context.h"
#include "continuation_handler.h"
#include "continuation_state.h"
#include "dummy_notification_request.h"
#include "fa_ability_context.h"
#include "free_install_observer_interface.h"
#include "iability_callback.h"
#include "want_agent.h"
#include "appexecfwk_errors.h"
#include "iremote_object.h"
#include "pac_map.h"
#include "want.h"

#ifdef SUPPORT_SCREEN
#include "ability_window.h"
#include "display_manager.h"
#include "form_state_info.h"
#include "session_info.h"
#include "window_option.h"
#include "window_scene.h"
#include "wm_common.h"
#endif

namespace OHOS {
namespace NativeRdb {
class AbsSharedResultSet;
class DataAbilityPredicates;
class ValueObject;
class ValuesBucket;
} // namespace NativeRdb
namespace AbilityRuntime {
class Runtime;
}
#ifdef SUPPORT_SCREEN
class KeyEvent;
namespace MMI {
class PointerEvent;
}
namespace Ace {
class UIContent;
}
#endif
namespace AppExecFwk {
using FeatureAbilityTask = std::function<void(int, const AAFwk::Want&)>;
class DataAbilityResult;
class DataAbilityOperation;
class AbilityPostEventTimeout;
class OHOSApplication;
class AbilityHandler;
#ifdef SUPPORT_SCREEN
class AbilityWindow;
class FormProviderInfo;
#endif
class ILifeCycle;
class ContinuationManager;
class ContinuationHandler;
class AbilityRecovery;
class ContinuationRegisterManager;
class Configuration;
class Ability : public IAbilityEvent,
                public ILifeCycle,
                public AbilityContext,
                public IAbilityContinuation,
                public IAbilityCallback,
                public std::enable_shared_from_this<Ability> {
public:
    static Ability* Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime);

    Ability() = default;
    virtual ~Ability() = default;

    /**
     * @brief Obtains the AbilityContext object of the ability.
     *
     * @return Returns the AbilityContext object of the ability.
     */
    std::shared_ptr<AbilityRuntime::AbilityContext> GetAbilityContext()
    {
        return abilityContext_;
    }

    /**
     * @brief Obtains the Lifecycle object of the current ability.
     *
     * @return Returns the Lifecycle object.
     */
    std::shared_ptr<LifeCycle> GetLifecycle() override final;

    /**
     * Register lifecycle observer on ability.
     *
     * @param observer the lifecycle observer to be registered on ability.
     */
    void RegisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer) override;

    /**
     * Unregister lifecycle observer on ability.
     *
     * @param observer the lifecycle observer to be unregistered on ability.
     */
    void UnregisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer) override;

    /**
     * @brief Obtains a resource manager.
     *
     * @return Returns a ResourceManager object.
     */
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;

    /**
     * Start other ability for result.
     *
     * @param want information of other ability
     * @param requestCode request code for abilityMS to return result
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResult(const Want &want, int requestCode) final;

    /**
     * Starts an ability with specific start settings and returns the execution result when the ability is destroyed.
     * When the ability is destroyed, onAbilityResult(int,int,ohos.aafwk.content.Want) is called and the returned
     * requestCode is transferred to the current method. The given requestCode is customized and cannot be a negative
     * number.
     *
     * @param want Indicates the ability to start.
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param abilityStartSetting Indicates the setting ability used to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbilityForResult(
        const Want &want, int requestCode, AbilityStartSetting abilityStartSetting) final;

    /**
     * Starts a new ability with specific start settings.
     * A Page or Service ability uses this method to start a specific ability.
     * The system locates the target ability from installed abilities based on
     * the value of the want parameter and then starts it. You can specify the
     * ability to start using the want parameter.
     *
     * @param want Indicates the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    using AbilityContext::StartAbility;
    ErrCode StartAbility(const Want &want, AbilityStartSetting abilityStartSetting);

    ErrCode AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> observer);
    /**
     * @brief A Page or Service ability uses this method to start a specific ability. The system locates the target
     * ability from installed abilities based on the value of the want parameter and then starts it. You can specify
     * the ability to start using the want parameter.
     *
     * @param want Indicates the ability to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbility(const Want &want) final;

    ErrCode StartFeatureAbilityForResult(const Want &want, int requestCode, FeatureAbilityTask &&task);

    virtual void Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<OHOSApplication> application, std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token);

    void AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext);

    /**
     * @brief Called when this ability is started. You must override this function if you want to perform some
     *        initialization operations during ability startup.
     *
     * This function can be called only once in the entire lifecycle of an ability.
     * @param Want Indicates the {@link Want} structure containing startup information about the ability.
     * @param sessionInfo  Indicates the sessionInfo.
     */
    virtual void OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo = nullptr);

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     *
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop();

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     *
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     *
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     */
    virtual void OnStop(AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback);

    /**
     * @brief The callback of OnStop.
     */
    virtual void OnStopCallback();

    /**
     * @brief Called when this ability enters the <b>STATE_ACTIVE</b> state.
     *
     * The ability in the <b>STATE_ACTIVE</b> state is visible and has focus.
     * You can override this function to implement your own processing logic.
     *
     * @param Want Indicates the {@link Want} structure containing activation information about the ability.
     */
    virtual void OnActive();

    /**
     * @brief Called when this ability enters the <b>STATE_INACTIVE</b> state.
     *
     * <b>STATE_INACTIVE</b> is an instantaneous state. The ability in this state may be visible but does not have
     * focus.You can override this function to implement your own processing logic.
     */
    virtual void OnInactive();

    /**
     * @brief Called when this Service ability is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Service ability.
     * @return Returns a pointer to the <b>sid</b> of the connected Service ability.
     */
    virtual sptr<IRemoteObject> OnConnect(const Want &want);

    /**
     * @brief Called when all abilities connected to this Service ability are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const Want &want);

    virtual std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap);

    /**
     * @brief Called to set caller information for the application. The default implementation returns null.
     *
     * @return Returns the caller information.
     */
    virtual Uri OnSetCaller();

    /**
     * @brief request a remote object of callee from this ability.
     * @return Returns the remote object of callee.
     */
    virtual sptr<IRemoteObject> CallRequest();

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    virtual void OnConfigurationUpdated(const Configuration &configuration);

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdatedNotify(const Configuration &configuration);

    /**
     * @brief Update context.config when configuration is updated.
     *
     */
    virtual void UpdateContextConfiguration() {}

    /**
     * @brief Checks whether the configuration of this ability is changing.
     *
     * @return Returns true if the configuration of this ability is changing and false otherwise.
     */
    bool IsUpdatingConfigurations() override;

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     *
     */
    virtual void OnMemoryLevel(int level);

    /**
     * @brief This is like openFile, open a file that need to be able to return sub-sections of files，often assets
     * inside of their .hap.
     *
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
     * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
     * data, or "rwt" for read and write access that truncates any existing file.
     *
     * @return Returns the RawFileDescriptor object containing file descriptor.
     */
    virtual int OpenRawFile(const Uri &uri, const std::string &mode);

    /**
     * @brief Updates one or more data records in the database. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the database table storing the data to update.
     * @param value Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be updated by
     * default.
     *
     * @return Returns the number of data records updated.
     */
    virtual int Update(
        const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief get application witch the ability belong
     *
     * @return Returns the application ptr
     */
    std::shared_ptr<OHOSApplication> GetApplication();

    /**
     * @brief Obtains the class name in this ability name, without the prefixed bundle name.
     *
     * @return Returns the class name of this ability.
     */
    std::string GetAbilityName();

    /**
     * @brief OChecks whether the current ability is being destroyed.
     * An ability is being destroyed if you called terminateAbility() on it or someone else requested to destroy it.
     *
     * @return Returns true if the current ability is being destroyed; returns false otherwise.
     */
    bool IsTerminating();

    /**
     * @brief Obtains the mole name in this ability name, without the prefixed bundle name.
     *
     * @return Returns the module name of this ability.
     */
    std::string GetModuleName();

    /**
     * @brief Called when startAbilityForResult(ohos.aafwk.content.Want,int) is called to start an ability and the
     * result is returned. This method is called only on Page abilities. You can start a new ability to perform some
     * calculations and use setResult (int,ohos.aafwk.content.Want) to return the calculation result. Then the system
     * calls back the current method to use the returned data to execute its own logic.
     *
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param resultCode Indicates the result code returned after the ability is started. You can define the result code
     * to identify an error.
     * @param want Indicates the data returned after the ability is started. You can define the data returned. The
     * value can be null.
     *
     */
    virtual void OnAbilityResult(int requestCode, int resultCode, const Want &want);

    virtual void OnFeatureAbilityResult(int requestCode, int resultCode, const Want &want);

    /**
     * @brief Called back when the Back key is pressed.
     * The default implementation destroys the ability. You can override this method.
     *
     */
    void OnBackPressed() override;

    /**
     * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch an
     * ability that has been at the top of the ability stack.
     *
     * @param want Indicates the new Want containing information about the ability.
     */
    virtual void OnNewWant(const Want &want);

    /**
     * @brief Restores data and states of an ability when it is restored by the system. This method should be
     * implemented by a Page ability. This method is called if an ability was destroyed at a certain time due to
     * resource reclaim or was unexpectedly destroyed and the onSaveAbilityState(ohos.utils.PacMap) method was called to
     * save its user data and states. Generally, this method is called after the onStart(ohos.aafwk.content.Want)
     * method.
     *
     *  @param inState Indicates the PacMap object used for storing data and states. This parameter can not be null.
     *
     */
    virtual void OnRestoreAbilityState(const PacMap &inState);

    /**
     * @brief Saves temporary data and states of this ability. This method should be implemented by a Page ability.
     * This method is called when the system determines that the ability may be destroyed in an unexpected situation,
     * for example, when the screen orientation changes or the user touches the Home key. Generally, this method is used
     * only to save temporary states.
     *
     *  @param outState Indicates the PacMap object used for storing user data and states. This parameter cannot be
     * null.
     *
     */
    virtual void OnSaveAbilityState(PacMap &outState);

    /**
     * @brief Called every time a key, touch, or trackball event is dispatched to this ability.
     * You can override this callback method if you want to know that the user has interacted with
     * the device in a certain way while this ability is running. This method, together with onLeaveForeground(),
     * is designed to help abilities intelligently manage status bar notifications. Specifically, they help
     * abilities determine when to cancel a notification.
     *
     */
    virtual void OnEventDispatch();

    /**
     * @brief Sets the want object that can be obtained by calling getWant().
     *
     * @param Want information of other ability
     */
    void SetWant(const AAFwk::Want &want);

    /**
     * @brief Obtains the Want object that starts this ability.
     *
     * @return Returns the Want object that starts this ability.
     */
    std::shared_ptr<AAFwk::Want> GetWant() override;

    /**
     * @brief Sets the result code and data to be returned by this Page ability to the caller.
     * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want) method to
     * receive the result set in the current method. This method can be called only after the ability has been
     * initialized.
     *
     * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result
     * code to identify an error.
     * @param resultData Indicates the data returned after the ability is destroyed. You can define the data returned.
     * This parameter can be null.
     */
    virtual void SetResult(int resultCode, const Want &resultData) final;

    /**
     * @brief Called back when Service is started.
     * This method can be called only by Service. You can use the StartAbility(ohos.aafwk.content.Want) method to start
     * Service. Then the system calls back the current method to use the transferred want parameter to execute its own
     * logic.
     *
     * @param want Indicates the want of Service to start.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by
     * 1 every time the ability is started. For example, if the ability has been started for six times, the value of
     * startId is 6.
     */
    virtual void OnCommand(const AAFwk::Want &want, bool restart, int startId);

    /**
     * @brief dump ability info
     *
     * @param extra dump ability info
     */
    virtual void Dump(const std::string &extra);

    /**
     * @brief dump ability info
     *
     * @param params dump params that indicate different dump targets
     * @param info dump ability info
     */
    virtual void Dump(const std::vector<std::string> &params, std::vector<std::string> &info);

    /**
     * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
     * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
     *
     * @param uri Indicates the URI of the data.
     *
     * @return Returns the MIME type that matches the data specified by uri.
     */
    virtual std::string GetType(const Uri &uri);

    /**
     * @brief Inserts a data record into the database. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the position where the data is to insert.
     * @param value Indicates the data to insert.
     *
     * @return Returns the index of the newly inserted data record.
     */
    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value);

    /**
     * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
     * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
     * context has changed. If you implement URI normalization for a Data ability, you must also implement
     * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to
     * any method that is called on the Data ability must require normalization verification and denormalization. The
     * default implementation of this method returns null, indicating that this Data ability does not support URI
     * normalization.
     *
     * @param uri Indicates the Uri object to normalize.
     *
     * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
     */
    virtual Uri NormalizeUri(const Uri &uri);

    /**
     * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
     * The default implementation of this method returns the original URI passed to it.
     *
     * @param uri uri Indicates the Uri object to denormalize.
     *
     * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed
     * to this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found
     * in the current environment.
     */
    virtual Uri DenormalizeUri(const Uri &uri);

    /**
     * @brief Deletes one or more data records. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the database table storing the data to delete.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be deleted by
     * default.
     *
     * @return Returns the number of data records deleted.
     */
    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Obtains the MIME type of files. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the path of the files to obtain.
     * @param mimeTypeFilter Indicates the MIME type of the files to obtain. This parameter cannot be set to null.
     * 1. * / *: Obtains all types supported by a Data ability.
     * 2. image/ *: Obtains files whose main type is image of any subtype.
     * 3. * /jpg: Obtains files whose subtype is JPG of any main type.
     *
     * @return Returns the MIME type of the matched files; returns null if there is no type that matches the Data
     * ability.
     */
    virtual std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter);

    /**
     * @brief Opens a file. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
     * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
     * or "rwt" for read and write access that truncates any existing file.
     *
     * @return Returns the FileDescriptor object of the file descriptor.
     */
    virtual int OpenFile(const Uri &uri, const std::string &mode);

    /**
     * @brief Queries one or more data records in the database. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the database table storing the data to query.
     * @param columns Indicates the columns to be queried, in array, for example, {"name","age"}. You should define the
     * processing logic when this parameter is null.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be queried by
     * default.
     *
     * @return Returns the queried data.
     */
    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates);

    /**
     * @brief Reloads data in the database.
     *
     * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
     * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call. This
     * parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be transferred across
     * processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for the custom object.
     *
     * @return Returns true if the data is successfully reloaded; returns false otherwise.
     */
    virtual bool Reload(const Uri &uri, const PacMap &extras);

    /**
     * @brief Inserts multiple data records into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param values Indicates the data records to insert.
     *
     * @return Returns the number of data records inserted.
     */
    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values);

    /**
     * @brief Performs batch operations on the database.
     *
     * @param operations Indicates a list of database operations on the database.
     * @return Returns the result of each operation, in array.
     */
    virtual std::vector<std::shared_ptr<DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<DataAbilityOperation>> &operations);

    /**
     * @brief Executes an operation among the batch operations to be executed.
     *
     * @param operation Indicates the operation to execute.
     * @param results Indicates a set of results of the batch operations.
     * @param index Indicates the index of the current operation result in the batch operation results.
     */
    void ExecuteOperation(std::shared_ptr<DataAbilityOperation> &operation,
        std::vector<std::shared_ptr<DataAbilityResult>> &results, int index);

    /**
     * @brief Save user data of local Ability generated at runtime.
     *
     * @param saveData Indicates the user data to be saved.
     * @return If the data is saved successfully, it returns true; otherwise, it returns false.
     */
    bool OnSaveData(WantParams &saveData) override;

    /**
     * @brief After creating the Ability on the remote device,
     *      immediately restore the user data saved during the migration of the Ability on the remote device.
     * @param restoreData Indicates the user data to be restored.
     * @return If the data is restored successfully, it returns true; otherwise, it returns false .
     */
    bool OnRestoreData(WantParams &restoreData) override;

    /**
     * @brief Migrates this ability to the given device on the same distributed network in a reversible way that allows
     * this ability to be migrated back to the local device through reverseContinueAbility(). The ability to migrate and
     * its ability slices must implement the IAbilityContinuation interface. Otherwise, an exception is thrown,
     * indicating that the ability does not support migration.
     *
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to. If this parameter
     * is null, this method has the same effect as ContinueAbilityReversibly().
     *
     */
    virtual void ContinueAbilityReversibly(const std::string &deviceId) final;

    /**
     * @brief  Obtains the ID of the source device from which this ability is migrated.
     *
     * @return Returns the source device ID.
     */
    virtual std::string GetOriginalDeviceId() final;

    /**
     * @brief Obtains the migration state of this ability.
     * @return Returns the migration state.
     */
    virtual ContinuationState GetContinuationState() final;

    /**
     * @brief Obtains the lifecycle state of this ability.
     *
     * @return Returns the lifecycle state of this ability.
     */
    virtual AbilityLifecycleExecutor::LifecycleState GetState() final;

    /**
     * @brief Release the ability instance.
     */
    void DestroyInstance();

    /**
     * @brief Posts a scheduled Runnable task to a new non-UI thread.
     * The task posted via this method will be executed in a new thread, which allows you to perform certain
     * time-consuming operations. To use this method, you must also override the supportHighPerformanceUI() method.
     * Additionally, the usage of this method must comply with the following constraints: 1、This method can only be
     * used to initialize the component tree in parallel mode. 2、The task can only be processed during system cold
     * start. 3、If the parallel loading mechanism is used, the component tree-related operations to be performed in
     * onActive() and onStop() of the ability slice must also be added to the parallel thread. 4、You must run
     * setUIContent(ohos.agp.components.ComponentContainer) to unlock the UI thread and wait until the component tree is
     * ready. 5、Other time-consuming operations, such as I/O and network processing, cannot be added to the parallel
     * task queue.
     *
     * @param task Indicates the Runnable task to post.
     *
     * @param delayTime Indicates the number of milliseconds after which the task will be executed.
     *
     * @return -
     */
    void PostTask(std::function<void()> task, long delayTime);

    /**
     * @brief Create a PostEvent timeout task. The default delay is 5000ms
     *
     * @return Return a smart pointer to a timeout object
     */
    std::shared_ptr<AbilityPostEventTimeout> CreatePostEventTimeouter(std::string taskstr);

    /**
     * @brief Keeps this Service ability in the background and displays a notification bar.
     * To use this method, you need to request the ohos.permission.KEEP_BACKGROUND_RUNNING permission from the system.
     * The ohos.permission.KEEP_BACKGROUND_RUNNING permission is of the normal level.
     * This method can be called only by Service abilities after the onStart(ohos.aafwk.content.Want) method is called.
     *
     * @param id Identifies the notification bar information.
     * @param notificationRequest Indicates the NotificationRequest instance containing information for displaying a
     * notification bar.
     */
    virtual void KeepBackgroundRunning(int id, const NotificationRequest &notificationRequest) final;

    /**
     * @brief Cancels background running of this ability to free up system memory.
     * This method can be called only by Service abilities when the onStop() method is called.
     *
     */
    virtual void CancelBackgroundRunning() final;

    /**
     * @brief Keep this Service ability in the background and displays a notification bar.
     *
     * @param wantAgent Indicates which ability to start when user click the notification bar.
     * @return the method result code, 0 means succeed
     */
    virtual int StartBackgroundRunning(const AbilityRuntime::WantAgent::WantAgent &wantAgent) final;

    /**
     * @brief Cancel background running of this ability to free up system memory.
     *
     * @return the method result code, 0 means succeed
     */
    virtual int StopBackgroundRunning() final;

    /**
     * @brief Prepare user data of local Ability.
     *
     * @param wantParams Indicates the user data to be saved.
     * @return If the ability is willing to continue and data saved successfully, it returns 0;
     * otherwise, it returns errcode.
     */
    virtual int32_t OnContinue(WantParams &wantParams);

    /**
     * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
     * ability slices must implement the IAbilityContinuation interface.
     *
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to.
     * @param versionCode Target bundle version.
     */
    virtual void ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode) final;

    /**
     * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
     * ability slices must implement the IAbilityContinuation interface.
     *
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to. If this parameter
     * is null, this method has the same effect as continueAbility().
     *
     */
    virtual void ContinueAbility(const std::string &deviceId) final;

    /**
     * @brief Callback function to ask the user whether to start the migration .
     *
     * @return If the user allows migration, it returns true; otherwise, it returns false.
     */
    virtual bool OnStartContinuation() override;

    /**
     * @brief This function can be used to implement the processing logic after the migration is completed.
     *
     * @param result Migration result code. 0 means the migration was successful, -1 means the migration failed.
     * @return None.
     */
    virtual void OnCompleteContinuation(int result) override;

    /**
     * @brief Used to notify the local Ability that the remote Ability has been destroyed.
     *
     * @return None.
     */
    virtual void OnRemoteTerminated() override;

    /**
     * @brief Prepare user data of local Ability.
     *
     * @param reason the reason why framework invoke this function
     * @param wantParams Indicates the user data to be saved.
     * @return result code defined in abilityConstants
     */
    virtual int32_t OnSaveState(int32_t reason, WantParams &wantParams);

    /**
     * @brief enable ability recovery.
     *
     * @param abilityRecovery shared_ptr of abilityRecovery
     */
    void EnableAbilityRecovery(const std::shared_ptr<AbilityRecovery>& abilityRecovery);

    /**
     * @brief Callback when the ability is shared.You can override this function to implement your own sharing logic.
     *
     * @param wantParams Indicates the user data to be saved.
     * @return the result of OnShare
     */
    virtual int32_t OnShare(WantParams &wantParams);

#ifdef SUPPORT_SCREEN
public:
    friend class PageAbilityImpl;
    uint32_t sceneFlag_ = 0;

    /**
     * @brief Sets the background color of the window in RGB color mode.
     *
     * @param red The value ranges from 0 to 255.
     *
     * @param green The value ranges from 0 to 255.
     *
     * @param blue The value ranges from 0 to 255.
     *
     * @return Returns the result of SetWindowBackgroundColor
     */
    virtual int SetWindowBackgroundColor(int red, int green, int blue) final;

    /**
     * @brief Informs the system of the time required for drawing this Page ability.
     *
     * @return Returns the notification is successful or fail
     */
    bool PrintDrawnCompleted() override;

    /**
     * @brief Called after instantiating WindowScene.
     *
     *
     * You can override this function to implement your own processing logic.
     */
    virtual void OnSceneCreated();

    /**
     * @brief Called after ability stoped.
     *
     *
     * You can override this function to implement your own processing logic.
     */
    virtual void onSceneDestroyed();

    /**
     * @brief Called after ability restored.
     *
     *
     * You can override this function to implement your own processing logic.
     */
    virtual void OnSceneRestored();

    /**
     * @brief Called when this ability enters the <b>STATE_FOREGROUND</b> state.
     *
     *
     * The ability in the <b>STATE_FOREGROUND</b> state is visible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnForeground(const Want &want);

    /**
     * @brief Called when this ability enters the <b>STATE_BACKGROUND</b> state.
     *
     *
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnBackground();

    /**
     * @brief Called when back press is dispatched.
     *
     * @return Return true if ability will be moved to background; return false if will be terminated.
     */
    virtual bool OnBackPress() override;

    /**
     * @brief Called when ability prepare terminate.
     *
     * @return Return true if ability need to stop terminating; return false if ability need to terminate.
     */
    virtual bool OnPrepareTerminate();

    /**
     * @brief Called when a key is pressed. When any component in the Ability gains focus, the key-down event for
     * the component will be handled first. This callback will not be invoked if the callback triggered for the
     * key-down event of the component returns true. The default implementation of this callback does nothing
     * and returns false.
     *
     * @param keyEvent Indicates the key-down event.
     *
     * @return Returns true if this event is handled and will not be passed further; returns false if this event
     * is not handled and should be passed to other handlers.
     */
    virtual void OnKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent);

    /**
     * @brief Called when a key is released. When any component in the Ability gains focus, the key-up event for
     * the component will be handled first. This callback will not be invoked if the callback triggered for the
     * key-up event of the component returns true. The default implementation of this callback does nothing and
     * returns false.
     *
     * @param keyEvent Indicates the key-up event.
     *
     * @return Returns true if this event is handled and will not be passed further; returns false if this event
     * is not handled and should be passed to other handlers.
     */
    virtual void OnKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent);

    /**
     * @brief Called when a touch event is dispatched to this ability. The default implementation of this callback
     * does nothing and returns false.
     *
     * @param event  Indicates information about the touch event.
     *
     * @return Returns true if the event is handled; returns false otherwise.
     */
    virtual void OnPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent);

    /**
     * @brief Called when this ability gains or loses window focus.
     *
     * @param hasFocus Specifies whether this ability has focus.
     */
    virtual void OnWindowFocusChanged(bool hasFocus);

    /**
     * @brief Called when this ability is moved to or removed from the top of the stack.
     *
     * @param topActive Specifies whether this ability is moved to or removed from the top of the stack. The value true
     * indicates that it is moved to the top, and false indicates that it is removed from the top of the stack.
     */
    virtual void OnTopActiveAbilityChanged(bool topActive);

    /**
     * @brief Inflates UI controls by using windowOption.
     *
     * @param windowOption Indicates the window option defined by the user.
     */
    virtual void InitWindow(int32_t displayId, sptr<Rosen::WindowOption> option);

    /**
     * @brief Get the window belong to the ability.
     *
     * @return Returns a Window object pointer.
     */
    virtual const sptr<Rosen::Window> GetWindow();

    /**
     * @brief get the scene belong to the ability.
     *
     * @return Returns a WindowScene object pointer.
     */
    std::shared_ptr<Rosen::WindowScene> GetScene();

    /**
     * @brief Checks whether the main window of this ability has window focus.
     *
     * @return Returns true if this ability currently has window focus; returns false otherwise.
     */
    bool HasWindowFocus();

    void SetShowOnLockScreen(bool showOnLockScreen);

    /**
     * @brief When the ability starts, set whether to wake up the screen.
     *
     * @param wakeUp Set true to wake up, false to not wake up.
     */
    void SetWakeUpScreen(bool wakeUp);

    /**
     * @brief Set the display orientation of the main window.
     *
     * @param orientation Indicates the display orientation of the window.
     */
    void SetDisplayOrientation(int orientation);

    /**
     * @brief Get the display orientation of the main window.
     *
     * @return Returns the display orientation of the window.
     */
    int GetDisplayOrientation() override;

    /**
     * @brief Called when this ability is about to leave the foreground and enter the background due to a user
     * operation, for example, when the user touches the Home key.
     *
     */
    virtual void OnLeaveForeground();

    /**
     * @brief Sets the type of audio whose volume will be adjusted by the volume button.
     *
     * @param volumeType Indicates the AudioManager.AudioVolumeType to set.
     */
    virtual void SetVolumeTypeAdjustedByKey(int volumeType);

    /**
     * @brief Called to return a FormProviderInfo object.
     *
     * <p>You must override this method if your ability will serve as a form provider to provide a form for clients.
     * The default implementation returns nullptr. </p>
     *
     * @param want   Indicates the detailed information for creating a FormProviderInfo.
     *               The Want object must include the form ID, form name of the form,
     *               which can be obtained from Ability#PARAM_FORM_IDENTITY_KEY,
     *               Ability#PARAM_FORM_NAME_KEY, and Ability#PARAM_FORM_DIMENSION_KEY,
     *               respectively. Such form information must be managed as persistent data for further form
     *               acquisition, update, and deletion.
     *
     * @return Returns the created FormProviderInfo object.
     */
    virtual FormProviderInfo OnCreate(const Want &want);

    virtual bool OnShare(int64_t formId, AAFwk::WantParams &wantParams);

    /**
     * @brief Called to notify the form provider that a specified form has been deleted. Override this method if
     * you want your application, as the form provider, to be notified of form deletion.
     *
     * @param formId Indicates the ID of the deleted form.
     * @return None.
     */
    virtual void OnDelete(const int64_t formId);

    /**
     * @brief Called when the form provider is notified that a temporary form is successfully converted to
     * a normal form.
     *
     * @param formId Indicates the ID of the form.
     * @return None.
     */
    virtual void OnCastTemptoNormal(const int64_t formId);

    /**
     * @brief Called to notify the form provider to update a specified form.
     *
     * @param formId Indicates the ID of the form to update.
     * @return none.
     */
    virtual void OnUpdate(const int64_t formId, const AAFwk::WantParams &wantParams);

    /**
     * @brief Called when the form provider receives form events from the fms.
     *
     * @param formEventsMap Indicates the form events occurred. The key in the Map object indicates the form ID,
     *                      and the value indicates the event type, which can be either FORM_VISIBLE
     *                      or FORM_INVISIBLE. FORM_VISIBLE means that the form becomes visible,
     *                      and FORM_INVISIBLE means that the form becomes invisible.
     * @return none.
     */
    virtual void OnVisibilityChanged(const std::map<int64_t, int32_t> &formEventsMap);
    /**
     * @brief Called to notify the form provider to update a specified form.
     *
     * @param formId Indicates the ID of the form to update.
     * @param message Form event message.
     */
    virtual void OnTriggerEvent(const int64_t formId, const std::string &message);
    /**
     * @brief Called to notify the form supplier to acquire form state.
     *
     * @param want Indicates the detailed information about the form to be obtained, including
     *             the bundle name, module name, ability name, form name and form dimension.
     */
    virtual FormState OnAcquireFormState(const Want &want);

    /**
     * @brief Get page ability stack info.
     *
     * @return A string represents page ability stack info, empty if failed;
     */
    virtual std::string GetContentInfo();

    /**
     * @brief Set WindowScene listener
     *
     * @param listener WindowScene listener
     * @return None.
     */
    void SetSceneListener(const sptr<Rosen::IWindowLifeCycle> &listener);

    /**
     * @brief Called back at ability context.
     *
     * @return current window mode of the ability.
     */
    virtual int GetCurrentWindowMode() override;

    /**
     * @brief Set mission label of this ability.
     *
     * @param label the label of this ability.
     * @return Returns ERR_OK if success.
     */
    virtual ErrCode SetMissionLabel(const std::string &label) override;

    /**
     * @brief Set mission icon of this ability.
     *
     * @param icon the icon of this ability.
     * @return Returns ERR_OK if success.
     */
    virtual ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;

    /**
     * @brief Get window rectangle of this ability.
     *
     * @param the left position of window rectangle.
     * @param the top position of window rectangle.
     * @param the width position of window rectangle.
     * @param the height position of window rectangle.
     */
    virtual void GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height) override;

    /**
     * @brief Get ui content object.
     *
     * @return UIContent object of ACE.
     */
    Ace::UIContent* GetUIContent() override;

    /**
     * @brief create modal UIExtension.
     * @param want Create modal UIExtension with want object.
     */
    int CreateModalUIExtension(const Want &want);

    /**
     * @brief Update sessionToken.
     * @param sessionToken The token of session.
     */
    void UpdateSessionToken(sptr<IRemoteObject> sessionToken);

    void EraseUIExtension(int32_t sessionId) override;

protected:
    class AbilityDisplayListener : public OHOS::Rosen::DisplayManager::IDisplayListener {
    public:
        explicit AbilityDisplayListener(const std::weak_ptr<Ability>& ability)
        {
            ability_ = ability;
        }

        void OnCreate(Rosen::DisplayId displayId) override
        {
            auto sptr = ability_.lock();
            if (sptr != nullptr) {
                sptr->OnCreate(displayId);
            }
        }

        void OnDestroy(Rosen::DisplayId displayId) override
        {
            auto sptr = ability_.lock();
            if (sptr != nullptr) {
                sptr->OnDestroy(displayId);
            }
        }

        void OnChange(Rosen::DisplayId displayId) override
        {
            auto sptr = ability_.lock();
            if (sptr != nullptr) {
                sptr->OnChange(displayId);
            }
        }
    private:
        std::weak_ptr<Ability> ability_;
    };

    /**
     * @brief override Rosen::DisplayManager::IDisplayListener virtual callback function
     *
     * @param displayId displayId
     */
    void OnCreate(Rosen::DisplayId displayId);
    void OnDestroy(Rosen::DisplayId displayId);
    void OnChange(Rosen::DisplayId displayId);

    class AbilityDisplayMoveListener : public OHOS::Rosen::IDisplayMoveListener {
    public:
        explicit AbilityDisplayMoveListener(std::weak_ptr<Ability>&& ability) : ability_(ability) {}

        void OnDisplayMove(Rosen::DisplayId from, Rosen::DisplayId to) override
        {
            auto sptr = ability_.lock();
            if (sptr != nullptr) {
                sptr->OnDisplayMove(from, to);
            }
        }
    private:
        std::weak_ptr<Ability> ability_;
    };

    /**
     * @brief override Rosen::IDisplayMoveListener virtual callback function
     *
     * @param from the displayId before display move
     * @param to the displayId after display move
     */
    void OnDisplayMove(Rosen::DisplayId from, Rosen::DisplayId to);

    /**
     * @brief process when foreground executed.
     *
     * You can override this function to implement your own processing logic
     */
    virtual void DoOnForeground(const Want& want);

    /**
     * @brief request focus for current window.
     *
     * You can override this function to implement your own processing logic
     */
    virtual void RequestFocus(const Want &want);

    /**
     * @brief Acquire the window option.
     * @return window option.
     */
    sptr<Rosen::WindowOption> GetWindowOption(const Want &want);

    /**
     * @brief Restore window stage data and scene in ability continuation.
     *
     */
    virtual void ContinuationRestore(const Want &want);

    void SetSessionToken(sptr<IRemoteObject> sessionToken);

    std::shared_ptr<Rosen::WindowScene> scene_ = nullptr;
    sptr<Rosen::IWindowLifeCycle> sceneListener_ = nullptr;
    sptr<AbilityDisplayListener> abilityDisplayListener_ = nullptr;
    sptr<Rosen::IDisplayMoveListener> abilityDisplayMoveListener_ = nullptr;
#endif

protected:
    /**
     * @brief Acquire the launch parameter.
     * @return launch parameter.
     */
    const AAFwk::LaunchParam& GetLaunchParam() const;

    /**
     * @brief judge where invoke restoreWindowStage in continuation
     * @return true if invoked restoreWindowStage in continuation.
     */
    bool IsRestoredInContinuation() const;

    /**
     * @brief Notify continuation
     *
     * @param want the want param.
     * @param success whether continuation success.
     */
    void NotifyContinuationResult(const Want& want, bool success);

    /**
     * @brief judge whether we should restore state
     * @return true if we we should restore state
     */
    bool ShouldRecoverState(const Want& want);

    bool IsUseNewStartUpRule();

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_ = nullptr;
    std::shared_ptr<AbilityStartSetting> setting_ = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo_ = nullptr;
    LaunchParam launchParam_;
    bool securityFlag_ = false;

private:
    std::shared_ptr<NativeRdb::DataAbilityPredicates> ParsePredictionArgsReference(
        std::vector<std::shared_ptr<DataAbilityResult>> &results, std::shared_ptr<DataAbilityOperation> &operation,
        int numRefs);

    std::shared_ptr<NativeRdb::ValuesBucket> ParseValuesBucketReference(
        std::vector<std::shared_ptr<DataAbilityResult>> &results, std::shared_ptr<DataAbilityOperation> &operation,
        int numRefs);

    int ChangeRef2Value(std::vector<std::shared_ptr<DataAbilityResult>> &results, int numRefs, int index);

    bool CheckAssertQueryResult(std::shared_ptr<NativeRdb::AbsSharedResultSet> &queryResult,
        std::shared_ptr<NativeRdb::ValuesBucket> &&valuesBucket);

    void DispatchLifecycleOnForeground(const Want &want);

    friend class AbilityImpl;
    bool VerifySupportForContinuation();
    void HandleCreateAsContinuation(const Want &want);
    bool IsFlagExists(unsigned int flag, unsigned int flagSet);
    void HandleCreateAsRecovery(const Want &want);
    /**
     * @brief Set the start ability setting.
     * @param setting the start ability setting.
     */
    void SetStartAbilitySetting(std::shared_ptr<AbilityStartSetting> setting);

    /**
     * @brief Set the launch param.
     *
     * @param launchParam the launch param.
     */
    void SetLaunchParam(const AAFwk::LaunchParam &launchParam);

    void InitConfigurationProperties(const Configuration& changeConfiguration, std::string& language,
        std::string& colormode, std::string& hasPointerDevice);

    void ParseIntValue(const NativeRdb::ValueObject &obj, const std::string &key,
        NativeRdb::ValuesBucket &retValueBucket) const;
    void ParseDoubleValue(const NativeRdb::ValueObject &obj, const std::string &key,
        NativeRdb::ValuesBucket &retValueBucket) const;
    void ParseStringValue(const NativeRdb::ValueObject &obj, const std::string &key,
        NativeRdb::ValuesBucket &retValueBucket) const;
    void ParseBlobValue(const NativeRdb::ValueObject &obj, const std::string &key,
        NativeRdb::ValuesBucket &retValueBucket) const;
    void ParseBoolValue(const NativeRdb::ValueObject &obj, const std::string &key,
        NativeRdb::ValuesBucket &retValueBucket) const;

    std::shared_ptr<ContinuationHandler> continuationHandler_ = nullptr;
    std::shared_ptr<ContinuationManager> continuationManager_ = nullptr;
    std::shared_ptr<AbilityHandler> handler_ = nullptr;
    std::shared_ptr<LifeCycle> lifecycle_ = nullptr;
    std::shared_ptr<AbilityLifecycleExecutor> abilityLifecycleExecutor_ = nullptr;
    std::shared_ptr<OHOSApplication> application_ = nullptr;
    std::vector<std::string> types_;
    std::map<int, FeatureAbilityTask> resultCallbacks_;
    std::shared_ptr<AAFwk::Want> setWant_ = nullptr;
    sptr<IRemoteObject> reverseContinuationSchedulerReplica_ = nullptr;

    static const std::string SYSTEM_UI;
    static const std::string STATUS_BAR;
    static const std::string NAVIGATION_BAR;
    static const std::string KEYGUARD;
    sptr<IRemoteObject> providerRemoteObject_ = nullptr;
    // Keep consistent with DMS defines. Used to callback to DMS.
    static const std::string DMS_SESSION_ID;

    // The originating deviceId passed by DMS using want param.
    static const std::string DMS_ORIGIN_DEVICE_ID;

    // If session id cannot get from want, assign it as default.
    static const int DEFAULT_DMS_SESSION_ID;

    bool isNewRuleFlagSetted_ = false;
    bool startUpNewRule_ = false;

#ifdef SUPPORT_SCREEN
private:
    void InitFAWindow(const Want &want, int32_t displayId);
    bool UpdateResMgrAndConfiguration(int32_t displayId);

    std::shared_ptr<AbilityWindow> abilityWindow_ = nullptr;
    bool bWindowFocus_ = false;
    bool showOnLockScreen_ = false;
#endif
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_H