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
    ErrCode processUpdate_ = ERR_OK;
    bool verifyCallingPermission_ = false;
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
    int32_t notifyLoadRepairPatch_ = 0;
    int32_t notifyHotReloadPage_ = 0;
    int32_t notifyUnLoadRepairPatch_ = 0;
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
    // remote client
    std::shared_ptr<AppExecFwk::AppSpawnClient> getSpawnClient_;
    std::shared_ptr<AppExecFwk::AppSpawnClient> getNWebSpawnClient_;
    int getSpawnClientCall_ = 0;
    // app spawn client
    int32_t startProcess_ = ERR_OK;
    bool getApplicationInfoWithAppIndexRet_ = false;
private:
    MyStatus() = default;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_MY_STATUS_H