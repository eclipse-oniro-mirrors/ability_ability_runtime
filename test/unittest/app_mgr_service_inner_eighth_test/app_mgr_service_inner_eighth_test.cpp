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
#include "child_process_record.h"
#include "cache_process_manager.h"
#undef private
#include "user_record_manager.h"
#include "mock_my_status.h"
#include "ability_manager_errors.h"
#include "overlay_manager_proxy.h"
#include "ability_connect_callback_stub.h"
#include "app_scheduler_const.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using OHOS::AppExecFwk::ExtensionAbilityType;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t QUICKFIX_UID = 5524;
constexpr int32_t SHADER_CACHE_GROUPID = 3099;
constexpr int32_t RESOURCE_MANAGER_UID = 1096;
constexpr int32_t DEFAULT_USER_ID = 0;
static int g_scheduleLoadChildCall = 0;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* DEBUG_APP = "debugApp";
}
class AppMgrServiceInnerEighthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
class MyKiaInterceptor : public IKiaInterceptor {
public:
    int OnIntercept(AAFwk::Want &want) override
    {
        return 0;
    }

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};
class MyRenderStateObserver : public IRenderStateObserver {
public:
    void OnRenderStateChanged(const RenderStateData &renderStateData) override {}
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};
class MyChildScheduler : public IChildScheduler {
public:
    bool ScheduleLoadChild() override
    {
        g_scheduleLoadChildCall++;
        return false;
    }
    bool ScheduleExitProcessSafely() override
    {
        return false;
    }
    bool ScheduleRunNativeProc(const sptr<IRemoteObject> &mainProcessCb) override
    {
        return false;
    }
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
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

class MyAbilityDebugResponse : public IAbilityDebugResponse {
public:
    void OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens) override
    {}

    void OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens) override
    {}

    void OnAbilitysAssertDebugChange(const std::vector<sptr<IRemoteObject>> &tokens,
        bool isAssertDebug) override {}
    
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class MyStartSpecifiedAbilityResponse : public IStartSpecifiedAbilityResponse {
public:
    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId) override
    {}
    void OnTimeoutResponse(int32_t requestId) override
    {}
    void OnNewProcessRequestResponse(const std::string &flag, int32_t requestId,
        const std::string &callerProcessName) override
    {}
    void OnNewProcessRequestTimeoutResponse(int32_t requestId) override
    {}
    void OnStartSpecifiedFailed(int32_t requestId) override
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
    
void AppMgrServiceInnerEighthTest::SetUpTestCase() {}

void AppMgrServiceInnerEighthTest::TearDownTestCase() {}

void AppMgrServiceInnerEighthTest::SetUp() {}

void AppMgrServiceInnerEighthTest::TearDown() {}

/**
 * @tc.name: NotifyLoadRepairPatch_001
 * @tc.desc: test NotifyLoadRepairPatch_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyLoadRepairPatch_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyLoadRepairPatch_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().notifyLoadRepairPatch_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getCallingUid_ = QUICKFIX_UID;

    auto ret = appMgrServiceInner->NotifyLoadRepairPatch("", nullptr);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyLoadRepairPatch_001 end");
}

/**
 * @tc.name: NotifyHotReloadPage_001
 * @tc.desc: test NotifyHotReloadPage_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyHotReloadPage_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyHotReloadPage_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().notifyHotReloadPage_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getCallingUid_ = QUICKFIX_UID;

    auto ret = appMgrServiceInner->NotifyHotReloadPage("", nullptr);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyHotReloadPage_001 end");
}

/**
 * @tc.name: NotifyUnLoadRepairPatch_001
 * @tc.desc: test NotifyUnLoadRepairPatch_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyUnLoadRepairPatch_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyUnLoadRepairPatch_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().notifyUnLoadRepairPatch_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getCallingUid_ = QUICKFIX_UID;

    auto ret = appMgrServiceInner->NotifyUnLoadRepairPatch("", nullptr);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyUnLoadRepairPatch_001 end");
}

/**
 * @tc.name: NotifyAppFaultBySA_001
 * @tc.desc: test NotifyAppFaultBySA_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyAppFaultBySA_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = nullptr;

    AppFaultDataBySA faultDataSA;
    auto ret = appMgrServiceInner->NotifyAppFaultBySA(faultDataSA);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_001 end");
}

/**
 * @tc.name: NotifyAppFaultBySA_002
 * @tc.desc: test NotifyAppFaultBySA_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyAppFaultBySA_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = false;

    AppFaultDataBySA faultDataSA;
    auto ret = appMgrServiceInner->NotifyAppFaultBySA(faultDataSA);
    EXPECT_EQ(ret, AAFwk::CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "NotifyAppFaultBySA_002 end");
}

/**
 * @tc.name: IsSharedBundleRunning_001
 * @tc.desc: test IsSharedBundleRunning_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsSharedBundleRunning_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSharedBundleRunning_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    BaseSharedBundleInfo baseSharedBundleInfo;
    baseSharedBundleInfo.bundleName = "111";
    baseSharedBundleInfo.versionCode = 1;
    appMgrServiceInner->runningSharedBundleList_.insert(std::pair<std::string,
        std::vector<BaseSharedBundleInfo>>("1", {baseSharedBundleInfo}));

    auto ret = appMgrServiceInner->IsSharedBundleRunning("111", 1);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "IsSharedBundleRunning_001 end");
}

/**
 * @tc.name: IsApplicationRunning_001
 * @tc.desc: test IsApplicationRunning_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsApplicationRunning_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsApplicationRunning_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = false;

    bool isRunning = true;
    auto ret = appMgrServiceInner->IsApplicationRunning("111", isRunning);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "IsApplicationRunning_001 end");
}

/**
 * @tc.name: IsAppRunning_001
 * @tc.desc: test IsAppRunning_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsAppRunning_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = false;
    std::string bundleName = "";
    int32_t appCloneIndex = 0;
    bool isRunning = false;

    auto ret = appMgrServiceInner->IsAppRunning(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_001 end");
}


/**
 * @tc.name: IsAppRunning_002
 * @tc.desc: test IsAppRunning_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsAppRunning_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    std::string bundleName = "";
    int32_t appCloneIndex = -1;
    bool isRunning = false;

    auto ret = appMgrServiceInner->IsAppRunning(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(ret, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_002 end");
}

/**
 * @tc.name: IsAppRunning_003
 * @tc.desc: test IsAppRunning_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsAppRunning_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = nullptr;
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    std::string bundleName = "";
    int32_t appCloneIndex = 1;
    bool isRunning = false;

    auto ret = appMgrServiceInner->IsAppRunning(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_003 end");
}

/**
 * @tc.name: IsAppRunning_004
 * @tc.desc: test IsAppRunning_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsAppRunning_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    std::string bundleName = "";
    int32_t appCloneIndex = 1;
    bool isRunning = false;

    auto ret = appMgrServiceInner->IsAppRunning(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_004 end");
}

/**
 * @tc.name: IsAppRunning_005
 * @tc.desc: test IsAppRunning_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsAppRunning_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    AAFwk::MyStatus::GetInstance().getCloneBundleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().checkAppClone_ = ERR_OK;
    std::string bundleName = "";
    int32_t appCloneIndex = 1;
    bool isRunning = false;

    auto ret = appMgrServiceInner->IsAppRunning(bundleName, appCloneIndex, isRunning);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunning_005 end");
}

/**
 * @tc.name: IsAppRunningByBundleNameAndUserId_001
 * @tc.desc: test IsAppRunningByBundleNameAndUserId_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsAppRunningByBundleNameAndUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunningByBundleNameAndUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().isAppRunningByBundleName_ = ERR_OK;

    std::string bundleName = "";
    int32_t userId = -1;
    bool isRunning = false;
    auto ret = appMgrServiceInner->IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "IsAppRunningByBundleNameAndUserId_001 end");
}

/**
 * @tc.name: CreateAbilityInfo_001
 * @tc.desc: test CreateAbilityInfo_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CreateAbilityInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateAbilityInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    
    AAFwk::Want want;
    AbilityInfo abilityInfo;
    auto ret = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CreateAbilityInfo_001 end");
}

/**
 * @tc.name: StartNativeProcessForDebugger_001
 * @tc.desc: test StartNativeProcessForDebugger_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartNativeProcessForDebugger_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_INVALID_OPERATION;
    AAFwk::MyStatus::GetInstance().queryAbilityInfo_ = true;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_ = {};
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_.applicationInfo.appProvisionType =
        Constants::APP_PROVISION_TYPE_DEBUG;

    AAFwk::Want want;
    auto ret = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_001 end");
}

/**
 * @tc.name: StartNativeProcessForDebugger_002
 * @tc.desc: test StartNativeProcessForDebugger_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartNativeProcessForDebugger_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().queryAbilityInfo_ = true;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_ = {};
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = true;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_.applicationInfo.appProvisionType =
        Constants::APP_PROVISION_TYPE_DEBUG;

    AAFwk::Want want;
    auto ret = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_002 end");
}

/**
 * @tc.name: StartNativeProcessForDebugger_003
 * @tc.desc: test StartNativeProcessForDebugger_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartNativeProcessForDebugger_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().queryAbilityInfo_ = true;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_ = {};
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = nullptr;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_.applicationInfo.appProvisionType =
        Constants::APP_PROVISION_TYPE_DEBUG;

    AAFwk::Want want;
    auto ret = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_003 end");
}

/**
 * @tc.name: StartNativeProcessForDebugger_004
 * @tc.desc: test StartNativeProcessForDebugger_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartNativeProcessForDebugger_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().queryAbilityInfo_ = true;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_ = {};
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_.applicationInfo.debug = false;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_.applicationInfo.appProvisionType =
        Constants::APP_PROVISION_TYPE_DEBUG;

    AAFwk::Want want;
    auto ret = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_004 end");
}

/**
 * @tc.name: StartNativeProcessForDebugger_005
 * @tc.desc: test StartNativeProcessForDebugger_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartNativeProcessForDebugger_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().queryAbilityInfo_ = true;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_ = {};
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = nullptr;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_.applicationInfo.debug = true;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_.applicationInfo.appProvisionType =
        Constants::APP_PROVISION_TYPE_DEBUG;

    AAFwk::Want want;
    auto ret = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_005 end");
}

/**
 * @tc.name: StartNativeProcessForDebugger_006
 * @tc.desc: test StartNativeProcessForDebugger_006
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartNativeProcessForDebugger_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().queryAbilityInfo_ = true;
    AAFwk::MyStatus::GetInstance().getHapModuleInfo_ = true;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_ = {};
    AAFwk::MyStatus::GetInstance().checkAppRunning_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = nullptr;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_.applicationInfo.debug = true;
    AAFwk::MyStatus::GetInstance().queryAbilityInfoValue_.applicationInfo.appProvisionType =
        Constants::APP_PROVISION_TYPE_DEBUG;

    std::string para = "2222";
    AAFwk::Want want;
    want.SetParam("perfCmd", para);
    auto ret = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_006 end");
}

/**
 * @tc.name: GetCurrentAccountId_001
 * @tc.desc: test GetCurrentAccountId_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetCurrentAccountId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetCurrentAccountId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().queryActiveOsAccountIds_ = ERR_NO_INIT;

    auto ret = appMgrServiceInner->GetCurrentAccountId();
    EXPECT_EQ(ret, DEFAULT_USER_ID);
    TAG_LOGI(AAFwkTag::TEST, "GetCurrentAccountId_001 end");
}

/**
 * @tc.name: GetCurrentAccountId_002
 * @tc.desc: test GetCurrentAccountId_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetCurrentAccountId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetCurrentAccountId_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().queryActiveOsAccountIds_ = ERR_OK;
    
    auto ret = appMgrServiceInner->GetCurrentAccountId();
    EXPECT_EQ(ret, DEFAULT_USER_ID);
    TAG_LOGI(AAFwkTag::TEST, "GetCurrentAccountId_002 end");
}

/**
 * @tc.name: SetCurrentUserId_001
 * @tc.desc: test SetCurrentUserId_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetCurrentUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetCurrentUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    
    appMgrServiceInner->SetCurrentUserId(2);
    EXPECT_EQ(appMgrServiceInner->currentUserId_, 2);
    TAG_LOGI(AAFwkTag::TEST, "SetCurrentUserId_001 end");
}

/**
 * @tc.name: GetRunningProcessInformation_001
 * @tc.desc: test GetRunningProcessInformation_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetRunningProcessInformation_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    std::string bundleName = "";
    int32_t userId = 1;
    std::vector<RunningProcessInfo> info;
    auto ret = appMgrServiceInner->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_001 end");
}

/**
 * @tc.name: GetRunningProcessInformation_002
 * @tc.desc: test GetRunningProcessInformation_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetRunningProcessInformation_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    std::string temp = "";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    appRecord->SetUid(0);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(0, nullptr));
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(0, appRecord));

    std::string bundleName = "";
    int32_t userId = 1;
    std::vector<RunningProcessInfo> info;
    auto ret = appMgrServiceInner->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_002 end");
}

/**
 * @tc.name: GetRunningProcessInformation_003
 * @tc.desc: test GetRunningProcessInformation_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetRunningProcessInformation_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    std::string temp = "";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    appRecord->SetUid(1);
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    AAFwk::MyStatus::GetInstance().getAppInfoList_.push_back(appInfo);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(0, appRecord));

    std::string bundleName = "";
    int32_t userId = 1;
    std::vector<RunningProcessInfo> info;
    auto ret = appMgrServiceInner->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_003 end");
}

/**
 * @tc.name: GetRunningProcessInformation_004
 * @tc.desc: test GetRunningProcessInformation_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetRunningProcessInformation_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    std::string temp = "";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    appRecord->SetUid(1);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "111";
    AAFwk::MyStatus::GetInstance().getAppInfoList_.push_back(appInfo);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(0, appRecord));

    std::string bundleName = "111";
    int32_t userId = 1;
    std::vector<RunningProcessInfo> info;
    auto ret = appMgrServiceInner->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessInformation_004 end");
}

/**
 * @tc.name: ChangeAppGcState_001
 * @tc.desc: test ChangeAppGcState_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, ChangeAppGcState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = RESOURCE_MANAGER_UID;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    pid_t pid = 0;
    int32_t state = 0;
    auto ret = appMgrServiceInner->ChangeAppGcState(pid, state);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_001 end");
}

/**
 * @tc.name: ChangeAppGcState_002
 * @tc.desc: test ChangeAppGcState_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, ChangeAppGcState_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = RESOURCE_MANAGER_UID;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().changeAppGcState_ = ERR_OK;

    pid_t pid = 0;
    int32_t state = 0;
    auto ret = appMgrServiceInner->ChangeAppGcState(pid, state);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ChangeAppGcState_002 end");
}

/**
 * @tc.name: SetAppWaitingDebug_001
 * @tc.desc: test SetAppWaitingDebug_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetAppWaitingDebug_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isShellCall_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;

    std::string bundleName = "";
    bool isPersist = false;
    auto ret = appMgrServiceInner->SetAppWaitingDebug(bundleName, isPersist);
    EXPECT_EQ(ret, ERR_NOT_DEVELOPER_MODE);
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_001 end");
}

/**
 * @tc.name: SetAppWaitingDebug_002
 * @tc.desc: test SetAppWaitingDebug_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetAppWaitingDebug_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isShellCall_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;

    std::string bundleName = "";
    bool isPersist = false;
    auto ret = appMgrServiceInner->SetAppWaitingDebug(bundleName, isPersist);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_002 end");
}

/**
 * @tc.name: SetAppWaitingDebug_003
 * @tc.desc: test SetAppWaitingDebug_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetAppWaitingDebug_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isShellCall_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    std::string bundleName = "1";
    bool isPersist = false;
    auto ret = appMgrServiceInner->SetAppWaitingDebug(bundleName, isPersist);
    EXPECT_EQ(ret, AAFwk::ERR_NOT_DEBUG_APP);
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_003 end");
}

/**
 * @tc.name: SetAppWaitingDebug_004
 * @tc.desc: test SetAppWaitingDebug_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetAppWaitingDebug_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isShellCall_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().v9BundleInfo_.applicationInfo.debug = true;
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().v9BundleInfo_.applicationInfo.appProvisionType =
        AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    appMgrServiceInner->waitingDebugBundleList_.clear();

    std::string bundleName = "1";
    bool isPersist = false;
    auto ret = appMgrServiceInner->SetAppWaitingDebug(bundleName, isPersist);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_004 end");
}

/**
 * @tc.name: SetAppWaitingDebug_005
 * @tc.desc: test SetAppWaitingDebug_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetAppWaitingDebug_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isShellCall_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().v9BundleInfo_.applicationInfo.debug = true;
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().v9BundleInfo_.applicationInfo.appProvisionType =
        AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    appMgrServiceInner->waitingDebugBundleList_.clear();
    appMgrServiceInner->waitingDebugBundleList_.insert(std::pair<std::string, bool>("11", true));

    std::string bundleName = "1";
    bool isPersist = false;
    auto ret = appMgrServiceInner->SetAppWaitingDebug(bundleName, isPersist);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_005 end");
}

/**
 * @tc.name: SetAppWaitingDebug_006
 * @tc.desc: test SetAppWaitingDebug_006
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetAppWaitingDebug_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isShellCall_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().v9BundleInfo_.applicationInfo.debug = true;
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().v9BundleInfo_.applicationInfo.appProvisionType =
        AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    appMgrServiceInner->waitingDebugBundleList_.clear();
    appMgrServiceInner->waitingDebugBundleList_.insert(std::pair<std::string, bool>("11", true));

    std::string bundleName = "1";
    bool isPersist = true;
    auto ret = appMgrServiceInner->SetAppWaitingDebug(bundleName, isPersist);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SetAppWaitingDebug_006 end");
}

/**
 * @tc.name: CancelAppWaitingDebug_001
 * @tc.desc: test CancelAppWaitingDebug_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CancelAppWaitingDebug_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CancelAppWaitingDebug_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isShellCall_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;

    auto ret = appMgrServiceInner->CancelAppWaitingDebug();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CancelAppWaitingDebug_001 end");
}

/**
 * @tc.name: GetWaitingDebugApp_001
 * @tc.desc: test GetWaitingDebugApp_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetWaitingDebugApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWaitingDebugApp_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->waitingDebugBundleList_.clear();
    AAFwk::MyStatus::GetInstance().isShellCall_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;

    std::vector<std::string> debugInfoList;
    auto ret = appMgrServiceInner->GetWaitingDebugApp(debugInfoList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(debugInfoList.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "GetWaitingDebugApp_001 end");
}

/**
 * @tc.name: GetWaitingDebugApp_002
 * @tc.desc: test GetWaitingDebugApp_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetWaitingDebugApp_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetWaitingDebugApp_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->waitingDebugBundleList_.clear();
    appMgrServiceInner->waitingDebugBundleList_.insert(std::pair<std::string, bool>("11", true));
    AAFwk::MyStatus::GetInstance().isShellCall_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;

    std::vector<std::string> debugInfoList;
    auto ret = appMgrServiceInner->GetWaitingDebugApp(debugInfoList);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(debugInfoList.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "GetWaitingDebugApp_002 end");
}

/**fCheckIsDebugApp_004
 * @tc.name: CheckIsDebugApp_001
 * @tc.desc: test CheckIsDebugApp_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CheckIsDebugApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIsDebugApp_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_ = nullptr;

    std::string bundleName;
    auto ret = appMgrServiceInner->CheckIsDebugApp(bundleName);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckIsDebugApp_001 end");
}

/**
 * @tc.name: CheckIsDebugApp_002
 * @tc.desc: test CheckIsDebugApp_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CheckIsDebugApp_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIsDebugApp_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    std::string bundleName;
    auto ret = appMgrServiceInner->CheckIsDebugApp(bundleName);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckIsDebugApp_002 end");
}

/**
 * @tc.name: CheckIsDebugApp_003
 * @tc.desc: test CheckIsDebugApp_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CheckIsDebugApp_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIsDebugApp_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_INVALID_OPERATION;

    std::string bundleName;
    auto ret = appMgrServiceInner->CheckIsDebugApp(bundleName);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckIsDebugApp_003 end");
}

/**
 * @tc.name: CheckIsDebugApp_004
 * @tc.desc: test CheckIsDebugApp_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CheckIsDebugApp_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIsDebugApp_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().v9BundleInfo_.applicationInfo.debug = true;
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().v9BundleInfo_.applicationInfo.appProvisionType =
        AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    
    std::string bundleName;
    auto ret = appMgrServiceInner->CheckIsDebugApp(bundleName);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "CheckIsDebugApp_004 end");
}

/**
 * @tc.name: RegisterAbilityDebugResponse_001
 * @tc.desc: test RegisterAbilityDebugResponse_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, RegisterAbilityDebugResponse_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterAbilityDebugResponse_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    
    sptr<IAbilityDebugResponse> response = nullptr;
    auto ret = appMgrServiceInner->RegisterAbilityDebugResponse(response);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "RegisterAbilityDebugResponse_001 end");
}

/**
 * @tc.name: RegisterAbilityDebugResponse_002
 * @tc.desc: test RegisterAbilityDebugResponse_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, RegisterAbilityDebugResponse_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterAbilityDebugResponse_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
    
    sptr<IAbilityDebugResponse> response = new MyAbilityDebugResponse();
    auto ret = appMgrServiceInner->RegisterAbilityDebugResponse(response);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "RegisterAbilityDebugResponse_002 end");
}

/**
 * @tc.name: IsAttachDebug_001
 * @tc.desc: test IsAttachDebug_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsAttachDebug_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAttachDebug_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    
    std::string bundleName = "";
    auto ret = appMgrServiceInner->IsAttachDebug(bundleName);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsAttachDebug_001 end");
}

/**
 * @tc.name: IsAttachDebug_002
 * @tc.desc: test IsAttachDebug_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsAttachDebug_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAttachDebug_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().isSACall_ = true;
    
    std::string bundleName = "111";
    auto ret = appMgrServiceInner->IsAttachDebug(bundleName);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsAttachDebug_002 end");
}

/**
 * @tc.name: NotifyPageShow_001
 * @tc.desc: test NotifyPageShow_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyPageShow_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPageShow_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    info1->accessTokenId = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getCallingTokenID_ = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    std::shared_ptr<AbilityInfo> info2 = std::make_shared<AbilityInfo>();
    info2->bundleName = "111";
    info2->name = "111";
    info2->moduleName = "111";
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ =
        std::make_shared<AbilityRunningRecord>(info2, nullptr, 0);
    PageStateData pageStateData;
    pageStateData.bundleName = "111";
    pageStateData.moduleName = "111";
    pageStateData.abilityName = "111";
    auto ret = appMgrServiceInner->NotifyPageShow(MyRemoteObject::GetInstance(), pageStateData);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyPageShow_001 end");
}

/**
 * @tc.name: NotifyPageHide_001
 * @tc.desc: test NotifyPageHide_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyPageHide_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPageHide_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    info1->accessTokenId = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getCallingTokenID_ = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    std::shared_ptr<AbilityInfo> info2 = std::make_shared<AbilityInfo>();
    info2->bundleName = "111";
    info2->name = "111";
    info2->moduleName = "111";
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ =
        std::make_shared<AbilityRunningRecord>(info2, nullptr, 0);
    PageStateData pageStateData;
    pageStateData.bundleName = "111";
    pageStateData.moduleName = "111";
    pageStateData.abilityName = "111";
    auto ret = appMgrServiceInner->NotifyPageHide(MyRemoteObject::GetInstance(), pageStateData);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyPageHide_001 end");
}

/**
 * @tc.name: JudgeSelfCalledByToken_001
 * @tc.desc: test JudgeSelfCalledByToken_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, JudgeSelfCalledByToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "JudgeSelfCalledByToken_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    info1->accessTokenId = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getCallingTokenID_ = FOUNDATION_UID;
    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = nullptr;
    PageStateData pageStateData;
    pageStateData.bundleName = "111";
    pageStateData.moduleName = "111";
    pageStateData.abilityName = "111";
    auto ret = appMgrServiceInner->JudgeSelfCalledByToken(MyRemoteObject::GetInstance(), pageStateData);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "JudgeSelfCalledByToken_001 end");
}

/**
 * @tc.name: RegisterAppRunningStatusListener_001
 * @tc.desc: test RegisterAppRunningStatusListener_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, RegisterAppRunningStatusListener_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterAppRunningStatusListener_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    auto ret = appMgrServiceInner->RegisterAppRunningStatusListener(MyRemoteObject::GetInstance());
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "RegisterAppRunningStatusListener_001 end");
}

/**
 * @tc.name: UnregisterAppRunningStatusListener_001
 * @tc.desc: test UnregisterAppRunningStatusListener_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, UnregisterAppRunningStatusListener_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterAppRunningStatusListener_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    auto ret = appMgrServiceInner->UnregisterAppRunningStatusListener(MyRemoteObject::GetInstance());
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterAppRunningStatusListener_001 end");
}

/**
 * @tc.name: StartChildProcess_001
 * @tc.desc: test StartChildProcess_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartChildProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().isChildProcessReachLimit_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = true;

    pid_t callingPid = 1;
    pid_t childPid = 1;
    ChildProcessRequest request;
    request.srcEntry = "111";
    auto ret = appMgrServiceInner->StartChildProcess(callingPid, childPid, request);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcess_001 end");
}

/**
 * @tc.name: StartChildProcessPreCheck_001
 * @tc.desc: test StartChildProcessPreCheck_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartChildProcessPreCheck_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessPreCheck_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().isChildProcessReachLimit_ = true;

    pid_t callingPid = 1;
    int32_t childProcessType = 1;
    auto ret = appMgrServiceInner->StartChildProcessPreCheck(callingPid, childProcessType);
    EXPECT_EQ(ret, AAFwk::ERR_CHILD_PROCESS_REACH_LIMIT);
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessPreCheck_001 end");
}

/**
 * @tc.name: StartChildProcessImpl_001
 * @tc.desc: test StartChildProcessImpl_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartChildProcessImpl_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    std::shared_ptr<ChildProcessRecord> childProcessRecord = nullptr;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    pid_t childPid = 1;
    ChildProcessArgs args;
    ChildProcessOptions options;
    auto ret = appMgrServiceInner->StartChildProcessImpl(childProcessRecord, appRecord, childPid, args, options);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_001 end");
}

/**
 * @tc.name: StartChildProcessImpl_002
 * @tc.desc: test StartChildProcessImpl_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartChildProcessImpl_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    std::shared_ptr<ChildProcessRecord> childProcessRecord = nullptr;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");;
    pid_t childPid = 1;
    ChildProcessArgs args;
    ChildProcessOptions options;
    auto ret = appMgrServiceInner->StartChildProcessImpl(childProcessRecord, appRecord, childPid, args, options);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_002 end");
}

/**
 * @tc.name: StartChildProcessImpl_003
 * @tc.desc: test StartChildProcessImpl_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartChildProcessImpl_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = nullptr;
    ChildProcessRequest request;
    std::shared_ptr<ChildProcessRecord> childProcessRecord =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");

    pid_t childPid = 1;
    ChildProcessArgs args;
    ChildProcessOptions options;
    auto ret = appMgrServiceInner->StartChildProcessImpl(childProcessRecord, appRecord, childPid, args, options);
    EXPECT_EQ(ret, ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT);
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_003 end");
}

/**
 * @tc.name: StartChildProcessImpl_004
 * @tc.desc: test StartChildProcessImpl_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartChildProcessImpl_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = std::make_shared<AppSpawnClient>();
    ChildProcessRequest request;
    std::shared_ptr<ChildProcessRecord> childProcessRecord =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");

    pid_t childPid = 1;
    ChildProcessArgs args;
    for (int i = 0; i < 20; i++) {
        args.fds.insert(std::pair<std::string, int32_t>(std::to_string(i), i));
    }
    ChildProcessOptions options;
    auto ret = appMgrServiceInner->StartChildProcessImpl(childProcessRecord, appRecord, childPid, args, options);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_004 end");
}

/**
 * @tc.name: StartChildProcessImpl_005
 * @tc.desc: test StartChildProcessImpl_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, StartChildProcessImpl_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getSpawnClient_ = std::make_shared<AppSpawnClient>();
    ChildProcessRequest request;
    std::shared_ptr<ChildProcessRecord> childProcessRecord =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");

    pid_t childPid = 1;
    ChildProcessArgs args;
    ChildProcessOptions options;
    auto ret = appMgrServiceInner->StartChildProcessImpl(childProcessRecord, appRecord, childPid, args, options);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartChildProcessImpl_005 end");
}

/**
 * @tc.name: GetChildProcessInfoForSelf_001
 * @tc.desc: test GetChildProcessInfoForSelf_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfoForSelf_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    ChildProcessInfo info;
    auto ret = appMgrServiceInner->GetChildProcessInfoForSelf(info);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_001 end");
}

/**
 * @tc.name: GetChildProcessInfoForSelf_002
 * @tc.desc: test GetChildProcessInfoForSelf_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfoForSelf_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");

    ChildProcessInfo info;
    auto ret = appMgrServiceInner->GetChildProcessInfoForSelf(info);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_002 end");
}

/**
 * @tc.name: GetChildProcessInfoForSelf_003
 * @tc.desc: test GetChildProcessInfoForSelf_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfoForSelf_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;

    ChildProcessInfo info;
    auto ret = appMgrServiceInner->GetChildProcessInfoForSelf(info);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_003 end");
}

/**
 * @tc.name: GetChildProcessInfoForSelf_004
 * @tc.desc: test GetChildProcessInfoForSelf_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfoForSelf_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_->childProcessRecordMap_.clear();

    ChildProcessInfo info;
    auto ret = appMgrServiceInner->GetChildProcessInfoForSelf(info);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_004 end");
}

/**
 * @tc.name: GetChildProcessInfoForSelf_005
 * @tc.desc: test GetChildProcessInfoForSelf_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfoForSelf_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_->childProcessRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_->childProcessRecordMap_.insert(std::pair<pid_t,
        std::shared_ptr<ChildProcessRecord>>(0, nullptr));
    ChildProcessInfo info;
    auto ret = appMgrServiceInner->GetChildProcessInfoForSelf(info);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfoForSelf_005 end");
}

/**
 * @tc.name: GetChildProcessInfo_001
 * @tc.desc: test GetChildProcessInfo_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    ChildProcessRequest request;
    std::shared_ptr<ChildProcessRecord> childProcessRecord =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    ChildProcessInfo info;
    bool isCallFromGetChildren = false;
    auto ret = appMgrServiceInner->GetChildProcessInfo(childProcessRecord, appRecord, info, isCallFromGetChildren);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_001 end");
}

/**
 * @tc.name: GetChildProcessInfo_002
 * @tc.desc: test GetChildProcessInfo_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getOsAccountLocalIdFromUid_ = ERR_NAME_NOT_FOUND;
    
    ChildProcessRequest request;
    std::shared_ptr<ChildProcessRecord> childProcessRecord =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    ChildProcessInfo info;
    bool isCallFromGetChildren = false;
    auto ret = appMgrServiceInner->GetChildProcessInfo(childProcessRecord, appRecord, info, isCallFromGetChildren);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_002 end");
}

/**
 * @tc.name: GetChildProcessInfo_003
 * @tc.desc: test GetChildProcessInfo_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getOsAccountLocalIdFromUid_ = ERR_OK;
    
    ChildProcessRequest request;
    std::shared_ptr<ChildProcessRecord> childProcessRecord =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    ChildProcessInfo info;
    bool isCallFromGetChildren = false;
    auto ret = appMgrServiceInner->GetChildProcessInfo(childProcessRecord, appRecord, info, isCallFromGetChildren);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_003 end");
}

/**
 * @tc.name: GetChildProcessInfo_004
 * @tc.desc: test GetChildProcessInfo_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getOsAccountLocalIdFromUid_ = ERR_OK;
    
    ChildProcessRequest request;
    std::shared_ptr<ChildProcessRecord> childProcessRecord =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    ChildProcessInfo info;
    bool isCallFromGetChildren = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    auto ret = appMgrServiceInner->GetChildProcessInfo(childProcessRecord, appRecord, info, isCallFromGetChildren);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_004 end");
}

/**
 * @tc.name: GetChildProcessInfo_005
 * @tc.desc: test GetChildProcessInfo_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfo_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getOsAccountLocalIdFromUid_ = ERR_OK;
    
    ChildProcessRequest request;
    std::shared_ptr<ChildProcessRecord> childProcessRecord =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    ChildProcessInfo info;
    bool isCallFromGetChildren = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getCloneBundleInfo_ = ERR_INVALID_VALUE;
    auto ret = appMgrServiceInner->GetChildProcessInfo(childProcessRecord, appRecord, info, isCallFromGetChildren);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_005 end");
}


/**
 * @tc.name: GetChildProcessInfo_006
 * @tc.desc: test GetChildProcessInfo_006
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetChildProcessInfo_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getOsAccountLocalIdFromUid_ = ERR_OK;
    
    ChildProcessRequest request;
    std::shared_ptr<ChildProcessRecord> childProcessRecord =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    ChildProcessInfo info;
    bool isCallFromGetChildren = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getCloneBundleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getBaseSharedBundleInfos_ = ERR_INVALID_VALUE;
    auto ret = appMgrServiceInner->GetChildProcessInfo(childProcessRecord, appRecord, info, isCallFromGetChildren);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetChildProcessInfo_006 end");
}

/**
 * @tc.name: AttachChildProcess_001
 * @tc.desc: test AttachChildProcess_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, AttachChildProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_ = 0;

    pid_t pid = -1;
    sptr<IChildScheduler> childScheduler = new MyChildScheduler();
    appMgrServiceInner->AttachChildProcess(pid, childScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_001 end");
}

/**
 * @tc.name: AttachChildProcess_002
 * @tc.desc: test AttachChildProcess_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, AttachChildProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_ = 0;
    appMgrServiceInner->appRunningManager_ = nullptr;

    pid_t pid = 1;
    sptr<IChildScheduler> childScheduler = new MyChildScheduler();
    appMgrServiceInner->AttachChildProcess(pid, childScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_002 end");
}

/**
 * @tc.name: AttachChildProcess_003
 * @tc.desc: test AttachChildProcess_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, AttachChildProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_ = 0;

    pid_t pid = 1;
    sptr<IChildScheduler> childScheduler = new MyChildScheduler();
    appMgrServiceInner->AttachChildProcess(pid, childScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getAppRunningProcessPidCall_, 1);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_, 0);
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_003 end");
}

/**
 * @tc.name: AttachChildProcess_004
 * @tc.desc: test AttachChildProcess_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, AttachChildProcess_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    g_scheduleLoadChildCall = 0;
    ChildProcessRequest request;
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_ =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_->childProcessType_ = CHILD_PROCESS_TYPE_NATIVE;
    pid_t pid = 1;
    sptr<IChildScheduler> childScheduler = new MyChildScheduler();
    appMgrServiceInner->AttachChildProcess(pid, childScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_, 1);
    EXPECT_EQ(g_scheduleLoadChildCall, 0);
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_004 end");
}

/**
 * @tc.name: AttachChildProcess_005
 * @tc.desc: test AttachChildProcess_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, AttachChildProcess_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_ = 0;
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    g_scheduleLoadChildCall = 0;
    ChildProcessRequest request;
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_ =
        std::make_shared<ChildProcessRecord>(0, request, nullptr);
    AAFwk::MyStatus::GetInstance().getChildProcessRecordByPid_->childProcessType_ = CHILD_PROCESS_TYPE_NOT_CHILD;
    pid_t pid = 1;
    sptr<IChildScheduler> childScheduler = new MyChildScheduler();
    appMgrServiceInner->AttachChildProcess(pid, childScheduler);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getChildProcessRecordByPidCall_, 1);
    EXPECT_EQ(g_scheduleLoadChildCall, 1);
    TAG_LOGI(AAFwkTag::TEST, "AttachChildProcess_005 end");
}

/**
 * @tc.name: DumpIpcAllStart_001
 * @tc.desc: test DumpIpcAllStart_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcAllStart_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStart_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcAllStart(result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStart_001 end");
}

/**
 * @tc.name: DumpIpcAllStart_002
 * @tc.desc: test DumpIpcAllStart_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcAllStart_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStart_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().dumpIpcAllStart_ = ERR_OK;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcAllStart(result);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStart_002 end");
}

/**
 * @tc.name: DumpIpcAllStop_001
 * @tc.desc: test DumpIpcAllStop_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcAllStop_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStop_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcAllStop(result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStop_001 end");
}

/**
 * @tc.name: DumpIpcAllStop_002
 * @tc.desc: test DumpIpcAllStop_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcAllStop_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStop_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().dumpIpcAllStop_ = ERR_OK;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcAllStop(result);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStop_002 end");
}

/**
 * @tc.name: DumpIpcAllStat_001
 * @tc.desc: test DumpIpcAllStat_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcAllStat_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStat_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcAllStat(result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStat_001 end");
}

/**
 * @tc.name: DumpIpcAllStat_002
 * @tc.desc: test DumpIpcAllStat_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcAllStat_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStat_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().dumpIpcAllStat_ = ERR_OK;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcAllStat(result);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcAllStat_002 end");
}

/**
 * @tc.name: DumpIpcStart_001
 * @tc.desc: test DumpIpcStart_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcStart_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStart_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcStart(0, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStart_001 end");
}

/**
 * @tc.name: DumpIpcStart_002
 * @tc.desc: test DumpIpcStart_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcStart_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStart_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().dumpIpcStart_ = ERR_OK;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcStart(0, result);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStart_002 end");
}

/**
 * @tc.name: DumpIpcStop_001
 * @tc.desc: test DumpIpcStop_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcStop_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStop_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcStop(0, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStop_001 end");
}

/**
 * @tc.name: DumpIpcStop_002
 * @tc.desc: test DumpIpcStop_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcStop_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStop_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().dumpIpcStop_ = ERR_OK;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcStop(0, result);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStop_002 end");
}

/**
 * @tc.name: DumpIpcStat_001
 * @tc.desc: test DumpIpcStat_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcStat_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStat_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcStat(0, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStat_001 end");
}

/**
 * @tc.name: DumpIpcStat_002
 * @tc.desc: test DumpIpcStat_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpIpcStat_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStat_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().dumpIpcStat_ = ERR_OK;

    std::string result = "";
    auto ret = appMgrServiceInner->DumpIpcStat(0, result);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DumpIpcStat_002 end");
}

/**
 * @tc.name: DumpFfrt_001
 * @tc.desc: test DumpFfrt_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpFfrt_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpFfrt_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string result = "";
    std::vector<int32_t> pid;
    auto ret = appMgrServiceInner->DumpFfrt(pid, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "DumpFfrt_001 end");
}

/**
 * @tc.name: DumpFfrt_002
 * @tc.desc: test DumpFfrt_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DumpFfrt_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpFfrt_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().dumpFfrt_ = ERR_OK;

    std::string result = "";
    std::vector<int32_t> pid;
    auto ret = appMgrServiceInner->DumpFfrt(pid, result);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DumpFfrt_002 end");
}

/**
 * @tc.name: IsFinalAppProcessByBundleName_001
 * @tc.desc: test IsFinalAppProcessByBundleName_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsFinalAppProcessByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string result = "";
    auto ret = appMgrServiceInner->IsFinalAppProcessByBundleName(result);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_001 end");
}

/**
 * @tc.name: IsFinalAppProcessByBundleName_002
 * @tc.desc: test IsFinalAppProcessByBundleName_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsFinalAppProcessByBundleName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string result = "";
    auto ret = appMgrServiceInner->IsFinalAppProcessByBundleName(result);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_002 end");
}

/**
 * @tc.name: IsFinalAppProcessByBundleName_003
 * @tc.desc: test IsFinalAppProcessByBundleName_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsFinalAppProcessByBundleName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    std::string result = "";
    auto ret = appMgrServiceInner->IsFinalAppProcessByBundleName(result);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_003 end");
}

/**
 * @tc.name: IsFinalAppProcessByBundleName_004
 * @tc.desc: test IsFinalAppProcessByBundleName_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsFinalAppProcessByBundleName_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getAllAppRunningRecordCount_ = 1;

    std::string result = "";
    auto ret = appMgrServiceInner->IsFinalAppProcessByBundleName(result);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_004 end");
}

/**
 * @tc.name: IsFinalAppProcessByBundleName_005
 * @tc.desc: test IsFinalAppProcessByBundleName_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsFinalAppProcessByBundleName_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    AAFwk::MyStatus::GetInstance().getAllAppRunningRecordCount_ = 1;
    
    std::string result = "111";
    auto ret = appMgrServiceInner->IsFinalAppProcessByBundleName(result);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "IsFinalAppProcessByBundleName_005 end");
}

/**
 * @tc.name: UnregisterRenderStateObserver_001
 * @tc.desc: test UnregisterRenderStateObserver_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, UnregisterRenderStateObserver_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterRenderStateObserver_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    sptr<IRenderStateObserver> observer = nullptr;
    auto ret = appMgrServiceInner->UnregisterRenderStateObserver(observer);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterRenderStateObserver_001 end");
}

/**
 * @tc.name: UnregisterRenderStateObserver_002
 * @tc.desc: test UnregisterRenderStateObserver_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, UnregisterRenderStateObserver_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterRenderStateObserver_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    
    sptr<IRenderStateObserver> observer = new MyRenderStateObserver();
    auto ret = appMgrServiceInner->UnregisterRenderStateObserver(observer);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UnregisterRenderStateObserver_002 end");
}

/**
 * @tc.name: SignRestartAppFlag_001
 * @tc.desc: test SignRestartAppFlag_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SignRestartAppFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SignRestartAppFlag_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    int32_t uid = 0;
    std::string instanceKey = "";
    auto ret = appMgrServiceInner->SignRestartAppFlag(uid, instanceKey);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "SignRestartAppFlag_001 end");
}

/**
 * @tc.name: GetAppRunningUniqueIdByPid_001
 * @tc.desc: test GetAppRunningUniqueIdByPid_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetAppRunningUniqueIdByPid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAppRunningUniqueIdByPid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    int32_t uid = 0;
    std::string instanceKey = "";
    auto ret = appMgrServiceInner->GetAppRunningUniqueIdByPid(uid, instanceKey);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "GetAppRunningUniqueIdByPid_001 end");
}

/**
 * @tc.name: NotifyMemMgrPriorityChanged_001
 * @tc.desc: test NotifyMemMgrPriorityChanged_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyMemMgrPriorityChanged_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemMgrPriorityChanged_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->priorityObject_ = nullptr;
    auto ret = appMgrServiceInner->NotifyMemMgrPriorityChanged(appRecord);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemMgrPriorityChanged_001 end");
}

/**
 * @tc.name: GetAllUIExtensionRootHostPid_001
 * @tc.desc: test GetAllUIExtensionRootHostPid_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetAllUIExtensionRootHostPid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllUIExtensionRootHostPid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    pid_t pid = 0;
    std::vector<pid_t> hostPids;
    auto ret = appMgrServiceInner->GetAllUIExtensionRootHostPid(pid, hostPids);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "GetAllUIExtensionRootHostPid_001 end");
}

/**
 * @tc.name: GetAllUIExtensionProviderPid_001
 * @tc.desc: test GetAllUIExtensionProviderPid_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, GetAllUIExtensionProviderPid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllUIExtensionProviderPid_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    
    pid_t pid = 0;
    std::vector<pid_t> hostPids;
    auto ret = appMgrServiceInner->GetAllUIExtensionProviderPid(pid, hostPids);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "GetAllUIExtensionProviderPid_001 end");
}

/**
 * @tc.name: NotifyMemorySizeStateChanged_001
 * @tc.desc: test NotifyMemorySizeStateChanged_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyMemorySizeStateChanged_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().checkSpecific_ = false;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = false;

    int32_t memorySizeState = 0;
    auto ret = appMgrServiceInner->NotifyMemorySizeStateChanged(memorySizeState);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_001 end");
}

/**
 * @tc.name: NotifyMemorySizeStateChanged_002
 * @tc.desc: test NotifyMemorySizeStateChanged_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyMemorySizeStateChanged_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().checkSpecific_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().handleMemorySizeInSufficent_ = ERR_OK;

    int32_t memorySizeState = MemoryState::LOW_MEMORY;
    auto ret = appMgrServiceInner->NotifyMemorySizeStateChanged(memorySizeState);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_002 end");
}

/**
 * @tc.name: NotifyMemorySizeStateChanged_003
 * @tc.desc: test NotifyMemorySizeStateChanged_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyMemorySizeStateChanged_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().checkSpecific_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().handleRequireBigMemoryOptimization_ = ERR_OK;

    int32_t memorySizeState = MemoryState::REQUIRE_BIG_MEMORY;
    auto ret = appMgrServiceInner->NotifyMemorySizeStateChanged(memorySizeState);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_003 end");
}

/**
 * @tc.name: NotifyMemorySizeStateChanged_004
 * @tc.desc: test NotifyMemorySizeStateChanged_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyMemorySizeStateChanged_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().checkSpecific_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().handleNoRequireBigMemoryOptimization_ = ERR_OK;

    int32_t memorySizeState = MemoryState::NO_REQUIRE_BIG_MEMORY;
    auto ret = appMgrServiceInner->NotifyMemorySizeStateChanged(memorySizeState);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_004 end");
}

/**
 * @tc.name: NotifyMemorySizeStateChanged_005
 * @tc.desc: test NotifyMemorySizeStateChanged_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyMemorySizeStateChanged_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().checkSpecific_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;

    int32_t memorySizeState = 4;
    auto ret = appMgrServiceInner->NotifyMemorySizeStateChanged(memorySizeState);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyMemorySizeStateChanged_005 end");
}

/**
 * @tc.name: SetSupportedProcessCache_001
 * @tc.desc: test SetSupportedProcessCache_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetSupportedProcessCache_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCache_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    int32_t pid = 0;
    bool isSupport = false;
    auto ret = appMgrServiceInner->SetSupportedProcessCache(pid, isSupport);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCache_001 end");
}

/**
 * @tc.name: SetSupportedProcessCache_002
 * @tc.desc: test SetSupportedProcessCache_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetSupportedProcessCache_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCache_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    
    int32_t pid = 0;
    bool isSupport = false;
    auto ret = appMgrServiceInner->SetSupportedProcessCache(pid, isSupport);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCache_002 end");
}

/**
 * @tc.name: SetSupportedProcessCache_003
 * @tc.desc: test SetSupportedProcessCache_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetSupportedProcessCache_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCache_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    DelayedSingleton<CacheProcessManager>::GetInstance()->maxProcCacheNum_ = 0;
    DelayedSingleton<CacheProcessManager>::GetInstance()->warmStartProcesEnable_ = false;

    int32_t pid = 0;
    bool isSupport = false;
    auto ret = appMgrServiceInner->SetSupportedProcessCache(pid, isSupport);
    EXPECT_EQ(ret, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCache_003 end");
}

/**
 * @tc.name: SetSupportedProcessCache_004
 * @tc.desc: test SetSupportedProcessCache_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetSupportedProcessCache_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCache_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    DelayedSingleton<CacheProcessManager>::GetInstance()->maxProcCacheNum_ = 1;
    DelayedSingleton<CacheProcessManager>::GetInstance()->warmStartProcesEnable_ = true;

    int32_t pid = 0;
    bool isSupport = false;
    auto ret = appMgrServiceInner->SetSupportedProcessCache(pid, isSupport);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SetSupportedProcessCache_004 end");
}

/**
 * @tc.name: IsAppProcessesAllCached_001
 * @tc.desc: test IsAppProcessesAllCached_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsAppProcessesAllCached_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAppProcessesAllCached_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string bundleName = "";
    int32_t uid = 0;
    std::set<std::shared_ptr<AppRunningRecord>> cachedSet;
    auto ret = appMgrServiceInner->IsAppProcessesAllCached(bundleName, uid, cachedSet);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsAppProcessesAllCached_001 end");
}

/**
 * @tc.name: IsSceneBoardCall_001
 * @tc.desc: test IsSceneBoardCall_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsSceneBoardCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSceneBoardCall_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->remoteClientManager_= nullptr;

    auto ret = appMgrServiceInner->IsSceneBoardCall();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsSceneBoardCall_001 end");
}

/**
 * @tc.name: IsSceneBoardCall_002
 * @tc.desc: test IsSceneBoardCall_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsSceneBoardCall_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSceneBoardCall_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    auto ret = appMgrServiceInner->IsSceneBoardCall();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsSceneBoardCall_002 end");
}

/**
 * @tc.name: CreateNativeChildProcess_001
 * @tc.desc: test CreateNativeChildProcess_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CreateNativeChildProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    pid_t hostPid = 0;
    std::string libName = "111";
    int32_t childProcessCount = 0;
    sptr<IRemoteObject> callback = MyRemoteObject::GetInstance();
    ChildProcessOptions options;
    options.isolationMode = false;
    options.customProcessName = "";
    options.isolationUid = false;
    ChildProcessRequest request;
    request.childProcessType = CHILD_PROCESS_TYPE_NATIVE;
    request.childProcessCount = childProcessCount;
    request.options = options;
    auto ret = appMgrServiceInner->CreateNativeChildProcess(hostPid, libName, callback, request);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_001 end");
}

/**
 * @tc.name: CreateNativeChildProcess_002
 * @tc.desc: test CreateNativeChildProcess_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CreateNativeChildProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().isChildProcessReachLimit_ = true;

    pid_t hostPid = 1;
    std::string libName = "111";
    int32_t childProcessCount = 0;
    sptr<IRemoteObject> callback = MyRemoteObject::GetInstance();
    ChildProcessOptions options;
    options.isolationMode = false;
    options.customProcessName = "";
    options.isolationUid = false;
    ChildProcessRequest request;
    request.childProcessType = CHILD_PROCESS_TYPE_NATIVE;
    request.childProcessCount = childProcessCount;
    request.options = options;
    auto ret = appMgrServiceInner->CreateNativeChildProcess(hostPid, libName, callback, request);
    EXPECT_EQ(ret, AAFwk::ERR_CHILD_PROCESS_REACH_LIMIT);
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_002 end");
}

/**
 * @tc.name: CreateNativeChildProcess_003
 * @tc.desc: test CreateNativeChildProcess_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CreateNativeChildProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().isChildProcessReachLimit_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = true;

    pid_t hostPid = 1;
    std::string libName = "111";
    int32_t childProcessCount = 0;
    sptr<IRemoteObject> callback = MyRemoteObject::GetInstance();
    ChildProcessOptions options;
    options.isolationMode = false;
    options.customProcessName = "";
    options.isolationUid = false;
    ChildProcessRequest request;
    request.childProcessType = CHILD_PROCESS_TYPE_NATIVE;
    request.childProcessCount = childProcessCount;
    request.options = options;
    auto ret = appMgrServiceInner->CreateNativeChildProcess(hostPid, libName, callback, request);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_003 end");
}

/**
 * @tc.name: CreateNativeChildProcess_004
 * @tc.desc: test CreateNativeChildProcess_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CreateNativeChildProcess_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().isChildProcessReachLimit_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;

    pid_t hostPid = 1;
    std::string libName = "111";
    int32_t childProcessCount = 0;
    sptr<IRemoteObject> callback = MyRemoteObject::GetInstance();
    ChildProcessOptions options;
    options.isolationMode = true;
    options.customProcessName = "";
    options.isolationUid = true;
    ChildProcessRequest request;
    request.childProcessType = CHILD_PROCESS_TYPE_NATIVE;
    request.childProcessCount = childProcessCount;
    request.options = options;
    auto ret = appMgrServiceInner->CreateNativeChildProcess(hostPid, libName, callback, request);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_004 end");
}

/**
 * @tc.name: CreateNativeChildProcess_005
 * @tc.desc: test CreateNativeChildProcess_005
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CreateNativeChildProcess_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_005 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().isChildProcessReachLimit_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;

    pid_t hostPid = 1;
    std::string libName = "111";
    int32_t childProcessCount = 0;
    sptr<IRemoteObject> callback = MyRemoteObject::GetInstance();
    ChildProcessOptions options;
    options.isolationMode = true;
    options.customProcessName = "";
    options.isolationUid = false;
    ChildProcessRequest request;
    request.childProcessType = CHILD_PROCESS_TYPE_NATIVE;
    request.childProcessCount = childProcessCount;
    request.options = options;
    auto ret = appMgrServiceInner->CreateNativeChildProcess(hostPid, libName, callback, request);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_005 end");
}

/**
 * @tc.name: CreateNativeChildProcess_006
 * @tc.desc: test CreateNativeChildProcess_006
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CreateNativeChildProcess_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_006 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningProcessPid_ = nullptr;
    std::shared_ptr<ApplicationInfo> info1 = std::make_shared<ApplicationInfo>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().isChildProcessReachLimit_ = false;
    AAFwk::MyStatus::GetInstance().isLogoutUser_ = false;

    pid_t hostPid = 1;
    std::string libName = "111";
    int32_t childProcessCount = 0;
    sptr<IRemoteObject> callback = MyRemoteObject::GetInstance();
    ChildProcessOptions options;
    options.isolationMode = false;
    options.customProcessName = "";
    options.isolationUid = true;
    ChildProcessRequest request;
    request.childProcessType = CHILD_PROCESS_TYPE_NATIVE;
    request.childProcessCount = childProcessCount;
    request.options = options;
    auto ret = appMgrServiceInner->CreateNativeChildProcess(hostPid, libName, callback, request);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CreateNativeChildProcess_006 end");
}

/**
 * @tc.name: NotifyProcessDependedOnWeb_001
 * @tc.desc: test NotifyProcessDependedOnWeb_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, NotifyProcessDependedOnWeb_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyProcessDependedOnWeb_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, "");

    auto ret = appMgrServiceInner->NotifyProcessDependedOnWeb();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "NotifyProcessDependedOnWeb_001 end");
}

/**
 * @tc.name: CleanAbilityByUserRequest_001
 * @tc.desc: test CleanAbilityByUserRequest_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CleanAbilityByUserRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanAbilityByUserRequest_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    sptr<IRemoteObject> token = nullptr;
    auto ret = appMgrServiceInner->CleanAbilityByUserRequest(token);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CleanAbilityByUserRequest_001 end");
}

/**
 * @tc.name: CleanAbilityByUserRequest_002
 * @tc.desc: test CleanAbilityByUserRequest_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CleanAbilityByUserRequest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanAbilityByUserRequest_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    sptr<IRemoteObject> token = MyRemoteObject::GetInstance();
    auto ret = appMgrServiceInner->CleanAbilityByUserRequest(token);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CleanAbilityByUserRequest_002 end");
}

/**
 * @tc.name: CleanAbilityByUserRequest_003
 * @tc.desc: test CleanAbilityByUserRequest_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CleanAbilityByUserRequest_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanAbilityByUserRequest_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().handleUserRequestClean_ = false;

    sptr<IRemoteObject> token = MyRemoteObject::GetInstance();
    auto ret = appMgrServiceInner->CleanAbilityByUserRequest(token);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CleanAbilityByUserRequest_003 end");
}

/**
 * @tc.name: CleanAbilityByUserRequest_004
 * @tc.desc: test CleanAbilityByUserRequest_004
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CleanAbilityByUserRequest_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanAbilityByUserRequest_004 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().handleUserRequestClean_ = true;
    AAFwk::MyStatus::GetInstance().handleUserRequestCleanPid_ = 0;
    AAFwk::MyStatus::GetInstance().handleUserRequestCleanUid_ = 0;

    sptr<IRemoteObject> token = MyRemoteObject::GetInstance();
    auto ret = appMgrServiceInner->CleanAbilityByUserRequest(token);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "CleanAbilityByUserRequest_004 end");
}

/**
 * @tc.name: RegisterKiaInterceptor_001
 * @tc.desc: test RegisterKiaInterceptor_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, RegisterKiaInterceptor_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterKiaInterceptor_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifySuperviseKiaServicePermission_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;

    sptr<IKiaInterceptor> interceptor = nullptr;
    auto ret = appMgrServiceInner->RegisterKiaInterceptor(interceptor);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "RegisterKiaInterceptor_001 end");
}

/**
 * @tc.name: RegisterKiaInterceptor_002
 * @tc.desc: test RegisterKiaInterceptor_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, RegisterKiaInterceptor_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterKiaInterceptor_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifySuperviseKiaServicePermission_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;

    sptr<IKiaInterceptor> interceptor = new MyKiaInterceptor();
    auto ret = appMgrServiceInner->RegisterKiaInterceptor(interceptor);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "RegisterKiaInterceptor_002 end");
}

/**
 * @tc.name: CheckIsKiaProcess_001
 * @tc.desc: test CheckIsKiaProcess_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CheckIsKiaProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIsKiaProcess_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifySuperviseKiaServicePermission_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    appMgrServiceInner->appRunningManager_ = nullptr;

    pid_t pid = 0;
    bool isKia = false;
    auto ret = appMgrServiceInner->CheckIsKiaProcess(pid, isKia);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "CheckIsKiaProcess_001 end");
}

/**
 * @tc.name: CheckIsKiaProcess_002
 * @tc.desc: test CheckIsKiaProcess_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, CheckIsKiaProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIsKiaProcess_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifySuperviseKiaServicePermission_ = true;
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    AAFwk::MyStatus::GetInstance().checkIsKiaProcess_ = ERR_OK;

    pid_t pid = 0;
    bool isKia = false;
    auto ret = appMgrServiceInner->CheckIsKiaProcess(pid, isKia);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "CheckIsKiaProcess_002 end");
}

/**
 * @tc.name: IsSpecifiedModuleLoaded_001
 * @tc.desc: test IsSpecifiedModuleLoaded_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, IsSpecifiedModuleLoaded_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSpecifiedModuleLoaded_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;

    AAFwk::Want want;
    AbilityInfo abilityInfo;
    bool isDebug = false;
    auto ret = appMgrServiceInner->IsSpecifiedModuleLoaded(want, abilityInfo, isDebug);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "IsSpecifiedModuleLoaded_001 end");
}

/**
 * @tc.name: LaunchAbility_001
 * @tc.desc: test LaunchAbility_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, LaunchAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LaunchAbility_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ = nullptr;

    sptr<IRemoteObject> token = nullptr;
    auto ret = appMgrServiceInner->LaunchAbility(token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "LaunchAbility_001 end");
}

/**
 * @tc.name: LaunchAbility_002
 * @tc.desc: test LaunchAbility_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, LaunchAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LaunchAbility_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningByToken_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_ =
        std::make_shared<AbilityRunningRecord>(nullptr, nullptr, 0);
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    AAFwk::MyStatus::GetInstance().getAbilityRunningRecordByToken_->SetWant(want);

    sptr<IRemoteObject> token = nullptr;
    auto ret = appMgrServiceInner->LaunchAbility(token);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "LaunchAbility_002 end");
}

/**
 * @tc.name: SetPreloadDebugApp_0100
 * @tc.desc: test SetPreloadDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, SetPreloadDebugApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetPreloadDebugApp_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);

    auto want = std::make_shared<AAFwk::Want>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(want, nullptr);
    ASSERT_NE(appInfo, nullptr);
    appInfo->debug = true;
    appInfo->appProvisionType = AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    std::string bundleName = "com.example.test";
    appInfo->bundleName = bundleName;
    
    OHOS::AAFwk::MyStatus::GetInstance().getBoolParameter_ = true;
    appMgrServiceInner->isInitAppWaitingDebugListExecuted_ = true;
    appMgrServiceInner->waitingDebugBundleList_.try_emplace(bundleName, true);

    appMgrServiceInner->SetPreloadDebugApp(want, appInfo);
    bool debugApp = want->GetBoolParam(DEBUG_APP, false);
    EXPECT_EQ(debugApp, true);
    TAG_LOGI(AAFwkTag::TEST, "SetPreloadDebugApp_0100 end");
}

/**
 * @tc.name: PromoteCurrentToCandidateMasterProcess_0100
 * @tc.desc: test SetPreloadDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, PromoteCurrentToCandidateMasterProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PromoteCurrentToCandidateMasterProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = true;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(true);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(0);
    int32_t res = appMgrServiceInner->PromoteCurrentToCandidateMasterProcess(true);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "PromoteCurrentToCandidateMasterProcess_0100 end");
}

/**
 * @tc.name: PromoteCurrentToCandidateMasterProcess_0200
 * @tc.desc: test SetPreloadDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, PromoteCurrentToCandidateMasterProcess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PromoteCurrentToCandidateMasterProcess_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = true;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(false);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(1);
    int32_t res = appMgrServiceInner->PromoteCurrentToCandidateMasterProcess(true);
    EXPECT_EQ(res, AAFwk::ERR_NOT_ISOLATION_PROCESS);
    TAG_LOGI(AAFwkTag::TEST, "PromoteCurrentToCandidateMasterProcess_0200 end");
}

/**
 * @tc.name: PromoteCurrentToCandidateMasterProcess_0300
 * @tc.desc: test SetPreloadDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, PromoteCurrentToCandidateMasterProcess_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PromoteCurrentToCandidateMasterProcess_0300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = false;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(false);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(0);
    int32_t res = appMgrServiceInner->PromoteCurrentToCandidateMasterProcess(true);
    EXPECT_EQ(res, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "PromoteCurrentToCandidateMasterProcess_0300 end");
}

/**
 * @tc.name: DemoteCurrentFromCandidateMasterProcess_0100
 * @tc.desc: test SetPreloadDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DemoteCurrentFromCandidateMasterProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DemoteCurrentFromCandidateMasterProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = true;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(false);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(1);
    int32_t res = appMgrServiceInner->DemoteCurrentFromCandidateMasterProcess();
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DemoteCurrentFromCandidateMasterProcess_0100 end");
}


/**
 * @tc.name: DemoteCurrentFromCandidateMasterProcess_0200
 * @tc.desc: test SetPreloadDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DemoteCurrentFromCandidateMasterProcess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DemoteCurrentFromCandidateMasterProcess_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = true;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(true);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(0);
    int32_t res = appMgrServiceInner->DemoteCurrentFromCandidateMasterProcess();
    EXPECT_EQ(res, AAFwk::ERR_ALREADY_MASTER_PROCESS);
    TAG_LOGI(AAFwkTag::TEST, "DemoteCurrentFromCandidateMasterProcess_0200 end");
}

/**
 * @tc.name: DemoteCurrentFromCandidateMasterProcess_0300
 * @tc.desc: test SetPreloadDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DemoteCurrentFromCandidateMasterProcess_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DemoteCurrentFromCandidateMasterProcess_0300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = false;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(false);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(0);
    int32_t res = appMgrServiceInner->DemoteCurrentFromCandidateMasterProcess();
    EXPECT_EQ(res, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "DemoteCurrentFromCandidateMasterProcess_0300 end");
}

/**
 * @tc.name: DemoteCurrentFromCandidateMasterProcess_0400
 * @tc.desc: test SetPreloadDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, DemoteCurrentFromCandidateMasterProcess_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DemoteCurrentFromCandidateMasterProcess_0400 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = true;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(false);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(0);
    int32_t res = appMgrServiceInner->DemoteCurrentFromCandidateMasterProcess();
    EXPECT_EQ(res, AAFwk::ERR_NOT_CANDIDATE_MASTER_PROCESS);
    TAG_LOGI(AAFwkTag::TEST, "DemoteCurrentFromCandidateMasterProcess_0400 end");
}

/**
 * @tc.name: ExitMasterProcessRole_0100
 * @tc.desc: test ExitMasterProcessRole
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, ExitMasterProcessRole_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExitMasterProcessRole_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = false;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(false);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(0);
    int32_t res = appMgrServiceInner->ExitMasterProcessRole();
    EXPECT_EQ(res, AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "ExitMasterProcessRole_0100 end");
}

/**
 * @tc.name: ExitMasterProcessRole_0200
 * @tc.desc: test ExitMasterProcessRole
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, ExitMasterProcessRole_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExitMasterProcessRole_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = true;
    AAFwk::MyStatus::GetInstance().specifiedProcessRequest_ = nullptr;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(false);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(0);
    int32_t res = appMgrServiceInner->ExitMasterProcessRole();
    EXPECT_EQ(res, AAFwk::ERR_NOT_MASTER_PROCESS);
    TAG_LOGI(AAFwkTag::TEST, "ExitMasterProcessRole_0200 end");
}

/**
 * @tc.name: ExitMasterProcessRole_0300
 * @tc.desc: test ExitMasterProcessRole
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, ExitMasterProcessRole_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExitMasterProcessRole_0300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = true;
    AAFwk::MyStatus::GetInstance().specifiedProcessRequest_ = std::make_shared<AppExecFwk::SpecifiedRequest>();
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(true);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(0);
    int32_t res = appMgrServiceInner->ExitMasterProcessRole();
    EXPECT_EQ(res, AAFwk::ERR_NOT_ON_NEW_PROCESS_REQUEST_DONE);
    TAG_LOGI(AAFwkTag::TEST, "ExitMasterProcessRole_0300 end");
}

/**
 * @tc.name: ExitMasterProcessRole_0400
 * @tc.desc: test ExitMasterProcessRole
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerEighthTest, ExitMasterProcessRole_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExitMasterProcessRole_0400 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    AAFwk::MyStatus::GetInstance().isStartSpecifiedProcess_ = true;
    AAFwk::MyStatus::GetInstance().specifiedProcessRequest_ = nullptr;
    std::shared_ptr<ApplicationInfo> info1 = nullptr;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(info1, 0, "");
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetMasterProcess(true);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_->SetTimeStamp(0);
    int32_t res = appMgrServiceInner->ExitMasterProcessRole();
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ExitMasterProcessRole_0400 end");
}
} // namespace AppExecFwk
} // namespace OHOS