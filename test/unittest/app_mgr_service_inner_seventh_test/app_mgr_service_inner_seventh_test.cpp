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

#include <gtest/gtest.h>
#define private public
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "app_spawn_client.h"
#include "app_utils.h"
#include "render_record.h"
#undef private
#include "user_record_manager.h"
#include "mock_my_status.h"
#include "ability_manager_errors.h"
#include "overlay_manager_proxy.h"
#include "ability_connect_callback_stub.h"
#include "session_manager_lite.h"
#include "process_uid_define.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::Rosen;
using OHOS::AppExecFwk::ExtensionAbilityType;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t SHADER_CACHE_GROUPID = 3099;
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
constexpr int32_t BASE_USER_RANGE = 200000;
namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInnerSeventhTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class MyRenderScheduler : public IRenderScheduler {
public:
    void NotifyBrowserFd(int32_t ipcFd, int32_t sharedFd,
                                    int32_t crashFd, sptr<IRemoteObject> browser) override {}
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class MyStartSpecifiedAbilityResponse : public IStartSpecifiedAbilityResponse {
public:
    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId,
        int32_t userId) override
    {}
    void OnTimeoutResponse(int32_t requestId, int32_t userId) override
    {}
    void OnNewProcessRequestResponse(const std::string &flag, int32_t userId, int32_t requestId,
        const std::string &callerProcessName, int32_t recordId) override
    {}
    void OnNewProcessRequestTimeoutResponse(int32_t requestId, int32_t userId) override
    {}
    void OnStartSpecifiedFailed(int32_t requestId, int32_t userId) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};
class MyRemoteObject : public IRemoteStub<IAbilityConnection> {
public:
    static sptr<MyRemoteObject> GetInstance()
    {
        static sptr<MyRemoteObject> instance = new MyRemoteObject();
        return instance;
    }

    void OnAbilityConnectDone(const AppExecFwk::ElementName& element,
        const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}

private:
    MyRemoteObject() = default;
    ~MyRemoteObject() override = default;
    MyRemoteObject(const MyRemoteObject&) = delete;
    MyRemoteObject& operator=(const MyRemoteObject&) = delete;
};
    
void AppMgrServiceInnerSeventhTest::SetUpTestCase() {}

void AppMgrServiceInnerSeventhTest::TearDownTestCase() {}

void AppMgrServiceInnerSeventhTest::SetUp() {}

void AppMgrServiceInnerSeventhTest::TearDown() {}

/**
 * @tc.name: GetBundleAndHapInfo_001
 * @tc.desc: test GetBundleAndHapInfo_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBundleAndHapInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    AbilityInfo abilityInfo;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = 0;
    bool ret = appMgrServiceInner->GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_001 end");
}

/**
 * @tc.name: GetBundleAndHapInfo_002
 * @tc.desc: test GetBundleAndHapInfo_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBundleAndHapInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getSandboxBundleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = ERR_OK;
    AbilityInfo abilityInfo;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = 1001;
    bool ret = appMgrServiceInner->GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_002 end");
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_001
 * @tc.desc: test UpdateApplicationInfoInstalled_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateApplicationInfoInstalled_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    AAFwk::MyStatus::GetInstance().getCallingUid_ = 0;
    auto bundleMgrHelper = appMgrServiceInner->remoteClientManager_->GetBundleManagerHelper();

    std::string bundleName = "";
    int uid = 0;
    std::string moduleName = "";
    bool isPlugin = true;
    int32_t ret = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, isPlugin);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_001 end");
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_002
 * @tc.desc: test UpdateApplicationInfoInstalled_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateApplicationInfoInstalled_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = 0;
    AAFwk::MyStatus::GetInstance().processUpdate_ = ERR_INVALID_STATE;

    std::string bundleName = "";
    int uid = 0;
    std::string moduleName = "";
    bool isPlugin = true;
    int32_t ret = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, isPlugin);
    EXPECT_EQ(ret, ERR_INVALID_STATE);
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_002 end");
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_003
 * @tc.desc: test UpdateApplicationInfoInstalled_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateApplicationInfoInstalled_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = 0;
    AAFwk::MyStatus::GetInstance().processUpdate_ = ERR_OK;

    std::string bundleName = "";
    int uid = 0;
    std::string moduleName = "";
    bool isPlugin = true;
    int32_t ret = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, isPlugin);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_003 end");
}

/**
 * @tc.name: KillApplication_001
 * @tc.desc: test KillApplication_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, KillApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    std::string bundleName = "";
    bool clearPageStack = false;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->KillApplication(bundleName, clearPageStack, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "KillApplication_001 end");
}

/**
 * @tc.name: ForceKillApplication_001
 * @tc.desc: test ForceKillApplication_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, ForceKillApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ForceKillApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getNameForUid_ = "com.ohos.sceneboard";
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string bundleName = "";
    int userId = 0;
    int appIndex = 0;
    int32_t ret = appMgrServiceInner->ForceKillApplication(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "ForceKillApplication_001 end");
}

/**
 * @tc.name: KillApplicationWithUserId_001
 * @tc.desc: test KillApplicationWithUserId_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, KillApplicationWithUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationWithUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;

    std::string bundleName = "";
    int userId = 0;
    int appIndex = 0;
    int32_t ret = appMgrServiceInner->KillApplicationWithUserId(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    ret = appMgrServiceInner->KillApplicationWithUserId(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    userId = -1;
    ret = appMgrServiceInner->KillApplicationWithUserId(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->KillApplicationWithUserId(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationWithUserId_001 end");
}

/**
 * @tc.name: KillProcessesByAccessTokenId_001
 * @tc.desc: test ForceKillApplicationInner_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, KillProcessesByAccessTokenId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByAccessTokenId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;

    int32_t accessTokenId = 0;
    int32_t ret = appMgrServiceInner->KillProcessesByAccessTokenId(accessTokenId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByAccessTokenId_001 end");
}

/**
 * @tc.name: KillProcessesByAccessTokenId_002
 * @tc.desc: test ForceKillApplicationInner_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, KillProcessesByAccessTokenId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByAccessTokenId_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    appMgrServiceInner->appRunningManager_ = nullptr;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;

    int32_t accessTokenId = 0;
    int32_t ret = appMgrServiceInner->KillProcessesByAccessTokenId(accessTokenId);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByAccessTokenId_002 end");
}

/**
 * @tc.name: UpdateProcessMemoryState_001
 * @tc.desc: test UpdateProcessMemoryState_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateProcessMemoryState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateProcessMemoryState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, temp);

    ProcessMemoryState state;
    std::vector<ProcessMemoryState> procMemState = {state};
    int32_t ret = appMgrServiceInner->UpdateProcessMemoryState(procMemState);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UpdateProcessMemoryState_001 end");
}

/**
 * @tc.name: UpdateProcessMemoryState_002
 * @tc.desc: test UpdateProcessMemoryState_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateProcessMemoryState_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateProcessMemoryState_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    ProcessMemoryState state;
    std::vector<ProcessMemoryState> procMemState = {state};
    int32_t ret = appMgrServiceInner->UpdateProcessMemoryState(procMemState);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UpdateProcessMemoryState_002 end");
}

/**
 * @tc.name: KillApplicationByUserIdLocked_001
 * @tc.desc: test KillApplicationByUserIdLocked_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, KillApplicationByUserIdLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUserIdLocked_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    std::string bundleName = "";
    int32_t appCloneIndex = 0;
    int userId = 0;
    KillProcessConfig config;
    int32_t ret = appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, appCloneIndex, userId, config);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUserIdLocked_001 end");
}

/**
 * @tc.name: ClearUpApplicationDataBySelf_001
 * @tc.desc: test ClearUpApplicationDataBySelf_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, ClearUpApplicationDataBySelf_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataBySelf_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, temp);

    int32_t callerUid = 0;
    pid_t callerPid = 0;
    int32_t userId = -1;
    int32_t ret = appMgrServiceInner->ClearUpApplicationDataBySelf(callerUid, callerPid, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataBySelf_001 end");
}

/**
* @tc.name: ClearUpApplicationDataByUserId_001
* @tc.desc: test ClearUpApplicationDataByUserId_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, ClearUpApplicationDataByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    const std::string bundleName = "";
    int32_t callerUid = 1;
    pid_t callerPid = 1;
    int32_t appCloneIndex = 1;
    int32_t userId = 0;
    bool isBySelf = false;
    std::string reason = "";
    int32_t ret = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
        userId, isBySelf, reason);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_001 end");
}

/**
* @tc.name: ClearUpApplicationDataByUserId_002
* @tc.desc: test ClearUpApplicationDataByUserId_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, ClearUpApplicationDataByUserId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().clearUserGranted_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().cleanBundleDataFiles_ = false;

    const std::string bundleName = "";
    int32_t callerUid = 1;
    pid_t callerPid = 1;
    int32_t appCloneIndex = 1;
    int32_t userId = 0;
    bool isBySelf = false;
    std::string reason = "";
    int32_t ret = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
        userId, isBySelf, reason);
    EXPECT_EQ(ret, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_002 end");
}

/**
* @tc.name: ClearUpApplicationDataByUserId_003
* @tc.desc: test ClearUpApplicationDataByUserId_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, ClearUpApplicationDataByUserId_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().clearUserGranted_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().cleanBundleDataFiles_ = true;
    appMgrServiceInner->appRunningManager_ = nullptr;

    const std::string bundleName = "";
    int32_t callerUid = 1;
    pid_t callerPid = 1;
    int32_t appCloneIndex = 1;
    int32_t userId = 0;
    bool isBySelf = true;
    std::string reason = "";
    int32_t ret = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
        userId, isBySelf, reason);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_003 end");
}

/**
* @tc.name: GetRunningProcessesByBundleType_001
* @tc.desc: test GetRunningProcessesByBundleType_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRunningProcessesByBundleType_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    std::string temp = "";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    appRecord->SetUid(0);
    auto appRecord2 = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    appRecord2->SetUid(200000);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(0, appRecord));
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));

    BundleType bundleType = BundleType::APP;
    std::vector<RunningProcessInfo> info;
    int32_t ret = appMgrServiceInner->GetRunningProcessesByBundleType(bundleType, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_001 end");
}

/**
* @tc.name: GetRunningProcessesByBundleType_002
* @tc.desc: test GetRunningProcessesByBundleType_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRunningProcessesByBundleType_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    std::string temp = "";
    auto appRecord2 = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    appRecord2->SetUid(200000);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));

    BundleType bundleType = BundleType::APP;
    std::vector<RunningProcessInfo> info;
    int32_t ret = appMgrServiceInner->GetRunningProcessesByBundleType(bundleType, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_002 end");
}

/**
* @tc.name: GetRunningProcessesByBundleType_003
* @tc.desc: test GetRunningProcessesByBundleType_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRunningProcessesByBundleType_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleType = BundleType::APP;
    auto appRecord2 = std::make_shared<AppRunningRecord>(info, 0, temp);
    appRecord2->SetUid(200000);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));

    BundleType bundleType = BundleType::APP;
    std::vector<RunningProcessInfo> info2;
    int32_t ret = appMgrServiceInner->GetRunningProcessesByBundleType(bundleType, info2);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_003 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleName_001
* @tc.desc: test GetAllRunningInstanceKeysByBundleName_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;

    std::string bundleName = "";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_001 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleNameInner_001
* @tc.desc: test GetAllRunningInstanceKeysByBundleNameInner_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleNameInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    std::string bundleName = "111";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_001 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleNameInner_002
* @tc.desc: test GetAllRunningInstanceKeysByBundleNameInner_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleNameInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().getApplicationInfo_ = true;
    AAFwk::MyStatus::GetInstance().applicationInfo_ = {};
    AAFwk::MyStatus::GetInstance().applicationInfo_.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::MULTI_INSTANCE;

    std::string bundleName = "111";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_002 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleNameInner_003
* @tc.desc: test GetAllRunningInstanceKeysByBundleNameInner_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleNameInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getApplicationInfo_ = true;
    AAFwk::MyStatus::GetInstance().applicationInfo_ = {};
    AAFwk::MyStatus::GetInstance().applicationInfo_.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::UNSPECIFIED;
    std::string bundleName = "111";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_MULTI_INSTANCE_NOT_SUPPORTED);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_003 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleNameInner_004
* @tc.desc: test GetAllRunningInstanceKeysByBundleNameInner_004
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleNameInner_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getApplicationInfo_ = true;
    AAFwk::MyStatus::GetInstance().applicationInfo_ = {};
    AAFwk::MyStatus::GetInstance().applicationInfo_.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, nullptr));

    std::string bundleName = "111";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_004 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleNameInner_005
* @tc.desc: test GetAllRunningInstanceKeysByBundleNameInner_005
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleNameInner_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getApplicationInfo_ = true;
    AAFwk::MyStatus::GetInstance().applicationInfo_ = {};
    AAFwk::MyStatus::GetInstance().applicationInfo_.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string temp = "";
    auto appRecord2 = std::make_shared<AppRunningRecord>(info, 0, temp);
    appRecord2->SetUid(200000);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));

    std::string bundleName = "111";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_005 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleNameInner_006
* @tc.desc: test GetAllRunningInstanceKeysByBundleNameInner_006
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleNameInner_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getApplicationInfo_ = true;
    AAFwk::MyStatus::GetInstance().applicationInfo_ = {};
    AAFwk::MyStatus::GetInstance().applicationInfo_.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string temp = "";
    auto appRecord2 = std::make_shared<AppRunningRecord>(info, 0, temp);
    appRecord2->SetUid(200000);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));

    std::string bundleName = "111";
    std::vector<std::string> instanceKeys;
    int32_t userId = 100;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_006 end");
}

/**
* @tc.name: GetProcessRunningInfosByUserId_001
* @tc.desc: test GetProcessRunningInfosByUserId_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetProcessRunningInfosByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInfosByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string temp = "";
    auto appRecord2 = std::make_shared<AppRunningRecord>(info, 0, temp);
    appRecord2->SetUid(200000);
    appRecord2->SetSpawned();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));

    std::vector<RunningProcessInfo> info1;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetProcessRunningInfosByUserId(info1, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInfosByUserId_001 end");
}

/**
* @tc.name: GetProcessRunningInfosByUserId_002
* @tc.desc: test GetProcessRunningInfosByUserId_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetProcessRunningInfosByUserId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInfosByUserId_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, nullptr));

    std::vector<RunningProcessInfo> info;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetProcessRunningInfosByUserId(info, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInfosByUserId_002 end");
}

/**
* @tc.name: GetProcessRunningInfosByUserId_003
* @tc.desc: test GetProcessRunningInfosByUserId_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetProcessRunningInfosByUserId_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInfosByUserId_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    std::string temp = "";
    auto appRecord2 = std::make_shared<AppRunningRecord>(info, 0, temp);
    appRecord2->SetUid(0);
    appRecord2->SetSpawned();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));

    std::vector<RunningProcessInfo> info1;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetProcessRunningInfosByUserId(info1, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInfosByUserId_003 end");
}

/**
* @tc.name: GetProcessRunningInformation_001
* @tc.desc: test GetProcessRunningInformation_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetProcessRunningInformation_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInformation_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    RunningProcessInfo info;
    int32_t ret = appMgrServiceInner->GetProcessRunningInformation(info);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInformation_001 end");
}

/**
* @tc.name: GetProcessRunningInformation_002
* @tc.desc: test GetProcessRunningInformation_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetProcessRunningInformation_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInformation_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    RunningProcessInfo info;
    int32_t ret = appMgrServiceInner->GetProcessRunningInformation(info);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInformation_002 end");
}

/**
* @tc.name: GetProcessRunningInformation_003
* @tc.desc: test GetProcessRunningInformation_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetProcessRunningInformation_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInformation_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, temp);

    RunningProcessInfo info;
    int32_t ret = appMgrServiceInner->GetProcessRunningInformation(info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetProcessRunningInformation_003 end");
}

/**
* @tc.name: NotifyProcMemoryLevel_001
* @tc.desc: test NotifyProcMemoryLevel_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, NotifyProcMemoryLevel_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyProcMemoryLevel_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().checkSpecific_ = true;
    AAFwk::MyStatus::GetInstance().notifyProcMemory_ = ERR_OK;

    const std::map<pid_t, MemoryLevel> procLevelMap;
    int32_t ret = appMgrServiceInner->NotifyProcMemoryLevel(procLevelMap);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyProcMemoryLevel_001 end");
}

/**
* @tc.name: DumpHeapMemory_001
* @tc.desc: test DumpHeapMemory_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, DumpHeapMemory_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapMemory_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    appMgrServiceInner->appRunningManager_ = nullptr;

    int32_t pid = 0;
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    int32_t ret = appMgrServiceInner->DumpHeapMemory(pid, mallocInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapMemory_001 end");
}

/**
* @tc.name: DumpHeapMemory_002
* @tc.desc: test DumpHeapMemory_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, DumpHeapMemory_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapMemory_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    AAFwk::MyStatus::GetInstance().dumpHeapMemory_ = ERR_OK;

    int32_t pid = 0;
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    int32_t ret = appMgrServiceInner->DumpHeapMemory(pid, mallocInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapMemory_002 end");
}

/**
* @tc.name: DumpJsHeapMemory_001
* @tc.desc: test DumpJsHeapMemory_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, DumpJsHeapMemory_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpJsHeapMemory_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    appMgrServiceInner->appRunningManager_ = nullptr;

    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = 1;
    int32_t ret = appMgrServiceInner->DumpJsHeapMemory(info);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "DumpJsHeapMemory_001 end");
}

/**
* @tc.name: DumpJsHeapMemory_002
* @tc.desc: test DumpJsHeapMemory_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, DumpJsHeapMemory_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpJsHeapMemory_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    AAFwk::MyStatus::GetInstance().dumpJsHeapMemory_ = ERR_OK;

    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = 1;
    int32_t ret = appMgrServiceInner->DumpJsHeapMemory(info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DumpJsHeapMemory_002 end");
}

/**
* @tc.name: DumpJsHandleMap_001
* @tc.desc: test DumpJsHandleMap_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, DumpJsHandleMap_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpJsHandleMap_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    appMgrServiceInner->appRunningManager_ = nullptr;

    OHOS::AppExecFwk::JsHandleMapInfo info;
    info.pid = 0;
    int32_t ret = appMgrServiceInner->DumpJsHandleMap(info);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    info.pid = 1;
    ret = appMgrServiceInner->DumpJsHandleMap(info);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "DumpJsHandleMap_001 end");
}

/**
* @tc.name: GetRunningProcesses_001
* @tc.desc: test GetRunningProcesses_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRunningProcesses_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcesses_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info1, 0, temp);
    std::shared_ptr<UserTestRecord> record = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(record);
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().getAppIndex_ = 0;

    std::vector<RunningProcessInfo> info;
    appMgrServiceInner->GetRunningProcesses(appRecord, info);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppIndex_, 0);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcesses_001 end");
}

/**
* @tc.name: GetRunningProcesses_002
* @tc.desc: test GetRunningProcesses_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRunningProcesses_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcesses_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    info1->multiAppMode.multiAppModeType = MultiAppModeType::MULTI_INSTANCE;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info1, 0, temp);
    std::shared_ptr<UserTestRecord> record = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(record);
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().getAppIndex_ = 0;

    std::vector<RunningProcessInfo> info;
    appMgrServiceInner->GetRunningProcesses(appRecord, info);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppIndex_, 0);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcesses_002 end");
}

/**
* @tc.name: GetRunningProcesses_002
* @tc.desc: test GetRunningProcesses_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRunningProcesses_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcesses_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    info1->multiAppMode.multiAppModeType = MultiAppModeType::APP_CLONE;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info1, 0, temp);
    std::shared_ptr<UserTestRecord> record = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(record);
    AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().getAppIndex_ = 0;

    std::vector<RunningProcessInfo> info;
    appMgrServiceInner->GetRunningProcesses(appRecord, info);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppIndex_, 1);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcesses_003 end");
}

/**
* @tc.name: GetRenderProcesses_001
* @tc.desc: test GetRenderProcesses_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRenderProcesses_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRenderProcesses_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();


    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    info1->multiAppMode.multiAppModeType = MultiAppModeType::APP_CLONE;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info1, 0, temp);
    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);

    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd), appRecord);
    renderRecord->SetPid(1);
    appRecord->renderRecordMap_.insert(std::pair<int32_t, std::shared_ptr<RenderRecord>>(1, renderRecord));
    std::vector<RenderProcessInfo> info;
    appMgrServiceInner->GetRenderProcesses(appRecord, info);
    EXPECT_EQ(info.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "GetRenderProcesses_001 end");
}

/**
* @tc.name: StartPerfProcessByStartMsg_001
* @tc.desc: test StartPerfProcessByStartMsg_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartPerfProcessByStartMsg_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartPerfProcessByStartMsg_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    AppSpawnStartMsg startMsg;
    std::string perfCmd = "";
    std::string debugCmd = "111";
    bool isSandboxApp = true;
    auto ret = appMgrServiceInner->StartPerfProcessByStartMsg(startMsg, perfCmd, debugCmd, isSandboxApp);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "StartPerfProcessByStartMsg_001 end");
}

/**
* @tc.name: StartPerfProcessByStartMsg_002
* @tc.desc: test StartPerfProcessByStartMsg_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartPerfProcessByStartMsg_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartPerfProcessByStartMsg_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = std::make_shared<AppSpawnClient>();
    AAFwk::MyStatus::GetInstance().startProcess_ = ERR_INVALID_VALUE;
    
    AppSpawnStartMsg startMsg;
    std::string perfCmd = "111";
    std::string debugCmd = "111";
    bool isSandboxApp = true;
    auto ret = appMgrServiceInner->StartPerfProcessByStartMsg(startMsg, perfCmd, debugCmd, isSandboxApp);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartPerfProcessByStartMsg_002 end");
}

/**
* @tc.name: SetOverlayInfo_001
* @tc.desc: test SetOverlayInfo_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SetOverlayInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    AAFwk::MyStatus::GetInstance().getOverlayCall_ = 0;

    std::string bundleName = "";
    int32_t userId = 0;
    AppSpawnStartMsg startMs;
    appMgrServiceInner->SetOverlayInfo(bundleName, userId, startMs);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getOverlayCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_001 end");
}

/**
* @tc.name: SetOverlayInfo_002
* @tc.desc: test SetOverlayInfo_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SetOverlayInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getOverlayCall_ = 0;

    std::string bundleName = "";
    int32_t userId = 0;
    AppSpawnStartMsg startMs;
    appMgrServiceInner->SetOverlayInfo(bundleName, userId, startMs);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getOverlayCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_002 end");
}

/**
* @tc.name: SetOverlayInfo_003
* @tc.desc: test SetOverlayInfo_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SetOverlayInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getOverlayCall_ = 0;
    AAFwk::MyStatus::GetInstance().getOverlay_ = new (std::nothrow) AppExecFwk::OverlayManagerProxy(nullptr);

    std::string bundleName = "";
    int32_t userId = 0;
    AppSpawnStartMsg startMs;
    appMgrServiceInner->SetOverlayInfo(bundleName, userId, startMs);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getOverlayCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_003 end");
}

/**
* @tc.name: CreatNewStartMsg_001
* @tc.desc: test CreatNewStartMsg_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, CreatNewStartMsg_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreatNewStartMsg_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    
    AAFwk::Want want;
    AbilityInfo abilityInfo;
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::string processName = "";
    AppSpawnStartMsg startMsg;
    auto ret = appMgrServiceInner->CreatNewStartMsg(want, abilityInfo, appInfo, processName, startMsg);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "CreatNewStartMsg_001 end");
}

/**
* @tc.name: CreateStartMsg_001
* @tc.desc: test CreateStartMsg_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, CreateStartMsg_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    
    CreateStartMsgParam param;
    AppSpawnStartMsg startMsg;
    auto ret = appMgrServiceInner->CreateStartMsg(param, startMsg);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_001 end");
}

/**
* @tc.name: CreateStartMsg_002
* @tc.desc: test CreateStartMsg_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, CreateStartMsg_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBaseSharedBundleInfos_ = ERR_NO_INIT;

    CreateStartMsgParam param;
    AppSpawnStartMsg startMsg;
    auto ret = appMgrServiceInner->CreateStartMsg(param, startMsg);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_002 end");
}

/**
* @tc.name: CreateStartMsg_003
* @tc.desc: test CreateStartMsg_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, CreateStartMsg_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBaseSharedBundleInfos_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().queryDataGroupInfos_ = false;

    CreateStartMsgParam param;
    AppSpawnStartMsg startMsg;
    auto ret = appMgrServiceInner->CreateStartMsg(param, startMsg);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_003 end");
}

/**
* @tc.name: CreateStartMsg_004
* @tc.desc: test CreateStartMsg_004
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, CreateStartMsg_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBaseSharedBundleInfos_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().queryDataGroupInfos_ = true;
    DataGroupInfo data = {};
    AAFwk::MyStatus::GetInstance().queryData_ = {data};

    CreateStartMsgParam param;
    AppSpawnStartMsg startMsg;
    auto ret = appMgrServiceInner->CreateStartMsg(param, startMsg);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CreateStartMsg_004 end");
}

/**
* @tc.name: CheckGetRunningInfoPermission_001
* @tc.desc: test CheckGetRunningInfoPermission_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, CheckGetRunningInfoPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckGetRunningInfoPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = false;
    
    auto ret = appMgrServiceInner->CheckGetRunningInfoPermission();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckGetRunningInfoPermission_001 end");
}

/**
* @tc.name: CheckRemoteClient_001
* @tc.desc: test CheckRemoteClient_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, CheckRemoteClient_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckRemoteClient_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = std::make_shared<AppSpawnClient>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    auto ret = appMgrServiceInner->CheckRemoteClient();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckRemoteClient_001 end");
}

/**
* @tc.name: RestartKeepAliveProcess_001
* @tc.desc: test RestartKeepAliveProcess_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, RestartKeepAliveProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RestartKeepAliveProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    AAFwk::MyStatus::GetInstance().getSpawnClientCall_ = 0;

    appMgrServiceInner->RestartKeepAliveProcess(appRecord);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getSpawnClientCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "RestartKeepAliveProcess_001 end");
}

/**
* @tc.name: RestartKeepAliveProcess_002
* @tc.desc: test RestartKeepAliveProcess_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, RestartKeepAliveProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RestartKeepAliveProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    AAFwk::MyStatus::GetInstance().getSpawnClientCall_ = 0;

    appMgrServiceInner->RestartKeepAliveProcess(appRecord);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getSpawnClientCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "RestartKeepAliveProcess_002 end");
}

/**
* @tc.name: RestartKeepAliveProcess_003
* @tc.desc: test RestartKeepAliveProcess_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, RestartKeepAliveProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RestartKeepAliveProcess_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    AAFwk::MyStatus::GetInstance().getSpawnClientCall_ = 0;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getCloneBundleInfo_ = ERR_OK;

    appMgrServiceInner->RestartKeepAliveProcess(appRecord);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getSpawnClientCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "RestartKeepAliveProcess_003 end");
}

/**
* @tc.name: GetForegroundApplications_001
* @tc.desc: test GetForegroundApplications_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetForegroundApplications_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetForegroundApplications_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = false;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;

    std::vector<AppStateData> list;
    auto ret = appMgrServiceInner->GetForegroundApplications(list);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "GetForegroundApplications_001 end");
}

/**
* @tc.name: StartUserTestProcess_001
* @tc.desc: test StartUserTestProcess_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartUserTestProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUserTestProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ == std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().processExit_ = false;
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    appMgrServiceInner->remoteClientManager_ = nullptr;

    AAFwk::Want want;
    std::string bundle_name = "test_bundle_name";
    want.SetParam("-b", bundle_name);

    sptr<IRemoteObject> observer = MyRemoteObject::GetInstance();
    BundleInfo info;
    std::string processName = "test_processName";
    auto ret = appMgrServiceInner->StartUserTestProcess(want, observer, info, 0);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartUserTestProcess_001 end");
}

/**
* @tc.name: StartUserTestProcess_002
* @tc.desc: test StartUserTestProcess_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartUserTestProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUserTestProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ == nullptr;

    AAFwk::Want want;
    std::string bundle_name = "test_bundle_name";
    want.SetParam("-b", bundle_name);
    sptr<IRemoteObject> observer = MyRemoteObject::GetInstance();
    BundleInfo info;
    std::string processName = "test_processName";
    auto ret = appMgrServiceInner->StartUserTestProcess(want, observer, info, 0);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartUserTestProcess_002 end");
}

/**
* @tc.name: GetHapModuleInfoForTestRunner_001
* @tc.desc: test GetHapModuleInfoForTestRunner_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetHapModuleInfoForTestRunner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ == nullptr;

    AAFwk::Want want;
    std::string bundle_name = "test_bundle_name";
    want.SetParam("-b", bundle_name);
    sptr<IRemoteObject> observer = MyRemoteObject::GetInstance();
    BundleInfo info;
    HapModuleInfo hapModuleInfo;
    auto ret = appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, info, hapModuleInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_001 end");
}

/**
* @tc.name: GetHapModuleInfoForTestRunner_002
* @tc.desc: test GetHapModuleInfoForTestRunner_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetHapModuleInfoForTestRunner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ == nullptr;

    AAFwk::Want want;
    std::string bundle_name = "";
    want.SetParam("-m", bundle_name);
    sptr<IRemoteObject> observer = MyRemoteObject::GetInstance();
    BundleInfo info;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isModuleJson = true;
    hapModuleInfo.moduleName = "";
    info.hapModuleInfos.push_back(hapModuleInfo);
    auto ret = appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, info, hapModuleInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_002 end");
}

/**
* @tc.name: GetHapModuleInfoForTestRunner_003
* @tc.desc: test GetHapModuleInfoForTestRunner_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetHapModuleInfoForTestRunner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ == nullptr;

    AAFwk::Want want;
    std::string bundle_name = "111";
    want.SetParam("-m", bundle_name);
    sptr<IRemoteObject> observer = MyRemoteObject::GetInstance();
    BundleInfo info;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isModuleJson = true;
    hapModuleInfo.moduleName = "11";
    info.hapModuleInfos.push_back(hapModuleInfo);
    auto ret = appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, info, hapModuleInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_003 end");
}

/**
* @tc.name: GetHapModuleInfoForTestRunner_004
* @tc.desc: test GetHapModuleInfoForTestRunner_004
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetHapModuleInfoForTestRunner_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ == nullptr;

    AAFwk::Want want;
    std::string bundle_name = "111";
    want.SetParam("-m", bundle_name);
    sptr<IRemoteObject> observer = MyRemoteObject::GetInstance();
    BundleInfo info;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isModuleJson = true;
    hapModuleInfo.moduleName = "111";
    info.hapModuleInfos.push_back(hapModuleInfo);
    auto ret = appMgrServiceInner->GetHapModuleInfoForTestRunner(want, observer, info, hapModuleInfo);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetHapModuleInfoForTestRunner_004 end");
}

/**
* @tc.name: StartSpecifiedAbility_001
* @tc.desc: test StartSpecifiedAbility_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartSpecifiedAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getCloneBundleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = true;
    AAFwk::MyStatus::GetInstance().checkAppRunningCall_ = 0;

    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 0;
    appMgrServiceInner->StartSpecifiedAbility(want, abilityInfo, requestId);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().checkAppRunningCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_001 end");
}

/**
* @tc.name: StartSpecifiedAbility_002
* @tc.desc: test StartSpecifiedAbility_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartSpecifiedAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getCloneBundleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().scheduleAcceptCall_ = 0;
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getModuleRecord_ = std::make_shared<ModuleRunningRecord>(nullptr, nullptr);

    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 0;
    appMgrServiceInner->StartSpecifiedAbility(want, abilityInfo, requestId);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().scheduleAcceptCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_002 end");
}

/**
* @tc.name: StartSpecifiedAbility_003
* @tc.desc: test StartSpecifiedAbility_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartSpecifiedAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().addModulesCall_ = 0;
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getModuleRecord_ = nullptr;

    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 0;
    appMgrServiceInner->StartSpecifiedAbility(want, abilityInfo, requestId);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().addModulesCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_003 end");
}

/**
* @tc.name: StartSpecifiedAbility_004
* @tc.desc: test StartSpecifiedAbility_004
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartSpecifiedAbility_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().addModulesCall_ = 0;
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().checkAppRunning_->SetDebugApp(true);
    AAFwk::MyStatus::GetInstance().getModuleRecord_ = nullptr;

    AAFwk::Want want;
    want.SetParam("debugApp", true);
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 0;
    appMgrServiceInner->StartSpecifiedAbility(want, abilityInfo, requestId);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().addModulesCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_004 end");
}

/**
* @tc.name: StartSpecifiedAbility_005
* @tc.desc: test StartSpecifiedAbility_005
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartSpecifiedAbility_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().isAppExistCall_ = 0;
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunningByUid_ = false;
    AAFwk::MyStatus::GetInstance().createAppRunning_ = nullptr;

    AAFwk::Want want;
    want.SetParam("debugApp", true);
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 0;
    appMgrServiceInner->StartSpecifiedAbility(want, abilityInfo, requestId);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().isAppExistCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_005 end");
}

/**
* @tc.name: StartSpecifiedAbility_006
* @tc.desc: test StartSpecifiedAbility_006
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartSpecifiedAbility_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().addModulesCall_ = 0;
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunningByUid_ = false;
    AAFwk::MyStatus::GetInstance().createAppRunning_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");

    AAFwk::Want want;
    want.SetParam("debugApp", true);
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 0;
    appMgrServiceInner->StartSpecifiedAbility(want, abilityInfo, requestId);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().addModulesCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_006 end");
}

/**
* @tc.name: StartSpecifiedAbility_007
* @tc.desc: test StartSpecifiedAbility_007
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartSpecifiedAbility_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().addModulesCall_ = 0;
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    AAFwk::MyStatus::GetInstance().checkAppRunningByUid_ = false;
    AAFwk::MyStatus::GetInstance().createAppRunning_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().createAppRunning_->SetDebugApp(true);

    AAFwk::Want want;
    want.SetParam("debugApp", true);
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 0;
    appMgrServiceInner->StartSpecifiedAbility(want, abilityInfo, requestId);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().addModulesCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "StartSpecifiedAbility_007 end");
}

/**
* @tc.name: RegisterStartSpecifiedAbilityResponse_001
* @tc.desc: test RegisterStartSpecifiedAbilityResponse_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, RegisterStartSpecifiedAbilityResponse_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterStartSpecifiedAbilityResponse_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    sptr<IRemoteObject> observer = MyRemoteObject::GetInstance();
    
    sptr<IStartSpecifiedAbilityResponse> response = new MyStartSpecifiedAbilityResponse();
    appMgrServiceInner->RegisterStartSpecifiedAbilityResponse(response);
    EXPECT_NE(appMgrServiceInner->startSpecifiedAbilityResponse_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "RegisterStartSpecifiedAbilityResponse_001 end");
}

/**
* @tc.name: SchedulePrepareTerminate_001
* @tc.desc: test SchedulePrepareTerminate_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SchedulePrepareTerminate_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SchedulePrepareTerminate_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().schedulePrepareCall_ = 0;
    
    pid_t pid = 0;
    std::string moduleName = "";
    appMgrServiceInner->SchedulePrepareTerminate(pid, moduleName);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().schedulePrepareCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SchedulePrepareTerminate_001 end");
}

/**
* @tc.name: SchedulePrepareTerminate_002
* @tc.desc: test SchedulePrepareTerminate_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SchedulePrepareTerminate_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SchedulePrepareTerminate_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;;
    AAFwk::MyStatus::GetInstance().schedulePrepareCall_ = 0;

    pid_t pid = 0;
    std::string moduleName = "";
    appMgrServiceInner->SchedulePrepareTerminate(pid, moduleName);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().schedulePrepareCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SchedulePrepareTerminate_002 end");
}

/**
* @tc.name: SchedulePrepareTerminate_003
* @tc.desc: test SchedulePrepareTerminate_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SchedulePrepareTerminate_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SchedulePrepareTerminate_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;;
    AAFwk::MyStatus::GetInstance().schedulePrepareCall_ = 0;

    pid_t pid = 0;
    std::string moduleName = "";
    appMgrServiceInner->SchedulePrepareTerminate(pid, moduleName);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().schedulePrepareCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SchedulePrepareTerminate_003 end");
}

/**
* @tc.name: ScheduleNewProcessRequestDone_001
* @tc.desc: test ScheduleNewProcessRequestDone_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, ScheduleNewProcessRequestDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ScheduleNewProcessRequestDone_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getNewProcessRequestIdCall_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();

    int32_t recordId = 0;
    AAFwk::Want want;
    std::string flag = "";
    appMgrServiceInner->ScheduleNewProcessRequestDone(recordId, want, flag);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getNewProcessRequestIdCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "ScheduleNewProcessRequestDone_001 end");
}

/**
* @tc.name: ScheduleNewProcessRequestDone_002
* @tc.desc: test ScheduleNewProcessRequestDone_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, ScheduleNewProcessRequestDone_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ScheduleNewProcessRequestDone_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getNewProcessRequestIdCall_ = 0;
    appMgrServiceInner->startSpecifiedAbilityResponse_ = nullptr;
    std::string temp = "";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(0, appRecord));

    int32_t recordId = 0;
    AAFwk::Want want;
    std::string flag = "";
    appMgrServiceInner->ScheduleNewProcessRequestDone(recordId, want, flag);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getNewProcessRequestIdCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "ScheduleNewProcessRequestDone_002 end");
}

/**
* @tc.name: ScheduleNewProcessRequestDone_003
* @tc.desc: test ScheduleNewProcessRequestDone_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, ScheduleNewProcessRequestDone_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ScheduleNewProcessRequestDone_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->startSpecifiedAbilityResponse_ = new MyStartSpecifiedAbilityResponse();
    AAFwk::MyStatus::GetInstance().getNewProcessRequestIdCall_ = 0;
    std::string temp = "";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(0, appRecord));

    int32_t recordId = 0;
    AAFwk::Want want;
    std::string flag = "";
    appMgrServiceInner->ScheduleNewProcessRequestDone(recordId, want, flag);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getNewProcessRequestIdCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "ScheduleNewProcessRequestDone_003 end");
}

/**
* @tc.name: HandleStartSpecifiedProcessTimeout_001
* @tc.desc: test HandleStartSpecifiedProcessTimeout_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, HandleStartSpecifiedProcessTimeout_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleStartSpecifiedProcessTimeout_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetNewProcessRequestCall_ = 0;

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    appMgrServiceInner->HandleStartSpecifiedProcessTimeout(appRecord);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().resetNewProcessRequestCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "HandleStartSpecifiedProcessTimeout_001 end");
}

/**
* @tc.name: HandleStartSpecifiedProcessTimeout_002
* @tc.desc: test HandleStartSpecifiedProcessTimeout_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, HandleStartSpecifiedProcessTimeout_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleStartSpecifiedProcessTimeout_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().resetNewProcessRequestCall_ = 0;

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appMgrServiceInner->HandleStartSpecifiedProcessTimeout(appRecord);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().resetNewProcessRequestCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "HandleStartSpecifiedProcessTimeout_002 end");
}

/**
* @tc.name: HandleStartSpecifiedProcessTimeout_003
* @tc.desc: test HandleStartSpecifiedProcessTimeout_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, HandleStartSpecifiedProcessTimeout_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleStartSpecifiedProcessTimeout_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->startSpecifiedAbilityResponse_ = new MyStartSpecifiedAbilityResponse();
    AAFwk::MyStatus::GetInstance().resetNewProcessRequestCall_ = 0;

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appMgrServiceInner->HandleStartSpecifiedProcessTimeout(appRecord);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().resetNewProcessRequestCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "HandleStartSpecifiedProcessTimeout_003 end");
}

/**
* @tc.name: DealWithUserConfiguration_001
* @tc.desc: test DealWithUserConfiguration_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, DealWithUserConfiguration_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DealWithUserConfiguration_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->multiUserConfigurationMgr_ = nullptr;

    Configuration config;
    int32_t userId = 0;
    int32_t notifyUserId = 0;
    auto ret = appMgrServiceInner->DealWithUserConfiguration(config, userId, notifyUserId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "DealWithUserConfiguration_001 end");
}

/**
* @tc.name: UpdateConfigurationByBundleName_001
* @tc.desc: test UpdateConfigurationByBundleName_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateConfigurationByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByBundleName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    Configuration config;
    std::string name = "";
    int32_t appIndex = 0;
    auto ret = appMgrServiceInner->UpdateConfigurationByBundleName(config, name, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByBundleName_001 end");
}

/**
* @tc.name: UpdateConfigurationByBundleName_002
* @tc.desc: test UpdateConfigurationByBundleName_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateConfigurationByBundleName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByBundleName_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyUpdateAPPConfigurationPerm_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().updateConfigurationByBundleName_ = ERR_INVALID_VALUE;
    Configuration config;
    std::string name = "";
    int32_t appIndex = 0;
    auto ret = appMgrServiceInner->UpdateConfigurationByBundleName(config, name, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByBundleName_002 end");
}

/**
* @tc.name: UpdateConfigurationByBundleName_002
* @tc.desc: test UpdateConfigurationByBundleName_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateConfigurationByBundleName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByBundleName_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyUpdateAPPConfigurationPerm_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().updateConfigurationByBundleName_ = ERR_OK;
    Configuration config;
    std::string name = "";
    int32_t appIndex = 0;
    auto ret = appMgrServiceInner->UpdateConfigurationByBundleName(config, name, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfigurationByBundleName_003 end");
}

/**
* @tc.name: RegisterConfigurationObserver_001
* @tc.desc: test RegisterConfigurationObserver_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, RegisterConfigurationObserver_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterConfigurationObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = false;

    auto ret = appMgrServiceInner->RegisterConfigurationObserver(nullptr, -1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "RegisterConfigurationObserver_001 end");
}

/**
* @tc.name: UnregisterConfigurationObserver_001
* @tc.desc: test UnregisterConfigurationObserver_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, UnregisterConfigurationObserver_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterConfigurationObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = false;

    auto ret = appMgrServiceInner->UnregisterConfigurationObserver(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterConfigurationObserver_001 end");
}

/**
* @tc.name: GetConfiguration_001
* @tc.desc: test GetConfiguration_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetConfiguration_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetConfiguration_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->multiUserConfigurationMgr_ = nullptr;

    auto ret = appMgrServiceInner->GetConfiguration();
    EXPECT_EQ(ret, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetConfiguration_001 end");
}

/**
* @tc.name: GetApplicationInfoByProcessID_001
* @tc.desc: test GetApplicationInfoByProcessID_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetApplicationInfoByProcessID_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetApplicationInfoByProcessID_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;

    int pid = 0;
    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto ret = appMgrServiceInner->GetApplicationInfoByProcessID(pid, application, debug);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "GetApplicationInfoByProcessID_001 end");
}

/**
* @tc.name: NotifyAppMgrRecordExitReason_001
* @tc.desc: test NotifyAppMgrRecordExitReason_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, NotifyAppMgrRecordExitReason_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppMgrRecordExitReason_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");

    int32_t pid = 0;
    int32_t reason = 0;
    std::string exitMsg = "";
    auto ret = appMgrServiceInner->NotifyAppMgrRecordExitReason(pid, reason, exitMsg);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppMgrRecordExitReason_001 end");
}

/**
* @tc.name: NotifyAppMgrRecordExitReason_002
* @tc.desc: test NotifyAppMgrRecordExitReason_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, NotifyAppMgrRecordExitReason_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppMgrRecordExitReason_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    int32_t pid = 0;
    int32_t reason = 0;
    std::string exitMsg = "";
    auto ret = appMgrServiceInner->NotifyAppMgrRecordExitReason(pid, reason, exitMsg);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppMgrRecordExitReason_002 end");
}

/**
* @tc.name: VerifyKillProcessPermission_001
* @tc.desc: test VerifyKillProcessPermission_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, VerifyKillProcessPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    sptr<IRemoteObject> observer = MyRemoteObject::GetInstance();
    auto ret = appMgrServiceInner->VerifyKillProcessPermission(observer);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermission_001 end");
}

/**
* @tc.name: VerifyKillProcessPermissionCommon_001
* @tc.desc: test VerifyKillProcessPermissionCommon_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, VerifyKillProcessPermissionCommon_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermissionCommon_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    info1->appPrivilegeLevel = "system_basic";
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, temp);

    auto ret = appMgrServiceInner->VerifyKillProcessPermissionCommon();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermissionCommon_001 end");
}

/**
* @tc.name: VerifyKillProcessPermissionCommon_002
* @tc.desc: test VerifyKillProcessPermissionCommon_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, VerifyKillProcessPermissionCommon_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermissionCommon_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    info1->appPrivilegeLevel = "system_basic";
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, temp);

    auto ret = appMgrServiceInner->VerifyKillProcessPermissionCommon();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "VerifyKillProcessPermissionCommon_002 end");
}

/**
* @tc.name: VerifyAPL_001
* @tc.desc: test VerifyAPL_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, VerifyAPL_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyAPL_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, temp);

    auto ret = appMgrServiceInner->VerifyAPL();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "VerifyAPL_001 end");
}

/**
* @tc.name: VerifyAPL_002
* @tc.desc: test VerifyAPL_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, VerifyAPL_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "VerifyAPL_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    std::string temp = "";
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, temp);

    auto ret = appMgrServiceInner->VerifyAPL();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "VerifyAPL_002 end");
}

/**
* @tc.name: PreStartNWebSpawnProcess_001
* @tc.desc: test PreStartNWebSpawnProcess_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, PreStartNWebSpawnProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreStartNWebSpawnProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    std::string temp = "";
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = std::make_shared<AppSpawnClient>();
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = true;

    pid_t hostPid = 1;
    auto ret = appMgrServiceInner->PreStartNWebSpawnProcess(hostPid);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "PreStartNWebSpawnProcess_001 end");
}

/**
* @tc.name: PreStartNWebSpawnProcess_002
* @tc.desc: test PreStartNWebSpawnProcess_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, PreStartNWebSpawnProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreStartNWebSpawnProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    std::string temp = "";
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = std::make_shared<AppSpawnClient>();
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_->state_ = SpawnConnectionState::STATE_CONNECTED;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;

    pid_t hostPid = 1;
    auto ret = appMgrServiceInner->PreStartNWebSpawnProcess(hostPid);
    EXPECT_EQ(ret, 0);
    TAG_LOGI(AAFwkTag::TEST, "PreStartNWebSpawnProcess_002 end");
}

/**
* @tc.name: StartRenderProcess_001
* @tc.desc: test StartRenderProcess_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = true;

    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    pid_t renderPid = 1;
    bool isGPU = false;
    auto ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd),
        std::move(sharedFd), std::move(crashFd), renderPid, isGPU);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_001 end");
}

/**
* @tc.name: StartRenderProcess_002
* @tc.desc: test StartRenderProcess_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    pid_t renderPid = 1;
    bool isGPU = true;
    auto ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd),
        std::move(sharedFd), std::move(crashFd), renderPid, isGPU);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_002 end");
}

/**
* @tc.name: StartRenderProcess_003
* @tc.desc: test StartRenderProcess_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = nullptr;

    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    pid_t renderPid = 1;
    bool isGPU = true;
    auto ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd),
        std::move(sharedFd), std::move(crashFd), renderPid, isGPU);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_003 end");
}

/**
* @tc.name: StartRenderProcess_004
* @tc.desc: test StartRenderProcess_004
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcess_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->renderRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::RenderRecord>>(0, nullptr));
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = nullptr;

    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    pid_t renderPid = 1;
    bool isGPU = false;
    auto ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd),
        std::move(sharedFd), std::move(crashFd), renderPid, isGPU);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_004 end");
}

/**
* @tc.name: StartRenderProcess_005
* @tc.desc: test StartRenderProcess_005
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcess_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;

    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    pid_t renderPid = 1;
    bool isGPU = false;
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd),
        AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->renderRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::RenderRecord>>(0, renderRecord));
    auto ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd),
        std::move(sharedFd), std::move(crashFd), renderPid, isGPU);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_005 end");
}

/**
* @tc.name: StartRenderProcess_006
* @tc.desc: test StartRenderProcess_006
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcess_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;

    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    pid_t renderPid = 1;
    bool isGPU = false;
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd),
        AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->renderRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::RenderRecord>>(0, renderRecord));
    for (int i = 1; i < 42; i++) {
        AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->renderRecordMap_.insert(std::pair<const int32_t,
            const std::shared_ptr<AppExecFwk::RenderRecord>>(i, nullptr));
    }
    auto ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd),
        std::move(sharedFd), std::move(crashFd), renderPid, isGPU);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_006 end");
}

/**
* @tc.name: StartRenderProcess_007
* @tc.desc: test StartRenderProcess_007
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcess_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;

    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    pid_t renderPid = 1;
    bool isGPU = false;
    sptr<IRenderScheduler> renderScheduler = new MyRenderScheduler();
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd),
        AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_);
    renderRecord->renderScheduler_ = renderScheduler;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->renderRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::RenderRecord>>(0, renderRecord));
    auto ret = appMgrServiceInner->StartRenderProcess(hostPid, renderParam, std::move(ipcFd),
        std::move(sharedFd), std::move(crashFd), renderPid, isGPU);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcess_007 end");
}

/**
* @tc.name: AttachRenderProcess_001
* @tc.desc: test AttachRenderProcess_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, AttachRenderProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    AAFwk::MyStatus::GetInstance().getBrowserHostCall_ = 0;

    sptr<IRenderScheduler> renderScheduler = new MyRenderScheduler();
    const pid_t pid = 1;
    appMgrServiceInner->AttachRenderProcess(pid, renderScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getBrowserHostCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_001 end");
}

/**
* @tc.name: AttachRenderProcess_002
* @tc.desc: test AttachRenderProcess_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, AttachRenderProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBrowserHostCall_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_ = nullptr;

    sptr<IRenderScheduler> renderScheduler = new MyRenderScheduler();
    const pid_t pid = 1;
    appMgrServiceInner->AttachRenderProcess(pid, renderScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getBrowserHostCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_002 end");
}

/**
* @tc.name: AttachRenderProcess_003
* @tc.desc: test AttachRenderProcess_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, AttachRenderProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBrowserHostCall_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_ =
        std::make_shared<AppRunningRecord>(nullptr, 0, "");

    sptr<IRenderScheduler> renderScheduler = new MyRenderScheduler();
    const pid_t pid = 1;
    appMgrServiceInner->AttachRenderProcess(pid, renderScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getBrowserHostCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_003 end");
}

/**
* @tc.name: AttachRenderProcess_004
* @tc.desc: test AttachRenderProcess_004
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, AttachRenderProcess_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBrowserHostCall_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_ =
        std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getRenderRecordByPid_ = nullptr;

    sptr<IRenderScheduler> renderScheduler = new MyRenderScheduler();
    const pid_t pid = 1;
    appMgrServiceInner->AttachRenderProcess(pid, renderScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getBrowserHostCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_004 end");
}

/**
* @tc.name: AttachRenderProcess_005
* @tc.desc: test AttachRenderProcess_005
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, AttachRenderProcess_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBrowserHostCall_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_ =
        std::make_shared<AppRunningRecord>(nullptr, 0, "");
    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd),
        AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_->SetBrowserHost(MyRemoteObject::GetInstance());
    renderRecord->processType_ = ProcessType::GPU;
    AAFwk::MyStatus::GetInstance().getRenderRecordByPid_ = renderRecord;

    sptr<IRenderScheduler> renderScheduler = new MyRenderScheduler();
    const pid_t pid = 1;
    appMgrServiceInner->AttachRenderProcess(pid, renderScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getBrowserHostCall_, 2);
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_005 end");
}

/**
* @tc.name: AttachRenderProcess_006
* @tc.desc: test AttachRenderProcess_006
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, AttachRenderProcess_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBrowserHostCall_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_ =
        std::make_shared<AppRunningRecord>(nullptr, 0, "");
    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd),
        AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByRenderPid_->SetBrowserHost(nullptr);
    renderRecord->processType_ = ProcessType::GPU;
    AAFwk::MyStatus::GetInstance().getRenderRecordByPid_ = renderRecord;

    sptr<IRenderScheduler> renderScheduler = new MyRenderScheduler();
    const pid_t pid = 1;
    appMgrServiceInner->AttachRenderProcess(pid, renderScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getBrowserHostCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "AttachRenderProcess_006 end");
}

/**
* @tc.name: SaveBrowserChannel_001
* @tc.desc: test SaveBrowserChannel_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SaveBrowserChannel_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SaveBrowserChannel_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    AAFwk::MyStatus::GetInstance().setBrowserHostCall_ = 0;

    const pid_t pid = 1;
    appMgrServiceInner->SaveBrowserChannel(pid, MyRemoteObject::GetInstance());
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().setBrowserHostCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "SaveBrowserChannel_001 end");
}

/**
* @tc.name: SaveBrowserChannel_002
* @tc.desc: test SaveBrowserChannel_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SaveBrowserChannel_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SaveBrowserChannel_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().setBrowserHostCall_ = 0;

    const pid_t pid = 1;
    appMgrServiceInner->SaveBrowserChannel(pid, MyRemoteObject::GetInstance());
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().setBrowserHostCall_, 1);
    TAG_LOGI(AAFwkTag::TEST, "SaveBrowserChannel_002 end");
}

/**
* @tc.name: StartRenderProcessImpl_001
* @tc.desc: test StartRenderProcessImpl_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcessImpl_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = nullptr;

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd), appRecord);
    pid_t pid = 1;
    auto ret = appMgrServiceInner->StartRenderProcessImpl(renderRecord, appRecord, pid, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_001 end");
}

/**
* @tc.name: StartRenderProcessImpl_002
* @tc.desc: test StartRenderProcessImpl_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcessImpl_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = std::make_shared<AppSpawnClient>();
    appMgrServiceInner->renderProcessIsolationUidSet_.clear();
    for (int32_t i = START_ID_FOR_RENDER_PROCESS_ISOLATION; i <= END_ID_FOR_RENDER_PROCESS_ISOLATION; i++) {
        appMgrServiceInner->renderProcessIsolationUidSet_.insert(i);
    }

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd), appRecord);
    pid_t pid = 1;
    auto ret = appMgrServiceInner->StartRenderProcessImpl(renderRecord, appRecord, pid, false);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_002 end");
}

/**
* @tc.name: StartRenderProcessImpl_003
* @tc.desc: test StartRenderProcessImpl_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcessImpl_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = std::make_shared<AppSpawnClient>();
    appMgrServiceInner->renderProcessIsolationUidSet_.clear();
    AAFwk::MyStatus::GetInstance().startProcess_ = ERR_OK;

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd), appRecord);
    pid_t pid = 1;
    auto ret = appMgrServiceInner->StartRenderProcessImpl(renderRecord, appRecord, pid, false);
    EXPECT_EQ(ret, 0);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_003 end");
}

/**
* @tc.name: StartRenderProcessImpl_004
* @tc.desc: test StartRenderProcessImpl_004
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, StartRenderProcessImpl_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = std::make_shared<AppSpawnClient>();
    appMgrServiceInner->renderProcessIsolationUidSet_.clear();
    AAFwk::MyStatus::GetInstance().startProcess_ = ERR_OK;

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd), appRecord);
    pid_t pid = 1;
    auto ret = appMgrServiceInner->StartRenderProcessImpl(renderRecord, appRecord, pid, true);
    EXPECT_EQ(ret, 0);
    TAG_LOGI(AAFwkTag::TEST, "StartRenderProcessImpl_004 end");
}

/**
* @tc.name: SetRenderStartMsg_001
* @tc.desc: test SetRenderStartMsg_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SetRenderStartMsg_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetRenderStartMsg_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = std::make_shared<AppSpawnClient>();
    appMgrServiceInner->renderProcessIsolationUidSet_.clear();
    AAFwk::MyStatus::GetInstance().startProcess_ = ERR_OK;

    AppSpawnStartMsg startMsg;
    startMsg.gids.push_back(0);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd), appRecord);
    int32_t renderUid = 1;
    bool isGPU = true;
    appMgrServiceInner->SetRenderStartMsg(startMsg, renderRecord, renderUid, isGPU);
    EXPECT_EQ(startMsg.gids.size(), 2);
    TAG_LOGI(AAFwkTag::TEST, "SetRenderStartMsg_001 end");
}

/**
* @tc.name: SetRenderStartMsg_002
* @tc.desc: test SetRenderStartMsg_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, SetRenderStartMsg_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetRenderStartMsg_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getNWebSpawnClient_ = std::make_shared<AppSpawnClient>();
    appMgrServiceInner->renderProcessIsolationUidSet_.clear();
    AAFwk::MyStatus::GetInstance().startProcess_ = ERR_OK;

    AppSpawnStartMsg startMsg;
    startMsg.gids.push_back(SHADER_CACHE_GROUPID);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    pid_t hostPid = 1;
    std::string renderParam = "111";
    FdGuard ipcFd(-1);
    FdGuard sharedFd(-1);
    FdGuard crashFd(-1);
    std::shared_ptr<RenderRecord> renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd), appRecord);
    int32_t renderUid = 1;
    bool isGPU = true;
    appMgrServiceInner->SetRenderStartMsg(startMsg, renderRecord, renderUid, isGPU);
    EXPECT_EQ(startMsg.gids.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "SetRenderStartMsg_002 end");
}

/**
* @tc.name: AllowNativeChildProcess_001
* @tc.desc: test AllowNativeChildProcess
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, AllowNativeChildProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AllowNativeChildProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    auto ret = appMgrServiceInner->AllowNativeChildProcess(CHILD_PROCESS_TYPE_JS, "");
    EXPECT_FALSE(ret);
    ret = appMgrServiceInner->AllowNativeChildProcess(CHILD_PROCESS_TYPE_NATIVE, "");
    EXPECT_FALSE(ret);
    ret = appMgrServiceInner->AllowNativeChildProcess(CHILD_PROCESS_TYPE_NATIVE_ARGS, "");
    EXPECT_FALSE(ret);

    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.allowStartNativeProcessApps_.isLoaded = true;
    std::vector<std::string> appIds;
    appIds.push_back("testAppId");
    appUtils.allowStartNativeProcessApps_.value = appIds;
    ret = appMgrServiceInner->AllowNativeChildProcess(CHILD_PROCESS_TYPE_NATIVE_ARGS, "testAppId");
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AllowNativeChildProcess_001 end");
}

/**
* @tc.name: AllowChildProcessInMultiProcessFeatureApp_001
* @tc.desc: test AllowChildProcessInMultiProcessFeatureApp
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, AllowChildProcessInMultiProcessFeatureApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AllowChildProcessInMultiProcessFeatureApp_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, 1, "com.example.child");
    EXPECT_NE(appRecord, nullptr);

    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.allowChildProcessInMultiProcessFeatureApp_.isLoaded = true;
    appUtils.allowChildProcessInMultiProcessFeatureApp_.value = false;
    auto ret = appMgrServiceInner->AllowChildProcessInMultiProcessFeatureApp(appRecord);
    EXPECT_FALSE(ret);
    ret = appMgrServiceInner->AllowChildProcessInMultiProcessFeatureApp(nullptr);
    EXPECT_FALSE(ret);

    appUtils.allowChildProcessInMultiProcessFeatureApp_.isLoaded = true;
    appUtils.allowChildProcessInMultiProcessFeatureApp_.value = true;
    appRecord->SetSupportMultiProcessDeviceFeature(false);
    ret = appMgrServiceInner->AllowChildProcessInMultiProcessFeatureApp(appRecord);
    EXPECT_FALSE(ret);
    appRecord->SetSupportMultiProcessDeviceFeature(true);
    ret = appMgrServiceInner->AllowChildProcessInMultiProcessFeatureApp(appRecord);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AllowChildProcessInMultiProcessFeatureApp_001 end");
}

/**
* @tc.name: AllowChildProcessInMultiProcessFeatureApp_002
* @tc.desc: test AllowChildProcessInMultiProcessFeatureApp
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, AllowChildProcessInMultiProcessFeatureApp_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AllowChildProcessInMultiProcessFeatureApp_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, 1, "com.example.child");
    EXPECT_NE(appRecord, nullptr);
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.allowChildProcessInMultiProcessFeatureApp_.isLoaded = true;
    appUtils.allowChildProcessInMultiProcessFeatureApp_.value = true;

    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = 1;
    auto ret = appMgrServiceInner->AllowChildProcessInMultiProcessFeatureApp(appRecord);
    EXPECT_FALSE(ret);

    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    ret = appMgrServiceInner->AllowChildProcessInMultiProcessFeatureApp(appRecord);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AllowChildProcessInMultiProcessFeatureApp_002 end");
}

/**
* @tc.name: GetBackgroundAppInfo_001
* @tc.desc: test GetBackgroundAppInfo_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto res = appMgrServiceInner->GetBackgroundAppInfo({});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_001 end");
}

/**
* @tc.name: GetBackgroundAppInfo_002
* @tc.desc: test GetBackgroundAppInfo_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = nullptr;
    auto res = appMgrServiceInner->GetBackgroundAppInfo({});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_002 end");
}

/**
* @tc.name: GetBackgroundAppInfo_003
* @tc.desc: test GetBackgroundAppInfo_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();
    auto res = appMgrServiceInner->GetBackgroundAppInfo({});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_003 end");
}

/**
* @tc.name: GetBackgroundAppInfo_004
* @tc.desc: test GetBackgroundAppInfo_004
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();

    RecentSessionInfo info1;
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_->recentSessionInfoList_ = {info1};
    auto res = appMgrServiceInner->GetBackgroundAppInfo({});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_004 end");
}

/**
* @tc.name: GetBackgroundAppInfo_005
* @tc.desc: test GetBackgroundAppInfo_005
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();

    RecentSessionInfo info1;
    RecentSessionInfo info2;
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_->recentSessionInfoList_ = {info1, info2};
    auto res = appMgrServiceInner->GetBackgroundAppInfo({});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_005 end");
}

/**
* @tc.name: GetBackgroundAppInfo_006
* @tc.desc: test GetBackgroundAppInfo_006
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();

    RecentSessionInfo info1;
    info1.bundleName = "app1";
    info1.appIndex = 0;

    RecentSessionInfo info2;
    info2.bundleName = "app2";
    info2.appIndex = 0;
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_->recentSessionInfoList_ = {info1, info2};
    auto res = appMgrServiceInner->GetBackgroundAppInfo({});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_006 end");
}

/**
* @tc.name: GetBackgroundAppInfo_007
* @tc.desc: test GetBackgroundAppInfo_007
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_007 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();

    RecentSessionInfo info1;
    info1.bundleName = "app1";
    info1.appIndex = 0;

    RecentSessionInfo info2;
    info2.bundleName = "app2";
    info2.appIndex = 0;
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_->recentSessionInfoList_ = {info1, info2};

    BackgroundAppInfo appInfo1;
    auto res = appMgrServiceInner->GetBackgroundAppInfo({appInfo1});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_007 end");
}

/**
* @tc.name: GetBackgroundAppInfo_008
* @tc.desc: test GetBackgroundAppInfo_008
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_008 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();

    RecentSessionInfo info1;
    info1.bundleName = "app1";
    info1.sessionState = RecentSessionState::BACKGROUND;

    RecentSessionInfo info2;
    info2.bundleName = "app2";
    info2.sessionState = RecentSessionState::BACKGROUND;

    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_->recentSessionInfoList_ = {info1, info2};

    BackgroundAppInfo appInfo1;
    appInfo1.bandleName = "app1";

    BackgroundAppInfo appInfo2;
    appInfo2.bandleName = "app2";

    auto res = appMgrServiceInner->GetBackgroundAppInfo({appInfo1, appInfo2});
    EXPECT_EQ(res.size(), 2);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_008 end");
}

/**
* @tc.name: GetBackgroundAppInfo_009
* @tc.desc: test GetBackgroundAppInfo_009
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_009 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();

    RecentSessionInfo info1;
    info1.bundleName = "app1";
    info1.sessionState = RecentSessionState::FOREGROUND;

    RecentSessionInfo info2;
    info2.bundleName = "app2";
    info2.sessionState = RecentSessionState::BACKGROUND;

    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_->recentSessionInfoList_ = {info1, info2};

    BackgroundAppInfo appInfo1;
    appInfo1.bandleName = "app1";

    BackgroundAppInfo appInfo2;
    appInfo2.bandleName = "app2";

    auto res = appMgrServiceInner->GetBackgroundAppInfo({appInfo1, appInfo2});
    EXPECT_EQ(res.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_009 end");
}

/**
* @tc.name: GetBackgroundAppInfo_010
* @tc.desc: test GetBackgroundAppInfo_010
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_010 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();

    RecentSessionInfo info1;
    info1.bundleName = "app1";
    info1.sessionState = RecentSessionState::FOREGROUND;

    RecentSessionInfo info2;
    info2.bundleName = "app2";
    info2.sessionState = RecentSessionState::FOREGROUND;

    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_->recentSessionInfoList_ = {info1, info2};

    BackgroundAppInfo appInfo1;
    appInfo1.bandleName = "app1";

    BackgroundAppInfo appInfo2;
    appInfo2.bandleName = "app2";

    auto res = appMgrServiceInner->GetBackgroundAppInfo({appInfo1, appInfo2});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_010 end");
}


/**
* @tc.name: GetBackgroundAppInfo_011
* @tc.desc: test GetBackgroundAppInfo_011
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_011, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_011 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();

    RecentSessionInfo info1;
    info1.bundleName = "app1";
    info1.sessionState = RecentSessionState::FOREGROUND;

    RecentSessionInfo info2;
    info2.bundleName = "app2";
    info2.sessionState = RecentSessionState::FOREGROUND;

    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_->recentSessionInfoList_ = {info1, info2};

    BackgroundAppInfo appInfo1;
    appInfo1.bandleName = "app3";

    BackgroundAppInfo appInfo2;
    appInfo2.bandleName = "app4";

    auto res = appMgrServiceInner->GetBackgroundAppInfo({appInfo1, appInfo2});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_011 end");
}

/**
* @tc.name: GetBackgroundAppInfo_012
* @tc.desc: test GetBackgroundAppInfo_012
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBackgroundAppInfo_012, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_012 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = std::make_shared<SceneSessionManagerLite>();

    RecentSessionInfo info1;
    info1.bundleName = "app1";
    info1.appIndex = 1;

    RecentSessionInfo info2;
    info2.bundleName = "app2";
    info2.appIndex = 1;
    SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_->recentSessionInfoList_ = {info1, info2};
    auto res = appMgrServiceInner->GetBackgroundAppInfo({});
    EXPECT_EQ(res.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetBackgroundAppInfo_012 end");
}

/**
 * @tc.name: CheckAppProvisionType_0100
 * @tc.desc: test CheckAppProvisionType
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, CheckAppProvisionType_0100, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    std::string bundleName = "testBundleName";
    int32_t callerUid = 0;
    int32_t appCloneIndex = 0;
    int32_t userId = DEFAULT_INVAL_VALUE;
    auto ret = appMgrServiceInner->CheckAppProvisionType(bundleName, callerUid, appCloneIndex, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    callerUid = BASE_USER_RANGE;
    ret = appMgrServiceInner->CheckAppProvisionType(bundleName, callerUid, appCloneIndex, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    userId = 100;
    ret = appMgrServiceInner->CheckAppProvisionType(bundleName, callerUid, appCloneIndex, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    callerUid = 100;
    ret = appMgrServiceInner->CheckAppProvisionType(bundleName, callerUid, appCloneIndex, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: CheckAppProvisionType_0200
 * @tc.desc: test CheckAppProvisionType
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, CheckAppProvisionType_0200, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();

    const std::string bundleName = "";
    int32_t callerUid = BASE_USER_RANGE;
    int32_t appCloneIndex = 1;
    int32_t userId = DEFAULT_INVAL_VALUE;
    auto ret = appMgrServiceInner->CheckAppProvisionType(bundleName, callerUid, appCloneIndex, userId);
    EXPECT_EQ(ret, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
}

/**
 * @tc.name: CheckAppProvisionType_0300
 * @tc.desc: test CheckAppProvisionType
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, CheckAppProvisionType_0300, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getApplicationInfoWithAppIndexRet_ = true;
    AAFwk::MyStatus::GetInstance().applicationInfo_ = {};

    const std::string bundleName = "";
    int32_t callerUid = BASE_USER_RANGE;
    int32_t appCloneIndex = 1;
    int32_t userId = DEFAULT_INVAL_VALUE;
    auto ret = appMgrServiceInner->CheckAppProvisionType(bundleName, callerUid, appCloneIndex, userId);
    EXPECT_EQ(ret, AAFwk::CHECK_PERMISSION_FAILED);

    AAFwk::MyStatus::GetInstance().applicationInfo_.appProvisionType = "debug";
    ret = appMgrServiceInner->CheckAppProvisionType(bundleName, callerUid, appCloneIndex, userId);
    EXPECT_EQ(ret, ERR_OK);
}
} // namespace AppExecFwk
} // namespace OHOS
