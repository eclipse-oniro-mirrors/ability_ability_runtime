/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_MY_STATUS_H
#define MOCK_MY_STATUS_H
#include "bundle_mgr_helper.h"
#include "remote_client_manager.h"
#include "app_running_record.h"
#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "user_record_manager.h"
#include "exit_resident_process_manager.h"

#include <mutex>
#include "hilog_tag_wrapper.h"
namespace OHOS {
namespace AAFwk {
class MyStatus {
public:
    static MyStatus& GetInstance();
    ~MyStatus() = default;
    std::shared_ptr<AppExecFwk::BundleMgrHelper> getBundleManagerHelper_ = nullptr;
    ErrCode getBundleInfoV9_ = ERR_OK;
    ErrCode getCloneBundleInfo_ = ERR_OK;
    ErrCode getSandboxBundleInfo_ = ERR_OK;
    bool getHapModuleInfo_ = false;
    ErrCode getSandboxHapModuleInfo_ = ERR_OK;
    bool isLogoutUser_ = false;
    pid_t getCallingUid_ = 0;
    pid_t getCallingTokenID_ = 0;
    ErrCode processUpdate_ = ERR_OK;
    bool verifyCallingPermission_ = false;
    bool verifySuperviseKiaServicePermission_ = false;
    bool isSACall_ = false;
    bool isShellCall_ = false;
    std::shared_ptr<AppExecFwk::AppRunningRecord> getAppRunningRecordByPid_ = nullptr;
    std::string getNameForUid_ = "";
    // mock app running manager
    std::list<pid_t> getPidsByBundleName_;
    bool getPidsByBundleNameRet_;
    std::map<const int32_t, const std::shared_ptr<AppExecFwk::AppRunningRecord>> getAppRunningRecordMap_;
    int notifyProcMemoryCall_ = 0;
    int notifyProcMemory_ = 0;
    int dumpHeapMemory_ = 0;
    int dumpJsHeapMemory_ = 0;
    bool processExit_ = false;
    std::shared_ptr<AppExecFwk::AppRunningRecord> checkAppRunning_ = nullptr;
    bool checkAppRunningByUid_ = false;
    int checkAppRunningCall_ = 0;
    int isAppExistCall_ = 0;
    std::shared_ptr<AppExecFwk::AppRunningRecord> createAppRunning_ = nullptr;
    int32_t updateConfigurationByBundleName_ = 0;
    std::shared_ptr<AppExecFwk::AppRunningRecord> getAppRunningRecordByRenderPid_ = nullptr;
    int32_t checkAppClone_ = 0;
    int32_t notifyLoadRepairPatch_ = 0;
    int32_t notifyHotReloadPage_ = 0;
    int32_t notifyUnLoadRepairPatch_ = 0;
    int32_t isAppRunningByBundleName_ = 0;
    std::shared_ptr<AppExecFwk::AppRunningRecord> getAppRunningByToken_ = nullptr;
    std::shared_ptr<AppExecFwk::AppRunningRecord> getAppRunningProcessPid_ = nullptr;
    int32_t getAppRunningProcessPidCall_ = 0;
    bool isChildProcessReachLimit_ = false;
    int dumpIpcAllStart_ = ERR_OK;
    int dumpIpcAllStop_ = ERR_OK;
    int dumpIpcAllStat_ = ERR_OK;
    int dumpIpcStart_ = ERR_OK;
    int dumpIpcStop_ = ERR_OK;
    int dumpIpcStat_ = ERR_OK;
    int dumpFfrt_ = ERR_OK;
    int32_t dumpArkWeb_ = ERR_OK;
    int32_t getAllAppRunningRecordCount_ = 0;
    pid_t handleUserRequestCleanPid_ = 0;
    pid_t handleUserRequestCleanUid_ = 0;
    bool handleUserRequestClean_ = false;
    ErrCode checkIsKiaProcess_ = ERR_OK;
    // mock accesstoken_kit
    int clearUserGranted_ = 0;
    // mock bundle
    bool cleanBundleDataFiles_ = false;
    bool getApplicationInfo_ = false;
    AppExecFwk::ApplicationInfo applicationInfo_;
    int getOverlayCall_ = 0;
    sptr<AppExecFwk::IOverlayManager> getOverlay_ = nullptr;
    int getBaseSharedBundleInfos_ = 0;
    std::vector<AppExecFwk::BaseSharedBundleInfo> baseSharedBundleInfos_;
    bool queryDataGroupInfos_ = false;
    std::vector<AppExecFwk::DataGroupInfo> queryData_;
    bool queryAbilityInfo_ = false;
    AppExecFwk::AbilityInfo queryAbilityInfoValue_;
    AppExecFwk::BundleInfo v9BundleInfo_;
    // permission verification
    bool judgeCallerIsAllowed_ = false;
    bool verifyRunningInfoPerm_ = false;
    bool checkSpecific_ = false;
    int32_t verifyUpdateAPPConfigurationPerm_ = 0;
    // parameters
    bool getBoolParameter_ = false;
    // app running record
    int32_t getAppIndex_ = 0;
    int scheduleAcceptCall_ = 0;
    std::shared_ptr<AppExecFwk::ModuleRunningRecord> getModuleRecord_ = nullptr;
    int addModulesCall_;
    int schedulePrepareCall_ = 0;
    int getNewProcessRequestId_ = 0;
    int getNewProcessRequestIdCall_ = 0;
    int resetNewProcessRequestCall_ = 0;
    int getBrowserHostCall_ = 0;
    std::shared_ptr<AppExecFwk::RenderRecord> getRenderRecordByPid_ = nullptr;
    int setBrowserHostCall_ = 0;
    std::list<std::shared_ptr<AppExecFwk::ApplicationInfo>> getAppInfoList_;
    int32_t changeAppGcState_ = 0;
    std::shared_ptr<AppExecFwk::AbilityRunningRecord> getAbilityRunningRecordByToken_ = nullptr;
    std::shared_ptr<AppExecFwk::AbilityRunningRecord> getAbilityRunningRecordByTokenModule_ = nullptr;
    std::shared_ptr<AppExecFwk::ChildProcessRecord> getChildProcessRecordByPid_ = nullptr;
    int32_t getChildProcessRecordByPidCall_ = 0;
    // remote client
    std::shared_ptr<AppExecFwk::AppSpawnClient> getSpawnClient_;
    std::shared_ptr<AppExecFwk::AppSpawnClient> getNWebSpawnClient_;
    int getSpawnClientCall_ = 0;
    // app spawn client
    int32_t startProcess_ = ERR_OK;
    // os account
    ErrCode queryActiveOsAccountIds_ = ERR_OK;
    ErrCode getOsAccountLocalIdFromUid_ = ERR_OK;
    // exit resident
    ErrCode handleMemorySizeInSufficent_ = ERR_OK;
    ErrCode handleRequireBigMemoryOptimization_ = ERR_OK;
    ErrCode handleNoRequireBigMemoryOptimization_ = ERR_OK;
    // preload related
    bool allowPreload_ = true;
    int32_t generatePreloadRequestRet_ = ERR_OK;
    
    // HandlePreloadApplication tracking variables
    bool handlePreloadApplication_nullAbilityInfo_called_ = false;
    bool handlePreloadApplication_nullAppRunningManager_called_ = false;
    bool handlePreloadApplication_existingAppRecord_called_ = false;
    bool handlePreloadApplication_appMultiUserNotExist_called_ = false;
    bool handlePreloadApplication_appRecordCreated_called_ = false;
    bool handlePreloadApplication_preloadModuleMode_called_ = false;
    bool handlePreloadApplication_normalFlow_completed_ = false;
    
    // RestartResidentProcessDependedOnWeb tracking variables
    bool restartResidentProcessDependedOnWeb_emptyBundleNames_called_ = false;
    bool restartResidentProcessDependedOnWeb_nullTaskHandler_called_ = false;
    bool restartResidentProcessDependedOnWeb_taskSubmitted_called_ = false;
    bool handleExitResidentBundleDependedOnWeb_called_ = false;
    std::vector<AppExecFwk::ExitResidentProcessInfo> mockBundleNames_;
    
    void resetHandlePreloadApplicationFlags()
    {
        handlePreloadApplication_nullAbilityInfo_called_ = false;
        handlePreloadApplication_nullAppRunningManager_called_ = false;
        handlePreloadApplication_existingAppRecord_called_ = false;
        handlePreloadApplication_appMultiUserNotExist_called_ = false;
        handlePreloadApplication_appRecordCreated_called_ = false;
        handlePreloadApplication_preloadModuleMode_called_ = false;
        handlePreloadApplication_normalFlow_completed_ = false;
    }

    void resetRestartResidentProcessDependedOnWebFlags()
    {
        restartResidentProcessDependedOnWeb_emptyBundleNames_called_ = false;
        restartResidentProcessDependedOnWeb_nullTaskHandler_called_ = false;
        restartResidentProcessDependedOnWeb_taskSubmitted_called_ = false;
        handleExitResidentBundleDependedOnWeb_called_ = false;
        mockBundleNames_.clear();
    }

    bool getApplicationInfoCalled_ = false;
    void resetGetApplicationInfoFlag()
    {
        getApplicationInfoCalled_ = false;
    }
    bool appDeathRecipientSetTaskHandlerCalled_ = false;
    void resetAppDeathRecipientSetTaskHandlerFlag()
    {
        appDeathRecipientSetTaskHandlerCalled_ = false;
    }

    //app running record function called
    bool appRunningRecordSetAppDeathRecipientCalled_ = false;
    bool getPidCall_ = false;
    bool addDeathRecipientReturn_ = false;
    bool setNWebPreloadCalled_ = false;
    bool addAppLifecycleEventCalled_ = false;
    bool getNameCalled_ = false;
    bool scheduleForegroundRunningCalled_ = false;
    bool setApplicationScheduleStateCalled_ = false;
    bool getPriorityObjectCalled_ = false;
    int debugAppCalledTimes_ = 0;
    int addModuleCalledTimes_ = 0;
    bool getStateCalled_ = false;
    bool getBundleNamesCalled_ = false;
    bool isStartSpecifiedAbility_ = false;
    bool isNewProcessRequest_ = false;
    int getSpecifiedRequestIdCall_ = 0;
    bool setStateCalled_ = false;
    int resetSpecifiedRequestCall_ = 0;
    bool getBundleNameCalled_ = false;
    int removeChildProcessRecordCall_ = 0;
    int getParentAppRecordCall_ = 0;
    bool getAbilityRunningRecordByTokenCalled_ = false;
    bool setApplicationClientCalled_ = false;
    bool getRecordIdCalled_ = false;
    bool isGetAppRunningByBundleName_ = false;
    bool getDialogEnabled_ = false;
    int getCallingPid_ = 0;
    bool setAssertionPauseFlagCalled_ = false;
    bool getAbilitiesCalled_ = false;
    std::map<const sptr<IRemoteObject>, std::shared_ptr<AppExecFwk::AbilityRunningRecord>> abilitiesMap_;
    bool notifyAppPreCacheCalled_ = false;
    bool notifyStartResidentProcessCalled_ = false;
    bool notifyStartKeepAliveProcessCalled_ = false;
    bool recordAppExitSignalReasonCall_ = false;
    int getCallingUidCalledTimes_ = 0;
    int getAppRunningRecordMapCall_ = 0;
    bool setKeepAliveEnableStateCalled_ = false;
    bool setKeepAliveDkvCalled_ = false;
    bool isKeepAliveApp_ = false;
    bool getUidCalled_ = false;
    bool queryExitBundleInfos_called_ = false;
    bool addUIExtensionBindItemCalled_ = false;
    bool killProcessByPidCalled_ = false;
    pid_t killProcessByPid_pid_ = 0;
    bool setUserRequestCleaningCalled_ = false;
    bool isAllAbilityReadyToCleanedByUserRequestCalled_ = false;
    bool isAllAbilityReadyToCleanedByUserRequest_ = false;
    bool queryUIExtensionBindItemByIdCalled_ = false;
    bool removeUIExtensionBindItemByIdCalled_ = false;
    int32_t queryUIExtensionBindItemByIdResult_ = ERR_OK;
    int32_t notifyProcessBind_ = 0;

    bool getAbilityInfoCalled_ = false;
    bool isKeepAliveAppCalled_ = false;
    bool getUIExtensionBindAbilityIdCalled_ = false;
    int32_t queryUIExtensionBindItemById_ = static_cast<int32_t>(ERR_INVALID_VALUE);
    bool getAbilityRunningRecordByTokenCalledAppRecord_ = false;
    void resetRunningRecordFunctionFlagExtend()
    {
        appRunningRecordSetAppDeathRecipientCalled_ = false;
        getPidCall_ = false;
        addDeathRecipientReturn_ = false;
        setNWebPreloadCalled_ = false;
        addAppLifecycleEventCalled_ = false;
        getNameCalled_ = false;
        scheduleForegroundRunningCalled_ = false;
        setApplicationScheduleStateCalled_ = false;
        getPriorityObjectCalled_ = false;
        debugAppCalledTimes_ = 0;
        addModuleCalledTimes_ = 0;
        getStateCalled_ = false;
        getBundleNamesCalled_ = false;
        isStartSpecifiedAbility_ = false;
        isNewProcessRequest_ = false;
        getSpecifiedRequestIdCall_ = 0;
        getNewProcessRequestIdCall_ = 0;
        setStateCalled_ = false;
        resetNewProcessRequestCall_ = 0;
        resetSpecifiedRequestCall_ = 0;
        getBundleNameCalled_ = false;
        getChildProcessRecordByPidCall_ = 0;
        getAppRunningProcessPidCall_ = 0;
        removeChildProcessRecordCall_ = 0;
        getParentAppRecordCall_ = 0;
        getAbilityRunningRecordByTokenCalled_ = false;
        getAbilityRunningRecordByTokenCalledAppRecord_ = false;
        setApplicationClientCalled_ = false;
        getRecordIdCalled_ = false;
        isGetAppRunningByBundleName_ = false;
        getDialogEnabled_ = false;
        getCallingPid_ = 0;
    }
    void resetRunningRecordFunctionFlag()
    {
        resetRunningRecordFunctionFlagExtend();
        setAssertionPauseFlagCalled_ = false;
        getAbilitiesCalled_ = false;
        abilitiesMap_.clear();
        notifyAppPreCacheCalled_ = false;
        notifyStartResidentProcessCalled_ = false;
        notifyStartKeepAliveProcessCalled_ = false;
        recordAppExitSignalReasonCall_ = false;
        getCallingUidCalledTimes_ = 0;
        getAppRunningRecordMapCall_ = 0;
        setKeepAliveEnableStateCalled_ = false;
        setKeepAliveDkvCalled_ = false;
        isKeepAliveApp_ = false;
        getUidCalled_ = false;
        queryExitBundleInfos_called_ = false;
        addUIExtensionBindItemCalled_ = false;
        killProcessByPidCalled_ = false;
        killProcessByPid_pid_ = 0;
        setUserRequestCleaningCalled_ = false;
        isAllAbilityReadyToCleanedByUserRequestCalled_ = false;
        isAllAbilityReadyToCleanedByUserRequest_ = false;
        queryUIExtensionBindItemByIdCalled_ = false;
        removeUIExtensionBindItemByIdCalled_ = false;
        queryUIExtensionBindItemByIdResult_ = ERR_OK;
        notifyProcessBind_ = 0;
        getAbilityInfoCalled_ = false;
        isKeepAliveAppCalled_ = false;
        getUIExtensionBindAbilityIdCalled_ = false;
        queryUIExtensionBindItemById_ = static_cast<int32_t>(ERR_INVALID_VALUE);
    }

    // StartAbility tracking variables
    bool startAbility_nullAbilityInfo_called_ = true;
    bool startAbility_nullAppRecord_called_ = true;
    bool startAbility_singletonAbilityExists_called_ = false;
    bool startAbility_abilityExistsWithPreToken_called_ = false;
    bool startAbility_addModuleFailed_called_ = false;
    bool startAbility_addAbilityFailed_called_ = false;
    bool startAbility_appStateCreate_called_ = false;
    bool startAbility_launchAbility_called_ = false;
    bool startAbility_processAppDebug_called_ = false;
    
    // Control variables for simulating different scenarios
    bool simulateAddModuleFails_ = false;
    bool simulateAddAbilityFails_ = false;
    bool simulateSingletonAbilityExists_ = false;
    bool simulateAbilityExistsWithPreToken_ = false;
    bool addModuleCalled_ = false;
    bool getModuleRecordByModuleNameCalled_ = false;
    
    void resetStartAbilityFlags()
    {
        startAbility_nullAbilityInfo_called_ = true;
        startAbility_nullAppRecord_called_ = true;
        startAbility_singletonAbilityExists_called_ = false;
        startAbility_abilityExistsWithPreToken_called_ = false;
        startAbility_addModuleFailed_called_ = false;
        startAbility_addAbilityFailed_called_ = false;
        startAbility_appStateCreate_called_ = false;
        startAbility_launchAbility_called_ = false;
        startAbility_processAppDebug_called_ = false;
        
        // Reset control variables
        simulateAddModuleFails_ = false;
        simulateAddAbilityFails_ = false;
        simulateSingletonAbilityExists_ = false;
        simulateAbilityExistsWithPreToken_ = false;
        addModuleCalled_ = false;
        getModuleRecordByModuleNameCalled_ = false;
    }
    std::shared_ptr<AppExecFwk::AppRunningRecord> runningRecord_ = nullptr;
    void resetModuleRunningFlags()
    {
        getAbilityRunningRecordByTokenCalled_ = false;
        runningRecord_ = nullptr;
    }
    std::shared_ptr<AppExecFwk::AppRunningRecord> masterProcessRunningRecord_ = nullptr;
    std::shared_ptr<AppExecFwk::AppRunningRecord> mainProcessRunningRecord_ = nullptr;
    std::shared_ptr<AppExecFwk::AppRunningRecord> appRecordForSpecifiedProcess_ = nullptr;

    bool exitByPidstatus_ = false;
    int32_t signRestartProcessStatus_ = ERR_OK;
private:
    MyStatus() = default;
    bool isLogoutUserCalled_ = false;
    bool isFindMasterProcessAppRunningRecordCalled_ = false;
    bool isFindMainProcessAppRunningRecordCalled_ = false;
    bool isCheckAppRunningRecordForSpecifiedProcessCalled_ = false;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_MY_STATUS_H