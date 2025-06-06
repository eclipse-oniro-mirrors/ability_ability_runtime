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

#include <unistd.h>
#include <gtest/gtest.h>

#include "app_mgr_proxy.h"
#include "app_record_id.h"
#include "app_scheduler_proxy.h"
#include "application_state_observer_stub.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "mock_application.h"
#include "mock_app_mgr_service.h"
#include "mock_kia_interceptor.h"
#include "native_child_notify_stub.h"

using namespace testing::ext;

using OHOS::iface_cast;
using OHOS::sptr;
using testing::_;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::Return;

namespace OHOS {
namespace AppExecFwk {

class NativeChildCallbackMock : public NativeChildNotifyStub {
public:
    NativeChildCallbackMock() = default;
    virtual ~NativeChildCallbackMock() = default;
    
    void OnNativeChildStarted(const sptr<IRemoteObject> &nativeChild) {}
    void OnError(int32_t errCode) {}
    int32_t OnNativeChildExit(int32_t pid, int32_t signal) { return 0; }
};

class AmsIpcAppMgrInterfaceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AmsIpcAppMgrInterfaceTest::SetUpTestCase()
{}

void AmsIpcAppMgrInterfaceTest::TearDownTestCase()
{}

void AmsIpcAppMgrInterfaceTest::SetUp()
{}

void AmsIpcAppMgrInterfaceTest::TearDown()
{}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: interface
 * CaseDescription: test interface of AttachApplication
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, Interface_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppMgrIpcInterfaceTest_AppMgr_001 start");
    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);
    sptr<MockApplication> app(new MockApplication());

    EXPECT_CALL(*mockAppMgr, AttachApplication(_))
        .WillOnce(InvokeWithoutArgs(mockAppMgr.GetRefPtr(), &MockAppMgrService::Post));
    appMgrClient->AttachApplication(app);
    mockAppMgr->Wait();
    TAG_LOGD(AAFwkTag::TEST, "AppMgrIpcInterfaceTest_AppMgr_001 end");
}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: interface
 * CaseDescription: test interface of ApplicationForegrounded
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, Interface_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppMgrIpcInterfaceTest_AppMgr_002 start");
    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, ApplicationForegrounded(_))
        .WillOnce(InvokeWithoutArgs(mockAppMgr.GetRefPtr(), &MockAppMgrService::Post));
    auto recordId = AppRecordId::Create();
    appMgrClient->ApplicationForegrounded(recordId);
    mockAppMgr->Wait();
    TAG_LOGD(AAFwkTag::TEST, "AppMgrIpcInterfaceTest_AppMgr_002 end");
}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: interface
 * CaseDescription: test interface of ApplicationBackgrounded
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, Interface_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppMgrIpcInterfaceTest_AppMgr_003 start");
    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, ApplicationBackgrounded(_))
        .WillOnce(InvokeWithoutArgs(mockAppMgr.GetRefPtr(), &MockAppMgrService::Post));
    auto recordId = AppRecordId::Create();
    appMgrClient->ApplicationBackgrounded(recordId);
    mockAppMgr->Wait();
    TAG_LOGD(AAFwkTag::TEST, "AppMgrIpcInterfaceTest_AppMgr_003 end");
}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: interface
 * CaseDescription: test interface of ApplicationTerminated
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, Interface_004, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppMgrIpcInterfaceTest_AppMgr_004 start");
    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, ApplicationTerminated(_))
        .WillOnce(InvokeWithoutArgs(mockAppMgr.GetRefPtr(), &MockAppMgrService::Post));
    auto recordId = AppRecordId::Create();
    appMgrClient->ApplicationTerminated(recordId);
    mockAppMgr->Wait();
    TAG_LOGD(AAFwkTag::TEST, "AppMgrIpcInterfaceTest_AppMgr_004 end");
}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: KillApplication interface
 * CaseDescription: test IPC can transact data
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, ClearUpApplicationData_008, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ClearUpApplicationData_008 start");

    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, ClearUpApplicationData(_, _, _)).Times(1);

    appMgrClient->ClearUpApplicationData("PROCESS", 0);

    TAG_LOGD(AAFwkTag::TEST, "ClearUpApplicationData_008 end");
}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: KillApplication interface
 * CaseDescription: test IPC can transact data
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, GetAllRunningProcesses_010, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetAllRunningProcesses_009 start");

    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, GetAllRunningProcesses(_)).Times(1).WillOnce(Return(OHOS::ERR_NULL_OBJECT));

    std::vector<RunningProcessInfo> runningProcessInfo;
    int32_t ret = appMgrClient->GetAllRunningProcesses(runningProcessInfo);
    EXPECT_EQ(ret, OHOS::ERR_NULL_OBJECT);

    EXPECT_CALL(*mockAppMgr, GetAllRunningProcesses(_)).Times(1).WillOnce(Return(OHOS::ERR_NONE));
    ret = appMgrClient->GetAllRunningProcesses(runningProcessInfo);
    EXPECT_EQ(ret, OHOS::ERR_NONE);

    TAG_LOGD(AAFwkTag::TEST, "GetAllRunningProcesses_009 end");
}

/*
 * @tc.name: RegisterApplicationStateObserver_001
 * @tc.desc: Register application state observer test.
 * @tc.type: FUNC
 * @tc.require: issueI5822Q
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, RegisterApplicationStateObserver_001, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "RegisterApplicationStateObserver_001 start");

    sptr<IApplicationStateObserver> observer = new ApplicationStateObserverStub();
    std::vector<std::string> bundleNameList;
    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, RegisterApplicationStateObserver(_, _)).Times(1).WillOnce(Return(OHOS::NO_ERROR));

    int32_t err = appMgrClient->RegisterApplicationStateObserver(observer, bundleNameList);

    EXPECT_EQ(OHOS::NO_ERROR, err);

    TAG_LOGD(AAFwkTag::TEST, "RegisterApplicationStateObserver_001 end");
}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: KillApplication interface
 * CaseDescription: test IPC can transact data
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, UnregisterApplicationStateObserver_001, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "UnregisterApplicationStateObserver_001 start");

    sptr<IApplicationStateObserver> observer = new ApplicationStateObserverStub();
    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, UnregisterApplicationStateObserver(_)).Times(1).WillOnce(Return(OHOS::NO_ERROR));

    int32_t err = appMgrClient->UnregisterApplicationStateObserver(observer);

    EXPECT_EQ(OHOS::NO_ERROR, err);

    TAG_LOGD(AAFwkTag::TEST, "UnregisterApplicationStateObserver_001 end");
}

/*
 * @tc.name: RegisterNativeChildExitNotify_001
 * @tc.desc: Register native child exit notify test.
 * @tc.type: FUNC
 * @tc.require: issueI5822Q
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, RegisterNativeChildExitNotify_001, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "RegisterNativeChildExitNotify_001 start");

    sptr<INativeChildNotify> notify = new NativeChildCallbackMock();
    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, RegisterNativeChildExitNotify(_)).Times(1).WillOnce(Return(OHOS::NO_ERROR));

    int32_t err = appMgrClient->RegisterNativeChildExitNotify(notify);

    EXPECT_EQ(OHOS::NO_ERROR, err);

    TAG_LOGD(AAFwkTag::TEST, "RegisterNativeChildExitNotify_001 end");
}

/*
 * @tc.name: UnregisterNativeChildExitNotify_001
 * @tc.desc: Unregister native child exit notify test.
 * @tc.type: FUNC
 * @tc.require: issueI5822Q
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, UnregisterNativeChildExitNotify_001, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "UnregisterNativeChildExitNotify_001 start");

    sptr<INativeChildNotify> notify = new NativeChildCallbackMock();
    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, UnregisterNativeChildExitNotify(_)).Times(1).WillOnce(Return(OHOS::NO_ERROR));

    int32_t err = appMgrClient->UnregisterNativeChildExitNotify(notify);

    EXPECT_EQ(OHOS::NO_ERROR, err);

    TAG_LOGD(AAFwkTag::TEST, "UnregisterNativeChildExitNotify_001 end");
}

/*
 * @tc.name: RegisterKiaInterceptor_001
 * @tc.desc: Register kia interceptor test.
 * @tc.type: FUNC
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, RegisterKiaInterceptor_001, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "RegisterKiaInterceptor_001 start");

    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, RegisterKiaInterceptor(_)).Times(1).WillOnce(Return(ERR_OK));

    sptr<IKiaInterceptor> interceptor = new MockKiaInterceptor();
    int32_t err = appMgrClient->RegisterKiaInterceptor(interceptor);
    EXPECT_EQ(ERR_OK, err);

    TAG_LOGD(AAFwkTag::TEST, "RegisterKiaInterceptor_001 end");
}

/*
 * @tc.name: RegisterKiaInterceptor_002
 * @tc.desc: Register kia interceptor test.
 * @tc.type: FUNC
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, RegisterKiaInterceptor_002, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "RegisterKiaInterceptor_002 start");

    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, RegisterKiaInterceptor(_)).Times(1).WillOnce(Return(ERR_INVALID_VALUE));

    sptr<IKiaInterceptor> interceptor = new MockKiaInterceptor();
    int32_t err = appMgrClient->RegisterKiaInterceptor(interceptor);
    EXPECT_EQ(ERR_INVALID_VALUE, err);

    TAG_LOGD(AAFwkTag::TEST, "RegisterKiaInterceptor_002 end");
}

/*
 * @tc.name: CheckIsKiaProcess_001
 * @tc.desc: Check if a process is kia protected.
 * @tc.type: FUNC
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, CheckIsKiaProcess_001, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "CheckIsKiaProcess_001 start");

    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, CheckIsKiaProcess(_, _)).Times(1).WillOnce(Return(ERR_OK));

    pid_t pid = 1234;
    bool isKia = false;
    int32_t err = appMgrClient->CheckIsKiaProcess(pid, isKia);
    EXPECT_EQ(ERR_OK, err);

    TAG_LOGD(AAFwkTag::TEST, "CheckIsKiaProcess_001 end");
}

/*
 * @tc.name: CheckIsKiaProcess_002
 * @tc.desc: Check if a process is kia protected.
 * @tc.type: FUNC
 */
HWTEST_F(AmsIpcAppMgrInterfaceTest, CheckIsKiaProcess_002, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "CheckIsKiaProcess_002 start");

    sptr<MockAppMgrService> mockAppMgr(new MockAppMgrService());
    sptr<IAppMgr> appMgrClient = iface_cast<IAppMgr>(mockAppMgr);

    EXPECT_CALL(*mockAppMgr, CheckIsKiaProcess(_, _)).Times(1).WillOnce(Return(ERR_INVALID_VALUE));

    pid_t pid = 1234;
    bool isKia = false;
    int32_t err = appMgrClient->CheckIsKiaProcess(pid, isKia);
    EXPECT_EQ(ERR_INVALID_VALUE, err);

    TAG_LOGD(AAFwkTag::TEST, "CheckIsKiaProcess_002 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
