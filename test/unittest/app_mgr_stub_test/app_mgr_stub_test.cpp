/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#define protected public
#include "app_foreground_state_observer_stub.h"
#include "app_mgr_stub.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "mock_app_mgr_service.h"
#include "render_state_observer_stub.h"
#include "native_child_notify_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t USER_ID = 100;
}  // namespace

class AppForegroundStateObserverMock : public AppForegroundStateObserverStub {
public:
    AppForegroundStateObserverMock() = default;
    virtual ~AppForegroundStateObserverMock() = default;
    void OnAppStateChanged(const AppStateData &appStateData) override
    {}
};

class RenderStateObserverMock : public RenderStateObserverStub {
public:
    RenderStateObserverMock() = default;
    virtual ~RenderStateObserverMock() = default;
    void OnRenderStateChanged(const RenderStateData &renderStateData) override
    {}
};

class NativeChildCallbackMock : public NativeChildNotifyStub {
public:
    NativeChildCallbackMock() = default;
    virtual ~NativeChildCallbackMock() = default;

    void OnNativeChildStarted(const sptr<IRemoteObject> &nativeChild) {}
    void OnError(int32_t errCode) {}
    int32_t OnNativeChildExit(int32_t pid, int32_t signal) { return 0; }
};

class AppMgrStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppMgrService> mockAppMgrService_;

    void WriteInterfaceToken(MessageParcel& data);
};

void AppMgrStubTest::SetUpTestCase(void)
{}

void AppMgrStubTest::TearDownTestCase(void)
{}

void AppMgrStubTest::SetUp()
{
    GTEST_LOG_(INFO) << "AppMgrStubTest::SetUp()";

    mockAppMgrService_ = new MockAppMgrService();
}

void AppMgrStubTest::TearDown()
{}

void AppMgrStubTest::WriteInterfaceToken(MessageParcel& data)
{
    GTEST_LOG_(INFO) << "AppMgrStubTest::WriteInterfaceToken()";

    data.WriteInterfaceToken(AppMgrStub::GetDescriptor());
}

/**
 * @tc.name: AppMgrStub_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrStubTest, AppMgrStub_GetProcessRunningInfosByUserId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppMgrStub_GetProcessRunningInfosByUserId_0100 start";

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteInt32(USER_ID);

    EXPECT_CALL(*mockAppMgrService_, GetProcessRunningInfosByUserId(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_USER_ID), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "AppMgrStub_GetProcessRunningInfosByUserId_0100 end";
}

/**
 * @tc.name: HandleGetAppRunningStateByBundleName_0100
 * @tc.desc: Handle get app running state by bundle name.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleGetAppRunningStateByBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, GetAppRunningStateByBundleName(_)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_APP_RUNNING_STATE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyLoadRepairPatch_0100
 * @tc.desc: Handle notify load repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleNotifyLoadRepairPatch_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);
    mockAppMgrService_->HandleNotifyLoadRepairPatch(data, reply);
    EXPECT_TRUE(mockAppMgrService_ != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyHotReloadPage_0100
 * @tc.desc: Handle notify ace hot reload page.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleNotifyHotReloadPage_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);
    mockAppMgrService_->HandleNotifyHotReloadPage(data, reply);
    EXPECT_TRUE(mockAppMgrService_ != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyUnLoadRepairPatch_0100
 * @tc.desc: Handle notify unload repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleNotifyUnLoadRepairPatch_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);
    mockAppMgrService_->HandleNotifyUnLoadRepairPatch(data, reply);
    EXPECT_TRUE(mockAppMgrService_ != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrStubTest, PreStartNWebSpawnProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    EXPECT_CALL(*mockAppMgrService_, PreStartNWebSpawnProcess()).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::PRE_START_NWEBSPAWN_PROCESS), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetProcessMemoryByPid_001
 * @tc.desc: Get memorySize by pid.
 * @tc.type: FUNC
 * @tc.require: issueI76JBF
 */
HWTEST_F(AppMgrStubTest, GetProcessMemoryByPid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t pid = 0;
    data.WriteInt32(pid);

    EXPECT_CALL(*mockAppMgrService_, GetProcessMemoryByPid(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_PROCESS_MEMORY_BY_PID), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRunningProcessInformation_001
 * @tc.desc: Get pid list by bundleName.
 * @tc.type: FUNC
 * @tc.require: issueI76JBF
 */
HWTEST_F(AppMgrStubTest, GetRunningProcessInformation_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    int32_t userId = 0;
    data.WriteString(bundleName);
    data.WriteInt32(userId);

    EXPECT_CALL(*mockAppMgrService_, GetRunningProcessInformation(_, _, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_PIDS_BY_BUNDLENAME), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyFault_001
 * @tc.desc: Handle notify fault.
 * @tc.type: FUNC
 * @tc.require: issueI79RY8
 */
HWTEST_F(AppMgrStubTest, HandleNotifyFault_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    FaultData faultData;
    faultData.errorObject.name = "testName";
    faultData.errorObject.message = "testMessage";
    faultData.errorObject.stack = "testStack";
    faultData.faultType = FaultDataType::UNKNOWN;
    data.WriteParcelable(&faultData);
    EXPECT_CALL(*mockAppMgrService_, NotifyAppFault(_)).Times(1);
    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleNotifyFaultBySA_001
 * @tc.desc: Handle notify fault by SA.
 * @tc.type: FUNC
 * @tc.require: issueI79RY8
 */
HWTEST_F(AppMgrStubTest, HandleNotifyFaultBySA_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    AppFaultDataBySA faultData;
    faultData.errorObject.name = "testName";
    faultData.errorObject.message = "testMessage";
    faultData.errorObject.stack = "testStack";
    faultData.faultType = FaultDataType::UNKNOWN;
    faultData.pid = 24;
    data.WriteParcelable(&faultData);
    EXPECT_CALL(*mockAppMgrService_, NotifyAppFaultBySA(_)).Times(1);
    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT_BY_SA), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleSetAppFreezeFilter_001
 * @tc.desc: Handle Set AppFreeze Filter.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleSetAppFreezeFilter_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    data.WriteInt32(0);
    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::SET_APPFREEZE_FILTER), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleChangeAppGcState_001
 * @tc.desc: Handle change app Gc state.
 * @tc.type: FUNC
 * @tc.require: issuesI85VVU
 */
HWTEST_F(AppMgrStubTest, HandleChangeAppGcState_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteInt32(1);
    auto result = mockAppMgrService_->OnRemoteRequest(
            static_cast<uint32_t>(AppMgrInterfaceCode::CHANGE_APP_GC_STATE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: IsAppRunning_001
 * @tc.desc: On remote request to query the running status of the application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, IsAppRunning_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    int32_t appCloneIndex = 0;
    bool isRunning = false;
    data.WriteString(bundleName);
    data.WriteInt32(appCloneIndex);
    data.WriteBool(isRunning);

    EXPECT_CALL(*mockAppMgrService_, IsAppRunning(_, _, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::IS_APP_RUNNING), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: IsApplicationRunning_001
 * @tc.desc: On remote request to query the running status of the application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, IsApplicationRunning_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    bool isRunning = false;
    data.WriteString(bundleName);
    data.WriteBool(isRunning);

    EXPECT_CALL(*mockAppMgrService_, IsApplicationRunning(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::IS_APPLICATION_RUNNING), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.number: HandleRegisterAbilityForegroundStateObserver_0100
 * @tc.desc: Verify it when write result success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleRegisterAbilityForegroundStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    mockAppMgrService_->HandleRegisterAbilityForegroundStateObserver(data, reply);
    EXPECT_TRUE(mockAppMgrService_ != nullptr);
}

/**
 * @tc.number: HandleUnregisterAbilityForegroundStateObserver_0100
 * @tc.desc: Verify it when write result success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUnregisterAbilityForegroundStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = mockAppMgrService_->HandleUnregisterAbilityForegroundStateObserver(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleRegisterAppForegroundStateObserver_0100
 * @tc.desc: Test when callback is not nullptr the return of writeInt32 is true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleRegisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) AppForegroundStateObserverMock();
    data.WriteRemoteObject(object);
    int32_t pid = 1;
    reply.WriteInt32(pid);
    auto res = mockAppMgrService_->HandleRegisterAppForegroundStateObserver(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleUnregisterAppForegroundStateObserver_0100
 * @tc.desc: Test when callback is not nullptr the return of writeInt32 is true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUnregisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) AppForegroundStateObserverMock();
    data.WriteRemoteObject(object);
    int32_t pid = 1;
    reply.WriteInt32(pid);
    auto res = mockAppMgrService_->HandleUnregisterAppForegroundStateObserver(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleRegisterNativeChildExitNotify_0100
 * @tc.desc: Test when callback is not nullptr the return of writeInt32 is true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleRegisterNativeChildExitNotify_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) NativeChildCallbackMock();
    data.WriteRemoteObject(object);
    reply.WriteInt32(0);
    auto res = mockAppMgrService_->HandleRegisterNativeChildExitNotify(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleUnregisterNativeChildExitNotify_0100
 * @tc.desc: Test when callback is not nullptr the return of writeInt32 is true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUnregisterNativeChildExitNotify_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) NativeChildCallbackMock();
    data.WriteRemoteObject(object);
    reply.WriteInt32(0);
    auto res = mockAppMgrService_->HandleUnregisterNativeChildExitNotify(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleRegisterRenderStateObserver_0100
 * @tc.desc: Test register observer success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleRegisterRenderStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) RenderStateObserverMock();
    data.WriteRemoteObject(object);
    EXPECT_CALL(*mockAppMgrService_, RegisterRenderStateObserver(_)).Times(1);
    auto res = mockAppMgrService_->HandleRegisterRenderStateObserver(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleUnregisterRenderStateObserver_0100
 * @tc.desc: Test unregister observer success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUnregisterRenderStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) RenderStateObserverMock();
    data.WriteRemoteObject(object);
    EXPECT_CALL(*mockAppMgrService_, UnregisterRenderStateObserver(_)).Times(1);
    auto res = mockAppMgrService_->HandleUnregisterRenderStateObserver(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleUpdateRenderState_0100
 * @tc.desc: Test update render state success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUpdateRenderState_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    pid_t renderPid = 0;
    data.WriteInt32(renderPid);
    int32_t state = 0;
    data.WriteInt32(state);
    EXPECT_CALL(*mockAppMgrService_, UpdateRenderState(_, _)).Times(1);
    auto res = mockAppMgrService_->HandleUpdateRenderState(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleSignRestartAppFlag_0100
 * @tc.desc: Test sign restart app flag success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleSignRestartAppFlag_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t uid = 0;
    data.WriteInt32(uid);
    auto res = mockAppMgrService_->HandleSignRestartAppFlag(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleNotifyMemorySizeStateChanged_0100
 * @tc.desc: Test notify memory size state changed.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleNotifyMemorySizeStateChanged_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    auto res = mockAppMgrService_->HandleNotifyMemorySizeStateChanged(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleNotifyMemorySizeStateChanged_0200
 * @tc.desc: Test notify memory size state changed.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleNotifyMemorySizeStateChanged_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(false);
    auto res = mockAppMgrService_->HandleNotifyMemorySizeStateChanged(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleGetAllUIExtensionRootHostPid_0100
 * @tc.desc: Get all ui extension root host pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleGetAllUIExtensionRootHostPid_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    pid_t pid = 1;
    data.WriteInt32(pid);
    auto res = mockAppMgrService_->HandleGetAllUIExtensionRootHostPid(data, reply);
    EXPECT_EQ(res, NO_ERROR);
    int32_t size = reply.ReadInt32();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: HandleGetAllUIExtensionProviderPid_0100
 * @tc.desc: Get all ui extension root host pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleGetAllUIExtensionProviderPid_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    pid_t hostPid = 1;
    data.WriteInt32(hostPid);
    auto res = mockAppMgrService_->HandleGetAllUIExtensionProviderPid(data, reply);
    EXPECT_EQ(res, NO_ERROR);
    int32_t size = reply.ReadInt32();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: PreloadApplication_0100
 * @tc.desc: Preload application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, PreloadApplication_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 100;
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    int32_t appIndex = 0;

    data.WriteString16(Str8ToStr16(bundleName));
    data.WriteInt32(userId);
    data.WriteInt32(static_cast<int32_t>(preloadMode));
    data.WriteInt32(appIndex);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_APPLICATION), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: PreloadModuleFinished_0100
 * @tc.desc: Preload application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, PreloadModuleFinished_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int32_t recordId = 0;
    data.WriteInt32(recordId);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_MODULE_FINISHED), data, reply, option);
    EXPECT_NE(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: SetSupportedProcessCacheSelf_001
 * @tc.desc: The application sets itself whether or not to support process cache.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, SetSupportedProcessCacheSelf_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    bool isSupported = false;
    data.WriteBool(isSupported);

    EXPECT_CALL(*mockAppMgrService_, SetSupportedProcessCacheSelf(_)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::SET_SUPPORTED_PROCESS_CACHE_SELF), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRunningMultiAppInfoByBundleName_001
 * @tc.desc: Get multiapp information by bundleName.
 * @tc.type: FUNC
 * @tc.require: issueI9HMAO
 */
HWTEST_F(AppMgrStubTest, GetRunningMultiAppInfoByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, GetRunningMultiAppInfoByBundleName(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_RUNNING_MULTIAPP_INFO_BY_BUNDLENAME), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetAllRunningInstanceKeysBySelf_001
 * @tc.desc: GetAllRunningInstanceKeysBySelf.
 * @tc.type: FUNC
 * @tc.require: issueI9HMAO
 */
HWTEST_F(AppMgrStubTest, GetAllRunningInstanceKeysBySelf_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);

    EXPECT_CALL(*mockAppMgrService_, GetAllRunningInstanceKeysBySelf(_)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_All_RUNNING_INSTANCE_KEYS_BY_SELF), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetAllRunningInstanceKeysByBundleName_001
 * @tc.desc: GetAllRunningInstanceKeysByBundleName.
 * @tc.type: FUNC
 * @tc.require: issueI9HMAO
 */
HWTEST_F(AppMgrStubTest, GetAllRunningInstanceKeysByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);
    int32_t userId = -1;
    data.WriteInt32(userId);

    EXPECT_CALL(*mockAppMgrService_, GetAllRunningInstanceKeysByBundleName(_, _, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_All_RUNNING_INSTANCE_KEYS_BY_BUNDLENAME), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetSupportedProcessCachePids_001
 * @tc.desc: Get pids of processes which belong to specific bundle name and support process cache feature.
 * @tc.type: FUNC
 * @tc.require: issueIAGZ7H
 */
HWTEST_F(AppMgrStubTest, GetSupportedProcessCachePids_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, GetSupportedProcessCachePids(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_SUPPORTED_PROCESS_CACHE_PIDS), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UpdateConfigurationForBackgroundApp_001
 * @tc.desc: UpdateConfigurationForBackgroundApp.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrStubTest, UpdateConfigurationForBackgroundApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);

    std::vector<BackgroundAppInfo> appInfos;
    AppExecFwk::ConfigurationPolicy policy;
    int32_t userId = -1;

    BackgroundAppInfo info;
    appInfos.push_back(info);
    auto size = appInfos.size();
    data.WriteUint32(size);

    for (const auto &info: appInfos) {
        data.WriteParcelable(&info);
    }
    data.WriteParcelable(&policy);
    data.WriteInt32(userId);

    EXPECT_CALL(*mockAppMgrService_, UpdateConfigurationForBackgroundApp(_, _, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION_POLICY), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleIsProcessCacheSupported_0100
 * @tc.desc: Test HandleIsProcessCacheSupported.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleIsProcessCacheSupported_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t pid = 1;
    bool isSupport = false;
    data.WriteInt32(pid);
    data.WriteBool(isSupport);

    EXPECT_CALL(*mockAppMgrService_, IsProcessCacheSupported(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::IS_PROCESS_CACHE_SUPPORTED), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleSetProcessCacheEnable_0100
 * @tc.desc: Test HandleSetProcessCacheEnable.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleSetProcessCacheEnable_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t pid = 1;
    bool enable = false;
    data.WriteInt32(pid);
    data.WriteBool(enable);

    EXPECT_CALL(*mockAppMgrService_, SetProcessCacheEnable(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::SET_PROCESS_CACHE_ENABLE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleLockProcessCache_001
 * @tc.desc: Test HandleLockProcessCache.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleLockProcessCache_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t pid = 1;
    bool isLock = false;
    data.WriteInt32(pid);
    data.WriteBool(isLock);

    EXPECT_CALL(*mockAppMgrService_, LockProcessCache(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::LOCK_PROCESS_CACHE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleQueryRunningSharedBundles_0100
 * @tc.desc: Test HandleQueryRunningSharedBundles.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleQueryRunningSharedBundles_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleQueryRunningSharedBundles_0100 start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    pid_t pid = 1;
    bool enable = false;
    data.WriteInt32(pid);
    data.WriteBool(enable);

    EXPECT_CALL(*mockAppMgrService_, QueryRunningSharedBundles(_, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));

    auto ret = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::QUERY_RUNNING_SHARED_BUNDLES), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    auto replyResult = reply.ReadInt32();
    EXPECT_EQ(replyResult, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "HandleQueryRunningSharedBundles_0100 end.");
}

/**
 * @tc.name: HandleQueryRunningSharedBundles_0200
 * @tc.desc: Test HandleQueryRunningSharedBundles.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleQueryRunningSharedBundles_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleQueryRunningSharedBundles_0200 start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    pid_t pid = 1;
    bool enable = false;
    data.WriteInt32(pid);
    data.WriteBool(enable);

    EXPECT_CALL(*mockAppMgrService_, QueryRunningSharedBundles(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    auto ret = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::QUERY_RUNNING_SHARED_BUNDLES), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    auto replyResult = reply.ReadInt32();
    EXPECT_EQ(replyResult, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "HandleQueryRunningSharedBundles_0200 end.");
}

/**
 * @tc.name: HandleGetConfigurationByUserId_0100
 * @tc.desc: Test HandleGetConfigurationByUserId.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleGetConfigurationByUserId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleGetConfigurationByUserId_0100 start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    Configuration config;
    int32_t userId = USER_ID;
    data.WriteParcelable(&config);
    data.WriteInt32(userId);

    EXPECT_CALL(*mockAppMgrService_, GetConfiguration(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    auto ret = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_CONFIGURATION_BY_USERID), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    std::unique_ptr<Configuration> replyConfig(reply.ReadParcelable<Configuration>());
    auto replyResult = reply.ReadInt32();
    EXPECT_EQ(replyResult, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "HandleGetConfigurationByUserId_0100 end.");
}

/**
 * @tc.name: HandleUpdateConfigurationMultiUser_0100
 * @tc.desc: Test HandleUpdateConfigurationMultiUser.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUpdateConfigurationMultiUser_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleUpdateConfigurationMultiUser_0100 start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    Configuration config;
    std::vector<int32_t> userIds;
    data.WriteParcelable(&config);
    data.WriteInt32Vector(userIds);

    EXPECT_CALL(*mockAppMgrService_, UpdateConfigurationByUserIds(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    auto ret = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION_MULTI_USER), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    auto replyResult = reply.ReadInt32();
    EXPECT_EQ(replyResult, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "HandleUpdateConfigurationMultiUser_0100 end.");
}

/**
 * @tc.name: HandlePreloadExtension_0100
 * @tc.desc: Test HandlePreloadExtension.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandlePreloadExtension_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);

    std::string bundleName = "com.example.hmos.inputmethod";
    std::string abilityName = "InputService";
    int32_t userId = 100;
    int32_t appIndex = 0;

    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    data.WriteParcelable(&want);
    data.WriteInt32(appIndex);
    data.WriteInt32(userId);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_EXTENSION), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandlePreloadExtension_0200
 * @tc.desc: Test HandlePreloadExtension.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandlePreloadExtension_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);

    std::string bundleName = "com.example.hmos.inputmethod";
    std::string abilityName = "InputService";
    int32_t userId = 100;
    int32_t appIndex = 0;

    data.WriteInt32(appIndex);
    data.WriteInt32(userId);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_EXTENSION), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: SetProcessPrepareExit_0100
 * @tc.desc: Test SetProcessPrepareExit.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, SetProcessPrepareExit_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t pid = 1;
    data.WriteInt32(pid);

    EXPECT_CALL(*mockAppMgrService_, SetProcessPrepareExit(_)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::SET_PROCESS_PREPARE_EXIT), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}
} // namespace AppExecFwk
} // namespace OHOS
